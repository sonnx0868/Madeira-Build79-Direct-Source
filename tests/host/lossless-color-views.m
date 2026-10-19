#import <Foundation/Foundation.h>
#import <Metal/Metal.h>
#include <assert.h>
#include <math.h>
#include <string.h>

static NSData *render(id<MTLDevice> device, BOOL pixelFormatView) {
    MTLTextureDescriptor *d = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatBGRA8Unorm
        width:16 height:16 mipmapped:NO];
    d.storageMode = MTLStorageModePrivate;
    d.usage = MTLTextureUsageShaderRead | MTLTextureUsageRenderTarget |
        (pixelFormatView ? MTLTextureUsagePixelFormatView : 0);
    id<MTLTexture> parent = [device newTextureWithDescriptor:d]; assert(parent);
    id<MTLTexture> view = [parent newTextureViewWithPixelFormat:MTLPixelFormatBGRA8Unorm_sRGB]; assert(view);
    NSError *error = nil;
    NSString *source = @"#include <metal_stdlib>\nusing namespace metal;\n"
        "vertex float4 v(uint i [[vertex_id]]) { float2 p[3]={float2(-1,-1),float2(3,-1),float2(-1,3)};return float4(p[i],0,1); }\n"
        "fragment float4 f() { return float4(0.25,0.5,0.75,1); }\n";
    id<MTLLibrary> library = [device newLibraryWithSource:source options:nil error:&error]; assert(library);
    MTLRenderPipelineDescriptor *pipeline = [MTLRenderPipelineDescriptor new];
    pipeline.vertexFunction = [library newFunctionWithName:@"v"];
    pipeline.fragmentFunction = [library newFunctionWithName:@"f"];
    pipeline.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm_sRGB;
    id<MTLRenderPipelineState> state = [device newRenderPipelineStateWithDescriptor:pipeline error:&error]; assert(state);
    id<MTLCommandQueue> queue = [device newCommandQueue]; assert(queue);
    id<MTLCommandBuffer> command = [queue commandBuffer];
    MTLRenderPassDescriptor *pass = [MTLRenderPassDescriptor renderPassDescriptor];
    pass.colorAttachments[0].texture = view;
    pass.colorAttachments[0].loadAction = MTLLoadActionClear;
    pass.colorAttachments[0].storeAction = MTLStoreActionStore;
    id<MTLRenderCommandEncoder> encoder = [command renderCommandEncoderWithDescriptor:pass];
    [encoder setViewport:(MTLViewport){0,0,16,16,0,1}];
    [encoder setRenderPipelineState:state];
    [encoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:3];
    [encoder endEncoding];
    id<MTLBuffer> readback = [device newBufferWithLength:256*16 options:MTLResourceStorageModeShared]; assert(readback);
    id<MTLBlitCommandEncoder> blit = [command blitCommandEncoder];
    [blit copyFromTexture:parent sourceSlice:0 sourceLevel:0 sourceOrigin:MTLOriginMake(0,0,0)
        sourceSize:MTLSizeMake(16,16,1) toBuffer:readback destinationOffset:0 destinationBytesPerRow:256 destinationBytesPerImage:256*16];
    [blit endEncoding]; [command commit]; [command waitUntilCompleted];
    assert(command.status == MTLCommandBufferStatusCompleted);
    NSMutableData *pixels = [NSMutableData dataWithLength:16*16*4];
    for (unsigned y=0;y<16;y++) memcpy((char *)pixels.mutableBytes+y*64,(char *)readback.contents+y*256,64);
    const uint8_t *bytes = pixels.bytes;
    const int expected[4] = {225,188,137,255}; // raw BGRA bytes after sRGB encoding
    for (unsigned i=0;i<16*16*4;i++) assert(abs((int)bytes[i]-expected[i%4]) <= 1);
    return pixels;
}

