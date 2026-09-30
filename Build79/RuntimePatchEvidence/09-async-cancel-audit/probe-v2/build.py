from pathlib import Path
import hashlib, importlib.util, json, subprocess
H=Path(__file__).resolve().parent; B=H/'build'; B.mkdir(exist_ok=True)
exports=['GetModuleFileNameW','GetCurrentProcessId','GetLastError','CreateNamedPipeW','ConnectNamedPipe','CreateFileW','CreateEventW','ResetEvent','ReadFile','WriteFile','CancelIo','CancelIoEx','GetOverlappedResult','WaitForSingleObject','SleepEx','FlushFileBuffers','CloseHandle','ExitProcess']
(B/'kernel32.def').write_text('LIBRARY kernel32.dll\nEXPORTS\n'+'\n'.join(exports)+'\n')
subprocess.run(['llvm-dlltool','-m','i386:x86-64','-d',str(B/'kernel32.def'),'-l',str(B/'kernel32.lib')],check=True)
subprocess.run(['clang','-target','x86_64-pc-windows-msvc','-std=c11','-ffreestanding','-fno-builtin','-fno-stack-protector','-funwind-tables','-O2','-Wall','-Wextra','-Werror','-c',str(H/'AsyncCancelV2.c'),'-o',str(B/'AsyncCancelV2.obj')],check=True)
exe=B/'MadeiraAsyncCancelV2.exe'
subprocess.run(['lld-link','/nodefaultlib','/machine:x64','/timestamp:0','/entry:mainCRTStartup','/subsystem:console','/out:'+str(exe),str(B/'AsyncCancelV2.obj'),str(B/'kernel32.lib')],check=True)
s=importlib.util.spec_from_file_location('link_builder',H.parents[1]/'legacy/content-session-launcher/build.py');m=importlib.util.module_from_spec(s);s.loader.exec_module(m)
m.NAME=exe.name
link=m.shortcut(exe.stat().st_size)
old='BeamNG 0.34.2 - Content launch'.encode('utf-16le');new='Async cancellation test v2'.encode('utf-16le')
import struct
old=struct.pack('<H',len(old)//2)+old;new=struct.pack('<H',len(new)//2)+new
assert link.count(old)==1
lnk=B/'Async cancellation test v2.lnk';lnk.write_bytes(link.replace(old,new))
report={p.name:{'bytes':p.stat().st_size,'sha256':hashlib.sha256(p.read_bytes()).hexdigest()} for p in (exe,lnk)}
(B/'manifest.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report,indent=2))
