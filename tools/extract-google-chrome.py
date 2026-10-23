#!/usr/bin/env python3
"""Read a pinned official Chrome MSI into a test folder without running its installers."""
from pathlib import Path
import argparse,hashlib,json,shutil,struct,subprocess

p=argparse.ArgumentParser(description=__doc__)
p.add_argument('--archive',type=Path,required=True)
p.add_argument('--sha256',required=True)
p.add_argument('--out',type=Path,required=True)
p.add_argument('--sevenzip',default=shutil.which('7zz') or shutil.which('7z'))
args=p.parse_args()
if not args.sevenzip:raise SystemExit('Standalone 7zz/7z archive reader required')
digest=hashlib.sha256(args.archive.read_bytes()).hexdigest()
if digest!=args.sha256:raise SystemExit('Chrome installer hash differs from the pinned Google download')
out=args.out.resolve()
if out.exists():raise SystemExit('Choose a fresh extraction folder; existing files are preserved')
out.mkdir(parents=True)
def unpack(source,target,member=None):
    cmd=[args.sevenzip,'x',str(source),'-o'+str(target),'-y']
    if member:cmd.append(member)
    subprocess.run(cmd,check=True)

unpack(args.archive,out/'msi','Binary.GoogleChromeInstaller')
installer=out/'msi/Binary.GoogleChromeInstaller'
assert installer.is_file()
unpack(installer,out/'outer')
if (out/'outer/updater.7z').is_file():
    unpack(out/'outer/updater.7z',out/'offline')
    candidates=list((out/'offline').rglob('*chrome_installer.exe'))
    assert len(candidates)==1,candidates
    unpack(candidates[0],out/'installer')
    archive=out/'installer/chrome.7z'
else:archive=out/'outer/chrome.7z'
assert archive.is_file()
unpack(archive,out/'program')
executables=list((out/'program').rglob('chrome.exe'))
assert len(executables)==1,executables
exe=executables[0];data=exe.read_bytes()
assert data[:2]==b'MZ'
offset=struct.unpack_from('<I',data,60)[0]
assert data[offset:offset+4]==b'PE\0\0'
machine=struct.unpack_from('<H',data,offset+4)[0]
assert machine==0x14c,hex(machine)
record={'distribution':'Google Chrome Stable Windows x86','archiveSHA256':digest,
        'installerExecuted':False,'executable':str(exe),'machine':'0x14c',
        'exeSHA256':hashlib.sha256(data).hexdigest(),
        'deviceExecutionVerified':False,'GoogleSyncVerified':False,'ChromeExtensionsVerified':False}
(out/'extracted.json').write_text(json.dumps(record,indent=2)+'\n')
print('PASS: pinned Google Chrome Stable32 extracted without executing MSI, Updater or Chrome installer')
print('Chrome executable:',exe)
