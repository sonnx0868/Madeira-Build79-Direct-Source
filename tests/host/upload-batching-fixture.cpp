// SPDX-License-Identifier: MIT
#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <memory>
#include <type_traits>
#include <unordered_map>
#include <vector>
#include "production_upload_batching.inc"

using namespace dxmt;

static EncoderDepKey key(uint64_t allocation) { return EncoderDepSet::generateNewKey(allocation); }

struct Arena {
  std::vector<std::unique_ptr<std::byte[]>> memory;
  template <class T> T *allocate() {
    static_assert(std::is_trivially_destructible_v<T>);
    auto storage = std::make_unique<std::byte[]>(sizeof(T));
    auto *value = new (storage.get()) T{};
    memory.push_back(std::move(storage));
    return value;
  }
  void append(BlitEncoderData *p, uint64_t src, uint64_t dst, uint64_t size = 256,
              uint64_t src_offset = 0, uint64_t dst_offset = 0) {
    auto *c = allocate<wmtcmd_blit_copy_from_buffer_to_buffer>();
    c->type = WMTBlitCommandCopyFromBufferToBuffer;
    c->src = src; c->dst = dst; c->copy_length = size;
    c->src_offset = src_offset; c->dst_offset = dst_offset;
    p->cmd_tail->next.set(c);
    p->cmd_tail = reinterpret_cast<wmtcmd_base *>(c);
    p->buf_read.add(key(src)); p->buf_write.add(key(dst));
  }
  BlitEncoderData *copy(uint64_t src, uint64_t dst, uint64_t size = 256,
                        uint64_t src_offset = 0, uint64_t dst_offset = 0) {
    auto *p = allocate<BlitEncoderData>();
    p->type = EncoderType::Blit;
    p->cmd_head.type = WMTBlitCommandNop;
    p->cmd_tail = reinterpret_cast<wmtcmd_base *>(&p->cmd_head);
    append(p, src, dst, size, src_offset, dst_offset);
    return p;
  }
  ComputeEncoderData *compute(uint64_t read = 0, uint64_t write = 0) {
    auto *p = allocate<ComputeEncoderData>();
    p->type = EncoderType::Compute;
    auto *cmd = allocate<wmtcmd_base>();
    cmd->type = WMTComputeCommandDispatch;
    p->cmd_head.next.set(cmd); p->cmd_tail = cmd;
    if (read) p->buf_read.add(key(read));
    if (write) p->buf_write.add(key(write));
    return p;
  }
  EncoderData *barrier(EncoderType type) {
    auto *p = allocate<EncoderData>(); p->type = type; return p;
  }
};

static std::vector<wmtcmd_blit_copy_from_buffer_to_buffer *> copies(EncoderData *p) {
  assert(p->type == EncoderType::Blit);
  std::vector<wmtcmd_blit_copy_from_buffer_to_buffer *> result;
  auto *blit = static_cast<BlitEncoderData *>(p);
  auto *last = reinterpret_cast<wmtcmd_base *>(&blit->cmd_head);
  for (auto *c = static_cast<wmtcmd_base *>(blit->cmd_head.next.get()); c;
       c = static_cast<wmtcmd_base *>(c->next.get())) {
    assert(c->type == WMTBlitCommandCopyFromBufferToBuffer);
    assert(result.size() < 10000); // Also catches a damaged cyclic chain.
    result.push_back(reinterpret_cast<wmtcmd_blit_copy_from_buffer_to_buffer *>(c));
    last = c;
  }
  assert(last == blit->cmd_tail);
  return result;
}

static void verifyCapture(std::vector<EncoderData *> passes) {
  ArgumentEncodingContext context;
  std::vector<wmtcmd_blit_copy_from_buffer_to_buffer *> commands;
  uint64_t bytes = 0;
  for (unsigned i = 0; i < passes.size(); i += 2) {
    auto *copy = copies(passes[i]).at(0);
    commands.push_back(copy); bytes += copy->copy_length;
    assert(context.hasDataDependency(passes[i + 1], passes[i]));
    // The existing forward optimizer must stop at each upload's consumer.
    for (unsigned j = 0; j < i; ++j)
      assert(!context.hasDataDependency(passes[i], passes[j]));
  }
  assert(bytes == 32512);
  auto live = context.batch(passes.data(), unsigned(passes.size()));
  assert(live == 51 && copies(passes[0]) == commands);
  for (unsigned i = 1; i < live; ++i)
    assert(passes[i]->type == EncoderType::Compute);
  std::printf("PASS: captured 50 uploads/50 consumers: 100 -> %u passes, 32512 bytes unchanged\n", live);
}

#include "captured_upload_dependencies.inc"

