// Controlled replay of a saved compute shader. This is not a game FPS test.
#import <Foundation/Foundation.h>
#import <Metal/Metal.h>
#include <math.h>
#include <string.h>

static id<MTLBuffer> words(id<MTLDevice> device, NSArray *values) {
  id<MTLBuffer> result=[device newBufferWithLength:MAX(16,values.count*4) options:MTLResourceStorageModeShared];
  uint32_t *p=result.contents;for(NSUInteger i=0;i<values.count;i++)p[i]=[values[i] unsignedIntValue];
  return result;
}
static void check(id<MTLCommandBuffer> command) {
  [command commit];[command waitUntilCompleted];
  if(command.status!=MTLCommandBufferStatusCompleted) {
    fprintf(stderr,"GPU replay failed: %s\n",command.error.description.UTF8String);exit(10);
  }
}
int main(int argc,char **argv) { @autoreleasepool {
  if(argc!=4)return 2;
  NSError *error=nil;
  NSDictionary *profile=[NSJSONSerialization JSONObjectWithData:[NSData dataWithContentsOfFile:@(argv[2])] options:0 error:&error];
  if(!profile)return 3;
  id<MTLDevice> device=MTLCreateSystemDefaultDevice();if(!device)return 4;
  id<MTLCommandQueue> queue=[device newCommandQueue];
  NSMutableArray<id<MTLResource>> *resources=[NSMutableArray array];
  NSMutableArray<id<MTLBuffer>> *cb=[NSMutableArray array];
  id<MTLBuffer> table29=[device newBufferWithLength:64 options:MTLResourceStorageModeShared];
  id<MTLBuffer> table30=[device newBufferWithLength:408 options:MTLResourceStorageModeShared];
  uint64_t *bindings=table30.contents,*constants=table29.contents;
  for(unsigned i=0;i<8;i++) {
    id<MTLBuffer> buffer=words(device,profile[@"constant_buffers"][[NSString stringWithFormat:@"%u",i]]);
    if(!buffer)return 5;
    [cb addObject:buffer];[resources addObject:buffer];constants[i]=buffer.gpuAddress;
  }
  NSArray *original=profile[@"arguments"];
  for(unsigned i=0;i<51;i++)bindings[i]=[original[i] unsignedLongLongValue];
  NSMutableArray<id<MTLSamplerState>> *samplers=[NSMutableArray array];
  for(unsigned i=0;i<5;i++) {
    MTLSamplerDescriptor *desc=[MTLSamplerDescriptor new];desc.supportArgumentBuffers=YES;
    desc.minFilter=desc.magFilter=MTLSamplerMinMagFilterLinear;
    desc.sAddressMode=desc.tAddressMode=desc.rAddressMode=MTLSamplerAddressModeClampToEdge;
    desc.compareFunction=i>=3?MTLCompareFunctionLessEqual:MTLCompareFunctionNever;
    id<MTLSamplerState> sampler=[device newSamplerStateWithDescriptor:desc];if(!sampler)return 6;
    [samplers addObject:sampler];bindings[i*3]=bindings[i*3+1]=sampler.gpuResourceID._impl;
  }
  // Unrelated raw resources are zeroed controlled inputs. The density-volume
  // buffer and all eight constant buffers come from the saved dispatch.
  for(NSNumber *slot in @[@0,@1,@4,@5,@6,@7,@8,@10,@11,@12,@13]) {
    unsigned i=slot.unsignedIntValue;NSUInteger length=MAX(16,bindings[16+2*i]);
    if(length>8*1024*1024)return 7;
    id<MTLBuffer> buffer=i==10?words(device,profile[@"density_words"]):[device newBufferWithLength:length options:MTLResourceStorageModeShared];
    if(!buffer)return 8;
    if(i!=10)memset(buffer.contents,0,buffer.length);
    bindings[15+2*i]=buffer.gpuAddress;[resources addObject:buffer];
  }
  NSMutableArray<id<MTLTexture>> *depths=[NSMutableArray array];
  for(NSNumber *slot in @[@2,@3,@9,@14,@15,@16]) {
    unsigned i=slot.unsignedIntValue;
    MTLTextureDescriptor *desc=[MTLTextureDescriptor new];
    desc.textureType=i>=14?MTLTextureType3D:MTLTextureType2DArray;
    desc.pixelFormat=(i==2 || i==3)?MTLPixelFormatDepth32Float:MTLPixelFormatRGBA16Float;
    desc.width=desc.height=64;desc.depth=i>=14?16:1;
    desc.arrayLength=i==3?10:1;desc.storageMode=MTLStorageModePrivate;
    desc.usage=MTLTextureUsageShaderRead|((i==2 || i==3)?MTLTextureUsageRenderTarget:0);
    id<MTLTexture> texture=[device newTextureWithDescriptor:desc];if(!texture)return 9;
    bindings[15+2*i]=texture.gpuResourceID._impl;
    [resources addObject:texture];
    if(i==2 || i==3) {
      [depths addObject:texture];
      for(unsigned slice=0;slice<desc.arrayLength;slice++) {
        MTLRenderPassDescriptor *pass=[MTLRenderPassDescriptor renderPassDescriptor];
        pass.depthAttachment.texture=texture;pass.depthAttachment.slice=slice;
        pass.depthAttachment.loadAction=MTLLoadActionClear;pass.depthAttachment.storeAction=MTLStoreActionStore;
        pass.depthAttachment.clearDepth=0.5;
        id<MTLCommandBuffer> command=[queue commandBuffer];
        [[command renderCommandEncoderWithDescriptor:pass] endEncoding];check(command);
      }
    } else {
      NSUInteger row=64*8,image=row*64,length=image*desc.depth;
      id<MTLBuffer> upload=[device newBufferWithLength:length options:MTLResourceStorageModeShared];
      _Float16 *p=upload.contents;for(NSUInteger j=0;j<length/2;j++)p[j]=(_Float16)(0.25+0.5*(j%23)/22.0);
      id<MTLCommandBuffer> command=[queue commandBuffer];id<MTLBlitCommandEncoder> blit=[command blitCommandEncoder];
      [blit copyFromBuffer:upload sourceOffset:0 sourceBytesPerRow:row sourceBytesPerImage:image sourceSize:MTLSizeMake(64,64,desc.depth)
                toTexture:texture destinationSlice:0 destinationLevel:0 destinationOrigin:MTLOriginMake(0,0,0)];
      [blit endEncoding];check(command);
    }
  }
  NSUInteger width=[profile[@"grid"][0] unsignedIntegerValue],height=[profile[@"grid"][1] unsignedIntegerValue],depth=[profile[@"grid"][2] unsignedIntegerValue];
  MTLTextureDescriptor *desc=[MTLTextureDescriptor new];desc.textureType=MTLTextureType3D;
  desc.width=width;desc.height=height;desc.depth=depth;desc.pixelFormat=MTLPixelFormatRGBA16Float;
  desc.storageMode=MTLStorageModePrivate;desc.usage=MTLTextureUsageShaderWrite;
  id<MTLTexture> output=[device newTextureWithDescriptor:desc];if(!output)return 11;
  bindings[49]=output.gpuResourceID._impl;
  NSUInteger row=(width*8+255)&~255UL,image=row*height;
  id<MTLBuffer> readback=[device newBufferWithLength:image*depth options:MTLResourceStorageModeShared];
  NSMutableDictionary *pipelines=[NSMutableDictionary dictionary];
  for(NSString *mode in @[@"baseline",@"guarded"]) {
    NSString *path=[@(argv[1]) stringByAppendingPathComponent:[mode stringByAppendingPathComponent:@"424068078B764242.metallib"]];
    id<MTLLibrary> library=[device newLibraryWithURL:[NSURL fileURLWithPath:path] error:&error];
    if(!library){fprintf(stderr,"Library: %s\n",error.description.UTF8String);return 12;}
    MTLComputePipelineDescriptor *pipeline=[MTLComputePipelineDescriptor new];pipeline.computeFunction=[library newFunctionWithName:profile[@"function"]];
    pipeline.threadGroupSizeIsMultipleOfThreadExecutionWidth=YES;
    pipeline.buffers[29].mutability=pipeline.buffers[30].mutability=MTLMutabilityImmutable;
    id<MTLComputePipelineState> state=[device newComputePipelineStateWithDescriptor:pipeline options:MTLPipelineOptionNone reflection:nil error:&error];
    if(!state){fprintf(stderr,"Pipeline: %s\n",error.description.UTF8String);return 13;}
    pipelines[mode]=state;
  }
  NSMutableArray *results=[NSMutableArray array];
  NSData *unlitReference=nil;
  for(unsigned fixture=0;fixture<2;fixture++) {
    // The second fixture executes the previously guarded shadow branch too.
    uint32_t *light=cb[5].contents;light[0]=fixture;
    if(fixture) {
      light[8]|=2;light[11]=0x3f800000; // nonzero light intensity
      // Controlled shadow matrices place every voxel in a valid shadow texel.
      float *shadow=cb[3].contents;
      for(unsigned matrix=0;matrix<4;matrix++) {
        for(unsigned j=0;j<16;j++)shadow[matrix*16+j]=0;
        shadow[matrix*16+12]=shadow[matrix*16+13]=0.5;
        shadow[matrix*16+14]=0.25;shadow[matrix*16+15]=1;
      }
    }
    NSMutableDictionary *reference=[NSMutableDictionary dictionary];
    NSMutableDictionary *timings=[NSMutableDictionary dictionary];
    for(unsigned round=0;round<8;round++) {
      NSArray *order=round%2?@[@"guarded",@"baseline"]:@[@"baseline",@"guarded"];
      for(NSString *mode in order) { @autoreleasepool {
        id<MTLCommandBuffer> command=[queue commandBuffer];id<MTLComputeCommandEncoder> encoder=[command computeCommandEncoder];
        [encoder setComputePipelineState:pipelines[mode]];
        [encoder setBuffer:table29 offset:0 atIndex:29];[encoder setBuffer:table30 offset:0 atIndex:30];
        for(id<MTLResource> resource in resources)[encoder useResource:resource usage:MTLResourceUsageRead];
        [encoder useResource:output usage:MTLResourceUsageWrite];
        [encoder dispatchThreadgroups:MTLSizeMake(width/8,height/8,depth) threadsPerThreadgroup:MTLSizeMake(8,8,1)];
        [encoder endEncoding];check(command);
        if(!timings[mode])timings[mode]=[NSMutableArray array];
        if(round>=2)[timings[mode] addObject:@((command.GPUEndTime-command.GPUStartTime)*1000)];
        id<MTLCommandBuffer> copy=[queue commandBuffer];id<MTLBlitCommandEncoder> blit=[copy blitCommandEncoder];
        [blit copyFromTexture:output sourceSlice:0 sourceLevel:0 sourceOrigin:MTLOriginMake(0,0,0) sourceSize:MTLSizeMake(width,height,depth)
                    toBuffer:readback destinationOffset:0 destinationBytesPerRow:row destinationBytesPerImage:image];
        [blit endEncoding];check(copy);
        if(round==7)reference[mode]=[NSData dataWithBytes:readback.contents length:readback.length];
      }}
    }
    const uint16_t *a=[reference[@"baseline"] bytes],*b=[reference[@"guarded"] bytes];
    uint64_t nonzero=0,nan=0,different=0;unsigned maxULP=0;double maxAbs=0;
    for(NSUInteger z=0;z<depth;z++)for(NSUInteger y=0;y<height;y++)for(NSUInteger x=0;x<width*4;x++) {
      NSUInteger index=(z*image+y*row)/2+x;uint16_t av=a[index],bv=b[index];
      _Float16 af,bf;memcpy(&af,&av,2);memcpy(&bf,&bv,2);
      if(!isfinite((float)af) || !isfinite((float)bf)){nan++;continue;}
      nonzero+=af!=0;different+=av!=bv;
      maxAbs=MAX(maxAbs,fabs((float)af-(float)bf));
      unsigned ao=av&0x8000?0x8000-(av&0x7fff):0x8000+av,bo=bv&0x8000?0x8000-(bv&0x7fff):0x8000+bv;
      maxULP=MAX(maxULP,ao>bo?ao-bo:bo-ao);
    }
    BOOL lightingChangesOutput=fixture && ![reference[@"baseline"] isEqualToData:unlitReference];
    if(!fixture)unlitReference=reference[@"baseline"];
    NSDictionary *result=@{@"directional_lights":@(fixture),@"lighting_changes_output":@(lightingChangesOutput),@"nonzero_components":@(nonzero),@"nonfinite_components":@(nan),
                          @"different_components":@(different),@"max_ulp":@(maxULP),@"max_abs_error":@(maxAbs),@"gpu_ms":timings};
    [results addObject:result];printf("Replay: %s\n",[[NSString alloc] initWithData:[NSJSONSerialization dataWithJSONObject:result options:0 error:nil] encoding:NSUTF8StringEncoding].UTF8String);
    if(nan || !nonzero || maxULP>2 || (fixture && !lightingChangesOutput))return 14;
  }
  NSDictionary *report=@{@"device":device.name,@"scope":@"Controlled shader replay with captured constants/density data and synthetic textures. Not game FPS or iPad thermal performance.",@"fixtures":results};
  NSData *json=[NSJSONSerialization dataWithJSONObject:report options:NSJSONWritingPrettyPrinted error:&error];
  return [json writeToFile:@(argv[3]) atomically:YES]?0:15;
}}
