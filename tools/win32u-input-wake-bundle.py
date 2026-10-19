#!/usr/bin/env python3
"""Bind input wake registration and idle-wait fallback to native Win32u."""
from pathlib import Path
import argparse
import hashlib
import importlib.util
import json
import subprocess

parser=argparse.ArgumentParser(description=__doc__)
parser.add_argument('--root',type=Path,required=True)
parser.add_argument('--stamp',action='store_true')
parser.add_argument('--base',type=Path)
args=parser.parse_args();root=args.root.resolve()
def module(name):
    spec=importlib.util.spec_from_file_location(name,root/('tools/'+name+'.py'))
    value=importlib.util.module_from_spec(spec);spec.loader.exec_module(value);return value
def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
archive=root/'app/Madeira/libwin32u_unix.a'
objects=module('nutssync-bundle').members(root,archive)
ar=module('ntdll-signal-bundle').ar_command()
driver=subprocess.check_output([*ar,'p',str(archive),'driver.o'])
message=subprocess.check_output([*ar,'p',str(archive),'message.o'])
assert b'input-wake-v1' in driver and b'winios_input_wake_create' in driver
assert b'winios_drv_input_wake_ready' in message,'message-wait fallback was not rebuilt'
sources=('build/win32u-unix/driver_ios.c','build/win32u-unix/message_ios.c',
         'app/Madeira/Winios/InputWakeQueue.h')
record={'format':'win32u-input-wake-v1','archive_sha256':sha(archive),'members':objects,
        'sources':{name:hashlib.sha256((root/name).read_bytes().replace(b'\r\n',b'\n')).hexdigest() for name in sources}}
receipt=root/'build/win32u-unix/input-wake.json'
if args.stamp:
    record['producer_commit']=subprocess.check_output(['git','-C',str(root),'rev-parse','HEAD'],text=True).strip()
    if args.base:
        old=module('nutssync-bundle').members(root,args.base)
        assert old.keys()==objects.keys(),'Win32u archive member set changed'
        changed=sorted(name for name in old if old[name]!=objects[name])
        assert changed==['driver.o','message.o'],('unrelated Win32u change',changed)
        record.update(base_archive_sha256=sha(args.base),changed_members=changed,preserved_members=len(objects)-2)
    receipt.write_text(json.dumps(record,indent=2)+'\n',encoding='utf-8')
else:
    previous=json.loads(receipt.read_text(encoding='utf-8'))
    for key,value in record.items():assert previous[key]==value,('stale input wake runtime',key)
print('PASS: Win32u input wake registration, message-wait fallback and native object identities match')