static void hazardControls() {
  ArgumentEncodingContext context;
  // Both texture and buffer dependency domains preserve RAW, WAR and WAW.
  for (bool texture : {false, true}) {
    for (unsigned hazard = 0; hazard < 3; ++hazard) {
      Arena a;
      auto *first = a.copy(1, 10);
      auto *compute = a.compute();
      auto *later = a.copy(2, 20);
      auto &read = texture ? compute->tex_read : compute->buf_read;
      auto &write = texture ? compute->tex_write : compute->buf_write;
      auto &upload_read = texture ? later->tex_read : later->buf_read;
      auto &upload_write = texture ? later->tex_write : later->buf_write;
      if (hazard == 0) { write.add(key(999)); upload_read.add(key(999)); }
      if (hazard == 1) { read.add(key(999)); upload_write.add(key(999)); }
      if (hazard == 2) { write.add(key(999)); upload_write.add(key(999)); }
      EncoderData *p[] = {first, compute, later};
      assert(context.batch(p, 3) == 3);
      assert(copies(first).size() == 1 && copies(later).size() == 1);
    }
  }
  for (unsigned hazard = 0; hazard < 3; ++hazard) {
    Arena a;
    auto *first = a.copy(1, 10);
    auto *later = a.copy(hazard == 0 ? 10 : 2, hazard == 1 ? 1 : hazard == 2 ? 10 : 20);
    EncoderData *p[] = {first, a.compute(), later};
    assert(context.batch(p, 3) == 3); // Hazards against merge target, too.
  }
  {
    Arena a;
    EncoderData *p[] = {a.copy(1, 10, 16, 0, 0), a.compute(), a.copy(2, 10, 16, 0, 128)};
    assert(context.batch(p, 3) == 3); // Allocation-wide hazard despite disjoint ranges.
  }
  {
    Arena a;
    auto *work = a.compute();
    auto *later = a.copy(2, 20);
    EncoderDepKey collision{}; collision.indices.fill(3);
    work->buf_read.add(collision); later->buf_write.add(collision);
    EncoderData *p[] = {a.copy(1, 10), work, later};
    assert(context.batch(p, 3) == 3); // Bloom false positives only lose batching.
  }
  {
    Arena a;
    auto *first = a.copy(1, 10);
    auto *second = a.copy(2, 20);
    auto *third = a.copy(20, 30);
    EncoderData *p[] = {first, a.compute(), second, a.compute(), third};
    assert(context.batch(p, 5) == 4); // Dependencies added by the first merge are retained.
    assert(copies(first).size() == 2 && copies(third).size() == 1);
  }
}

static void boundaryControls() {
  ArgumentEncodingContext context;
  for (auto type : {EncoderType::Render, EncoderType::Clear, EncoderType::Resolve,
                    EncoderType::StretchBlit, EncoderType::Present, EncoderType::SpatialUpscale,
                    EncoderType::TemporalUpscale, EncoderType::SignalEvent, EncoderType::WaitForEvent}) {
    Arena a;
    EncoderData *p[] = {a.copy(1, 10), a.barrier(type), a.copy(2, 20)};
    assert(context.batch(p, 3) == 3);
  }
  for (auto command : {WMTBlitCommandNop, WMTBlitCommandCopyFromBufferToTexture,
                       WMTBlitCommandCopyFromTextureToBuffer, WMTBlitCommandCopyFromTextureToTexture,
                       WMTBlitCommandGenerateMipmaps, WMTBlitCommandWaitForFence,
                       WMTBlitCommandUpdateFence, WMTBlitCommandFillBuffer,
                       WMTBlitCommandOptimizeContentsForGPUAccess}) {
    Arena a;
    auto *middle = a.copy(3, 30);
    middle->cmd_tail->type = command;
    EncoderData *p[] = {a.copy(1, 10), middle, a.copy(2, 20)};
    assert(context.batch(p, 3) == 3);
  }
  for (auto command : {WMTComputeCommandWaitForFence, WMTComputeCommandUpdateFence}) {
    Arena a;
    auto *middle = a.compute(); middle->cmd_tail->type = command;
    EncoderData *p[] = {a.copy(1, 10), middle, a.copy(2, 20)};
    assert(context.batch(p, 3) == 3);
  }
  {
    Arena a;
    auto *middle = a.copy(3, 30);
    middle->cmd_head.next.set(nullptr); middle->cmd_tail = reinterpret_cast<wmtcmd_base *>(&middle->cmd_head);
    EncoderData *p[] = {a.copy(1, 10), middle, a.copy(2, 20)};
    assert(context.batch(p, 3) == 3);
  }
  {
    Arena a;
    auto *first = a.copy(1, 10);
    EncoderData *p[] = {first, a.barrier(EncoderType::Null), a.compute(), a.copy(2, 20)};
    assert(context.batch(p, 4) == 2 && copies(first).size() == 2);
  }
  // Submission boundaries cannot be crossed: each flush gets its own array.
  {
    Arena a;
    EncoderData *first[] = {a.copy(1, 10)};
    EncoderData *second[] = {a.compute(), a.copy(2, 20)};
    assert(context.batch(first, 1) == 1 && context.batch(second, 2) == 2);
  }
}

