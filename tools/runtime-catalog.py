#!/usr/bin/env python3
"""Maintainer tool: audit pinned official packages through the app's CAB decoder.

Packages stay outside the repo. No Windows executable is ever launched.
Requires pefile and a host build of RuntimeCabinet + the vendored CAB sources.
"""
import argparse, ctypes, hashlib, json, re, struct, tempfile
from pathlib import Path
import pefile
import xml.etree.ElementTree as ET

def digest(data): return hashlib.sha256(data).hexdigest()

def main():
    p = argparse.ArgumentParser()
    p.add_argument('packages', type=Path)
    p.add_argument('decoder', type=Path)
    p.add_argument('output', type=Path)
    args = p.parse_args()
    cab = ctypes.CDLL(str(args.decoder.resolve()))
    callback = ctypes.CFUNCTYPE(ctypes.c_int, ctypes.c_void_p, ctypes.c_char_p, ctypes.c_char_p, ctypes.c_uint32)
    cab.madeira_cab_extract.argtypes = [ctypes.c_char_p, ctypes.c_char_p, callback, ctypes.c_void_p]
    catalog = []
    for meta in sorted(args.packages.glob('*.json')):
        if not re.fullmatch(r'(vc(?:14|120|110|100|90|80)-x(?:86|64)|directx)', meta.stem): continue
        package = json.loads(meta.read_text())
        source = args.packages / (meta.stem + '.exe')
        data = source.read_bytes()
        assert digest(data) == package['sha256'] and len(data) == package['bytes']
        assert package['url'].startswith(('https://download.microsoft.com/', 'https://download.visualstudio.microsoft.com/'))
        package['id'] = meta.stem
        family = meta.stem.split('-')[0]
        package['family'] = family
        package['bits'] = 0 if family == 'directx' else (32 if meta.stem.endswith('x86') else 64)
        package['files'] = []
        seen = set()
        with tempfile.TemporaryDirectory(prefix='madeira-catalog-') as work:
            work = Path(work)
            def visit(content, depth=0, name=''):
                assert depth <= 4
                if (content[:2] == b'MZ' or (family == 'vc80' and content[:4] == b'\xd0\xcf\x11\xe0')) and depth < 3:
                    candidates = []
                    for m in re.finditer(b'MSCF', content):
                        start = m.start()
                        if start + 36 <= len(content):
                            size = struct.unpack_from('<I',content,start+8)[0]
                            if 36 <= size <= len(content)-start: candidates.append((size,start))
                    if candidates:
                        if content[:4] == b'\xd0\xcf\x11\xe0':
                            for size,start in candidates: visit(content[start:start+size],depth+1)
                            return
                        size,start = max(candidates)
                        return visit(content[start:start+size],depth+1)
                if content[:4] == b'MSCF':
                    folder = work / str(len(list(work.iterdir())))
                    folder.mkdir(); path=folder/'input.cab'; path.write_bytes(content)
                    rows=[]
                    @callback
                    def record(ctx,n,out,size): rows.append((n.decode('utf8',errors='replace'),Path(out.decode()).read_bytes())); return 0
                    result = cab.madeira_cab_extract(str(path).encode(),str(folder).encode(),record,None)
                    assert result == 0, (meta.stem,name,result)
                    for n,b in rows: visit(b,depth+1,n)
                    return
                if content[:2] != b'MZ':
                    if family in ['vc80','vc90'] and 'manifest' in name.lower():
                        try:
                            identity = next(x for x in ET.fromstring(content).iter() if x.tag.endswith('assemblyIdentity'))
                            assembly = identity.attrib['name']
                            bits = {'x86':32,'amd64':64}.get(identity.attrib.get('processorArchitecture'))
                            if re.fullmatch('Microsoft.VC'+family[2:]+r'\.(CRT|ATL|MFC|MFCLOC)',assembly) and bits == package['bits']:
                                dll=assembly.lower()+'.manifest';key=(dll,bits)
                                if key not in seen:
                                    seen.add(key);package['files'].append(dict(name=dll,bits=bits,sha256=digest(content),bytes=len(content),kind='manifest'))
                        except (ET.ParseError,StopIteration,KeyError): pass
                    return
                try:
                    pe = pefile.PE(data=content,fast_load=True)
                    pe.parse_data_directories(directories=[0])
                    dll = pe.get_string_at_rva(pe.DIRECTORY_ENTRY_EXPORT.struct.Name).decode('ascii').lower()
                    bits = {0x14c:32,0x8664:64}.get(pe.FILE_HEADER.Machine)
                except Exception: return
                if not bits or (package['bits'] and bits != package['bits']): return
                if family == 'directx':
                    allowed = re.fullmatch(r'(d3dx(?:9|10|11)_\d+|d3dcompiler_\d+|x3daudio\d_\d|xapofx\d_\d|xaudio2_\d|xactengine\d_\d|xinput1_[1-3])\.dll',dll)
                else:
                    version = '140' if family=='vc14' else family[2:]
                    allowed = re.fullmatch(r'(?:msvcp|msvcr|msvcm|concrt|vcamp|vccorlib|vcomp|vcruntime|mfc|mfcm|atl)'+version+r'(?:u|_[a-z0-9_]+|[a-z]{3})?\.dll',dll)
                if not allowed: return
                key=(dll,bits)
                if key in seen: return
                seen.add(key)
                package['files'].append(dict(name=dll,bits=bits,sha256=digest(content),bytes=len(content)))
            visit(data)
        assert package['files'], meta.stem
        package['files'].sort(key=lambda x:(x['bits'],x['name']))
        catalog.append(package)
        print(package['id'],len(package['files']))
    args.output.write_text(json.dumps(dict(revision=1,packages=catalog),indent=2)+'\n',encoding='utf8')

if __name__ == '__main__': main()