int main(int argc, char **argv) {
    @autoreleasepool {
        id<MTLDevice> device = MTLCreateSystemDefaultDevice(); assert(device);
        NSArray *families = @[
            @[@(MTLPixelFormatRGBA8Unorm),@(MTLPixelFormatRGBA8Unorm_sRGB),@(MTLPixelFormatRGBA8Snorm),@(MTLPixelFormatRGBA8Uint),@(MTLPixelFormatRGBA8Sint)],
            @[@(MTLPixelFormatRGBA16Float),@(MTLPixelFormatRGBA16Unorm),@(MTLPixelFormatRGBA16Snorm),@(MTLPixelFormatRGBA16Uint),@(MTLPixelFormatRGBA16Sint)],
            @[@(MTLPixelFormatRGBA32Float),@(MTLPixelFormatRGBA32Uint),@(MTLPixelFormatRGBA32Sint)],
            @[@(MTLPixelFormatBGRA8Unorm),@(MTLPixelFormatBGRA8Unorm_sRGB)],
            @[@(MTLPixelFormatRG32Uint),@(MTLPixelFormatRG32Float),@(MTLPixelFormatRG32Sint)],
            @[@(MTLPixelFormatRG16Float),@(MTLPixelFormatRG16Unorm),@(MTLPixelFormatRG16Snorm),@(MTLPixelFormatRG16Uint),@(MTLPixelFormatRG16Sint)],
            @[@(MTLPixelFormatRG8Unorm),@(MTLPixelFormatRG8Snorm),@(MTLPixelFormatRG8Uint),@(MTLPixelFormatRG8Sint)],
            @[@(MTLPixelFormatR32Uint),@(MTLPixelFormatR32Float),@(MTLPixelFormatR32Sint)],
            @[@(MTLPixelFormatR16Unorm),@(MTLPixelFormatR16Float),@(MTLPixelFormatR16Snorm),@(MTLPixelFormatR16Uint),@(MTLPixelFormatR16Sint)],
            @[@(MTLPixelFormatR8Unorm),@(MTLPixelFormatR8Snorm),@(MTLPixelFormatR8Uint),@(MTLPixelFormatR8Sint)],
            @[@(MTLPixelFormatRGB10A2Unorm),@(MTLPixelFormatRGB10A2Uint)]
        ];
        NSUInteger views = 0; NSMutableArray *unavailable = [NSMutableArray new];
        for (NSArray *family in families) {
            for (NSNumber *base in family) {
                for (unsigned restore=0;restore<2;restore++) {
                    MTLTextureDescriptor *d = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:base.unsignedIntegerValue
                        width:16 height:16 mipmapped:NO];
                    d.storageMode = MTLStorageModePrivate;
                    BOOL srgbOnly = base.unsignedIntegerValue == MTLPixelFormatBGRA8Unorm ||
                                    base.unsignedIntegerValue == MTLPixelFormatBGRA8Unorm_sRGB;
                    // Exercise the narrowed production policy, not the unsafe
                    // broad Playport policy rejected by CI197/198. Numeric
                    // families retain PFV for UNORM/SNORM/UINT reinterpretation.
                    d.usage = MTLTextureUsageShaderRead | ((restore || !srgbOnly) ? MTLTextureUsagePixelFormatView : 0);
                    id<MTLTexture> parent = [device newTextureWithDescriptor:d];
                    if (!parent) { [unavailable addObject:base];continue; }
                    for (NSNumber *to in family) {
                        id<MTLTexture> view = [parent newTextureViewWithPixelFormat:to.unsignedIntegerValue];
                        assert(view && view.pixelFormat == to.unsignedIntegerValue); views++;
                    }
                }
            }
        }
        NSData *old = render(device,YES), *eligible = render(device,NO);
        assert([old isEqualToData:eligible]);
        NSDictionary *result = @{@"device":device.name,@"viewsCreated":@(views),@"families":@(families.count),
            @"supportsApple5":@([device supportsFamily:MTLGPUFamilyApple5]),
            @"optimizedFamilies":@[@"BGRA8 linear/sRGB (BGRX via swizzle)"],
            @"unsupportedBaseFormats":unavailable,@"renderedPixelsEqual":@YES,@"renderedPixels":@256,
            @"validationLayerRequested":@YES,@"physicalGameFpsMeasured":@NO,@"compressionRatioMeasured":@NO};
        assert(views >= 200);
        if (argc>1) {
            NSData *data = [NSJSONSerialization dataWithJSONObject:result options:NSJSONWritingPrettyPrinted error:nil];
            assert([data writeToFile:[NSString stringWithUTF8String:argv[1]] atomically:YES]);
        }
        puts("PASS: actual Metal same-layout views with and without PixelFormatView; sRGB render/readback pixels match");
    }
}