static void boundControls() {
  ArgumentEncodingContext context;
  for (unsigned computes : {63u, 64u, 65u, 130u}) {
    Arena a;
    std::vector<EncoderData *> p{a.copy(1, 10)};
    for (unsigned i = 0; i < computes; ++i) p.push_back(a.compute());
    p.push_back(a.copy(2, 20));
    auto live = context.batch(p.data(), unsigned(p.size()));
    assert(live == computes + (computes < 64 ? 1 : 2));
  }
  for (bool compute : {false, true}) {
    Arena a;
    EncoderData *middle;
    if (compute) {
      auto *p = a.compute(); middle = p;
      for (unsigned n = 1; n < 65; ++n) {
        auto *c = a.allocate<wmtcmd_base>(); c->type = WMTComputeCommandDispatch;
        p->cmd_tail->next.set(c); p->cmd_tail = c;
      }
    } else {
      auto *p = a.copy(3, 30); middle = p;
      for (unsigned n = 1; n < 65; ++n) a.append(p, 3, 30);
    }
    EncoderData *p[] = {a.copy(1, 10), middle, a.copy(2, 20)};
    assert(context.batch(p, 3) == 3);
  }
  // Classify once per input, inspect no more than 64 predecessors per copy.
  Arena a;
  std::vector<EncoderData *> p;
  for (unsigned i = 0; i < 1000; ++i) p.push_back(a.compute());
  p.push_back(a.copy(1, 10));
  unsigned classifications = 0, dependencies = 0;
  madeira::batch_buffer_uploads(p.data(), unsigned(p.size()), [&](auto *pass) {
    ++classifications; return classifyUploadBatchPass(pass);
  }, [&](auto *, auto *) { ++dependencies; return false; }, mergeUploadBatchPasses);
  assert(classifications == 1001 && dependencies == 64);
}

static void commandOrderAndReplay() {
  // Re-recording the same deferred-style stream resolves the current allocation
  // handles each time. Staging bytes and GPU write destinations remain distinct.
  using Bytes = std::vector<uint8_t>;
  using Memory = std::unordered_map<uint64_t, Bytes>;
  ArgumentEncodingContext context;
  for (unsigned replay = 0; replay < 3; ++replay) {
    Arena a;
    std::vector<EncoderData *> p;
    std::unordered_map<EncoderData *, std::pair<uint64_t, uint64_t>> dispatches;
    Memory initial;
    auto source = 1 + replay * 1000;
    initial[source] = Bytes(50 * 256);
    for (unsigned j = 0; j < 50 * 256; ++j) initial[source][j] = uint8_t(j * 13 + replay);
    for (unsigned i = 0; i < 50; ++i) {
      auto dst = 100 + i + replay * 1000, output = 200 + i + replay * 1000;
      initial[dst] = Bytes(256, 0xcc); initial[output] = Bytes(256, 0xaa);
      p.push_back(a.copy(source, dst, 256, i * 256));
      auto *work = a.compute(dst, output); p.push_back(work); dispatches[work] = {dst, output};
    }
    auto execute = [&](unsigned count) {
      auto memory = initial;
      for (unsigned n = 0; n < count; ++n) {
        if (p[n]->type == EncoderType::Blit) {
          for (auto *c : copies(p[n]))
            std::memcpy(memory[c->dst].data() + c->dst_offset, memory[c->src].data() + c->src_offset,
                        size_t(c->copy_length));
        } else {
          auto [src, dst] = dispatches.at(p[n]);
          for (unsigned j = 0; j < 256; ++j) memory[dst][j] = memory[src][j] ^ uint8_t(j);
        }
      }
      return memory;
    };
    auto before = execute(unsigned(p.size()));
    auto live = context.batch(p.data(), unsigned(p.size()));
    assert(live == 51 && execute(live) == before);
    auto commands = copies(p[0]);
    for (unsigned i = 0; i < 50; ++i) assert(commands[i]->src_offset == i * 256);
  }
}

int main() {
  replayCapturedDependencies();
  hazardControls(); boundaryControls(); boundControls(); commandOrderAndReplay();
  std::puts("PASS: RAW/WAR/WAW, aliases, Bloom collisions, merged dependencies, event/query/present/fence boundaries");
  std::puts("PASS: 64-pass/64-command limits, command order and byte-identical synthetic allocation re-recording");
}
