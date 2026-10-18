// Offline PSO compilation, using actual saved compute shaders. No game is run.
#import <Foundation/Foundation.h>
#import <Metal/Metal.h>
int main(int argc,char **argv) { @autoreleasepool {
  if(argc!=3) return 2;
  id<MTLDevice> device=MTLCreateSystemDefaultDevice();if(!device)return 3;
  printf("Metal device: %s\n",device.name.UTF8String);
  id<MTLCommandQueue> queue=[device newCommandQueue];
  MTLCaptureManager *manager=MTLCaptureManager.sharedCaptureManager;
  if(![manager supportsDestination:MTLCaptureDestinationGPUTraceDocument])return 4;
  for(NSString *mode in @[@"baseline",@"loops",@"o2"]) { @autoreleasepool {
    NSError *error=nil;
    NSString *folder=[@(argv[1]) stringByAppendingPathComponent:mode];
    NSString *trace=[@(argv[2]) stringByAppendingPathComponent:[mode stringByAppendingString:@".gputrace"]];
    MTLCaptureDescriptor *capture=[MTLCaptureDescriptor new];
    capture.captureObject=device;capture.destination=MTLCaptureDestinationGPUTraceDocument;
    capture.outputURL=[NSURL fileURLWithPath:trace];
    if(![manager startCaptureWithDescriptor:capture error:&error]) {
      fprintf(stderr,"Capture unavailable: %s\n",error.description.UTF8String);return 5;
    }
    NSArray *files=[[NSFileManager defaultManager] contentsOfDirectoryAtPath:folder error:&error];
    NSMutableArray *pipelines=[NSMutableArray array];
    for(NSString *file in files) {
      if(![file.pathExtension isEqualToString:@"metallib"])continue;
      id<MTLLibrary> library=[device newLibraryWithURL:[NSURL fileURLWithPath:[folder stringByAppendingPathComponent:file]] error:&error];
      if(!library) {fprintf(stderr,"Library rejected: %s %s\n",file.UTF8String,error.description.UTF8String);return 6;}
      for(NSString *name in library.functionNames) {
        id<MTLFunction> function=[library newFunctionWithName:name];
        if(function.functionType!=MTLFunctionTypeKernel)continue;
        MTLComputePipelineDescriptor *desc=[MTLComputePipelineDescriptor new];
        desc.computeFunction=function;desc.label=[mode stringByAppendingFormat:@"/%@",name];
        id<MTLComputePipelineState> pso=[device newComputePipelineStateWithDescriptor:desc options:MTLPipelineOptionNone reflection:nil error:&error];
        if(!pso){fprintf(stderr,"PSO rejected: %s %s\n",name.UTF8String,error.description.UTF8String);return 7;}
        [pipelines addObject:pso];
      }
    }
    // Referencing each PSO records its compiler metadata. No dispatch is issued:
    // no unbound game resources are accessed and no synthetic FPS is reported.
    id<MTLCommandBuffer> commands=[queue commandBuffer];
    id<MTLComputeCommandEncoder> encoder=[commands computeCommandEncoder];
    for(id<MTLComputePipelineState> pso in pipelines)[encoder setComputePipelineState:pso];
    id<MTLLibrary> markerLibrary=[device newLibraryWithSource:@"#include <metal_stdlib>\nusing namespace metal;\nkernel void lab_marker(uint id [[thread_position_in_grid]]) {}" options:nil error:&error];
    id<MTLComputePipelineState> marker=[device newComputePipelineStateWithFunction:[markerLibrary newFunctionWithName:@"lab_marker"] error:&error];
    if(!marker)return 9;
    [encoder setComputePipelineState:marker];
    [encoder dispatchThreadgroups:MTLSizeMake(1,1,1) threadsPerThreadgroup:MTLSizeMake(1,1,1)];
    [encoder endEncoding];[commands commit];[commands waitUntilCompleted];
    [manager stopCapture];
    if(commands.error)return 8;
    printf("%s: compiled %lu compute pipelines\n",mode.UTF8String,(unsigned long)pipelines.count);
  }}
  return 0;
}}
