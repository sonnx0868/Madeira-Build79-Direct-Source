#include <metal_stdlib>
using namespace metal;

// Must match adaptive_lighting.hpp. All allocations are bounded independently
// of untrusted game dimensions. Unsupported descriptions use the reference.
constant uint maskOffset = 4147200;
constant uint listOffset = 6220800;
constant uint tileOffset = 8309760;
constant uint headerOffset = 8317920;
constant uint fixedWidth = 1920;
constant uint coarseWidth = 960;
constant uint fixedTileWidth = 120;

struct MadeiraLightingRoot {
  array<ulong, 9> beforeDepth [[id(0)]];
  texture2d_array<float, access::read> depth [[id(9)]];
  array<ulong, 47> beforeBase [[id(10)]];
  texture2d_array<float, access::read> baseColor [[id(57)]];
  ulong gap0 [[id(58)]];
  texture2d_array<float, access::read> normalRough [[id(59)]];
  ulong gap1 [[id(60)]];
  texture2d_array<float, access::read> material [[id(61)]];
  array<ulong, 7> beforeStencil [[id(62)]];
  texture2d_array<uint, access::read> stencil [[id(69)]];
  ulong gap2 [[id(70)]];
  device const uint *tiles [[id(71)]];
  ulong tilesMetadata [[id(72)]];
};

struct MadeiraLightingConstants {
  constant float4 *screen [[id(0)]];
  array<ulong, 2> unused [[id(1)]];
  constant uint4 *tileConstants [[id(3)]];
  ulong last [[id(4)]];
};

kernel void madeira_prepare_lighting_v1(
    device const uint *arguments [[buffer(26)]],
    constant uint4 &settings [[buffer(27)]],
    device uint *scratch [[buffer(28)]],
    constant MadeiraLightingConstants &cb [[buffer(29)]],
    constant MadeiraLightingRoot &root [[buffer(30)]],
    uint index [[thread_position_in_grid]]) {
  if (index < 8160) scratch[tileOffset + index] = 0;
  if (index != 0) return;
  device uint *h = scratch + headerOffset;
  uint x = arguments[0], y = arguments[1], z = arguments[2];
  float2 dimensions = cb.screen[46].xy;
  bool finite = all(isfinite(dimensions));
  bool sizeOK = finite && all(dimensions >= 1.0f) && all(dimensions <= float2(1920, 1080)) &&
                all(floor(dimensions) == dimensions);
  uint2 size = sizeOK ? uint2(dimensions) : uint2(0);
  uint count = x / 4, offset = cb.tileConstants[0].x;
  uint capacity = uint(root.tilesMetadata) / 4;
  bool valid = sizeOK && y == 1 && z == 1 && (x & 3u) == 0 && count <= 8160 &&
               offset <= capacity && count <= capacity - offset;
  h[0] = x; h[1] = y; h[2] = z; h[3] = valid;
  h[4] = size.x; h[5] = size.y;
  h[6] = (size.x + 15) / 16; h[7] = (size.y + 15) / 16;
  h[8] = 0; h[9] = 0; h[10] = 1; h[11] = 1;
  h[12] = valid ? count : 0; h[13] = 1; h[14] = 1;
  h[15] = valid ? h[6] * h[7] : 0; h[16] = 1; h[17] = 1;
  h[18] = min(settings.x, 2u); h[19] = settings.y;
}

static float3 decodeNormal(float3 g) {
  uint3 packed = uint3(g * 255.5f);
  uint2 pair(packed.x | ((packed.z << 8) & 3840u),
             packed.y | ((packed.z << 4) & ~255u));
  float2 oct = fma(float2(pair), float2(0x1.001002p-11f), float2(-1));
  float z = 1 - abs(oct.x) - abs(oct.y), fold = max(-z, 0.0f);
  oct += select(float2(-fold), float2(fold), oct < float2(0));
  float3 n(oct, z);
  return n * fast::rsqrt(dot(n, n));
}

struct LightingSample { float values[6]; uint valid; };

