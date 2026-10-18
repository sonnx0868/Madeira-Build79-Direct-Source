#import <Foundation/Foundation.h>
#import <Metal/Metal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

static id<MTLComputePipelineState> pipeline(id<MTLDevice> device, NSString *file) {
    NSError *error=nil;
    id<MTLLibrary> library=[device newLibraryWithURL:[NSURL fileURLWithPath:file] error:&error];
    if(!library) { fprintf(stderr,"Metal library: %s\n",error.description.UTF8String);exit(3); }
    id<MTLComputePipelineState> result=[device newComputePipelineStateWithFunction:[library newFunctionWithName:@"test_srv4"] error:&error];
    if(!result) { fprintf(stderr,"Metal pipeline: %s\n",error.description.UTF8String);exit(4); }
    return result;
}
static double run(id<MTLCommandQueue> queue,id<MTLComputePipelineState> pso,id<MTLBuffer> data,
                  NSUInteger offset,id<MTLBuffer> cases,id<MTLBuffer> output,NSUInteger count,unsigned repeats) {
    id<MTLCommandBuffer> command=[queue commandBuffer];
    id<MTLComputeCommandEncoder> encoder=[command computeCommandEncoder];
    [encoder setComputePipelineState:pso];
    [encoder setBuffer:data offset:offset atIndex:0];[encoder setBuffer:cases offset:0 atIndex:1];
    [encoder setBuffer:output offset:0 atIndex:2];
    for(unsigned i=0;i<repeats;i++) [encoder dispatchThreads:MTLSizeMake(count,1,1) threadsPerThreadgroup:MTLSizeMake(64,1,1)];
    [encoder endEncoding];[command commit];[command waitUntilCompleted];
    if(command.error) { fprintf(stderr,"GPU: %s\n",command.error.description.UTF8String);exit(5); }
    return 1000*(command.GPUEndTime-command.GPUStartTime);
}
int main(int argc,char **argv) { @autoreleasepool {
    if(argc!=3 && argc!=5) return 2;
    id<MTLDevice> device=MTLCreateSystemDefaultDevice();if(!device) return 6;
    id<MTLCommandQueue> queue=[device newCommandQueue];
    id<MTLComputePipelineState> baseline=pipeline(device,@(argv[1])),vector=pipeline(device,@(argv[2]));
    uint32_t input[256];for(unsigned i=0;i<256;i++)input[i]=0xdead0000u+i*97;
    // Empty, partial, full, unaligned 60-byte records, and UINT32 address wrap.
    uint32_t pairs[20000];unsigned count=0;
    for(uint32_t words=0;words<=128;words++) {
        for(uint32_t index=0;index<=words+4;index++) { pairs[2*count]=index;pairs[2*count+1]=words*4;count++; }
        for(uint32_t edge=0;edge<4;edge++) { pairs[2*count]=UINT32_MAX-edge;pairs[2*count+1]=words*4;count++; }
    }
    id<MTLBuffer> data=[device newBufferWithBytes:input length:sizeof(input) options:MTLResourceStorageModeShared];
    id<MTLBuffer> cases=[device newBufferWithBytes:pairs length:count*8 options:MTLResourceStorageModeShared];
    id<MTLBuffer> output=[device newBufferWithLength:count*16 options:MTLResourceStorageModeShared];
    for(unsigned method=0;method<2;method++) for(unsigned wordOffset=0;wordOffset<4;wordOffset++) {
        run(queue,method?vector:baseline,data,wordOffset*4,cases,output,count,1);
        uint32_t *got=output.contents;
        for(unsigned i=0;i<count;i++)for(unsigned c=0;c<4;c++) {
            uint32_t index=pairs[i*2]+c,words=pairs[i*2+1]/4;
            uint32_t expected=index<words?input[wordOffset+index]:0;
            if(got[i*4+c]!=expected) {
                fprintf(stderr,"Mismatch method=%u offset=%u index=%u words=%u component=%u actual=%x expected=%x\n",
                        method,wordOffset,pairs[i*2],words,c,got[i*4+c],expected);return 7;
            }
        }
    }
    printf("PASS: %u cases x 4 components x 4 alignments x 2 production AIR paths match CPU bounds/wrap semantics\n",count);
    unsigned total=262144;NSMutableData *benchPairs=[NSMutableData dataWithLength:total*8];uint32_t *p=benchPairs.mutableBytes;
    for(unsigned i=0;i<total;i++) {p[2*i]=(i*15)%125;p[2*i+1]=128*4;}
    cases=[device newBufferWithBytes:p length:total*8 options:MTLResourceStorageModeShared];
    output=[device newBufferWithLength:total*16 options:MTLResourceStorageModeShared];
    run(queue,baseline,data,0,cases,output,total,8);run(queue,vector,data,0,cases,output,total,8);
    NSMutableArray *a=[NSMutableArray new],*b=[NSMutableArray new];
    for(unsigned i=0;i<10;i++) {
        if(i%2) { [b addObject:@(run(queue,vector,data,0,cases,output,total,32))];[a addObject:@(run(queue,baseline,data,0,cases,output,total,32))]; }
        else { [a addObject:@(run(queue,baseline,data,0,cases,output,total,32))];[b addObject:@(run(queue,vector,data,0,cases,output,total,32))]; }
    }
    [a sortUsingSelector:@selector(compare:)];[b sortUsingSelector:@selector(compare:)];
    printf("BENCH: device=%s scalar_median_ms=%.6f vector_median_ms=%.6f scope=isolated-SRV-read-not-game-FPS\n",
           device.name.UTF8String,[a[5] doubleValue],[b[5] doubleValue]);
    if(argc==5) {
        baseline=pipeline(device,@(argv[3]));vector=pipeline(device,@(argv[4]));
        total=65536;NSMutableData *saved=[NSMutableData dataWithLength:total*16];
        run(queue,baseline,data,0,cases,output,total,1);memcpy(saved.mutableBytes,output.contents,total*16);
        run(queue,vector,data,0,cases,output,total,1);
        if(memcmp(saved.bytes,output.contents,total*16)) {fprintf(stderr,"Dependent-read loop outputs differ\n");return 8;}
        [a removeAllObjects];[b removeAllObjects];
        for(unsigned i=0;i<10;i++) {
            if(i%2) {[b addObject:@(run(queue,vector,data,0,cases,output,total,4))];[a addObject:@(run(queue,baseline,data,0,cases,output,total,4))];}
            else {[a addObject:@(run(queue,baseline,data,0,cases,output,total,4))];[b addObject:@(run(queue,vector,data,0,cases,output,total,4))];}
        }
        [a sortUsingSelector:@selector(compare:)];[b sortUsingSelector:@selector(compare:)];
        printf("PASS: 64-read loop outputs match; BENCH_LOOP: scalar_median_ms=%.6f vector_median_ms=%.6f scope=isolated-SRV-loop-not-game-FPS\n",[a[5] doubleValue],[b[5] doubleValue]);
    }
} return 0; }
