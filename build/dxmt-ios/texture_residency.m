/* SPDX-License-Identifier: MIT
 * Lossless, bounded paging of cold native BC textures. Included by winemetal's
 * Objective-C boundary (MRC), and by the real-Metal regression executable.
 *
 * A command-buffer lease pins every texture from encoding through completion,
 * including indirect argument-buffer resources and views. No D3D object, view,
 * GPU resource ID, mip, sampler, or image size is replaced. Only completed, cold
 * allocations with a complete file-backed checkpoint may become Empty.
 *
 * Snapshots run off the render thread, at most 32 MiB at a time. They use a
 * separate Metal queue after prior users have completed. A new writer waits
 * for an outstanding snapshot copy, invalidates its generation, then proceeds.
 * A read can overlap that read-only copy. Restore completes before encoding any
 * new use, so it cannot miss DXMT's earlier initializer-event snapshot.
 */
#import <Foundation/Foundation.h>
#import <Metal/Metal.h>
#import <objc/runtime.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdatomic.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <limits.h>

#define MDR_MIB (UINT64_C(1024) * 1024)
#define MDR_STAGE_BYTES (32 * MDR_MIB)
#define MDR_BACKING_LIMIT (8 * UINT64_C(1024) * MDR_MIB)
#define MDR_MAX_SUBRESOURCES 96

typedef struct {
  NSUInteger level, slice, width, height, row, offset, length;
} MDRSubresource;

static NSRecursiveLock *mdr_lock;
static NSLock *mdr_restore_lock;
static NSMapTable *mdr_registry;
static dispatch_queue_t mdr_worker;
static uint64_t mdr_registered, mdr_empty, mdr_backed, mdr_evictions, mdr_restores;
static uint64_t mdr_restore_ns, mdr_io_failures;
static NSTimeInterval mdr_last_scan, mdr_last_report;
static BOOL mdr_running;
static char mdr_texture_key, mdr_lease_key;
static id<MTLDevice> mdr_device;
static id<MTLCommandQueue> mdr_queue;
static id<MTLBuffer> mdr_snapshot_buffer, mdr_restore_buffer;
static _Atomic BOOL mdr_available;
static _Atomic BOOL mdr_device_failed;
static uint64_t mdr_target;
static NSTimeInterval mdr_idle_seconds = 10.0;
#ifdef MADEIRA_RESIDENCY_TEST
static BOOL mdr_test_disk_failure, mdr_test_restore_failure;
static void (^mdr_test_after_snapshot)(void);
#endif

@interface MDRTexture : NSObject {
@public
  MDRSubresource sub[MDR_MAX_SUBRESOURCES];
  NSUInteger count, bytes, mapped_bytes;
  uint64_t allocation_bytes, generation, users;
  NSTimeInterval last_use, retry_after;
  BOOL empty, snapshot;
  void *backing;
  id<MTLCommandBuffer> snapshot_command;
}
- (void)dropBacking;
@end

@implementation MDRTexture
- (void)dropBacking {
  if (backing) {
    munmap(backing, mapped_bytes);
    mdr_backed -= bytes;
    backing = NULL;
  }
}
- (void)dealloc {
  [mdr_lock lock];
  [self dropBacking];
  mdr_registered -= allocation_bytes;
  if (empty) mdr_empty -= allocation_bytes;
  [snapshot_command release];
  [mdr_lock unlock];
  [super dealloc];
}
@end

@interface MDRLease : NSObject {
@public
  NSMapTable *textures;
  BOOL retired, failed;
}
- (void)finish;
@end

@implementation MDRLease
- (id)init {
  if ((self = [super init])) textures = [[NSMapTable strongToStrongObjectsMapTable] retain];
  return self;
}
- (void)finish {
  [mdr_lock lock];
  if (!retired) {
    retired = YES;
    for (MDRTexture *record in textures.objectEnumerator) {
      NSCAssert(record->users, @"unbalanced residency lease");
      --record->users;
    }
    [textures removeAllObjects];
  }
  [mdr_lock unlock];
}
- (void)dealloc {
  [self finish];
  [textures release];
  [super dealloc];
}
@end

static NSUInteger mdr_block_bytes(MTLPixelFormat f) {
  switch (f) {
  case MTLPixelFormatBC1_RGBA: case MTLPixelFormatBC1_RGBA_sRGB:
  case MTLPixelFormatBC4_RUnorm: case MTLPixelFormatBC4_RSnorm: return 8;
  case MTLPixelFormatBC2_RGBA: case MTLPixelFormatBC2_RGBA_sRGB:
  case MTLPixelFormatBC3_RGBA: case MTLPixelFormatBC3_RGBA_sRGB:
  case MTLPixelFormatBC5_RGUnorm: case MTLPixelFormatBC5_RGSnorm:
  case MTLPixelFormatBC6H_RGBFloat: case MTLPixelFormatBC6H_RGBUfloat:
  case MTLPixelFormatBC7_RGBAUnorm: case MTLPixelFormatBC7_RGBAUnorm_sRGB: return 16;
  default: return 0;
  }
}

