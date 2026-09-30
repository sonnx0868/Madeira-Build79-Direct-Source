#!/usr/bin/env python3
"""Independent lifecycle additions using the already-extracted production C bodies.

The production cancel/APC/completion functions are unchanged; this file only
replaces the fixture's main with additional completion ordering and mutation cases.
"""
from pathlib import Path
import hashlib, json, os, subprocess

HERE = Path(__file__).resolve().parent
AUDIT = HERE.parent
sha = lambda data: hashlib.sha256(data).hexdigest()

EXTRA = r'''
static struct async *review_future;
static int review_added;
static void review_complete_future(void *arg)
{
    assert(arg == review_future);
    async_terminate(review_future, STATUS_CANCELLED);
}
static void review_add_operation(void *arg)
{
    assert(!arg && !review_added);
    review_added = 1;
    make_async(63, 0);
}
static void review_complete_index(unsigned index)
{
    assert(index < pending_count);
    struct thread_apc *apc = pending[index];
    memmove(pending + index, pending + index + 1,
            (--pending_count - index) * sizeof(*pending));
    release_object(apc);
}
int main(void)
{
    unsigned cases = 0;
    /* Completion callbacks can remove a not-yet-selected process-list member. */
    for (int grouped = 0; grouped < 2; grouped++)
    for (unsigned n = 2; n <= 16; n++)
    {
        setup();
        struct async *first = make_async(0, 0);
        for (unsigned i = 1; i < n; i++) review_future = make_async(i, 0);
        first->completion_callback = review_complete_future;
        first->completion_callback_private = review_future;
        obj_handle_t handle = 0;
        assert(cancel_process_async(&process, file->user, grouped ? owner : NULL,
                                    0, &handle) == (int)n - 1);
        assert(cancel_calls == (int)n - 1 && !handle && !count_asyncs());
        assert(!signal_calls); /* All finished before a wait handle existed. */
        finish(); cases++;
    }
    /* A callback can add work: restart must inspect it without losing links. */
    for (int grouped = 0; grouped < 2; grouped++)
    {
        setup(); review_added = 0;
        struct async *first = make_async(0, 0);
        first->completion_callback = review_add_operation;
        obj_handle_t handle = 0;
        assert(cancel_process_async(&process, file->user, grouped ? owner : NULL,
                                    0, &handle) == 2);
        assert(review_added && cancel_calls == 2 && !handle && !count_asyncs());
        finish(); cases++;
    }
    /* A prior process-wide cancel can leave a device operation outstanding.
       Attaching a first group must not send a duplicate FD cancel request. */
    for (unsigned n = 1; n <= 16; n++)
    {
        setup(); struct async *ops[16];
        for (unsigned i = 0; i < n; i++) { ops[i] = make_async(i, 0); ops[i]->canceled = 1; }
        obj_handle_t handle = 0;
        assert(cancel_process_async(&process, file->user, owner, 0, &handle) == (int)n);
        assert(handle == 0x40 && !cancel_calls && count_asyncs() == (int)n && !signal_calls);
        for (unsigned i = 0; i < n; i++)
        {
            async_terminate(ops[i], STATUS_CANCELLED);
            assert(signal_calls == (int)(i + 1 == n));
        }
        assert(!count_asyncs()); finish(); cases++;
    }
    /* Direct-result cancellation does not queue an APC. The group must live
       until the explicit result arrives, in an order different from scanning. */
    for (unsigned n = 1; n <= 16; n++)
    {
        setup(); struct async *ops[16];
        for (unsigned i = 0; i < n; i++) { ops[i] = make_async(i, 0); ops[i]->direct_result = 1; }
        obj_handle_t handle = 0;
        assert(cancel_process_async(&process, file->user, owner, 0, &handle) == (int)n);
        assert(handle == 0x40 && !pending_count && !signal_calls && count_asyncs() == (int)n);
        for (unsigned i = n; i; i--)
        {
            async_set_result(&ops[i - 1]->obj, STATUS_CANCELLED, 0);
            assert(signal_calls == (int)(i == 1));
        }
        assert(!count_asyncs()); finish(); cases++;
    }
    /* Deferred real APC destructors complete center-out, rather than FIFO. */
    for (unsigned n = 2; n <= 24; n++)
    {
        setup();
        for (unsigned i = 0; i < n; i++) make_async(i, 1);
        obj_handle_t handle = 0;
        assert(cancel_process_async(&process, file->user, owner, 0, &handle) == (int)n);
        assert(handle == 0x40 && pending_count == n);
        while (pending_count)
        {
            review_complete_index(pending_count / 2);
            assert(signal_calls == (int)(pending_count == 0));
        }
        assert(!count_asyncs()); finish(); cases++;
    }
    /* Closing a returned wait handle early must leave the aggregate-owned
       reference alive until the final outstanding operation completes. */
    for (unsigned n = 1; n <= 16; n++)
    {
        setup();
        for (unsigned i = 0; i < n; i++) make_async(i, 1);
        obj_handle_t handle = 0;
        assert(cancel_process_async(&process, file->user, owner, 0, &handle) == (int)n);
        assert(handle == 0x40 && held_handle && pending_count == n);
        release_object(held_handle); held_handle = NULL;
        while (pending_count)
        {
            complete_first();
            assert(signal_calls == (int)(pending_count == 0));
        }
        assert(!count_asyncs()); finish(); cases++;
    }
    printf("PASS %u additional lifecycle cases; every fixture-owned object released.\n", cases);
    return 0;
}
'''

def main():
    original = (AUDIT / 'build/fixed.c').read_text()
    prefix, old_main = original.split('int main(int argc,char **argv)', 1)
    assert old_main.startswith(' {')
    output = HERE / 'review_source.c'
    output.write_text(prefix + EXTRA)
    exe = HERE / 'review_source'
    compile_cmd = ['clang', '-std=c11', '-g', '-O1', '-Wall', '-Wextra',
                   '-Wno-unused-parameter', '-fno-omit-frame-pointer',
                   '-fsanitize=address,undefined', '-I' + str(AUDIT / 'build'),
                   str(output), '-o', str(exe)]
    cc = subprocess.run(compile_cmd, text=True, capture_output=True)
    (HERE / 'review_source_compile.log').write_text(cc.stdout + cc.stderr)
    assert cc.returncode == 0, cc.stderr
    env = dict(os.environ, ASAN_OPTIONS='detect_leaks=0:halt_on_error=1',
               UBSAN_OPTIONS='halt_on_error=1')
    run = subprocess.run([str(exe)], text=True, capture_output=True, env=env)
    (HERE / 'review_source.log').write_text(run.stdout + run.stderr)
    report = {'passed': run.returncode == 0, 'returncode': run.returncode,
              'result': run.stdout, 'input_fixture_sha256': sha(original.encode()),
              'test_source_sha256': sha(output.read_bytes()),
              'binary_sha256': sha(exe.read_bytes()),
              'scope': 'Additional host fixtures using unchanged production C function bodies from the root source fixture; callback insertion/removal, pre-canceled work, direct results, and non-FIFO real APC-destructor completion. ASan/UBSan enabled; LSan disabled for host sandbox, explicit allocation accounting retained. No device execution.'}
    (HERE / 'review_source_report.json').write_text(json.dumps(report, indent=2) + '\n')
    print(json.dumps(report, indent=2))
    assert report['passed'], run.stderr

if __name__ == '__main__':
    main()
