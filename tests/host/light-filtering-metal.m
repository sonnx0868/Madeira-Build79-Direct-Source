// Controlled 1080p filtering comparison, not an iPad game FPS measurement.
#import <Foundation/Foundation.h>
#import <Metal/Metal.h>
#import <CoreGraphics/CoreGraphics.h>
#import <ImageIO/ImageIO.h>
#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include "light_graphics.h"

static void finish(id<MTLCommandBuffer> buffer) {
    [buffer commit]; [buffer waitUntilCompleted];
    assert(buffer.status == MTLCommandBufferStatusCompleted);
}
static void png(NSData *bytes, NSURL *url) {
    CGDataProviderRef provider = CGDataProviderCreateWithCFData((__bridge CFDataRef)bytes);
    CGColorSpaceRef space = CGColorSpaceCreateDeviceRGB();
    CGImageRef image = CGImageCreate(1920, 1080, 8, 32, 1920 * 4, space,
        kCGBitmapByteOrder32Little | kCGImageAlphaPremultipliedFirst, provider, NULL, false, kCGRenderingIntentDefault);
    CGImageDestinationRef destination = CGImageDestinationCreateWithURL((__bridge CFURLRef)url, CFSTR("public.png"), 1, NULL);
    assert(destination && image);CGImageDestinationAddImage(destination, image, NULL);assert(CGImageDestinationFinalize(destination));
    CFRelease(destination);CGImageRelease(image);CGColorSpaceRelease(space);CGDataProviderRelease(provider);
}
int main(int argc, char **argv) { @autoreleasepool {
    assert(argc == 2);NSURL *folder = [NSURL fileURLWithPath:@(argv[1]) isDirectory:YES];
    id<MTLDevice> device = MTLCreateSystemDefaultDevice();assert(device);
    id<MTLCommandQueue> queue = [device newCommandQueue];assert(queue);
    NSError *error = nil;
    NSString *source = @"#include <metal_stdlib>\nusing namespace metal;\n"
        "struct V{float4 p[[position]];float2 uv;};\n"
        "vertex V v(uint i[[vertex_id]]){float2 p[3]={float2(-1,-1),float2(3,-1),float2(-1,3)};V o;o.p=float4(p[i],0,1);o.uv=p[i]*0.5+0.5;return o;}\n"
        "fragment float4 f(V i[[stage_in]],texture2d<float> t[[texture(0)]],sampler s[[sampler(0)]]){return t.sample(s,float2(i.uv.x*24.0,i.uv.y));}\n";
    id<MTLLibrary> library = [device newLibraryWithSource:source options:nil error:&error];assert(library && !error);
    MTLRenderPipelineDescriptor *pipeline = [MTLRenderPipelineDescriptor new];
    pipeline.vertexFunction = [library newFunctionWithName:@"v"];pipeline.fragmentFunction = [library newFunctionWithName:@"f"];
    pipeline.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm;
    id<MTLRenderPipelineState> state = [device newRenderPipelineStateWithDescriptor:pipeline error:&error];assert(state && !error);
    MTLTextureDescriptor *inputDesc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatRGBA8Unorm width:1024 height:1024 mipmapped:YES];
    inputDesc.storageMode = MTLStorageModeShared;inputDesc.usage = MTLTextureUsageShaderRead;
    id<MTLTexture> input = [device newTextureWithDescriptor:inputDesc];assert(input);
    NSMutableData *pixels = [NSMutableData dataWithLength:1024*1024*4];uint8_t *p = pixels.mutableBytes;
    for (unsigned y=0;y<1024;++y)for(unsigned x=0;x<1024;++x){
        unsigned n=(y*1024+x)*4;uint8_t grid=((x/8+y/8)%2)?160:80;
        p[n]=grid;p[n+1]=(uint8_t)(64+x/8);p[n+2]=(uint8_t)(64+y/8);p[n+3]=255;
    }
    [input replaceRegion:MTLRegionMake2D(0,0,1024,1024) mipmapLevel:0 withBytes:p bytesPerRow:4096];
    id<MTLCommandBuffer> upload=[queue commandBuffer];id<MTLBlitCommandEncoder> blit=[upload blitCommandEncoder];
    [blit generateMipmapsForTexture:input];[blit endEncoding];finish(upload);
    NSMutableArray<NSData *> *images=[NSMutableArray array];
    for(unsigned mode=0;mode<2;++mode){
        MTLSamplerDescriptor *descriptor=[MTLSamplerDescriptor new];
        descriptor.minFilter=descriptor.magFilter=MTLSamplerMinMagFilterLinear;descriptor.mipFilter=MTLSamplerMipFilterLinear;
        descriptor.sAddressMode=descriptor.tAddressMode=MTLSamplerAddressModeRepeat;
        descriptor.maxAnisotropy=madeira_light_anisotropy(16,mode,0);
        id<MTLSamplerState> sampler=[device newSamplerStateWithDescriptor:descriptor];assert(sampler);
        MTLTextureDescriptor *target=[MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatBGRA8Unorm width:1920 height:1080 mipmapped:NO];
        target.storageMode=MTLStorageModeShared;target.usage=MTLTextureUsageRenderTarget;
        id<MTLTexture> output=[device newTextureWithDescriptor:target];assert(output);
        MTLRenderPassDescriptor *pass=[MTLRenderPassDescriptor renderPassDescriptor];
        pass.colorAttachments[0].texture=output;pass.colorAttachments[0].loadAction=MTLLoadActionClear;pass.colorAttachments[0].storeAction=MTLStoreActionStore;
        id<MTLCommandBuffer> command=[queue commandBuffer];id<MTLRenderCommandEncoder> encoder=[command renderCommandEncoderWithDescriptor:pass];
        [encoder setRenderPipelineState:state];[encoder setFragmentTexture:input atIndex:0];[encoder setFragmentSamplerState:sampler atIndex:0];
        [encoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:3];[encoder endEncoding];finish(command);
        NSMutableData *image=[NSMutableData dataWithLength:1920*1080*4];
        [output getBytes:image.mutableBytes bytesPerRow:1920*4 fromRegion:MTLRegionMake2D(0,0,1920,1080) mipmapLevel:0];
        [images addObject:image];png(image,[folder URLByAppendingPathComponent:mode?@"filtering-8x.png":@"filtering-16x.png"]);
    }
    const uint8_t *a=images[0].bytes,*b=images[1].bytes;double sum=0,square=0;unsigned max=0,changed=0;
    for(unsigned i=0;i<1920*1080*4;++i){unsigned difference=abs((int)a[i]-(int)b[i]);sum+=difference;square+=difference*difference;if(difference>max)max=difference;changed+=difference!=0;}
    double mean=sum/(1920.0*1080*4*255),rms=sqrt(square/(1920.0*1080*4))/255;
    assert(mean<0.03); // Reject a visibly large difference in this controlled scene.
    NSDictionary *report=@{@"output":@"1920x1080",@"textureDimensionsUnchanged":@YES,@"textureMipLevelsUnchanged":@YES,
        @"ordinaryAnisotropy":@[@16,@8],@"meanAbsoluteNormalizedDifference":@(mean),@"rmsNormalizedDifference":@(rms),
        @"maximumChannelDifference":@(max),@"changedChannels":@(changed),@"device":device.name,
        @"scope":@"Controlled textured-plane image comparison, not game appearance, FPS or iPad thermal performance."};
    NSData *json=[NSJSONSerialization dataWithJSONObject:report options:NSJSONWritingPrettyPrinted error:&error];assert(json && !error);
    assert([json writeToURL:[folder URLByAppendingPathComponent:@"light-filtering-visual-validation.json"] atomically:YES]);
    NSLog(@"[light-filtering-visual] mean-absolute=%.6f rms=%.6f max-channel=%u",mean,rms,max);
}return 0;}