static BOOL mdr_enabled(void) {
  const char *value = getenv("MADEIRA_TEXTURE_RESIDENCY");
  return value && !strcmp(value, "1");
}

static BOOL mdr_initialize(id<MTLDevice> device) {
  static dispatch_once_t once;
  dispatch_once(&once, ^{
    mdr_lock = [NSRecursiveLock new];
    mdr_restore_lock = [NSLock new];
    // The resource association owns its record. The registry must not keep
    // file mappings/byte counters alive after the last resource/view dies.
    mdr_registry = [[NSMapTable weakToWeakObjectsMapTable] retain];
    mdr_worker = dispatch_queue_create("madeira.texture-checkpoint", DISPATCH_QUEUE_SERIAL);
    dispatch_set_target_queue(mdr_worker, dispatch_get_global_queue(QOS_CLASS_UTILITY, 0));
    if (!device.hasUnifiedMemory || !device.supportsBCTextureCompression) return;
    mdr_device = [device retain];
    mdr_queue = [device newCommandQueueWithMaxCommandBufferCount:2];
    mdr_snapshot_buffer = [device newBufferWithLength:MDR_STAGE_BYTES options:MTLResourceStorageModeShared];
    mdr_restore_buffer = [device newBufferWithLength:MDR_STAGE_BYTES options:MTLResourceStorageModeShared];
    mdr_available = mdr_queue && mdr_snapshot_buffer && mdr_restore_buffer;
    if (!mdr_available) {
      [mdr_snapshot_buffer release]; mdr_snapshot_buffer = nil;
      [mdr_restore_buffer release]; mdr_restore_buffer = nil;
      [mdr_queue release]; mdr_queue = nil;
      return;
    }
    // This is a resident-BC budget, not Metal's total allocation counter:
    // allocatedSize can continue to count an Empty resource's virtual storage.
    mdr_target = MIN(NSProcessInfo.processInfo.physicalMemory / 4,
                     device.recommendedMaxWorkingSetSize / 2);
    mdr_target = MAX(mdr_target, 512 * MDR_MIB);
    fprintf(stderr, "[texture-residency-v1] lossless BC target=%lluMiB staging=64MiB cold=10s\n",
            (unsigned long long)(mdr_target / MDR_MIB));
  });
  return mdr_available && device == mdr_device;
}

