#!/usr/bin/env python3
"""Survey a local PE for static ARM64 recompilation research; never execute it.

Reports file identity, executable sections, unwind function ranges and dynamic
runtime dependencies. This produces a plan, not translated machine code.
"""
import argparse
import hashlib
import json
import struct
from pathlib import Path

MAX_FILE = 256 * 1024 * 1024


class PE:
    def __init__(self, data):
        self.data = data
        if len(data) > MAX_FILE or len(data) < 64 or data[:2] != b"MZ":
            raise ValueError("Not a bounded PE file")
        pe = self.u32(60)
        if pe > 16 * 1024 * 1024 or self.read(pe, 4) != b"PE\0\0":
            raise ValueError("Invalid PE signature")
        self.machine, count = struct.unpack("<HH", self.read(pe + 4, 4))
        opt_size = self.u16(pe + 20)
        optional = pe + 24
        magic = self.u16(optional)
        if magic not in (0x10b, 0x20b) or not 1 <= count <= 96:
            raise ValueError("Unsupported PE headers")
        minimum, directory = (112, 112) if magic == 0x20b else (96, 96)
        if opt_size < minimum or opt_size > 4096:
            raise ValueError("Invalid optional header")
        self.read(optional, opt_size)
        self.entry = self.u32(optional + 16)
        self.dll_characteristics = self.u16(optional + 70)
        self.directories = []
        directory_count = min(self.u32(optional + directory - 4), 16)
        if directory + directory_count * 8 > opt_size:
            raise ValueError("Directory table exceeds optional header")
        for i in range(directory_count):
            self.directories.append(struct.unpack("<II", self.read(optional + directory + i * 8, 8)))
        self.sections = []
        for i in range(count):
            off = optional + opt_size + i * 40
            header = self.read(off, 40)
            size, rva, raw_size, raw = struct.unpack_from("<IIII", header, 8)
            flags = struct.unpack_from("<I", header, 36)[0]
            self.read(raw, raw_size)
            self.sections.append(dict(name=header[:8].split(b"\0")[0].decode("ascii", "replace"),
                rva=rva, virtual_size=size, raw_size=raw_size, raw=raw,
                executable=bool(flags & 0x20000000), writable=bool(flags & 0x80000000)))

    def read(self, offset, size):
        if offset < 0 or size < 0 or offset + size > len(self.data):
            raise ValueError("PE range outside file")
        return self.data[offset:offset + size]

    def u16(self, offset):
        return struct.unpack("<H", self.read(offset, 2))[0]

    def u32(self, offset):
        return struct.unpack("<I", self.read(offset, 4))[0]

    def offset(self, rva, size=1):
        for s in self.sections:
            delta = rva - s["rva"]
            if delta >= 0 and delta + size <= s["raw_size"]:
                return s["raw"] + delta
        raise ValueError("RVA not backed by a file section")

    def directory(self, i):
        return self.directories[i] if i < len(self.directories) else (0, 0)

    def imports(self):
        rva, size = self.directory(1)
        if not rva or not size:
            return []
        out = []
        for i in range(min(size // 20, 1024)):
            descriptor = self.read(self.offset(rva + i * 20, 20), 20)
            if descriptor == bytes(20):
                break
            name_rva = struct.unpack_from("<I", descriptor, 12)[0]
            name = bytearray()
            for j in range(256):
                b = self.read(self.offset(name_rva + j), 1)[0]
                if not b:
                    break
                name.append(b)
            else:
                raise ValueError("Unterminated import name")
            out.append(name.decode("ascii", "replace").lower())
        return sorted(set(out))

    def functions(self):
        # AMD64's .pdata gives many function boundaries, not a complete CFG.
        rva, size = self.directory(3)
        if self.machine != 0x8664 or not rva or not size:
            return []
        if size % 12 or size > 12 * 200000:
            raise ValueError("Invalid AMD64 exception table")
        table = self.read(self.offset(rva, size), size)
        out = []
        for begin, end, unwind in struct.iter_unpack("<III", table):
            if not begin and not end and not unwind:
                continue
            if begin >= end:
                raise ValueError("Invalid unwind function range")
            section = next((s for s in self.sections if s["executable"] and
                begin >= s["rva"] and end <= s["rva"] + s["raw_size"]), None)
            if not section:
                continue
            offset = self.offset(begin, end - begin)
            code = self.read(offset, end - begin)
            out.append(dict(rva=begin, size=end - begin, unwind_rva=unwind,
                            writable=section["writable"], sha256=hashlib.sha256(code).hexdigest()))
        return out


def survey(path):
    path = Path(path)
    if not path.is_file() or path.stat().st_size > MAX_FILE:
        raise ValueError("Input must be a PE file no larger than 256 MiB")
    data = path.read_bytes()
    pe = PE(data)
    functions = pe.functions()
    imports = pe.imports()
    # A bounded look beside this module is an engine hint, not proof of the
    # running engine or proof that a dependency's code is statically known.
    siblings = []
    for i, sibling in enumerate(path.parent.iterdir()):
        if i >= 1024:
            break
        siblings.append(sibling.name.lower())
    hints = []
    names = set(imports + siblings)
    if "unityplayer.dll" in names:
        hints.append("Unity: include UnityPlayer.dll in the survey; the EXE may be only a launcher")
    if any("mono" in n for n in names) or any(n.endswith("_data") for n in names):
        hints.append("Possible managed runtime: inspect Mono/IL2CPP before choosing AOT boundaries")
    if "gameassembly.dll" in names:
        hints.append("IL2CPP native module detected; survey GameAssembly.dll separately")
    if any("python" in n for n in names):
        hints.append("Python runtime detected: native-module AOT does not compile interpreted game scripts")
    candidates = [f for f in functions if not f["writable"]]
    return dict(schema=1, kind="static-aot-survey", executable=path.name,
        sha256=hashlib.sha256(data).hexdigest(), machine=f"0x{pe.machine:04x}",
        entry_rva=pe.entry, aslr=bool(pe.dll_characteristics & 0x40),
        sections=pe.sections, imports=imports, delayed_import_directory=bool(pe.directory(13)[0]),
        tls_directory=bool(pe.directory(9)[0]), clr_directory=bool(pe.directory(14)[0]),
        unwind_function_count=len(functions), readonly_function_count=len(candidates),
        readonly_function_bytes=sum(f["size"] for f in candidates),
        candidate_ranges=candidates[:256], candidate_ranges_truncated=len(candidates) > 256,
        engine_hints=hints,
        limitations=["No ARM64 code is emitted or executed by this survey",
            "Unwind ranges omit leaf functions and don't prove indirect-call targets",
            "Dynamic DLL loading, ASLR relocations, SEH and generated/self-modifying code still require a runtime",
            "No hotness profile: listed ranges are candidates for analysis, not measured hot functions"])


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("pe", type=Path)
    parser.add_argument("--output", type=Path)
    args = parser.parse_args()
    try:
        result = survey(args.pe)
    except (ValueError, OSError, struct.error) as error:
        parser.exit(1, f"AOT survey failed: {error}\n")
    text = json.dumps(result, ensure_ascii=False, indent=2) + "\n"
    if args.output:
        args.output.write_text(text, encoding="utf-8")
    else:
        print(text, end="")


if __name__ == "__main__":
    main()
