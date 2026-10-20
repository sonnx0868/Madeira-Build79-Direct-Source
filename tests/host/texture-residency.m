/* Exercise the production pager, with real Metal storage, views and queues. */
#define MADEIRA_RESIDENCY_TEST 1
#include "../../build/dxmt-ios/texture_residency.m"
#include <mach/mach.h>
#include <assert.h>
static id<MTLCommandQueue> game_queue;

static uint64_t footprint(void) {
  task_vm_info_data_t vm = {0}; mach_msg_type_number_t count = TASK_VM_INFO_COUNT;
  assert(task_info(mach_task_self(), TASK_VM_INFO, (task_info_t)&vm, &count) == KERN_SUCCESS);
  return vm.phys_footprint;
}

static id<MTLTexture> make_texture(id<MTLDevice> device, MTLPixelFormat format, NSUInteger size,
                                  NSUInteger slices, NSUInteger mips, uint8_t seed) {
  MTLTextureDescriptor *d = [MTLTextureDescriptor new];
  d.textureType = slices > 1 ? MTLTextureType2DArray : MTLTextureType2D;
  d.pixelFormat = format; d.width = size; d.height = size; d.mipmapLevelCount = mips;
  d.arrayLength = slices; d.storageMode = MTLStorageModePrivate;
  d.usage = MTLTextureUsageShaderRead | MTLTextureUsagePixelFormatView;
  id<MTLTexture> texture = [device newTextureWithDescriptor:d]; [d release]; assert(texture);
  mdr_register(texture, NO);
  MDRTexture *r = objc_getAssociatedObject(texture, &mdr_texture_key); assert(r);
  uint8_t *bytes = mdr_restore_buffer.contents;
  for (NSUInteger i = 0; i < r->bytes; ++i) bytes[i] = (uint8_t)(i * 17 + seed);
  id<MTLCommandBuffer> cb = [game_queue commandBuffer];
  id<MTLBlitCommandEncoder> blit = [cb blitCommandEncoder]; mdr_encoder(cb, blit);
  assert(mdr_use(blit, texture, YES));
  mdr_copy(blit, texture, r, mdr_restore_buffer, YES);
  [blit endEncoding]; assert(mdr_commit(cb)); [cb commit]; [cb waitUntilCompleted];
  assert(cb.status == MTLCommandBufferStatusCompleted);
  [mdr_lease(cb, NO) finish];
  return texture;
}

static NSData *read_texture(id<MTLTexture> texture, BOOL view) {
  (void)view;
  MDRTexture *r = objc_getAssociatedObject(texture, &mdr_texture_key); assert(r);
  id<MTLCommandBuffer> cb = [game_queue commandBuffer];
  id<MTLBlitCommandEncoder> blit = [cb blitCommandEncoder]; mdr_encoder(cb, blit);
  assert(mdr_use(blit, texture, NO));
  memset(mdr_restore_buffer.contents, 0, r->bytes);
  // The actual view handle is used for readback, not silently replaced by root.
  mdr_copy(blit, texture, r, mdr_restore_buffer, NO);
  [blit endEncoding]; assert(mdr_commit(cb)); [cb commit]; [cb waitUntilCompleted];
  assert(cb.status == MTLCommandBufferStatusCompleted);
  [mdr_lease(cb, NO) finish];
  return [NSData dataWithBytes:mdr_restore_buffer.contents length:r->bytes];
}

static void make_cold(id<MTLTexture> t) {
  MDRTexture *r = objc_getAssociatedObject(t, &mdr_texture_key);
  assert(r && !r->users); r->last_use = 0; r->retry_after = 0;
}

