// SPDX-License-Identifier: MIT
// Native Metal control: same synthetic compute and bytes, only encoder batching
// changes. This measures this scheduling mechanism; it is not a game FPS test.
// clang++ -std=c++20 -O2 -fobjc-arc -framework Foundation -framework Metal \
//   -Ibuild/dxmt-ios tests/host/upload-batching-metal.mm -o /tmp/upload-batching-metal
#import <Foundation/Foundation.h>
#import <Metal/Metal.h>
#include <algorithm>
#include <array>
#include <bitset>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include "upload_batching.hpp"

static constexpr unsigned jobs = 50, words_out = 1024;
static constexpr std::array<unsigned, jobs> copy_bytes = {
  384,576,128,384,256,320,128,192,960,320,384,384,384,256,960,64,384,448,64,2304,
  384,1792,2304,384,960,384,128,768,3328,384,384,256,2496,1152,192,192,512,640,
  1600,384,512,512,2304,128,64,64,128,64,448,384
};
using Clock = std::chrono::steady_clock;
static double milliseconds(Clock::time_point a, Clock::time_point b) {
  return std::chrono::duration<double, std::milli>(b - a).count();
}
static void require(bool condition, const char *message) {
  if (!condition) { std::fprintf(stderr, "%s\n", message); std::exit(1); }
}

struct Pass {
  madeira::UploadBatchKind kind;
  std::vector<unsigned> copies;
  unsigned compute = 0;
  std::bitset<128> reads, writes;
};
struct Schedule {
  std::array<Pass, jobs * 2> storage;
  std::array<Pass *, jobs * 2> order;
  unsigned count = jobs * 2;
  explicit Schedule(bool batch) {
    for (unsigned i = 0; i < jobs; ++i) {
      auto &copy = storage[i * 2]; copy.kind = madeira::UploadBatchKind::Copy;
      copy.copies.push_back(i); copy.reads.set(0); copy.writes.set(i + 1);
      auto &compute = storage[i * 2 + 1]; compute.kind = madeira::UploadBatchKind::Compute;
      compute.compute = i; compute.reads.set(i + 1); compute.writes.set(i + jobs + 1);
      order[i * 2] = &copy; order[i * 2 + 1] = &compute;
    }
    if (batch) count = madeira::batch_buffer_uploads(order.data(), count,
      [](Pass *p) { return p->kind; },
      [](Pass *later, Pass *earlier) {
        return ((earlier->writes & later->reads) | (earlier->writes & later->writes) |
                (earlier->reads & later->writes)).any();
      }, [](Pass *target, Pass *source) {
        target->copies.insert(target->copies.end(), source->copies.begin(), source->copies.end());
        target->reads |= source->reads; target->writes |= source->writes;
      });
    require(count == (batch ? jobs + 1 : jobs * 2), "Unexpected pass schedule");
  }
};

struct Sample { double encode, commit, gpu, wall; };
static double percentile(std::vector<double> values, double fraction) {
  std::sort(values.begin(), values.end());
  return values.at(size_t(fraction * (values.size() - 1)));
}
static NSDictionary *summarize(const std::vector<Sample> &samples) {
  NSMutableDictionary *summary = [NSMutableDictionary dictionary];
  for (auto member : {&Sample::encode, &Sample::commit, &Sample::gpu, &Sample::wall}) {
    std::vector<double> values;
    for (const auto &sample : samples) values.push_back(sample.*member);
    NSString *name = member == &Sample::encode ? @"host_encode_ms" : member == &Sample::commit ?
      @"host_commit_ms" : member == &Sample::gpu ? @"gpu_ms" : @"wall_ms";
    summary[name] = @{@"median": @(percentile(values, .5)), @"p90": @(percentile(values, .9))};
  }
  return summary;
}

