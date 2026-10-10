// SPDX-License-Identifier: MIT
// Madeira's bounded, optional backend cache. A hash is only an index: key and
// normalized machine-code payload are compared byte for byte before promotion.
#pragma once
#include <algorithm>
#include <memory>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
#ifdef _WIN32
#include <share.h>
#endif

namespace Madeira::CPUCache {
using Bytes = std::vector<uint8_t>;
inline constexpr uint32_t MaxRecord = 512 * 1024;
inline constexpr uint64_t MaxFile = 48 * 1024 * 1024;
inline constexpr uint32_t MaxEntries = 16384;

inline uint64_t hash(const Bytes& bytes) {
    uint64_t h = 14695981039346656037ULL;
    for (uint8_t b : bytes) h = (h ^ b) * 1099511628211ULL;
    return h;
}
inline void append(Bytes& out, const void* data, size_t size) {
    if (!size) return;
    const auto* begin = static_cast<const uint8_t*>(data);
    out.insert(out.end(), begin, begin + size);
}
template<class T> inline void scalar(Bytes& out, T value) { append(out, &value, sizeof(value)); }
struct Value { Bytes payload; bool validated = false; };

struct Point { uint64_t guest, offset; };
struct Opcode { uint64_t guestOffset; int64_t hostOffset; };
struct Subblock { uint32_t offset, size; };
struct Block {
    uint64_t codeOnlySize = 0;
    Bytes code, relocations;
    std::vector<Point> points;
    std::vector<Opcode> opcodes;
    std::vector<Subblock> subblocks;
    std::vector<uint64_t> symbols;
};
inline Bytes encode(const Block& block) {
    Bytes out;
    scalar(out, block.codeOnlySize);
    for (size_t size : {block.code.size(), block.relocations.size(), block.points.size(), block.opcodes.size(),
                        block.subblocks.size(), block.symbols.size()}) scalar(out, uint32_t(size));
    append(out, block.code.data(), block.code.size());
    append(out, block.relocations.data(), block.relocations.size());
    append(out, block.points.data(), block.points.size() * sizeof(Point));
    append(out, block.opcodes.data(), block.opcodes.size() * sizeof(Opcode));
    append(out, block.subblocks.data(), block.subblocks.size() * sizeof(Subblock));
    append(out, block.symbols.data(), block.symbols.size() * sizeof(uint64_t));
    return out;
}
inline std::optional<Block> decode(const Bytes& bytes) {
    if (bytes.size() < 32 || bytes.size() > MaxRecord) return {};
    Block result;
    memcpy(&result.codeOnlySize, bytes.data(), 8);
    uint32_t sizes[6]; memcpy(sizes, bytes.data() + 8, sizeof(sizes));
    const uint64_t expected = 32ULL + sizes[0] + sizes[1] + uint64_t(sizes[2]) * sizeof(Point) +
        uint64_t(sizes[3]) * sizeof(Opcode) + uint64_t(sizes[4]) * sizeof(Subblock) + uint64_t(sizes[5]) * 8;
    if (expected != bytes.size() || sizes[0] < 64 || sizes[0] % 4 || sizes[1] % 48 ||
        !sizes[2] || sizes[2] > 4096 || !sizes[3] || sizes[3] > 32768 || sizes[4] > 4096 ||
        !sizes[5] || sizes[5] > 4096 || result.codeOnlySize > sizes[0] - 4) return {};
    size_t offset = 32;
    auto read = [&](auto& vector, uint32_t count) {
        vector.resize(count);
        size_t length = count * sizeof(vector[0]);
        if (length) memcpy(vector.data(), bytes.data() + offset, length);
        offset += length;
    };
    read(result.code, sizes[0]); read(result.relocations, sizes[1]); read(result.points, sizes[2]);
    read(result.opcodes, sizes[3]); read(result.subblocks, sizes[4]); read(result.symbols, sizes[5]);
    uint32_t tail; memcpy(&tail, result.code.data(), 4);
    if (tail < 8 || tail > result.code.size() - 40 || result.codeOnlySize > tail - 4) return {};
    for (const auto& point : result.points) if (point.offset < 4 || point.offset >= tail || point.offset % 4) return {};
    for (const auto& op : result.opcodes) if (op.hostOffset < 4 || uint64_t(op.hostOffset) >= tail) return {};
    for (const auto& sub : result.subblocks) if (uint64_t(sub.offset) + sub.size > result.code.size()) return {};
    for (uint64_t sym : result.symbols) if (sym < 4 || sym > result.code.size() - 8 || sym % 4) return {};
    return result;
}

class Store {
    struct Item { Bytes key; Value value; };
    // Fixed-width framing, independent of compiler struct layout.
    struct Header {
        uint32_t magic, version, keySize, payloadSize;
        uint64_t keyHash, payloadHash;
        uint32_t validated, reserved;
    };
    static_assert(sizeof(Header) == 40);
    static constexpr uint32_t Magic = 0x3143434d; // MCC1
    static uint64_t payloadHash(const Bytes& payload, bool validated) {
        return hash(payload) ^ (validated ? 0xe70cceb1d65a37f9ULL : 0);
    }
    std::unordered_map<uint64_t, std::vector<Item>> items;
    std::mutex lock;
    std::string path;
    FILE* writer = nullptr;
    uint64_t fileBytes = 0, memoryBytes = 0;
    uint32_t entries = 0;
    bool enabled = true, cleanTail = true;

