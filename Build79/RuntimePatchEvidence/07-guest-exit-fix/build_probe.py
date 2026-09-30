from pathlib import Path
import hashlib, importlib.util, json, subprocess
H=Path(__file__).resolve().parent; B=H/'probe'; B.mkdir(exist_ok=True)
exports=['GetCommandLineW','GetModuleFileNameW','GetCurrentProcess','TerminateProcess','ExitProcess','CreateFileW','WriteFile','FlushFileBuffers','CloseHandle','CreateProcessW','WaitForSingleObject','GetExitCodeProcess']
(B/'kernel32.def').write_text('LIBRARY kernel32.dll\nEXPORTS\n'+'\n'.join(exports)+'\n')
subprocess.run(['llvm-dlltool','-m','i386:x86-64','-d',str(B/'kernel32.def'),'-l',str(B/'kernel32.lib')],check=True)
subprocess.run(['clang','-target','x86_64-pc-windows-msvc','-std=c11','-ffreestanding','-fno-builtin','-fno-stack-protector','-funwind-tables','-O2','-Wall','-Wextra','-Werror','-c',str(H/'GuestExit.c'),'-o',str(B/'GuestExit.obj')],check=True)
exe=B/'MadeiraGuestExit.exe'
subprocess.run(['lld-link','/nodefaultlib','/machine:x64','/timestamp:0','/entry:mainCRTStartup','/subsystem:console','/out:'+str(exe),str(B/'GuestExit.obj'),str(B/'kernel32.lib')],check=True)
s=importlib.util.spec_from_file_location('link_builder',H.parent/'legacy/content-session-launcher/build.py');m=importlib.util.module_from_spec(s);s.loader.exec_module(m)
m.NAME=exe.name
link=m.shortcut(exe.stat().st_size)
old='BeamNG 0.34.2 - Content launch'.encode('utf-16le');new='Guest process exit test'.encode('utf-16le')
import struct
old=struct.pack('<H',len(old)//2)+old;new=struct.pack('<H',len(new)//2)+new
assert link.count(old)==1
lnk=B/'Guest process exit test.lnk';lnk.write_bytes(link.replace(old,new))
report={p.name:{'bytes':p.stat().st_size,'sha256':hashlib.sha256(p.read_bytes()).hexdigest()} for p in (exe,lnk)}
(B/'manifest.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report,indent=2))
