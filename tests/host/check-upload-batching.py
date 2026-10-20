#!/usr/bin/env python3
"""Compile the patched DXMT batching path and replay its captured dependencies."""
from pathlib import Path
import argparse
import importlib.util
import json
import os
import platform
import shutil
import subprocess
import tempfile

root = Path(__file__).resolve().parents[2]
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--trace-report', type=Path, help='also verify the saved fixture against the decoded original capture')
args = parser.parse_args()

spec = importlib.util.spec_from_file_location('patches', root / 'tests/host/check-dxmt-patches.py')
module = importlib.util.module_from_spec(spec)
spec.loader.exec_module(module)
sources = module.patched_sources(root)
context = sources['src/dxmt/dxmt_context.cpp']


def declaration(text, prefix):
    start = text.index(prefix)
    opening = text.index('{', start)
    depth = 1
    end = opening + 1
    while depth:
        depth += (text[end] == '{') - (text[end] == '}')
        end += 1
    return text[start:end]


with tempfile.TemporaryDirectory(prefix='madeira-upload-batching-') as temporary:
    folder = Path(temporary)
    if 'MADEIRA_UPLOAD_BATCHING_HELPERS_BEGIN' not in context:
        # Until this candidate joins the shared patch pipeline, apply it on top
        # of the same fully patched source the production build consumes.
        target = folder / 'src/dxmt/dxmt_context.cpp'
        target.parent.mkdir(parents=True)
        target.write_text(context, encoding='utf-8', newline='\n')
        patch = folder / 'batching.patch'
        patch.write_bytes((root / 'patches/dxmt-upload-batching.patch').read_bytes().replace(b'\r\n', b'\n'))
        subprocess.run(['git', '-C', str(folder), 'apply', str(patch)], check=True)
        subprocess.run(['git', '-C', str(folder), 'apply', '--reverse', '--check', str(patch)], check=True)
        context = target.read_text(encoding='utf-8')
    header = (root / 'dxmt/src/dxmt/dxmt_context.hpp').read_text(encoding='utf-8')
    fixture_header = '''#define DXMT_NATIVE 1
#include "winemetal.h"
#include "dxmt_deptrack.hpp"
#include "upload_batching.hpp"
namespace WMT { struct Buffer { uint64_t handle; }; }
namespace dxmt {
'''
    for prefix in ('enum class EncoderType', 'struct EncoderData {',
                   'struct ComputeEncoderData :', 'struct BlitEncoderData :'):
        fixture_header += declaration(header, prefix) + ';\n'
    fixture_header += context.split('// MADEIRA_UPLOAD_BATCHING_HELPERS_BEGIN', 1)[1].split(
        '// MADEIRA_UPLOAD_BATCHING_HELPERS_END', 1)[0]
    fixture_header += '''
class ArgumentEncodingContext {
public:
  bool hasDataDependency(EncoderData *latter, EncoderData *former);
  unsigned batch(EncoderData **encoders, unsigned encoder_count) {
    unsigned encoder_index = encoder_count;
'''
    call = context.split('// MADEIRA_UPLOAD_BATCHING_CALL_BEGIN', 1)[1].split(
        '// MADEIRA_UPLOAD_BATCHING_CALL_END', 1)[0]
    fixture_header += call + '\nassert(encoder_index == encoder_count); return encoder_count;\n}\n};\n'
    fixture_header += declaration(context, 'bool\nArgumentEncodingContext::hasDataDependency(') + '\n}\n'
    (folder / 'production_upload_batching.inc').write_text(fixture_header, encoding='utf-8')

    capture = json.loads((root / 'tests/host/fixtures/liars-upload-dependencies.json').read_text(encoding='utf-8'))
    if args.trace_report:
        original = json.loads(args.trace_report.read_text(encoding='utf-8'))
        decoded = [p for p in original['passes'] if p['frame'] == 0][:100]
        normalized = [dict(index=p['index'], kind=p['kind'], commandOffset=p['firstCommandOffset'],
                           commandBuffer=p['commandBuffer'], copies=p['blitCopies'],
                           uses=[dict(root=u['root'], usage=u['usage']) for u in p['uses']]) for p in decoded]
        assert normalized == capture['passes'], 'saved dependency fixture differs from the original capture'
    assert len(capture['passes']) == 100
    assert len({p['commandBuffer'] for p in capture['passes']}) == 1
    trace_cpp = ['static void replayCapturedDependencies() { Arena arena; std::vector<EncoderData *> passes;']
    for p in capture['passes']:
        if p['kind'] == 'blit':
            assert len(p['copies']) == 1
            c = p['copies'][0]
            trace_cpp.append('passes.push_back(arena.copy(%sull, %sull, %d, %d, %d));' % (
                c['source'], c['destination'], c['bytes'], c['sourceOffset'], c['destinationOffset']))
        else:
            assert p['kind'] == 'compute'
            trace_cpp.append('{ auto *p = arena.compute(); passes.push_back(p);')
            for use in p['uses']:
                if use['usage'] & 1:
                    trace_cpp.append('p->buf_read.add(key(%sull));' % use['root'])
                if use['usage'] & 2:
                    trace_cpp.append('p->buf_write.add(key(%sull));' % use['root'])
            trace_cpp.append('}')
    trace_cpp += ['verifyCapture(passes);', '}']
    (folder / 'captured_upload_dependencies.inc').write_text('\n'.join(trace_cpp), encoding='utf-8')

    compiler = os.environ.get('CXX') or shutil.which('clang++') or shutil.which('c++')
    if not compiler:
        raise SystemExit('C++ compiler required (set CXX)')
    binary = folder / ('check.exe' if os.name == 'nt' else 'check')
    command = [compiler, '-std=c++20', '-O2', '-Wall', '-Wextra', '-Werror',
               '-I' + str(folder), '-I' + str(root / 'build/dxmt-ios'),
               '-I' + str(root / 'dxmt/src/dxmt'), '-I' + str(root / 'dxmt/src/util'),
               '-I' + str(root / 'dxmt/src/winemetal'),
               str(root / 'tests/host/upload-batching-fixture.cpp'),
               str(root / 'dxmt/src/util/util_bloom.cpp'), '-o', str(binary)]
    if platform.system() == 'Darwin':
        command += ['-fsanitize=address,undefined', '-fno-omit-frame-pointer']
    subprocess.run(command, check=True)
    run_env = dict(os.environ)
    if os.name == 'nt':
        run_env['PATH'] = str(Path(compiler).parent) + os.pathsep + run_env.get('PATH', '')
    if platform.system() == 'Darwin':
        run_env.setdefault('ASAN_OPTIONS', 'halt_on_error=1')
        run_env.setdefault('UBSAN_OPTIONS', 'halt_on_error=1:print_stacktrace=1')
    subprocess.run([str(binary)], check=True, env=run_env)

assert context.index('MADEIRA_UPLOAD_BATCHING_CALL_BEGIN') > context.index('checkEncoderRelation(encoders[j], encoders[i])')
assert context.index('MADEIRA_UPLOAD_BATCHING_CALL_END') < context.index('std::unique_ptr<VisibilityResultReadback> visibility_readback')
print('PASS: production source integration, bounded scan and recorded allocation dependencies; no GPU timing claim')
