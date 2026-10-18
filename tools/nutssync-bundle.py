#!/usr/bin/env python3
"""Verify the actual linked NutsSync producer and preserve unrelated native objects."""
import argparse, hashlib, json, subprocess
from pathlib import Path

SOURCES = ('build/madsync/madsync_bridge.c', 'build/madsync/nutssync_adapter.c',
           'build/madsync/madsync.c', 'build/madsync/linux/ntsync.h',
           'build/madsync/suspend_guard.h', 'build/madsync/refresh.sh', 'build/madeira_cfg.h',
           'build/ntdll-unix/build.sh', 'build/wineserver/build.sh', 'build/wineserver/ios_symbol_aliases.h',
           'third_party/nutssync/nutssync.h', 'third_party/nutssync/source-origin.json',
           'patches/wine-nutssync.patch', 'build/wineserver/mach_ios.c',
           'wine/server/thread.c', 'wine/server/process.c', 'wine/server/inproc_sync.c',
           'wine/dlls/ntdll/unix/sync.c', 'build/ntdll-unix/thread_ios.c',
           'build/ntdll-unix/ios_yield_policy.h', 'patches/wine-yield-budget.patch')

def sha(path): return hashlib.sha256(path.read_bytes()).hexdigest()
def members(root, path):
    result={}; long_names=b''
    with path.open('rb') as f:
        assert f.read(8)==b'!<arch>\n'
        while header:=f.read(60):
            assert len(header)==60 and header[-2:]==b'`\n'
            name=header[:16].decode('ascii').strip(); size=int(header[48:58]); data=f.read(size)
            assert len(data)==size
            if size&1: assert len(f.read(1))==1
            if name.startswith('#1/'):
                length=int(name[3:]); name=data[:length].rstrip(b'\0').decode(); data=data[length:]
            elif name=='//': long_names=data; continue
            elif name.startswith('/') and name[1:].isdigit():
                name=long_names[int(name[1:]):].split(b'/\n',1)[0].decode()
            else: name=name.rstrip('/')
            if name.startswith('__.SYMDEF') or name in ('','/','/SYM64'): continue
            assert name.endswith('.o') and name not in result, ('invalid object',name)
            result[name]=hashlib.sha256(data).hexdigest()
    return result

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--root', type=Path, required=True); p.add_argument('--stamp', action='store_true')
    p.add_argument('--base-ntdll', type=Path); p.add_argument('--base-server', type=Path)
    p.add_argument('--scheduling-refresh', action='store_true')
    a=p.parse_args(); root=a.root.resolve()
    archives={'ntdll':root/'app/Madeira/libntdll_unix.a', 'server':root/'app/Madeira/libwineserver.a'}
    current={k:members(root,v) for k,v in archives.items()}
    assert b'nutssync-v1' in archives['ntdll'].read_bytes(), 'NutsSync dispatcher is absent'
    record={'format':'madeira-nutssync-v1', 'archives':{k:{'sha256':sha(v),'members':current[k]} for k,v in archives.items()},
            'sources':{s:hashlib.sha256((root/s).read_bytes().replace(b'\r\n',b'\n')).hexdigest() for s in SOURCES}}
    path=root/'build/madsync/native-receipt.json'
    if a.stamp:
        record['producer_commit']=subprocess.check_output(['git','-C',str(root),'rev-parse','HEAD'],text=True).strip()
        assert bool(a.base_ntdll)==bool(a.base_server), 'both base archives are required'
        comparisons = ([('ntdll',a.base_ntdll,{'sync.o'}),('server',a.base_server,set())]
                       if a.scheduling_refresh else
                       [('ntdll',a.base_ntdll,{'madsync.o','sync.o','thread.o'}),
                        ('server',a.base_server,{'mach_ios.o','process_ios.o','thread.o','inproc_sync.o'})]) if a.base_ntdll else []
        assert not a.scheduling_refresh or a.base_ntdll, 'scheduling refresh requires both base archives'
        for name,base,expected in comparisons:
            before=members(root,base); after=current[name]
            assert before.keys()==after.keys(), ('archive membership changed',name, before.keys()^after.keys())
            changed={n for n in before if before[n]!=after[n]}
            assert changed==expected, ('unexpected native changes',name,changed)
            record['archives'][name].update(base_sha256=sha(base),changed=sorted(changed),preserved=len(before)-len(changed))
        path.parent.mkdir(parents=True,exist_ok=True)
        path.write_text(json.dumps(record,indent=2)+'\n')
    else:
        actual=json.loads(path.read_text())
        assert actual['sources']==record['sources'], 'stale NutsSync source receipt'
        for name,archive in record['archives'].items():
            assert all(actual['archives'][name][k]==v for k,v in archive.items()), ('stale native sync',name)
    print('PASS NutsSync source, both native archives and preserved objects match')

if __name__=='__main__': main()
