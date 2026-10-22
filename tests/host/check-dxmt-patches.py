#!/usr/bin/env python3
"""Reproduce the complete ordered DXMT patch pipeline on LF-only pinned sources."""
from pathlib import Path
import os
import shutil
import subprocess
import tempfile

PATCH_NAMES = ("patches/dxmt-resolve-srv-mips.patch", "patches/dxmt-marshal-progress.patch", "patches/dxmt-madeira-query-log.patch", "patches/dxmt-runtime-lifecycle.patch",
               "patches/dxmt-gameplay-performance.patch", "build/dxmt-ios/clean-build.patch",
               "build/dxmt-ios/pipeline-cache.patch", "patches/dxmt-metal-public-atomic.patch",
               "build/dxmt-ios/pipeline-warmup.patch", "patches/dxmt-gpu-foreground.patch",
               "patches/dxmt-submit-before-present.patch", "patches/dxmt-cached-cpu-mappings.patch",
               "patches/dxmt-producer-pacing.patch", "patches/dxmt-query-park.patch",
               "patches/dxmt-dead-color-store.patch", "patches/dxmt-swapchain-hook-abi.patch",
               "patches/dxmt-gpu-trace.patch", "patches/dxmt-bounded-srv-vector-load.patch",
               "patches/dxmt-explicit-lod-guard.patch",
               "patches/dxmt-texture-headroom.patch",
               "patches/dxmt-lossless-color-views.patch",
               "patches/dxmt-frameflow.patch",
               "patches/dxmt-light-graphics.patch", "patches/dxmt-guarded-pcf.patch",
               "patches/dxmt-zero-light-guard.patch",
               "patches/dxmt-retirement-lifetime.patch",
               "patches/dxmt-gpu-stages.patch", "patches/dxmt-present-resource-lifetime.patch",
               "patches/dxmt-depth-rename-retention.patch",
               "patches/dxmt-upload-batching.patch",
               "patches/dxmt-lighting-phase.patch",
               "patches/dxmt-multikernel-metallib.patch",
               "patches/dxmt-padded-light-table.patch",
               "patches/dxmt-bounded-command-reuse.patch", "patches/dxmt-logarithmic-dynamic-trim.patch", "patches/dxmt-lazy-shader-objects.patch")
SOURCE_FILES = ("src/dxmt/dxmt_dynamic.cpp", "src/dxmt/dxmt_dynamic.hpp", "src/dxmt/dxmt_ring_bump_allocator.hpp", "src/d3d11/d3d11_resource.hpp", "src/dxmt/dxmt_command.metal", "src/util/com/com_guid.cpp", "src/winemetal/unix/winemetal_unix.c",
                "src/winemetal/unix/cache.c", "src/d3d11/d3d11_pipeline_cache.cpp",
                "src/d3d9/d3d9_shader.cpp", "src/dxmt/dxmt_tasks.hpp", "src/dxmt/dxmt_shader_cache.hpp",
                "src/dxmt/dxmt_command.cpp", "src/airconv/shaders/air_tessellation.metal",
                "src/d3d11/d3d11_swapchain.cpp", "src/dxmt/dxmt_command_list.hpp",
                "src/dxmt/dxmt_command_queue.hpp", "src/d3d11/d3d11_context_imm.cpp",
                "src/d3d11/d3d11_query.hpp", "src/dxmt/dxmt_context.cpp",
                "src/airconv/nt/dxbc_converter_base.cpp", "src/airconv/nt/air_builder.cpp",
                "src/airconv/airconv_context.cpp", "src/d3d11/d3d11_resource_helper.cpp",
                "src/dxmt/dxmt_resource_initializer.cpp", "src/dxmt/dxmt_resource_initializer.hpp",
                "src/dxmt/dxmt_staging.cpp", "src/dxmt/dxmt_staging.hpp",
                "src/dxmt/dxmt_command_queue.cpp", "src/d3d11/d3d11_state_object.cpp",
                "src/d3d11/d3d11_texture_device.cpp", "src/d3d11/d3d11_context_impl.cpp",
                "src/dxmt/dxmt_allocation.hpp", "src/dxmt/dxmt_texture.hpp",
                "src/dxmt/dxmt_texture.cpp", "src/airconv/metallib_writer.cpp")


