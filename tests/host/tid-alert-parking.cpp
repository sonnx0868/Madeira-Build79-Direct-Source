/* Exercise the shipping alert handshake against actual OS compare-and-wait.
 * Covers preposted alerts, timeout/post races, several alert senders, and a
 * deliberately missing-wake control which must fail the liveness watchdog.
 */
#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <thread>
#include <vector>
#include "../../build/ntdll-unix/shims/ios_tid_alert.h"
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <errno.h>
extern "C" int __ulock_wait(uint32_t, void *, uint64_t, uint32_t);
extern "C" int __ulock_wake(uint32_t, void *, uint64_t);
#endif

static std::atomic<unsigned long long> wakes{}, waits{};
static bool broken_wake;
static void require(bool ok, const char *message) {
    if (!ok) { std::fprintf(stderr, "FAIL: %s\n", message); std::exit(3); }
}
static void kernel_wake(volatile int32_t *word) {
    wakes.fetch_add(1, std::memory_order_relaxed);
#ifdef _WIN32
    WakeByAddressSingle((void *)word);
#else
    __ulock_wake(1, (void *)word, 0);
#endif
}
static bool kernel_wait(volatile int32_t *word, unsigned ms) {
    waits.fetch_add(1, std::memory_order_relaxed);
#ifdef _WIN32
    int32_t expected = IOS_TID_ALERT_PARKED;
    if (WaitOnAddress((volatile void *)word, &expected, 4, ms)) return true;
    require(GetLastError() == ERROR_TIMEOUT, "WaitOnAddress error");
    return false;
#else
    int rc = __ulock_wait(1, (void *)word, IOS_TID_ALERT_PARKED, ms * 1000);
    if (!rc || errno != ETIMEDOUT) return true;
    return false;
#endif
}
static void post(volatile int32_t *word) {
    if (ios_tid_alert_post(word)) {
        std::this_thread::yield(); // widen the post-before-syscall race
        if (!broken_wake) kernel_wake(word);
    }
}
static bool take(volatile int32_t *word, unsigned ms) {
    while (!ios_tid_alert_take_or_park(word)) {
        if (!kernel_wait(word, ms)) {
            ios_tid_alert_unpark(word);
            return false;
        }
    }
    return true;
}
static void concurrent(unsigned timeout_ms) {
    alignas(4) int32_t word = 0;
    constexpr unsigned senders = 8, messages = 20000;
    std::atomic<unsigned> published[senders]{}, acknowledged[senders]{};
    std::atomic<bool> stop{};
    std::thread consumer([&] {
        while (!stop.load(std::memory_order_acquire)) {
            if (!take(&word, timeout_ms)) continue;
            for (unsigned i = 0; i < senders; ++i) {
                unsigned value = published[i].load(std::memory_order_acquire);
                unsigned done = acknowledged[i].load(std::memory_order_relaxed);
                require(value == done || value == done + 1, "work was lost or consumed twice");
                acknowledged[i].store(value, std::memory_order_release);
            }
        }
    });
    // Ensure that the broken-wake control exercises a genuine parked waiter.
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    std::vector<std::thread> producers;
    for (unsigned i = 0; i < senders; ++i) producers.emplace_back([&, i] {
        for (unsigned value = 1; value <= messages; ++value) {
            published[i].store(value, std::memory_order_release);
            post(&word);
            auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(2);
            while (acknowledged[i].load(std::memory_order_acquire) != value) {
                require(std::chrono::steady_clock::now() < deadline, "lost wake: published work did not run");
                std::this_thread::yield();
            }
        }
    });
    for (auto &thread : producers) thread.join();
    for (unsigned i = 0; i < senders; ++i)
        require(acknowledged[i].load() == messages, "incomplete sender");
    stop.store(true, std::memory_order_release);
    post(&word);
    consumer.join();
    std::printf("PASS: %u delivered requests, timeout=%ums\n", senders * messages, timeout_ms);
}
int main(int argc, char **argv) {
    broken_wake = argc > 1 && !std::strcmp(argv[1], "--broken-wake");
    alignas(4) int32_t word = 0;
    auto started = std::chrono::steady_clock::now();
    for (unsigned i = 0; i < 1000000; ++i) {
        require(!ios_tid_alert_post(&word), "preposted alert requested a kernel wake");
        require(ios_tid_alert_take_or_park(&word), "preposted alert lost");
        require(!__atomic_load_n(&word, __ATOMIC_SEQ_CST), "stale parked marker");
    }
    auto optimized = std::chrono::steady_clock::now() - started;
    std::puts("PASS: 1000000 preposted alerts, zero kernel wake calls");
    started = std::chrono::steady_clock::now();
    for (unsigned i = 0; i < 1000000; ++i) {
        // Exact old binary-flag prepost/consume path, with the OS wake it made.
        if (!__atomic_exchange_n(&word, 1, __ATOMIC_SEQ_CST)) kernel_wake(&word);
        require(__atomic_exchange_n(&word, 0, __ATOMIC_SEQ_CST) == 1, "baseline alert lost");
    }
    auto baseline = std::chrono::steady_clock::now() - started;
    std::printf("PREPOST BENCH: old=%.3fms parked-handshake=%.3fms (host OS, not game FPS)\n",
        std::chrono::duration<double, std::milli>(baseline).count(),
        std::chrono::duration<double, std::milli>(optimized).count());
    require(!ios_tid_alert_take_or_park(&word), "empty alert was consumed");
    require(ios_tid_alert_post(&word), "registered waiter was not notified");
    require(kernel_wait(&word, 5000), "post-before-park slept despite a pending alert");
    require(ios_tid_alert_take_or_park(&word), "post-before-park lost");
    require(!ios_tid_alert_take_or_park(&word), "empty alert was consumed");
    require(!kernel_wait(&word, 1), "timeout did not expire");
    ios_tid_alert_unpark(&word);
    require(!ios_tid_alert_post(&word), "timed-out waiter remained registered");
    require(ios_tid_alert_take_or_park(&word), "post-timeout alert lost");
    require(!ios_tid_alert_take_or_park(&word), "empty alert was consumed");
    require(ios_tid_alert_post(&word), "parked alert did not request wake");
    ios_tid_alert_unpark(&word); // timeout cleanup racing with a new alert
    require(ios_tid_alert_take_or_park(&word), "timeout cleanup erased a pending alert");
    concurrent(5000);
    concurrent(1);
    std::printf("PASS: kernel waits=%llu wake requests=%llu\n", waits.load(), wakes.load());
}
