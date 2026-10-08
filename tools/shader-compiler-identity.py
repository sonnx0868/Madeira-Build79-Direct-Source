#!/usr/bin/env python3
"""Generate a shader cache identity from compiler inputs, independent of build time."""
import argparse
import hashlib
from pathlib import Path

SOURCE_DIRECTORIES = ("dxmt/src/airconv", "dxmt/libs/DXBCParser", "dxmt/include", "madeira-d3d12/src")
SOURCE_FILES = ("build/dxmt-ios/build.sh", "build/madeira_cfg.h", "build/llvm-ios/static-libs.txt",
                "dxmt/src/d3d9/d3d9_shader.cpp", "dxmt/src/d3d11/d3d11_shader.cpp",
                "dxmt/src/d3d11/d3d11_shader.hpp", "dxmt/src/dxmt/dxmt_shader_cache.hpp")


def source_inputs(root):
    inputs = []
    for relative in SOURCE_DIRECTORIES:
        folder = root / relative
        files = sorted(p for p in folder.rglob("*") if p.is_file() and
                       p.suffix in (".c", ".cpp", ".mm", ".h", ".hpp", ".metal"))
        if not files:
            raise SystemExit(f"Missing shader compiler inputs: {folder}")
        inputs.extend((p.relative_to(root).as_posix(), p) for p in files)
    for relative in SOURCE_FILES:
        path = root / relative
        if not path.is_file():
            raise SystemExit(f"Missing shader compiler input file: {path}")
        inputs.append((relative, path))
    return inputs


def identity(inputs):
    digest = hashlib.sha256(b"madeira-shader-compiler-v1\0")
    for label, data in sorted(inputs):
        name = label.encode("utf-8")
        digest.update(len(name).to_bytes(8, "little"))
        digest.update(name)
        if isinstance(data, Path):
            digest.update(data.stat().st_size.to_bytes(8, "little"))
            with data.open("rb") as stream:
                for chunk in iter(lambda: stream.read(1024 * 1024), b""):
                    digest.update(chunk)
        else:
            digest.update(len(data).to_bytes(8, "little"))
            digest.update(data)
    return digest.hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, required=True)
    parser.add_argument("--output", type=Path)
    parser.add_argument("--llvm-libs", type=Path)
    parser.add_argument("--converter-include", type=Path)
    parser.add_argument("--toolchain")
    parser.add_argument("--check-source-inputs", action="store_true")
    args = parser.parse_args()
    root = args.root.resolve()
    inputs = source_inputs(root)
    if args.check_source_inputs:
        print(f"Validated {len(inputs)} shader compiler source inputs")
        return
    if args.output is None or args.llvm_libs is None or args.toolchain is None:
        parser.error("generation requires --output, --llvm-libs and --toolchain")
    inputs.append(("toolchain", args.toolchain.encode()))
    for target in (root / "build/llvm-ios/static-libs.txt").read_text().splitlines():
        if target.strip():
            lib = args.llvm_libs / f"lib{target.strip()}.a"
            inputs.append(("llvm/" + lib.name, lib))
    if args.converter_include:
        headers = sorted(p for p in args.converter_include.rglob("*") if p.is_file())
        if not headers:
            raise SystemExit("Missing Metal Shader Converter headers")
        inputs.extend(("converter/" + p.relative_to(args.converter_include).as_posix(), p) for p in headers)
    else:
        inputs.append(("converter", b"not-built"))
    stamp = identity(inputs)
    output = '#pragma once\n#define MADEIRA_SHADER_COMPILER_ID "' + stamp + '"\n'
    args.output.parent.mkdir(parents=True, exist_ok=True)
    if not args.output.exists() or args.output.read_text() != output:
        args.output.write_text(output, encoding="utf-8")
    print(f"shader-compiler-content-v1 {stamp}")


if __name__ == "__main__":
    main()