int main(int argc, char **argv) {
  @autoreleasepool {
    const unsigned rounds = argc > 1 ? unsigned(std::atoi(argv[1])) : 40;
    require(rounds >= 4 && rounds <= 1000, "rounds must be in [4,1000]");
    id<MTLDevice> device = MTLCreateSystemDefaultDevice();
    require(device != nil, "No Metal device");
    id<MTLCommandQueue> queue = [device newCommandQueue];
    NSError *error = nil;
    NSString *shader = @"#include <metal_stdlib>\nusing namespace metal;\n"
      "kernel void same_work(device const uint *input [[buffer(0)]], device uint *output [[buffer(1)]],"
      "constant uint &words [[buffer(2)]], uint i [[thread_position_in_grid]]) {"
      "uint v=input[i%words]; for(uint j=0;j<16;j++) v=(v^(i+j))*1664525u+1013904223u; output[i]=v;}";
    id<MTLLibrary> library = [device newLibraryWithSource:shader options:nil error:&error];
    require(library != nil, error.localizedDescription.UTF8String ?: "Metal library failure");
    id<MTLComputePipelineState> pipeline = [device newComputePipelineStateWithFunction:
      [library newFunctionWithName:@"same_work"] error:&error];
    require(pipeline != nil, error.localizedDescription.UTF8String ?: "Metal pipeline failure");
    std::array<unsigned, jobs> offsets{};
    unsigned bytes = 0;
    for (unsigned i = 0; i < jobs; ++i) { offsets[i] = bytes; bytes += copy_bytes[i]; }
    require(bytes == 32512, "Captured byte count changed");
    id<MTLBuffer> staging = [device newBufferWithLength:bytes options:
      MTLResourceStorageModeShared | MTLResourceHazardTrackingModeUntracked];
    require(staging != nil, "Staging allocation failed");
    auto *source = static_cast<uint32_t *>(staging.contents);
    for (unsigned i = 0; i < bytes / 4; ++i) source[i] = uint32_t(0x12345678u ^ i * 2654435761u);
    NSMutableArray<id<MTLBuffer>> *inputs = [NSMutableArray arrayWithCapacity:jobs];
    NSMutableArray<id<MTLBuffer>> *outputs = [NSMutableArray arrayWithCapacity:jobs];
    for (unsigned i = 0; i < jobs; ++i) {
      id<MTLBuffer> input = [device newBufferWithLength:std::max(256u, copy_bytes[i])
                                             options:MTLResourceStorageModePrivate | MTLResourceHazardTrackingModeTracked];
      id<MTLBuffer> output = [device newBufferWithLength:words_out * 4 options:MTLResourceStorageModeShared];
      require(input != nil && output != nil, "GPU allocation failed");
      [inputs addObject:input]; [outputs addObject:output];
    }
    auto run = [&](Schedule &schedule) -> Sample {
      @autoreleasepool {
        auto start = Clock::now();
        id<MTLCommandBuffer> cb = [queue commandBuffer];
        for (unsigned n = 0; n < schedule.count; ++n) {
          auto *pass = schedule.order[n];
          if (pass->kind == madeira::UploadBatchKind::Copy) {
            id<MTLBlitCommandEncoder> encoder = [cb blitCommandEncoder];
            for (auto i : pass->copies)
              [encoder copyFromBuffer:staging sourceOffset:offsets[i] toBuffer:inputs[i]
                    destinationOffset:0 size:copy_bytes[i]];
            [encoder endEncoding];
          } else {
            unsigned i = pass->compute, words = copy_bytes[i] / 4;
            id<MTLComputeCommandEncoder> encoder = [cb computeCommandEncoder];
            [encoder setComputePipelineState:pipeline];
            [encoder setBuffer:inputs[i] offset:0 atIndex:0];
            [encoder setBuffer:outputs[i] offset:0 atIndex:1];
            [encoder setBytes:&words length:sizeof(words) atIndex:2];
            [encoder dispatchThreads:MTLSizeMake(words_out, 1, 1) threadsPerThreadgroup:MTLSizeMake(64, 1, 1)];
            [encoder endEncoding];
          }
        }
        auto encoded = Clock::now();
        [cb commit];
        auto committed = Clock::now();
        [cb waitUntilCompleted];
        auto completed = Clock::now();
        require(cb.status == MTLCommandBufferStatusCompleted,
                cb.error.localizedDescription.UTF8String ?: "GPU execution failed");
        for (unsigned i = 0; i < jobs; ++i) {
          const auto *actual = static_cast<const uint32_t *>(outputs[i].contents);
          const auto *input = source + offsets[i] / 4;
          for (unsigned x = 0; x < words_out; ++x) {
            uint32_t v = input[x % (copy_bytes[i] / 4)];
            for (uint32_t j = 0; j < 16; ++j) v = (v ^ (x + j)) * 1664525u + 1013904223u;
            require(actual[x] == v, "GPU output differs from CPU reference");
          }
        }
        require(cb.GPUEndTime >= cb.GPUStartTime && cb.GPUStartTime > 0, "GPU timestamps unavailable");
        return {milliseconds(start, encoded), milliseconds(encoded, committed),
                (cb.GPUEndTime - cb.GPUStartTime) * 1000, milliseconds(start, completed)};
      }
    };
    Schedule baseline(false), batched(true);
    std::vector<Sample> before, after;
    for (unsigned round = 0; round < rounds + 8; ++round) {
      Sample a, b;
      if (round & 1) { b = run(batched); a = run(baseline); }
      else { a = run(baseline); b = run(batched); }
      if (round >= 8) { before.push_back(a); after.push_back(b); }
    }
    NSDictionary *result = @{
      @"schema": @1, @"device": device.name, @"rounds_per_variant": @(rounds),
      @"scope": @"synthetic scheduling control; original captured copy sizes, not original game shaders or FPS",
      @"copies": @(jobs), @"copied_bytes": @(bytes), @"compute_passes": @(jobs),
      @"baseline_blit_passes": @(jobs), @"batched_blit_passes": @1,
      @"output_words_checked_per_run": @(jobs * words_out), @"byte_parity": @YES,
      @"baseline": summarize(before), @"batched": summarize(after)
    };
    NSData *json = [NSJSONSerialization dataWithJSONObject:result options:NSJSONWritingPrettyPrinted error:&error];
    require(json != nil, "JSON serialization failure");
    std::fwrite(json.bytes, 1, json.length, stdout); std::putchar('\n');
  }
}