    Item* locate(const Bytes& key) {
        auto it = items.find(hash(key));
        if (it == items.end()) return nullptr;
        for (auto& item : it->second) if (item.key == key) return &item;
        return nullptr;
    }
    bool retain(const Bytes& key, const Value& value) {
        if (auto* item = locate(key)) {
            if (memoryBytes - item->value.payload.size() + value.payload.size() > MaxFile) return false;
            memoryBytes -= item->value.payload.size();
            item->value = value;
            memoryBytes += value.payload.size();
            return true;
        }
        if (entries >= MaxEntries || memoryBytes + key.size() + value.payload.size() > MaxFile) return false;
        items[hash(key)].push_back({key, value});
        memoryBytes += key.size() + value.payload.size(); ++entries;
        return true;
    }
    void write(const Bytes& key, const Value& value) {
        const uint64_t size = sizeof(Header) + key.size() + value.payload.size();
        if (!writer || fileBytes + size > MaxFile) return;
        Header head {Magic, 2, static_cast<uint32_t>(key.size()), static_cast<uint32_t>(value.payload.size()),
                     hash(key), payloadHash(value.payload, value.validated), value.validated ? 1U : 0U, 0};
        Bytes record; record.reserve(size);
        append(record, &head, sizeof(head)); append(record, key.data(), key.size());
        append(record, value.payload.data(), value.payload.size());
        if (fwrite(record.data(), 1, record.size(), writer) != record.size()) {
            fclose(writer); writer = nullptr; // Existing cache remains usable.
        } else { fileBytes += size; }
    }
public:
    explicit Store(std::string file) : path(std::move(file)) {
        if (FILE* disabled = fopen((path + ".disabled").c_str(), "rb")) {
            fclose(disabled); enabled = false; return;
        }
        if (FILE* reader = fopen(path.c_str(), "rb")) {
            if (fseek(reader, 0, SEEK_END) || ftell(reader) < 0 || uint64_t(ftell(reader)) > MaxFile) {
                fclose(reader); enabled = false; return;
            }
            fileBytes = static_cast<uint64_t>(ftell(reader)); rewind(reader);
            uint64_t consumed = 0;
            while (consumed < fileBytes) {
                Header head {};
                if (fread(&head, 1, sizeof(head), reader) != sizeof(head) || head.magic != Magic || head.version != 2 ||
                    head.reserved || head.validated > 1 || !head.keySize || !head.payloadSize ||
                    uint64_t(head.keySize) + head.payloadSize > MaxRecord ||
                    sizeof(head) + uint64_t(head.keySize) + head.payloadSize > fileBytes - consumed) {
                    cleanTail = false; break;
                }
                Bytes key(head.keySize), payload(head.payloadSize);
                if (fread(key.data(), 1, key.size(), reader) != key.size() ||
                    fread(payload.data(), 1, payload.size(), reader) != payload.size()) { cleanTail = false; break; }
                consumed += sizeof(head) + key.size() + payload.size();
                if (hash(key) == head.keyHash && payloadHash(payload, head.validated == 1) == head.payloadHash) {
                    retain(key, {std::move(payload), head.validated == 1});
                } else { cleanTail = false; } // Never append behind corruption.
            }
            fclose(reader);
        }
        if (cleanTail) {
#ifdef _WIN32
            writer = _fsopen(path.c_str(), "ab", _SH_DENYWR);
#else
            writer = fopen(path.c_str(), "ab");
#endif
            // One contiguous write per record. A game/iOS exit may skip CRT
            // destructors; users must not depend on buffered data being flushed.
            if (writer) setvbuf(writer, nullptr, _IONBF, 0);
        }
    }
    ~Store() { if (writer) fclose(writer); }
    Store(const Store&) = delete;
    Store& operator=(const Store&) = delete;

    std::optional<Value> find(const Bytes& key) {
        std::lock_guard guard(lock);
        if (!enabled) return {};
        if (auto* item = locate(key)) return item->value;
        return {};
    }
    // Called only with newly emitted, normalized code. First observation learns;
    // an independent identical emission promotes it. A mismatch shuts down this
    // store and removes its file so no old promoted record survives next launch.
    bool observe(const Bytes& key, const Bytes& payload) {
        std::lock_guard guard(lock);
        if (!enabled || key.empty() || payload.empty() || key.size() + payload.size() > MaxRecord) return false;
        if (auto* item = locate(key)) {
            if (item->value.payload != payload) {
                enabled = false; items.clear(); memoryBytes = 0; entries = 0;
                if (writer) { fclose(writer); writer = nullptr; }
                // A second PE instance may own the writer. Its share lock can
                // prevent removal, so persist a separate disable marker too.
                if (FILE* disabled = fopen((path + ".disabled").c_str(), "wb")) {
                    fputs("CPU code validation mismatch\n", disabled); fclose(disabled);
                }
                std::remove(path.c_str());
                return false;
            }
            if (!item->value.validated) { item->value.validated = true; write(key, item->value); }
            return true;
        }
        Value value {payload, false};
        if (retain(key, value)) write(key, value);
        return true;
    }
    void flush() { std::lock_guard guard(lock); if (writer) fflush(writer); }
};

// Shared within a PE translator instance. No new game threads; sequential
// writes are bounded. Losing the Windows writer lock is a read-only
// cache, so Steam children cannot interleave writers into the same file.
inline std::shared_ptr<Store> open(const std::string& path) {
    static std::mutex guard;
    static std::unordered_map<std::string, std::shared_ptr<Store>> stores;
    std::lock_guard hold(guard);
    if (auto it = stores.find(path); it != stores.end()) return it->second;
    if (stores.size() >= 16 || path.empty()) return {};
    return stores.emplace(path, std::make_shared<Store>(path)).first->second;
}
} // namespace Madeira::CPUCache
