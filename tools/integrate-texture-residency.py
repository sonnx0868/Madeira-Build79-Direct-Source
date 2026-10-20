"""Create the reversible native boundary overlay from the existing patch stack."""
from pathlib import Path
import difflib

root = Path(__file__).resolve().parents[1]
path = root / 'dxmt/src/winemetal/unix/winemetal_unix.c'
old = path.read_text(encoding='utf-8')
new = old
def replace(a, b, count=1):
    global new
    assert new.count(a) == count, (a[:100], new.count(a), count)
    new = new.replace(a, b)

replace('#include "../airconv_thunks.h"', '#include "../airconv_thunks.h"\n#include "../../../../build/dxmt-ios/texture_residency.m"')
replace('  id<MTLTexture> ret = [device newTextureWithDescriptor:desc];\n',
        '  id<MTLTexture> ret = [device newTextureWithDescriptor:desc];\n  mdr_register(ret, NO);\n')
replace('  params->gpu_resource_id = [ret gpuResourceID]._impl;\n',
        '  mdr_register_view(texture, ret);\n  params->gpu_resource_id = [ret gpuResourceID]._impl;\n')
replace('  [(id<MTLCommandBuffer>)params->handle waitUntilCompleted];',
        '  if (!mdr_failed((id)params->handle)) [(id<MTLCommandBuffer>)params->handle waitUntilCompleted];')
replace('  params->ret = [(id<MTLCommandBuffer>)params->handle status];',
        '  params->ret = mdr_failed((id)params->handle) ? MTLCommandBufferStatusError : [(id<MTLCommandBuffer>)params->handle status];')
replace('  params->ret = (obj_handle_t)[(id<MTLCommandBuffer>)params->handle error];',
        '  params->ret = (obj_handle_t)(mdr_failed((id)params->handle)\n'
        '      ? [NSError errorWithDomain:@"MadeiraTextureResidency" code:1 userInfo:@{NSLocalizedDescriptionKey: @"Unable to restore texture backing"}]\n'
        '      : [(id<MTLCommandBuffer>)params->handle error]);')
replace('#if TARGET_OS_IOS\n  extern void madeira_gpu_submit_begin(void);',
        '  if (!mdr_commit((id<MTLCommandBuffer>)params->handle)) return STATUS_SUCCESS;\n'
        '#if TARGET_OS_IOS\n  extern void madeira_gpu_submit_begin(void);')
for code in (1, 2):
    replace(f'      if (encoder) {{ params->ret = (obj_handle_t)encoder; if (madeira_frame_hooks_on()) ios_frame_pass({code}, 0, 0, 0); return STATUS_SUCCESS; }}',
            f'      if (encoder) {{ mdr_encoder((id)params->handle, encoder); params->ret = (obj_handle_t)encoder; if (madeira_frame_hooks_on()) ios_frame_pass({code}, 0, 0, 0); return STATUS_SUCCESS; }}')
    replace(f'  if (params->ret && madeira_frame_hooks_on()) ios_frame_pass({code}, 0, 0, 0);',
            f'  mdr_encoder((id)params->handle, (id)params->ret);\n  if (params->ret && madeira_frame_hooks_on()) ios_frame_pass({code}, 0, 0, 0);')
replace('  /* Count realized native passes, including clears and the final present.',
        '  mdr_encoder((id)params->handle, (id)params->ret);\n\n  /* Count realized native passes, including clears and the final present.')
replace('      [encoder copyFromBuffer:(id<MTLBuffer>)body->src\n                 sourceOffset:body->src_offset\n            sourceBytesPerRow:',
        '      if (!mdr_use(encoder, dst, YES)) break;\n'
        '      [encoder copyFromBuffer:(id<MTLBuffer>)body->src\n                 sourceOffset:body->src_offset\n            sourceBytesPerRow:')
replace('      [encoder copyFromTexture:src\n',
        '      if (!mdr_use(encoder, src, NO)) break;\n      [encoder copyFromTexture:src\n')
replace('      [encoder copyFromTexture:(id<MTLTexture>)body->src\n',
        '      if (!mdr_use(encoder, (id)body->src, NO) || !mdr_use(encoder, (id)body->dst, YES)) break;\n'
        '      [encoder copyFromTexture:(id<MTLTexture>)body->src\n')
replace('      [encoder generateMipmapsForTexture:(id<MTLTexture>)body->texture];',
        '      if (!mdr_use(encoder, (id)body->texture, YES)) break;\n      [encoder generateMipmapsForTexture:(id<MTLTexture>)body->texture];')
replace('      [encoder optimizeContentsForGPUAccess:(id<MTLTexture>)body->texture',
        '      if (!mdr_use(encoder, (id)body->texture, YES)) break;\n      [encoder optimizeContentsForGPUAccess:(id<MTLTexture>)body->texture')
replace('      [encoder useResource:(id<MTLResource>)body->resource usage:(MTLResourceUsage)body->usage];',
        '      if (!mdr_use(encoder, (id)body->resource, body->usage & MTLResourceUsageWrite)) break;\n'
        '      [encoder useResource:(id<MTLResource>)body->resource usage:(MTLResourceUsage)body->usage];')
replace('      [encoder useResource:(id<MTLResource>)body->resource',
        '      if (!mdr_use(encoder, (id)body->resource, body->usage & MTLResourceUsageWrite)) break;\n'
        '      [encoder useResource:(id<MTLResource>)body->resource', count=2)
# Remove the duplicate guard on compute added by the broad render/compute edit.
double = ('      if (!mdr_use(encoder, (id)body->resource, body->usage & MTLResourceUsageWrite)) break;\n' * 2)
replace(double, double[:len(double)//2])
for method in ('setTexture', 'setFragmentTexture', 'setVertexTexture'):
    replace(f'      [encoder {method}:(id<MTLTexture>)body->texture',
            f'      if (!mdr_use(encoder, (id)body->texture, NO)) break;\n      [encoder {method}:(id<MTLTexture>)body->texture')
# Private BC resources cannot be CPU-mapped/replaced. The eligibility gate
# excludes every CPU-visible and buffer-backed texture. Imported/heap resources
# use other constructors and are never registered here.
patch = ''.join(difflib.unified_diff(old.splitlines(True), new.splitlines(True),
                                   'a/src/winemetal/unix/winemetal_unix.c',
                                   'b/src/winemetal/unix/winemetal_unix.c'))
(root/'patches/dxmt-texture-residency.patch').write_text(patch, encoding='utf-8', newline='\n')
path.write_text(new, encoding='utf-8', newline='\n')
print('Created native texture residency overlay:', len(patch), 'bytes')
