/* SPDX-License-Identifier: MIT
 * Learn a bounded inventory of already translated metallibs and exact ordinary
 * DXMT pipeline descriptors. Replay archive hits on a background queue next run.
 * No render-thread wait, no missing draw, no cold pipeline compilation in replay.
 */
#import "pipeline_warmup_ios.h"
#import <CommonCrypto/CommonDigest.h>
#import <objc/runtime.h>
#include <stdatomic.h>
#include <stdlib.h>
#include <string.h>

static char libraryKey, functionKey, allowedKey;
static const NSUInteger libraryLimit = 2 * 1024 * 1024, diskLimit = 32 * 1024 * 1024;
static const NSUInteger recipeLimit = 128, warmLimit = 64, manifestLimit = 512 * 1024;

static NSString *digest(NSData *bytes) {
    unsigned char hash[CC_SHA256_DIGEST_LENGTH];
    CC_SHA256(bytes.bytes, (CC_LONG)bytes.length, hash);
    NSMutableString *out = [NSMutableString string];
    for (unsigned i = 0; i < sizeof(hash); ++i) [out appendFormat:@"%02x", hash[i]];
    return out;
}
static NSData *json(id value) {
    return value ? [NSJSONSerialization dataWithJSONObject:value options:NSJSONWritingSortedKeys error:nil] : nil;
}
static BOOL hashName(id value) {
    if (![value isKindOfClass:NSString.class] || [value length] != 64) return NO;
    return [value rangeOfCharacterFromSet:[[NSCharacterSet characterSetWithCharactersInString:@"0123456789abcdef"] invertedSet]].location == NSNotFound;
}
static NSUInteger constantSize(MTLDataType type) {
    switch (type) {
        case MTLDataTypeBool: case MTLDataTypeChar: case MTLDataTypeUChar: return 1;
        case MTLDataTypeShort: case MTLDataTypeUShort: case MTLDataTypeHalf: return 2;
        case MTLDataTypeInt: case MTLDataTypeUInt: case MTLDataTypeFloat: return 4;
        case MTLDataTypeLong: case MTLDataTypeULong: return 8;
        default: return 0; // vector/aggregate constants aren't silently approximated
    }
}
NSDictionary *madeira_warmup_constant(const void *value, MTLDataType type, NSUInteger index) {
    NSUInteger size = constantSize(type);
    if (!value || !size || index > 65535) return nil;
    return @{@"type": @(type), @"index": @(index),
             @"value": [[NSData dataWithBytes:value length:size] base64EncodedStringWithOptions:0]};
}
void madeira_pipeline_allow_warmup(id descriptor, BOOL allowed) {
    objc_setAssociatedObject(descriptor, &allowedKey, allowed ? @YES : nil, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
}
void madeira_warmup_track_function(id<MTLLibrary> library, id<MTLFunction> function, NSString *name, NSArray *constants, BOOL specialized) {
    NSString *hash = objc_getAssociatedObject(library, &libraryKey);
    if (!function || !hash || !name.length || name.length > 512 || !constants || constants.count > 64) return;
    objc_setAssociatedObject(function, &functionKey, @{@"library": hash, @"name": name, @"constants": constants, @"specialized": @(specialized)}, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
}
void madeira_warmup_track_library(id<MTLLibrary> library, dispatch_data_t data) {
    if (!library || !data || dispatch_data_get_size(data) > libraryLimit) return;
    MadeiraPipelineWarmup *warmup = madeira_pipeline_warmup_for_device(library.device);
    if (!warmup) return;
    const void *bytes = NULL; size_t length = 0;
    dispatch_data_t mapped = dispatch_data_create_map(data, &bytes, &length);
    if (!mapped || !bytes || !length || length > libraryLimit) return;
    NSData *copy = [NSData dataWithBytes:bytes length:length];
    NSString *hash = digest(copy);
    // Functions retain only identities, not another copy of each shader library.
    objc_setAssociatedObject(library, &libraryKey, hash, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
    [warmup storeLibrary:copy hash:hash];
}

static NSDictionary *functionRecipe(id<MTLFunction> fn) { return objc_getAssociatedObject(fn, &functionKey); }
static NSDictionary *recipe(id descriptor, BOOL compute) {
    if (!objc_getAssociatedObject(descriptor, &allowedKey)) return nil;
    if (compute) {
        // Metal returns internal descriptor implementations; their class
        // ancestry is not the public API contract. The thunk marker plus
        // required selectors identifies the ordinary descriptor we recorded.
        if (![descriptor respondsToSelector:@selector(computeFunction)] ||
            ![descriptor respondsToSelector:@selector(buffers)]) return nil;
        MTLComputePipelineDescriptor *d = descriptor;
        NSDictionary *fn = functionRecipe(d.computeFunction); if (!fn) return nil;
        NSMutableArray *buffers = [NSMutableArray array];
        for (unsigned i = 0; i < 31; ++i) [buffers addObject:@(d.buffers[i].mutability)];
        return @{@"kind": @"compute", @"function": fn, @"buffers": buffers,
                 @"multiple": @(d.threadGroupSizeIsMultipleOfThreadExecutionWidth)};
    }
    if (![descriptor respondsToSelector:@selector(vertexFunction)] ||
        ![descriptor respondsToSelector:@selector(colorAttachments)]) return nil;
    MTLRenderPipelineDescriptor *d = descriptor;
    // Metal may materialize an empty vertex descriptor even when the thunk
    // never assigned one (observed on macOS 26). Empty is the default, not a
    // custom input layout. Defined attributes/strides and patch shaders remain
    // outside this ordinary DXMT recipe.
    MTLVertexDescriptor *layout = d.vertexDescriptor;
    for (unsigned i = 0; layout && i < 31; ++i)
        if (layout.attributes[i].format != MTLVertexFormatInvalid || layout.layouts[i].stride) return nil;
    if (d.vertexFunction.patchType != MTLPatchTypeNone) return nil;
    NSDictionary *vs = functionRecipe(d.vertexFunction), *ps = functionRecipe(d.fragmentFunction);
    if (!vs || (d.fragmentFunction && !ps)) return nil;
    NSMutableArray *colors = [NSMutableArray array], *vb = [NSMutableArray array], *fb = [NSMutableArray array];
    for (unsigned i = 0; i < 8; ++i) {
        MTLRenderPipelineColorAttachmentDescriptor *c = d.colorAttachments[i];
        [colors addObject:@[@(c.pixelFormat), @(c.blendingEnabled), @(c.writeMask), @(c.alphaBlendOperation),
            @(c.rgbBlendOperation), @(c.sourceRGBBlendFactor), @(c.sourceAlphaBlendFactor),
            @(c.destinationRGBBlendFactor), @(c.destinationAlphaBlendFactor)]];
    }
    for (unsigned i = 0; i < 31; ++i) { [vb addObject:@(d.vertexBuffers[i].mutability)]; [fb addObject:@(d.fragmentBuffers[i].mutability)]; }
    return @{@"kind": @"render", @"vertex": vs, @"fragment": ps ?: NSNull.null,
             @"colors": colors, @"vb": vb, @"fb": fb,
             @"state": @[@(d.depthAttachmentPixelFormat), @(d.stencilAttachmentPixelFormat),
                @(d.alphaToCoverageEnabled), @(d.rasterizationEnabled), @(d.rasterSampleCount),
                @(d.inputPrimitiveTopology), @(d.tessellationPartitionMode), @(d.tessellationFactorStepFunction),
                @(d.tessellationOutputWindingOrder), @(d.maxTessellationFactor)]};
}
static BOOL numbers(id value, NSUInteger count, NSUInteger maximum) {
    if (![value isKindOfClass:NSArray.class] || [value count] != count) return NO;
    for (id n in value) if (![n isKindOfClass:NSNumber.class] || [n doubleValue] < 0 ||
                           [n doubleValue] > maximum || [n doubleValue] != [n unsignedIntegerValue]) return NO;
    return YES;
}

@interface MadeiraPipelineWarmup () {
    id<MTLDevice> device;
    id<MTLBinaryArchive> archive;
    NSURL *folder;
    dispatch_queue_t queue;
    NSLock *lock;
    NSMutableDictionary *ready, *records;
    NSMutableSet *libraries, *submittedLibraries;
    NSMutableDictionary *librarySizes;
    NSMutableArray *order;
    NSUInteger libraryBytes;
    atomic_uint pending;
    unsigned warmHits;
    BOOL flushScheduled;
}
@end
@implementation MadeiraPipelineWarmup
- (instancetype)initWithDevice:(id<MTLDevice>)d folder:(NSURL *)f archive:(id<MTLBinaryArchive>)a {
    if (!(self = [super init])) return nil;
    const char *mode = getenv("MADEIRA_SHADER_WARMUP");
    if (!mode || strcmp(mode, "1") || !f) return nil;
    device = d; folder = f; archive = a; lock = [NSLock new];
    ready = [NSMutableDictionary dictionary]; records = [NSMutableDictionary dictionary];
    libraries = [NSMutableSet set]; submittedLibraries = [NSMutableSet set]; librarySizes = [NSMutableDictionary dictionary];
    order = [NSMutableArray array]; atomic_init(&pending, 0);
    queue = dispatch_queue_create("madeira.metal.warmup", dispatch_queue_attr_make_with_qos_class(DISPATCH_QUEUE_SERIAL, QOS_CLASS_BACKGROUND, 0));
    dispatch_async(queue, ^{ @autoreleasepool { [self replay]; } });
    return self;
}
- (NSURL *)libraryFile:(NSString *)hash { return [folder URLByAppendingPathComponent:[hash stringByAppendingString:@".metallib"]]; }
- (void)storeLibrary:(NSData *)bytes hash:(NSString *)hash {
    // Reserve once per content hash, across all live DLLs/functions. Bounds
    // also cap pending copies on the serial writer, not only disk contents.
    [lock lock];
    NSUInteger previous = [librarySizes[hash] unsignedIntegerValue];
    BOOL admit = ![submittedLibraries containsObject:hash] &&
        ([libraries containsObject:hash] || libraries.count < 256) && libraryBytes - previous + bytes.length <= diskLimit;
    if (admit) {
        [libraries addObject:hash]; [submittedLibraries addObject:hash]; librarySizes[hash] = @(bytes.length);
        libraryBytes = libraryBytes - previous + bytes.length;
    }
    [lock unlock];
    if (!admit) return;
    dispatch_async(queue, ^{ @autoreleasepool {
        [NSFileManager.defaultManager createDirectoryAtURL:folder withIntermediateDirectories:YES attributes:nil error:nil];
        NSURL *file = [self libraryFile:hash];
        // Verify existing bytes too: a partial/corrupt write must be repairable.
        NSDictionary *attrs = [NSFileManager.defaultManager attributesOfItemAtPath:file.path error:nil];
        NSData *old = [attrs[NSFileSize] unsignedLongLongValue] <= libraryLimit ? [NSData dataWithContentsOfURL:file] : nil;
        if (!old || ![digest(old) isEqual:hash]) [bytes writeToURL:file options:NSDataWritingAtomic error:nil];
    } });
}
- (id)take:(id)descriptor compute:(BOOL)compute {
    NSData *bytes = json(recipe(descriptor, compute)); if (!bytes) return nil;
    NSString *key = digest(bytes);
    [lock lock]; id state = ready[key]; [ready removeObjectForKey:key];
    if (state) ++warmHits;
    unsigned hits = warmHits; [lock unlock];
    if (state && (hits == 1 || hits % 32 == 0)) NSLog(@"[shader-warmup] pipeline-hits=%u", hits);
    return state;
}
- (void)record:(id)descriptor compute:(BOOL)compute {
    if (atomic_fetch_add_explicit(&pending, 1, memory_order_relaxed) >= recipeLimit) {
        atomic_fetch_sub_explicit(&pending, 1, memory_order_relaxed); return;
    }
    NSDictionary *r = recipe(descriptor, compute);
    if (!r) { atomic_fetch_sub_explicit(&pending, 1, memory_order_relaxed); return; }
    NSString *key = digest(json(r));
    dispatch_async(queue, ^{ @autoreleasepool {
        // Recent ordinary pipelines replace old entries when a game changes.
        if (records[key]) [order removeObject:key];
        records[key] = r; [order addObject:key];
        if (order.count > recipeLimit) { [records removeObjectForKey:order.firstObject]; [order removeObjectAtIndex:0]; }
        if (!flushScheduled) {
            flushScheduled = YES;
            dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 2 * NSEC_PER_SEC), queue, ^{ @autoreleasepool {
                flushScheduled = NO;
                NSMutableArray *items = [NSMutableArray array];
                for (NSString *k in order) if (records[k]) [items addObject:records[k]];
                NSData *bytes = json(@{@"schema": @1, @"recipes": items});
                if (bytes.length <= manifestLimit) {
                    [NSFileManager.defaultManager createDirectoryAtURL:folder withIntermediateDirectories:YES attributes:nil error:nil];
                    [bytes writeToURL:[folder URLByAppendingPathComponent:@"recipes.json"] options:NSDataWritingAtomic error:nil];
                }
            } });
        }
        atomic_fetch_sub_explicit(&pending, 1, memory_order_relaxed);
    } });
}
- (id<MTLFunction>)function:(id)r libraries:(NSMutableDictionary *)loaded {
    if (![r isKindOfClass:NSDictionary.class] || !hashName(r[@"library"]) ||
        ![r[@"name"] isKindOfClass:NSString.class] || ![r[@"name"] length] || [r[@"name"] length] > 512 ||
        ![r[@"constants"] isKindOfClass:NSArray.class] || [r[@"constants"] count] > 64 ||
        !numbers(@[r[@"specialized"] ?: NSNull.null], 1, 1)) return nil;
    NSString *hash = r[@"library"]; id<MTLLibrary> lib = loaded[hash];
    if (!lib) {
        NSURL *file = [self libraryFile:hash];
        NSDictionary *attrs = [NSFileManager.defaultManager attributesOfItemAtPath:file.path error:nil];
        if (!attrs || [attrs[NSFileSize] unsignedLongLongValue] > libraryLimit) return nil;
        NSData *bytes = [NSData dataWithContentsOfURL:file];
        if (!bytes.length || ![digest(bytes) isEqual:hash]) return nil;
        dispatch_data_t data = dispatch_data_create(bytes.bytes, bytes.length, queue, ^{ (void)bytes; });
        lib = [device newLibraryWithData:data error:nil]; if (!lib) return nil;
        objc_setAssociatedObject(lib, &libraryKey, hash, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
        loaded[hash] = lib;
    }
    MTLFunctionConstantValues *values = [MTLFunctionConstantValues new];
    for (id c in r[@"constants"]) {
        if (![c isKindOfClass:NSDictionary.class] || !numbers(@[c[@"type"] ?: NSNull.null, c[@"index"] ?: NSNull.null], 2, 65535) ||
            ![c[@"value"] isKindOfClass:NSString.class] || [c[@"value"] length] > 16) return nil;
        NSUInteger size = constantSize([c[@"type"] unsignedIntegerValue]);
        NSData *data = [[NSData alloc] initWithBase64EncodedString:c[@"value"] options:0];
        if (!size || data.length != size) return nil;
        [values setConstantValue:data.bytes type:[c[@"type"] unsignedIntegerValue] atIndex:[c[@"index"] unsignedIntegerValue]];
    }
    id<MTLFunction> fn = [r[@"specialized"] boolValue] ? [lib newFunctionWithName:r[@"name"] constantValues:values error:nil]
                                               : [lib newFunctionWithName:r[@"name"]];
    madeira_warmup_track_function(lib, fn, r[@"name"], r[@"constants"], [r[@"specialized"] boolValue]); return fn;
}
- (id)descriptor:(NSDictionary *)r libraries:(NSMutableDictionary *)loaded {
    if ([r[@"kind"] isEqual:@"compute"]) {
        if (!numbers(r[@"buffers"], 31, MTLMutabilityImmutable) || !numbers(@[r[@"multiple"] ?: NSNull.null], 1, 1)) return nil;
        MTLComputePipelineDescriptor *d = [MTLComputePipelineDescriptor new];
        d.computeFunction = [self function:r[@"function"] libraries:loaded]; if (!d.computeFunction) return nil;
        d.threadGroupSizeIsMultipleOfThreadExecutionWidth = [r[@"multiple"] boolValue];
        for (unsigned i = 0; i < 31; ++i) d.buffers[i].mutability = [r[@"buffers"][i] unsignedIntegerValue];
        d.binaryArchives = @[archive]; madeira_pipeline_allow_warmup(d, YES); return d;
    }
    if (![r[@"kind"] isEqual:@"render"] || !numbers(r[@"state"], 10, 1024) ||
        !numbers(r[@"vb"], 31, MTLMutabilityImmutable) || !numbers(r[@"fb"], 31, MTLMutabilityImmutable) ||
        ![r[@"colors"] isKindOfClass:NSArray.class] || [r[@"colors"] count] != 8) return nil;
    NSArray *s = r[@"state"];
    if ([s[2] unsignedIntegerValue] > 1 || [s[3] unsignedIntegerValue] > 1 ||
        ![@[@1, @2, @4, @8] containsObject:s[4]] || [s[5] unsignedIntegerValue] > 3 ||
        [s[6] unsignedIntegerValue] > 3 || [s[7] unsignedIntegerValue] > 4 || [s[8] unsignedIntegerValue] > 1 || [s[9] unsignedIntegerValue] > 64) return nil;
    MTLRenderPipelineDescriptor *d = [MTLRenderPipelineDescriptor new];
    d.vertexFunction = [self function:r[@"vertex"] libraries:loaded]; if (!d.vertexFunction || d.vertexFunction.patchType != MTLPatchTypeNone) return nil;
    if (r[@"fragment"] != NSNull.null) { d.fragmentFunction = [self function:r[@"fragment"] libraries:loaded]; if (!d.fragmentFunction) return nil; }
    for (unsigned i = 0; i < 8; ++i) {
        NSArray *v = r[@"colors"][i]; if (!numbers(v, 9, 1024) || [v[1] unsignedIntegerValue] > 1 ||
            [v[2] unsignedIntegerValue] > 15 || [v[3] unsignedIntegerValue] > 4 || [v[4] unsignedIntegerValue] > 4) return nil;
        for (unsigned j = 5; j < 9; ++j) if ([v[j] unsignedIntegerValue] > 18) return nil;
        MTLRenderPipelineColorAttachmentDescriptor *c = d.colorAttachments[i];
        c.pixelFormat = [v[0] unsignedIntegerValue]; c.blendingEnabled = [v[1] boolValue]; c.writeMask = [v[2] unsignedIntegerValue];
        c.alphaBlendOperation = [v[3] unsignedIntegerValue]; c.rgbBlendOperation = [v[4] unsignedIntegerValue];
        c.sourceRGBBlendFactor = [v[5] unsignedIntegerValue]; c.sourceAlphaBlendFactor = [v[6] unsignedIntegerValue];
        c.destinationRGBBlendFactor = [v[7] unsignedIntegerValue]; c.destinationAlphaBlendFactor = [v[8] unsignedIntegerValue];
    }
    for (unsigned i = 0; i < 31; ++i) { d.vertexBuffers[i].mutability = [r[@"vb"][i] unsignedIntegerValue]; d.fragmentBuffers[i].mutability = [r[@"fb"][i] unsignedIntegerValue]; }
    d.depthAttachmentPixelFormat = [s[0] unsignedIntegerValue]; d.stencilAttachmentPixelFormat = [s[1] unsignedIntegerValue];
    d.alphaToCoverageEnabled = [s[2] boolValue]; d.rasterizationEnabled = [s[3] boolValue]; d.rasterSampleCount = [s[4] unsignedIntegerValue];
    d.inputPrimitiveTopology = [s[5] unsignedIntegerValue]; d.tessellationPartitionMode = [s[6] unsignedIntegerValue];
    d.tessellationFactorStepFunction = [s[7] unsignedIntegerValue]; d.tessellationOutputWindingOrder = [s[8] unsignedIntegerValue];
    d.maxTessellationFactor = [s[9] unsignedIntegerValue]; d.binaryArchives = @[archive];
    madeira_pipeline_allow_warmup(d, YES); return d;
}
- (void)replay {
    @autoreleasepool {
        NSURL *manifest = [folder URLByAppendingPathComponent:@"recipes.json"];
        NSDictionary *attrs = [NSFileManager.defaultManager attributesOfItemAtPath:manifest.path error:nil];
        NSData *bytes = attrs && [attrs[NSFileSize] unsignedLongLongValue] <= manifestLimit ? [NSData dataWithContentsOfURL:manifest] : nil;
        id saved = bytes ? [NSJSONSerialization JSONObjectWithData:bytes options:0 error:nil] : nil;
        if (![saved isKindOfClass:NSDictionary.class] || ![saved[@"schema"] isEqual:@1] ||
            ![saved[@"recipes"] isKindOfClass:NSArray.class] || [saved[@"recipes"] count] > recipeLimit) saved = nil;
        NSMutableDictionary *loaded = [NSMutableDictionary dictionary];
        // Count old files so subsequent sessions cannot grow the folder past
        // the limit. Drop unused libraries on this background queue only.
        NSMutableSet *referenced = [NSMutableSet set];
        for (id r in saved[@"recipes"]) if ([r isKindOfClass:NSDictionary.class])
            for (NSString *stage in @[@"vertex", @"fragment", @"function"]) {
                id fn = r[stage]; if ([fn isKindOfClass:NSDictionary.class] && hashName(fn[@"library"])) [referenced addObject:fn[@"library"]];
            }
        for (NSURL *file in [NSFileManager.defaultManager contentsOfDirectoryAtURL:folder includingPropertiesForKeys:nil options:0 error:nil]) {
            NSString *hash = file.lastPathComponent.stringByDeletingPathExtension;
            if (![file.pathExtension isEqual:@"metallib"] || !hashName(hash)) continue;
            NSDictionary *a = [NSFileManager.defaultManager attributesOfItemAtPath:file.path error:nil];
            NSUInteger size = [a[NSFileSize] unsignedIntegerValue];
            [lock lock]; BOOL keep = [referenced containsObject:hash] && size <= libraryLimit &&
                ([libraries containsObject:hash] || (libraries.count < 256 && libraryBytes + size <= diskLimit));
            if (keep && ![libraries containsObject:hash]) { [libraries addObject:hash]; librarySizes[hash] = @(size); libraryBytes += size; }
            [lock unlock];
            if (!keep) [NSFileManager.defaultManager removeItemAtURL:file error:nil];
        }
        CFAbsoluteTime start = CFAbsoluteTimeGetCurrent(); unsigned warmed = 0, missed = 0;
        for (id r in saved[@"recipes"]) {
            if (![r isKindOfClass:NSDictionary.class]) continue;
            NSData *keyBytes = json(r); if (!keyBytes || keyBytes.length > 16384) continue;
            NSString *key = digest(keyBytes); records[key] = r; [order addObject:key];
            if (!archive || warmed + missed >= warmLimit || CFAbsoluteTimeGetCurrent() - start > 1.5 ||
                NSProcessInfo.processInfo.thermalState >= NSProcessInfoThermalStateSerious) continue;
            @try {
                id d = [self descriptor:r libraries:loaded];
                BOOL compute = [r[@"kind"] isEqual:@"compute"];
                // Reconstruction must cover every recorded field byte-for-byte.
                if (!d || ![json(recipe(d, compute)) isEqual:keyBytes]) { ++missed; continue; }
                id state = compute ? [device newComputePipelineStateWithDescriptor:d options:MTLPipelineOptionFailOnBinaryArchiveMiss reflection:nil error:nil]
                                   : [device newRenderPipelineStateWithDescriptor:d options:MTLPipelineOptionFailOnBinaryArchiveMiss reflection:nil error:nil];
                if (state) { [lock lock]; ready[key] = state; [lock unlock]; ++warmed; } else ++missed;
            } @catch (NSException *exception) { ++missed; }
        }
        NSLog(@"[shader-warmup] learned-pipeline-replay-v1 recipes=%lu ready=%u skipped=%u archive=%d elapsed-ms=%.2f", (unsigned long)records.count,
              warmed, missed, archive != nil, (CFAbsoluteTimeGetCurrent() - start) * 1000);
    }
}
@end
