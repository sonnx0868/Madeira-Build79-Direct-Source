#!/usr/bin/env python3
"""Extract pinned function bodies, run original negative controls and fix fixtures."""
import difflib, hashlib, json, os, pathlib, re, subprocess
HERE=pathlib.Path(__file__).resolve().parent
ROOT=HERE.parents[2]
SRC=ROOT/'work/vulkan/build-audit/wine-source'
BUILD=HERE/'build'
LOCK={
'server/async.c':'a8673f9332acab7289e1914244b25f929ee837f9571eaa3b20174430b37267ad',
'server/thread.c':'d275c250f96674ccd7ae4575ab107ab9ee86fc5c2cabd7290f5cc79f77f61b13',
'server/fd.c':'8308074cf93d7b816e05e2ae89bed14c3761550711b528bac86c3fb4830d419a',
'include/wine/list.h':'3ba11208280b4fbf42630d03b50b753b94e0aef0414bf8a893392bf9c4bd28a6',
}
def sha(b): return hashlib.sha256(b).hexdigest()
def function(src,name):
    m=re.search(r'^([^\n;]*\b'+re.escape(name)+r'\([^;]*?\)\n\{)',src,re.M)
    if not m: raise RuntimeError(name)
    start=m.start(); brace=src.index('{',m.start()); depth=1; i=brace+1
    # Function bodies here contain no brace characters in strings/comments.
    while depth:
        depth += (src[i]=='{')-(src[i]=='}'); i+=1
    return src[start:i]+'\n'
def structure(src,name):
    return re.search(r'^struct '+name+r'\n\{.*?^\};',src,re.M|re.S).group()+'\n'
def main():
    BUILD.mkdir(exist_ok=True)
    src={}
    for path,h in LOCK.items():
        data=(SRC/path).read_bytes()
        if sha(data)!=h: raise RuntimeError('source changed: '+path)
        src[path]=data.decode()
    (BUILD/'list.h').write_text(src['include/wine/list.h'])
    a,t=src['server/async.c'],src['server/thread.c']
    names=['async_cancel_destroy','create_async_cancel','async_reselect','async_destroy',
           'async_call_completion_callback','async_complete_cancel','async_set_result','async_terminate','cancel_async']
    body=''.join(structure(a,n) for n in ['async_cancel','async'])+structure(t,'thread_apc')
    # These function bodies are byte-for-byte source extractions. Only static
    # linkage is added to public functions to match the test declarations.
    for name in names:
        f=function(a,name)
        if not f.startswith('static '): f='static '+f
        body+='\n'+f
    for name in ['thread_apc_destroy','create_apc','thread_queue_apc']:
        f=function(t,name)
        if not f.startswith('static '): f='static '+f
        body+='\n'+f
    original=function(a,'cancel_process_async')
    fixed=(HERE/'cancel_process_async_fixed.c').read_text()
    # Deliberately incomplete alternative proves why just holding async alive
    # across cancellation cannot fix completion-group ordering.
    naive=original.replace('            if (!async->canceled) cancel_async( async );',
                           '            grab_object( async );\n            if (!async->canceled) cancel_async( async );')
    naive=naive.replace('        list_add_tail( &process->asyncs, &async->process_entry );',
                        '        list_add_tail( &process->asyncs, &async->process_entry );\n        release_object( async );')
    patch=''.join(difflib.unified_diff(a.splitlines(True),a.replace(original,fixed).splitlines(True),
                                     fromfile='a/server/async.c',tofile='b/server/async.c'))
    (HERE/'cancel-process-async.patch').write_text(patch)
    results={}
    for variant,code in [('original',original),('keepalive_only',naive),('fixed',fixed)]:
        c=BUILD/(variant+'.c')
        c.write_text((HERE/'fixture_prefix.h').read_text()+body+'\n'+code+'\n'+(HERE/'fixture_tests.c').read_text())
        exe=BUILD/variant
        cmd=['clang','-std=c11','-g','-O1','-Wall','-Wextra','-Wno-unused-parameter',
             '-fno-omit-frame-pointer','-fsanitize=address,undefined','-I'+str(BUILD),str(c),'-o',str(exe)]
        cc=subprocess.run(cmd,text=True,capture_output=True)
        (BUILD/(variant+'-compile.log')).write_text(cc.stdout+cc.stderr)
        if cc.returncode: raise RuntimeError(cc.stderr)
        args=[str(exe),'all' if variant=='fixed' else ('dead-no-group' if variant=='original' else 'dead-group')]
        # The host's ptrace sandbox prevents LSan's thread scan. ASan/UBSan
        # remain enabled; the fixture explicitly counts every allocated object.
        env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0:halt_on_error=1',UBSAN_OPTIONS='halt_on_error=1')
        run=subprocess.run(args,text=True,capture_output=True,env=env)
        (BUILD/(variant+'.log')).write_text(run.stdout+run.stderr)
        expected=(run.returncode==0 if variant=='fixed' else
                  ('heap-use-after-free' in run.stderr if variant=='original' else '!async->async_cancel' in run.stderr))
        results[variant]={'returncode':run.returncode,'expected_result':expected,'stdout':run.stdout,
                          'source_sha256':sha(c.read_bytes()),'binary_sha256':sha(exe.read_bytes())}
        if not expected: raise RuntimeError(variant+' unexpected result: '+run.stdout+run.stderr)
    report={'source_lock':LOCK,'scope':'Extracted production bodies; reduced host-only surrounding types and API fixtures. No native/device execution. ASan and UBSan enabled, LSan disabled due ptrace sandbox; fixture allocation balance asserted.',
            'results':results,'patch_sha256':sha(patch.encode()),'passed':True}
    (BUILD/'report.json').write_text(json.dumps(report,indent=2)+'\n')
    print(json.dumps(report,indent=2))
if __name__=='__main__': main()