static void mdr_register(id<MTLTexture> texture, BOOL shared) {
  if (!texture || shared || !mdr_enabled() || !mdr_block_bytes(texture.pixelFormat) ||
      texture.storageMode != MTLStorageModePrivate || texture.parentTexture || texture.buffer || texture.heap ||
      texture.iosurface || texture.sampleCount != 1 ||
      (texture.usage & (MTLTextureUsageRenderTarget | MTLTextureUsageShaderWrite)) ||
      texture.width < 256 || texture.height < 256) return;
  MTLTextureType type = texture.textureType;
  if (type != MTLTextureType2D && type != MTLTextureType2DArray &&
      type != MTLTextureTypeCube && type != MTLTextureTypeCubeArray) return;
  NSUInteger slices = texture.arrayLength * ((type == MTLTextureTypeCube || type == MTLTextureTypeCubeArray) ? 6 : 1);
  if (slices * texture.mipmapLevelCount > MDR_MAX_SUBRESOURCES || !mdr_initialize(texture.device)) return;
  MDRTexture *record = [MDRTexture new];
  NSUInteger block = mdr_block_bytes(texture.pixelFormat);
  for (NSUInteger slice = 0; slice < slices; ++slice) {
    for (NSUInteger level = 0; level < texture.mipmapLevelCount; ++level) {
      NSUInteger w = MAX(texture.width >> level, 1), h = MAX(texture.height >> level, 1);
      NSUInteger row = (((w + 3) / 4 * block) + 255) & ~(NSUInteger)255;
      NSUInteger length = row * ((h + 3) / 4);
      record->sub[record->count++] = (MDRSubresource){level, slice, w, h, row, record->bytes, length};
      record->bytes += length;
    }
  }
  if (record->bytes > MDR_STAGE_BYTES || !record->bytes) { [record release]; return; }
  record->last_use = NSProcessInfo.processInfo.systemUptime;
  record->mapped_bytes = (record->bytes + getpagesize() - 1) & ~(NSUInteger)(getpagesize() - 1);
  [mdr_lock lock];
  record->allocation_bytes = texture.allocatedSize;
  mdr_registered += record->allocation_bytes;
  objc_setAssociatedObject(texture, &mdr_texture_key, record, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
  [mdr_registry setObject:record forKey:texture];
  [record release];
  [mdr_lock unlock];
}

static void mdr_register_view(id<MTLTexture> parent, id<MTLTexture> view) {
  MDRTexture *record = objc_getAssociatedObject(parent, &mdr_texture_key);
  if (record && view) objc_setAssociatedObject(view, &mdr_texture_key, record, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
}

static MDRLease *mdr_lease(id object, BOOL create) {
  if (!mdr_available) return nil;
  MDRLease *lease = objc_getAssociatedObject(object, &mdr_lease_key);
  if (!lease && create && mdr_available) {
    lease = [[MDRLease alloc] init];
    objc_setAssociatedObject(object, &mdr_lease_key, lease, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
    [lease release];
  }
  return lease;
}

static void mdr_encoder(id<MTLCommandBuffer> command, id<MTLCommandEncoder> encoder) {
  if (mdr_available && encoder)
    objc_setAssociatedObject(encoder, &mdr_lease_key, mdr_lease(command, YES), OBJC_ASSOCIATION_RETAIN_NONATOMIC);
}

static BOOL mdr_failed(id object) {
  MDRLease *lease = mdr_lease(object, NO);
  return lease && lease->failed;
}

static void mdr_copy(id<MTLBlitCommandEncoder> encoder, id<MTLTexture> texture,
                     MDRTexture *record, id<MTLBuffer> buffer, BOOL restore) {
  for (NSUInteger i = 0; i < record->count; ++i) {
    MDRSubresource s = record->sub[i];
    if (restore) {
      [encoder copyFromBuffer:buffer sourceOffset:s.offset sourceBytesPerRow:s.row sourceBytesPerImage:0
                   sourceSize:MTLSizeMake(s.width, s.height, 1) toTexture:texture
             destinationSlice:s.slice destinationLevel:s.level destinationOrigin:MTLOriginMake(0, 0, 0)];
    } else {
      [encoder copyFromTexture:texture sourceSlice:s.slice sourceLevel:s.level sourceOrigin:MTLOriginMake(0, 0, 0)
                    sourceSize:MTLSizeMake(s.width, s.height, 1) toBuffer:buffer destinationOffset:s.offset
           destinationBytesPerRow:s.row destinationBytesPerImage:0];
    }
  }
}

// A lease already pins the resource. Serialize the shared restore buffer but
// never hold mdr_lock across GPU completion: game completion handlers need
// that lock to release their leases, including on a driver's shared callback
// thread. The record stays Empty until restoration completes, so another
// reader/writer must join this same restore lock before encoding its work.
static BOOL mdr_restore(id<MTLTexture> texture, MDRTexture *record) {
  @autoreleasepool {
  [mdr_restore_lock lock];
  [mdr_lock lock];
  if (!record->empty) { [mdr_lock unlock]; [mdr_restore_lock unlock]; return YES; }
  if (!record->backing) { [mdr_lock unlock]; [mdr_restore_lock unlock]; return NO; }
#ifdef MADEIRA_RESIDENCY_TEST
  if (mdr_test_restore_failure) { [mdr_lock unlock]; [mdr_restore_lock unlock]; return NO; }
#endif
  [mdr_lock unlock];
  NSTimeInterval begin = NSProcessInfo.processInfo.systemUptime;
  memcpy(mdr_restore_buffer.contents, record->backing, record->bytes);
  madvise(record->backing, record->mapped_bytes, MADV_DONTNEED);
  id<MTLCommandBuffer> command = [mdr_queue commandBuffer];
  id<MTLBlitCommandEncoder> encoder = [command blitCommandEncoder];
  if (!encoder) { [mdr_restore_lock unlock]; return NO; }
  [texture setPurgeableState:MTLPurgeableStateNonVolatile];
  mdr_copy(encoder, texture, record, mdr_restore_buffer, YES);
  [encoder endEncoding];
  [command commit];
  [command waitUntilCompleted];
  if (command.status != MTLCommandBufferStatusCompleted) { [mdr_restore_lock unlock]; return NO; }
  [mdr_lock lock];
  record->empty = NO;
  mdr_empty -= record->allocation_bytes;
  ++mdr_restores;
  mdr_restore_ns += (NSProcessInfo.processInfo.systemUptime - begin) * 1e9;
  [mdr_lock unlock];
  [mdr_restore_lock unlock];
  return YES;
  }
}

// Both texture views and base textures share a record. A lease is keyed by the
// record identity (not the view); its key also retains the exact native resource
// used, keeping the parent alive until the command completes or is abandoned.
static BOOL mdr_use(id encoder, id<MTLResource> resource, BOOL write) {
  if (!mdr_available) return YES;
  MDRTexture *record = objc_getAssociatedObject(resource, &mdr_texture_key);
  if (!record) return !mdr_failed(encoder);
  MDRLease *lease = mdr_lease(encoder, NO);
  if (!lease) return NO; // no untracked encoding of a pageable allocation
  id<MTLTexture> root = (id<MTLTexture>)resource;
  while (root.parentTexture) root = root.parentTexture;
  [mdr_lock lock];
  if (lease->failed || lease->retired || mdr_device_failed) {
    lease->failed = YES;
    [mdr_lock unlock];
    return NO;
  }
  if (![lease->textures objectForKey:root]) {
    ++record->users;
    [lease->textures setObject:record forKey:root];
  }
  record->last_use = NSProcessInfo.processInfo.systemUptime;
  BOOL restore = record->empty;
  [mdr_lock unlock];
  if (restore && !mdr_restore(root, record)) {
    [mdr_lock lock]; lease->failed = YES; mdr_device_failed = YES; [mdr_lock unlock];
    fprintf(stderr, "[texture-residency-v1] restore failed; cancelling affected and subsequent commands\n");
    return NO;
  }
  [mdr_lock lock];
  id<MTLCommandBuffer> snapshot_command = nil;
  if (write) {
    // A readback uses another queue. Serialize only a concurrent WRITE with
    // that readback, not ordinary sampling or the subsequent file write.
    snapshot_command = [record->snapshot_command retain];
    ++record->generation;
    [record dropBacking];
  }
  [mdr_lock unlock];
  [snapshot_command waitUntilCompleted];
  [snapshot_command release];
  return YES;
}

static BOOL mdr_make_empty(id<MTLTexture> texture, MDRTexture *record) {
  [texture setPurgeableState:MTLPurgeableStateEmpty];
  if ([texture setPurgeableState:MTLPurgeableStateKeepCurrent] != MTLPurgeableStateEmpty) {
    record->retry_after = NSProcessInfo.processInfo.systemUptime + 30;
    return NO;
  }
  record->empty = YES;
  mdr_empty += record->allocation_bytes;
  ++mdr_evictions;
  return YES;
}

// A private unlinked mapping owns its backing file without keeping one fd per
// texture. It is clean VM, not a second malloc/NSData copy of the texture pool.
// Failed/truncated writes never authorize eviction. A crash leaves no cache.
static void *mdr_checkpoint(MDRTexture *record, const void *data) {
#ifdef MADEIRA_RESIDENCY_TEST
  if (mdr_test_disk_failure) { errno = ENOSPC; return NULL; }
#endif
  NSString *folder = NSTemporaryDirectory();
  struct statfs space;
  if (statfs(folder.fileSystemRepresentation, &space) ||
      (uint64_t)space.f_bavail * space.f_bsize < record->mapped_bytes + 2 * UINT64_C(1024) * MDR_MIB) return NULL;
  char path[PATH_MAX];
  int path_length = snprintf(path, sizeof(path), "%s/madeira-bc-XXXXXX", folder.fileSystemRepresentation);
  if (path_length < 0 || (size_t)path_length >= sizeof(path)) return NULL;
  int fd = mkstemp(path);
  if (fd < 0) return NULL;
  unlink(path);
  fcntl(fd, F_NOCACHE, 1);
  BOOL okay = ftruncate(fd, record->mapped_bytes) == 0;
  NSUInteger offset = 0;
  while (okay && offset < record->bytes) {
    ssize_t n = pwrite(fd, (const char *)data + offset, record->bytes - offset, offset);
    if (n < 0 && errno == EINTR) continue;
    if (n <= 0) { okay = NO; break; }
    offset += n;
  }
  void *mapping = okay ? mmap(NULL, record->mapped_bytes, PROT_READ, MAP_PRIVATE, fd, 0) : MAP_FAILED;
  close(fd);
  if (mapping == MAP_FAILED) return NULL;
  madvise(mapping, record->mapped_bytes, MADV_RANDOM);
  return mapping;
}

// At most one worker owns the snapshot staging buffer. Selection, generation
// checks and Empty transitions share the same lock as encoding and completion.
static BOOL mdr_checkpoint_one(void) {
  @autoreleasepool {
  id<MTLTexture> texture = nil;
  MDRTexture *record = nil;
  uint64_t generation = 0;
  NSTimeInterval selected_use = 0, now = NSProcessInfo.processInfo.systemUptime;
  [mdr_lock lock];
  if (mdr_registered - mdr_empty <= mdr_target) { [mdr_lock unlock]; return NO; }
  for (id<MTLTexture> candidate in mdr_registry.keyEnumerator) {
    MDRTexture *r = [mdr_registry objectForKey:candidate];
    if (!r || r->users || r->empty || r->snapshot || now < r->retry_after ||
        now - r->last_use < mdr_idle_seconds) continue;
    if (!record || r->last_use < record->last_use) {
      [texture release]; texture = [candidate retain]; record = r;
    }
  }
  if (!record) { [mdr_lock unlock]; return NO; }
  if (record->backing) {
    BOOL evicted = mdr_make_empty(texture, record);
    [texture release];
    [mdr_lock unlock];
    return evicted;
  }
  if (mdr_backed + record->bytes > MDR_BACKING_LIMIT) { [texture release]; [mdr_lock unlock]; return NO; }
  [record retain];
  record->snapshot = YES;
  generation = record->generation;
  selected_use = record->last_use;
  id<MTLCommandBuffer> command = [mdr_queue commandBuffer];
  id<MTLBlitCommandEncoder> encoder = [command blitCommandEncoder];
  if (encoder) {
    memset(mdr_snapshot_buffer.contents, 0, record->bytes);
    mdr_copy(encoder, texture, record, mdr_snapshot_buffer, NO);
    [encoder endEncoding];
    record->snapshot_command = [command retain];
    [command commit];
  }
  [mdr_lock unlock];
  [command waitUntilCompleted];
#ifdef MADEIRA_RESIDENCY_TEST
  if (mdr_test_after_snapshot) mdr_test_after_snapshot();
#endif
  void *mapping = encoder && command.status == MTLCommandBufferStatusCompleted
                      ? mdr_checkpoint(record, mdr_snapshot_buffer.contents) : NULL;
  [mdr_lock lock];
  [record->snapshot_command release]; record->snapshot_command = nil;
  record->snapshot = NO;
  if (mapping && generation == record->generation) {
    record->backing = mapping;
    mdr_backed += record->bytes;
    if (!record->users && record->last_use == selected_use) {
      mdr_make_empty(texture, record);
    }
  } else {
    if (mapping) munmap(mapping, record->mapped_bytes);
    else ++mdr_io_failures;
    record->retry_after = NSProcessInfo.processInfo.systemUptime + 30;
  }
  [record release]; [texture release];
  [mdr_lock unlock];
  return mapping != NULL;
  }
}

static void mdr_schedule(void) {
  if (!mdr_available) return;
  NSTimeInterval now = NSProcessInfo.processInfo.systemUptime;
  [mdr_lock lock];
  if (now - mdr_last_report >= 10) {
    mdr_last_report = now;
    fprintf(stderr, "[texture-residency-v1] tracked=%lluMiB empty=%lluMiB backing=%lluMiB evict=%llu restore=%llu restore_ms=%.2f io_fail=%llu\n",
            (unsigned long long)(mdr_registered/MDR_MIB), (unsigned long long)(mdr_empty/MDR_MIB),
            (unsigned long long)(mdr_backed/MDR_MIB), (unsigned long long)mdr_evictions,
            (unsigned long long)mdr_restores, mdr_restore_ns/1e6, (unsigned long long)mdr_io_failures);
  }
  if (mdr_running || now - mdr_last_scan < 0.25 ||
      mdr_registered - mdr_empty < mdr_target + 256*MDR_MIB) { [mdr_lock unlock]; return; }
  mdr_last_scan = now;
  mdr_running = YES;
  dispatch_async(mdr_worker, ^{
    @autoreleasepool {
      mdr_checkpoint_one();
      [mdr_lock lock]; mdr_running = NO; [mdr_lock unlock];
    }
  });
  [mdr_lock unlock];
}

static BOOL mdr_commit(id<MTLCommandBuffer> command) {
  MDRLease *lease = mdr_lease(command, NO);
  // Do not poison already committed buffers: their GPU work must finish
  // before releasing resources. Cancel only new submissions after a failure.
  if (mdr_device_failed && !lease) lease = mdr_lease(command, YES);
  if (lease) {
    if (mdr_device_failed) lease->failed = YES;
    if (lease->failed) { [lease finish]; return NO; }
    [command addCompletedHandler:^(id<MTLCommandBuffer> completed) { (void)completed; [lease finish]; }];
  }
  mdr_schedule();
  return YES;
}