int main(void) {
  @autoreleasepool {
    setenv("MADEIRA_TEXTURE_RESIDENCY", "1", 1);
    id<MTLDevice> device = MTLCreateSystemDefaultDevice(); assert(device);
    assert(mdr_initialize(device));
    game_queue = [device newCommandQueue];
    mdr_running = YES; // manually drive the SAME worker deterministically
    mdr_target = 0;
    unsigned parity_cases = 0;
    for (unsigned f = 0; f < 4; ++f) {
      fprintf(stderr, "fixture: format %u begin\n", f);
      @autoreleasepool {
        MTLPixelFormat format[] = {MTLPixelFormatBC1_RGBA, MTLPixelFormatBC3_RGBA,
                                   MTLPixelFormatBC5_RGUnorm, MTLPixelFormatBC7_RGBAUnorm};
        id<MTLTexture> t = make_texture(device, format[f], 512, 2, 10, 7+f);
        NSData *before = [read_texture(t, NO) retain];
        id<MTLTexture> view = [t newTextureViewWithPixelFormat:format[f]];
        assert(view); mdr_register_view(t, view);
        uint64_t resource_id = view.gpuResourceID._impl;
        make_cold(t);
        assert(mdr_checkpoint_one());
        fprintf(stderr, "fixture: format %u first eviction\n", f);
        MDRTexture *r = objc_getAssociatedObject(t, &mdr_texture_key);
        assert(r->empty && r->backing && !r->users);
        NSData *after = read_texture(view, YES);
        assert([before isEqualToData:after]);
        assert(view.gpuResourceID._impl == resource_id && !r->empty);
        ++parity_cases;
        // Pinning starts at ENCODE, before commit; shared views count once.
        id<MTLCommandBuffer> hold = [[game_queue commandBuffer] retain];
        id<MTLBlitCommandEncoder> enc = [hold blitCommandEncoder]; mdr_encoder(hold, enc);
        assert(mdr_use(enc, t, NO) && mdr_use(enc, view, NO));
        r->last_use = 0; assert(r->users == 1); assert(!mdr_checkpoint_one());
        [enc endEncoding];
        [mdr_lease(hold, NO) finish]; [hold release];
        // Disk failure leaves the live texture untouched and readable.
        [r dropBacking]; make_cold(t); mdr_test_disk_failure = YES;
        assert(!mdr_checkpoint_one()); assert(!r->empty && !r->backing);
        assert([before isEqualToData:read_texture(t, NO)]);
        mdr_test_disk_failure = NO; make_cold(t); assert(mdr_checkpoint_one());
        // A write must restore untouched subresources first, then invalidate
        // the checkpoint. Update only one BC block in one array slice/mip.
        id<MTLCommandBuffer> write = [game_queue commandBuffer];
        id<MTLBlitCommandEncoder> wb = [write blitCommandEncoder]; mdr_encoder(write, wb);
        assert(mdr_use(wb, t, YES)); assert(!r->empty && !r->backing);
        NSUInteger block = mdr_block_bytes(format[f]);
        memset(mdr_restore_buffer.contents, 0xa3, 256);
        [wb copyFromBuffer:mdr_restore_buffer sourceOffset:0 sourceBytesPerRow:256 sourceBytesPerImage:0
                 sourceSize:MTLSizeMake(4,4,1) toTexture:t destinationSlice:1 destinationLevel:0
           destinationOrigin:MTLOriginMake(4,4,0)];
        [wb endEncoding]; assert(mdr_commit(write)); [write commit]; [write waitUntilCompleted];
        [mdr_lease(write, NO) finish]; assert(write.status == MTLCommandBufferStatusCompleted);
        NSMutableData *expected = [before mutableCopy];
        MDRSubresource s = r->sub[10];
        memset((char *)expected.mutableBytes + s.offset + s.row + block, 0xa3, block);
        assert([expected isEqualToData:read_texture(t, NO)]);
        make_cold(t); assert(mdr_checkpoint_one());
        assert([expected isEqualToData:read_texture(view, YES)]);
        ++parity_cases;
        [expected release]; [before release]; [view release]; [t release];
        fprintf(stderr, "fixture: format %u passed\n", f);
      }
    }
    // Native allocation/footprint reduction, not just a simulated byte counter.
    NSMutableArray *pool = [NSMutableArray new];
    for (unsigned i = 0; i < 48; ++i) {
      @autoreleasepool {
        id<MTLTexture> t = make_texture(device, MTLPixelFormatBC7_RGBAUnorm, 4096, 1, 13, i);
        [pool addObject:t]; [t release];
      }
    }
    uint64_t full = footprint(), metal_full = device.currentAllocatedSize;
    fprintf(stderr, "fixture: large pool ready, footprint=%lluMiB\n", (unsigned long long)(full/MDR_MIB));
    uint64_t expected_bytes = mdr_registered;
    for (id<MTLTexture> t in pool) make_cold(t);
    unsigned evicted = 0;
    while (mdr_checkpoint_one()) ++evicted;
    assert(evicted == 48);
    uint64_t paged = footprint(), metal_paged = device.currentAllocatedSize;
    assert(mdr_empty >= expected_bytes);
    // The resident allocation must actually leave physical memory. Allow
    // native allocator/bookkeeping variance, but require >512 MiB reclaimed.
    assert(full > paged + 512*MDR_MIB);
    for (unsigned i = 0; i < 8; ++i) {
      @autoreleasepool { assert(read_texture(pool[i], NO).length > 0); }
    }
    // Restore failure cancels only the affected native command; its status
    // adapter reports device error rather than submitting undefined texels.
    id<MTLTexture> failed = pool[10];
    mdr_test_restore_failure = YES;
    id<MTLCommandBuffer> cb = [game_queue commandBuffer];
    id<MTLBlitCommandEncoder> enc = [cb blitCommandEncoder]; mdr_encoder(cb, enc);
    assert(!mdr_use(enc, failed, NO)); [enc endEncoding];
    assert(mdr_failed(cb) && !mdr_commit(cb));
    mdr_test_restore_failure = NO;
    printf("{\"actualMetal\":true,\"parityCases\":%u,\"coldTextures\":%u,\"beforeFootprint\":%llu,\"afterFootprint\":%llu,"
           "\"beforeMetalAllocated\":%llu,\"afterMetalAllocated\":%llu,\"emptyBytes\":%llu,\"backingBytes\":%llu,"
           "\"restores\":%llu,\"restoreTotalMs\":%.3f,\"gameFPSMeasured\":false}\n",
           parity_cases, evicted, (unsigned long long)full, (unsigned long long)paged,
           (unsigned long long)metal_full, (unsigned long long)metal_paged,
           (unsigned long long)mdr_empty, (unsigned long long)mdr_backed,
           (unsigned long long)mdr_restores, mdr_restore_ns/1e6);
    [pool release]; [device release];
  }
  return 0;
}