kernel void madeira_classify_lighting_v1(
    device uint *scratch [[buffer(28)]],
    constant MadeiraLightingRoot &root [[buffer(30)]],
    uint tile [[threadgroup_position_in_grid]], uint local [[thread_index_in_threadgroup]],
    uint lane [[thread_index_in_simdgroup]], uint simd [[simdgroup_index_in_threadgroup]]) {
  device uint *h = scratch + headerOffset;
  uint2 size(h[4], h[5]);
  uint2 origin((tile % h[6]) * 16, (tile / h[6]) * 16);
  uint2 p = origin + uint2(local & 15u, local >> 4);
  bool within = all(p < size);
  uint tileIndex = (origin.y / 16) * fixedTileWidth + origin.x / 16;
  // One 16x16 tile needs only a 10x10 coarse neighbourhood (including its
  // margin). Load it once per group instead of 54 global loads and a private
  // 54-float array per pixel. Keep the barrier outside divergent pixel tests.
  threadgroup LightingSample neighbours[100];
  if (local < 100) {
    uint2 cs = (size + 1u) >> 1;
    int2 q = int2(origin / 2u) + int2(int(local % 10) - 1, int(local / 10) - 1);
    bool exists = all(q >= 0) && all(q < int2(cs));
    q = clamp(q, int2(0), int2(cs) - 1);
    uint sample = (uint(q.y) * coarseWidth + uint(q.x)) * 8u;
    uint map = (uint(q.y) * 2u / 16u) * fixedTileWidth + uint(q.x) * 2u / 16u;
    neighbours[local].valid = exists && scratch[tileOffset + map] != 0 && scratch[sample + 6u] != 0;
    for (uint k = 0; k < 6; ++k) neighbours[local].values[k] = as_type<float>(scratch[sample + k]);
  }
  threadgroup_barrier(mem_flags::mem_threadgroup);
  bool valid = within && scratch[tileOffset + tileIndex] != 0;
  float z = valid ? root.depth.read(p, 0).x : 0;
  valid = valid && z != 0 && (root.stencil.read(p, 0).g & 2u) != 0;
  bool trusted = false;
  if (valid) {
    uint2 coarse = p >> 1, centerPixel = coarse * 2u;
    uint base = (coarse.y * coarseWidth + coarse.x) * 8u;
    bool exact = ((p.x | p.y) & 1u) == 0;
    if (scratch[base + 6u] != 0) {
      if (exact) {
        trusted = true;
      } else {
      float4 a = root.normalRough.read(p, 0), b = root.normalRough.read(centerPixel, 0);
      uint mode = h[18];
      float roughMin = mode == 0 ? .35f : (mode == 1 ? .30f : .25f);
      float normalMin = mode == 0 ? .9995f : (mode == 1 ? .995f : .99f);
      float depthMax = mode == 0 ? .002f : (mode == 1 ? .003f : .004f);
      float roughMax = mode == 0 ? .005f : .01f;
      float materialMax = mode == 0 ? .005f : (mode == 1 ? .01f : .015f);
      // A fixed angular threshold allowed several degrees of normal change
      // on a narrow specular lobe. Bound it by the lobe's roughness instead.
      float r2 = min(a.a, b.a) * min(a.a, b.a);
      normalMin = max(normalMin, 1.0f - .004f * r2 * r2);
      trusted = a.a >= roughMin && abs(a.a - b.a) <= roughMax;
      if (trusted) trusted = dot(decodeNormal(a.rgb), decodeNormal(b.rgb)) >= normalMin;
      if (trusted) {
        float cz = root.depth.read(centerPixel, 0).x;
        trusted = abs(z - cz) <= max(z * depthMax, .000001f);
      }
      if (trusted) {
        float4 m = root.material.read(p, 0), cm = root.material.read(centerPixel, 0);
        trusted = all(abs(m - cm) <= float4(materialMax));
      }
      if (trusted) {
        uint tag = uint(int(fma(root.baseColor.read(p, 0).a, 0x1.fdfffep+0f, 0x1.010102p-8f)));
        uint ct = uint(int(fma(root.baseColor.read(centerPixel, 0).a, 0x1.fdfffep+0f, 0x1.010102p-8f)));
        trusted = tag == ct;
      }
      // A flat wall can cross a shadow boundary. Reject curvature/discontinuity
      // in irradiance itself; geometry checks alone caused bright leaks.
      if (trusted && !exact) {
        int center = int(((p.y & 15u) / 2u + 1u) * 10u + (p.x & 15u) / 2u + 1u);
        for (int dy = -1; dy <= 1; ++dy) for (int dx = -1; dx <= 1; ++dx)
          if (!neighbours[center + dy * 10 + dx].valid) trusted = false;
        for (uint k = 0; k < 6 && trusted; ++k) for (uint axis = 0; axis < 4; ++axis) {
          int delta = axis == 0 ? 1 : (axis == 1 ? 10 : (axis == 2 ? 11 : 9));
          float c = neighbours[center].values[k];
          float lo = neighbours[center - delta].values[k], hi = neighbours[center + delta].values[k];
          float scale = max(abs(c), max(abs(lo), abs(hi)));
          if (!isfinite(c) || !isfinite(lo) || !isfinite(hi) ||
              abs(lo + hi - 2 * c) > .003f + .10f * scale) trusted = false;
        }
      }
      }
    }
  }
  if (h[19]) trusted = false;
  if (within) scratch[maskOffset + p.y * fixedWidth + p.x] = valid ? (trusted ? 1u : 2u) : 0u;
  bool needed = valid && !trusted;
  uint count = simd_sum(needed ? 1u : 0u), rank = simd_prefix_exclusive_sum(needed ? 1u : 0u);
  threadgroup uint counts[8], sharedBase;
  if (lane == 0) counts[simd] = count;
  threadgroup_barrier(mem_flags::mem_threadgroup);
  uint total = 0, prefix = 0;
  for (uint i = 0; i < 8; ++i) { if (i < simd) prefix += counts[i]; total += counts[i]; }
  // Keep each SIMD wave inside one tile, while allowing the two waves in a
  // 64-thread fallback group to serve different tiles. Padding every tile to
  // 64 wasted a second wave on many small edge lists.
  uint slots = (total + 31u) & ~31u;
  if (local == 0) sharedBase = slots ? atomic_fetch_add_explicit(
      (device atomic_uint *)(h + 8), slots, memory_order_relaxed) : 0u;
  threadgroup_barrier(mem_flags::mem_threadgroup);
  if (needed) scratch[listOffset + sharedBase + prefix + rank] = p.x | (p.y << 16);
  if (local < slots - total) scratch[listOffset + sharedBase + total + local] = 0xffffffffu;
}

kernel void madeira_finalize_lighting_v1(device uint *scratch [[buffer(28)]]) {
  device uint *h = scratch + headerOffset;
  h[9] = (h[8] + 63u) / 64u; h[10] = 1; h[11] = 1;
  for (uint i = h[8]; i < h[9] * 64u; ++i) scratch[listOffset + i] = 0xffffffffu;
}