def patched_sources(root):
    pin = subprocess.check_output(["git", "-C", str(root), "rev-parse", "HEAD:dxmt"], text=True).strip()
    with tempfile.TemporaryDirectory(prefix="madeira-dxmt-pipeline-") as scratch:
        folder = Path(scratch)
        checkout = folder / "dxmt"
        patch_folder = folder / "patches"
        patch_folder.mkdir()
        originals = {}
        for name in SOURCE_FILES:
            path = checkout / name
            path.parent.mkdir(parents=True, exist_ok=True)
            originals[name] = subprocess.check_output(["git", "-C", str(root / "dxmt"), "show", pin + ":" + name])
            path.write_bytes(originals[name])
        patches = []
        for name in PATCH_NAMES:
            path = folder / name
            path.parent.mkdir(parents=True, exist_ok=True)
            # Model macOS/Linux checkout regardless of the test host's autocrlf.
            path.write_bytes((root / name).read_bytes().replace(b"\r\n", b"\n"))
            patches.append(path)
        residency_patch = folder / "patches/dxmt-texture-residency.patch"
        residency_patch.write_bytes((root / "patches/dxmt-texture-residency.patch").read_bytes().replace(b"\r\n", b"\n"))
        legacy = folder / "build/dxmt-ios/legacy-metal-source-fallback.patch"
        legacy.write_bytes((root / "build/dxmt-ios/legacy-metal-source-fallback.patch").read_bytes().replace(b"\r\n", b"\n"))
        script = folder / "scripts/apply-dxmt-patches.sh"
        script.parent.mkdir()
        script.write_bytes((root / "scripts/apply-dxmt-patches.sh").read_bytes().replace(b"\r\n", b"\n"))
        git_bash = Path(os.environ.get("ProgramFiles", "C:/Program Files")) / "Git/bin/bash.exe"
        bash = str(git_bash) if os.name == "nt" and git_bash.exists() else shutil.which("bash")
        if not bash:
            raise RuntimeError("bash is required to test the production DXMT patch pipeline")
        # Run the actual production shell script for both a clean checkout and
        # an already patched checkout; Windows WSL is not required.
        for _ in range(2):
            result = subprocess.run([bash, script.as_posix()], capture_output=True, text=True)
            if result.returncode:
                raise RuntimeError("DXMT patch pipeline failed:\n" + result.stdout + result.stderr)
        # A restored/previous build tree can contain the old ungated fallback.
        # Upgrade that exact known state while preserving all unrelated patches.
        command_name = "src/dxmt/dxmt_command.cpp"
        expected_command = (checkout / command_name).read_bytes()
        (checkout / command_name).write_bytes(originals[command_name])
        subprocess.run(["git", "-C", str(checkout), "apply", str(legacy)], check=True)
        result = subprocess.run([bash, script.as_posix()], capture_output=True, text=True)
        if result.returncode:
            raise RuntimeError("Legacy fallback upgrade failed:\n" + result.stdout + result.stderr)
        assert (checkout / command_name).read_bytes() == expected_command
        # Verify re-running the production pipeline will recognize every patch,
        # including the lifecycle patch AFTER all gameplay changes have landed.
        # Verify the dependent top layer first, then validate all older layers
        # against the exact source they extend. Restore it before returning.
        upload_batching_patch = folder / "patches/dxmt-upload-batching.patch"
        phase_patch = folder / "patches/dxmt-lighting-phase.patch"
        padded_patch = folder / "patches/dxmt-padded-light-table.patch"
        writer_patch = folder / "patches/dxmt-multikernel-metallib.patch"
        upstream_patches = [folder / name for name in PATCH_NAMES[-3:]]
        for patch in reversed(upstream_patches):
            subprocess.run(["git", "-C", str(checkout), "apply", "--reverse", str(patch)], check=True)
        for patch in (padded_patch, writer_patch):
            subprocess.run(["git", "-C", str(checkout), "apply", "--reverse", str(patch)], check=True)
        subprocess.run(["git", "-C", str(checkout), "apply", "--reverse", "--check", str(residency_patch)], check=True)
        subprocess.run(["git", "-C", str(checkout), "apply", "--reverse", str(residency_patch)], check=True)
        subprocess.run(["git", "-C", str(checkout), "apply", "--reverse", "--check", str(phase_patch)], check=True)
        subprocess.run(["git", "-C", str(checkout), "apply", "--reverse", str(phase_patch)], check=True)
        zero_patch = folder / "patches/dxmt-zero-light-guard.patch"
        subprocess.run(["git", "-C", str(checkout), "apply", "--reverse", str(zero_patch)], check=True)
        subprocess.run(["git", "-C", str(checkout), "apply", "--reverse", "--check", str(upload_batching_patch)], check=True)
        subprocess.run(["git", "-C", str(checkout), "apply", "--reverse", str(upload_batching_patch)], check=True)
        depth_retention_patch = folder / "patches/dxmt-depth-rename-retention.patch"
        subprocess.run(["git", "-C", str(checkout), "apply", "--reverse", "--check", str(depth_retention_patch)], check=True)
        subprocess.run(["git", "-C", str(checkout), "apply", "--reverse", str(depth_retention_patch)], check=True)
        present_resource_patch = folder / "patches/dxmt-present-resource-lifetime.patch"
        subprocess.run(["git", "-C", str(checkout), "apply", "--reverse", "--check", str(present_resource_patch)], check=True)
        subprocess.run(["git", "-C", str(checkout), "apply", "--reverse", str(present_resource_patch)], check=True)
        stage_patch = folder / "patches/dxmt-gpu-stages.patch"
        subprocess.run(["git", "-C", str(checkout), "apply", "--reverse", "--check", str(stage_patch)], check=True)
        subprocess.run(["git", "-C", str(checkout), "apply", "--reverse", str(stage_patch)], check=True)
        retirement_patch = folder / "patches/dxmt-retirement-lifetime.patch"
        subprocess.run(["git", "-C", str(checkout), "apply", "--reverse", "--check", str(retirement_patch)], check=True)
        subprocess.run(["git", "-C", str(checkout), "apply", "--reverse", str(retirement_patch)], check=True)
        for path in patches:
            if path in upstream_patches:
                continue
            if path in {phase_patch, retirement_patch, stage_patch, present_resource_patch, depth_retention_patch, upload_batching_patch, padded_patch, writer_patch}:
                continue
            if path == zero_patch:
                continue
            subprocess.run(["git", "-C", str(checkout), "apply", "--reverse", "--check", str(path)], check=True)
        subprocess.run(["git", "-C", str(checkout), "apply", str(retirement_patch)], check=True)
        subprocess.run(["git", "-C", str(checkout), "apply", str(stage_patch)], check=True)
        subprocess.run(["git", "-C", str(checkout), "apply", str(present_resource_patch)], check=True)
        subprocess.run(["git", "-C", str(checkout), "apply", str(depth_retention_patch)], check=True)
        subprocess.run(["git", "-C", str(checkout), "apply", str(upload_batching_patch)], check=True)
        subprocess.run(["git", "-C", str(checkout), "apply", str(zero_patch)], check=True)
        subprocess.run(["git", "-C", str(checkout), "apply", str(phase_patch)], check=True)
        subprocess.run(["git", "-C", str(checkout), "apply", str(residency_patch)], check=True)
        subprocess.run(["git", "-C", str(checkout), "apply", str(writer_patch)], check=True)
        subprocess.run(["git", "-C", str(checkout), "apply", str(padded_patch)], check=True)
        for patch in upstream_patches:
            subprocess.run(["git", "-C", str(checkout), "apply", str(patch)], check=True)
        return {name: (checkout / name).read_text(encoding="utf-8") for name in SOURCE_FILES}


if __name__ == "__main__":
    patched_sources(Path(__file__).resolve().parents[2])
    print("PASS: ordered DXMT patches and native-only fallback apply cleanly, repeat safely and upgrade legacy fallback")
