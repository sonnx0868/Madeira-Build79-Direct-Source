/* SPDX-License-Identifier: MIT */
#pragma once
#import <Foundation/Foundation.h>
#import <Metal/Metal.h>

// Only the ordinary, fully described DXMT render/compute thunks opt in.
// Mesh/tessellation/private logic-operation paths keep their existing behavior.
void madeira_pipeline_allow_warmup(id descriptor, BOOL allowed);
NSDictionary *madeira_warmup_constant(const void *value, MTLDataType type, NSUInteger index);
void madeira_warmup_track_library(id<MTLLibrary> library, dispatch_data_t data);
void madeira_warmup_track_function(id<MTLLibrary> library, id<MTLFunction> function,
                                  NSString *name, NSArray *constants, BOOL specialized);

@interface MadeiraPipelineWarmup : NSObject
- (instancetype)initWithDevice:(id<MTLDevice>)device folder:(NSURL *)folder
                       archive:(id<MTLBinaryArchive>)archive;
- (id)take:(id)descriptor compute:(BOOL)compute;
- (void)record:(id)descriptor compute:(BOOL)compute;
- (void)storeLibrary:(NSData *)bytes hash:(NSString *)hash;
@end

// Called on device enumeration, before the first game's shader/draw requests.
void madeira_prepare_pipeline_cache(id<MTLDevice> device);
MadeiraPipelineWarmup *madeira_pipeline_warmup_for_device(id<MTLDevice> device);
