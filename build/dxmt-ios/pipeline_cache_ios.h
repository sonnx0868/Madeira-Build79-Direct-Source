/* SPDX-License-Identifier: MIT */
#pragma once
#import <Metal/Metal.h>
#import "pipeline_warmup_ios.h"
void madeira_flush_pipeline_cache(void);

id<MTLRenderPipelineState> madeira_new_render_pipeline(id<MTLDevice> device,
    MTLRenderPipelineDescriptor *descriptor, MTLPipelineOption options,
    MTLRenderPipelineReflection **reflection, NSError **error, BOOL cacheAllowed) NS_RETURNS_RETAINED;
id<MTLComputePipelineState> madeira_new_compute_pipeline(id<MTLDevice> device,
    MTLComputePipelineDescriptor *descriptor, MTLPipelineOption options,
    MTLComputePipelineReflection **reflection, NSError **error, BOOL cacheAllowed) NS_RETURNS_RETAINED;
