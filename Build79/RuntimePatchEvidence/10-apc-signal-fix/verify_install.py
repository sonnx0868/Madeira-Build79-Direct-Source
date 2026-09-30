"""Verify/archive the retained signed staging bundle actually given to installer."""
from pathlib import Path
import hashlib,json,struct,zipfile
H=Path(__file__).resolve().parent;ROOT=H.parents[2]
sha=lambda b:hashlib.sha256(b).hexdigest()
def macho(data):
 magic=struct.unpack_from('>I',data)[0];start=0
 if magic==0xcafebabe:
  assert struct.unpack_from('>I',data,4)[0]==1
  cpu,sub,start,size,align=struct.unpack_from('>5I',data,8);assert cpu==0x100000c
 assert struct.unpack_from('<I',data,start)[0]==0xfeedfacf
 count=struct.unpack_from('<I',data,start+16)[0];p=start+32;sections={};signature=None
 for _ in range(count):
  cmd,size=struct.unpack_from('<II',data,p)
  if cmd==0x1d:signature=struct.unpack_from('<II',data,p+8)
  if cmd==0x19:
   nsects=struct.unpack_from('<I',data,p+64)[0]
   for j in range(nsects):
    q=p+72+80*j;name=data[q:q+16].rstrip(b'\0').decode();seg=data[q+16:q+32].rstrip(b'\0').decode()
    addr,n,off,alignment=struct.unpack_from('<QQII',data,q+32);flags=struct.unpack_from('<I',data,q+64)[0]
    if flags&255 not in (1,12,18):sections[(seg,name)]=data[start+off:start+off+n]
  p+=size
 return start,sections,signature
stage=list((H/'signing-stage').glob('plume_stage_*/Payload/Madeira.app'));assert len(stage)==1
bundle=stage[0];base=ROOT/'outputs/ipad-installers/Madeira-iPad-APC-Signal-v1-Test.ipa'
assert 'Installation complete!' in (H/'sign-install.log').read_text(errors='replace')
with zipfile.ZipFile(base)as z:
 intended=z.read('Payload/Madeira.app/Madeira');actual=(bundle/'Madeira').read_bytes()
 _,isects,_=macho(intended);at,asects,sig=macho(actual)
 assert isects==asects,'A native Mach-O section changed during signing'
 assert sig
 off,n=sig;blob=actual[at+off:at+off+n];magic,length,count=struct.unpack_from('>III',blob)
 assert magic==0xfade0cc0 and length<=n
 cd=None
 for i in range(count):
  typ,o=struct.unpack_from('>II',blob,12+i*8)
  if typ==0:cd=blob[o:o+struct.unpack_from('>I',blob,o+4)[0]]
 assert cd and struct.unpack_from('>I',cd)[0]==0xfade0c02
 hash_off,identifier,n_special,n_code,limit=struct.unpack_from('>IIIII',cd,16)
 hsize,htype,platform,page=struct.unpack_from('>BBBB',cd,36)
 assert htype==2 and hsize==32
 for i in range(n_code):
  piece=actual[at+i*(1<<page):at+min((i+1)*(1<<page),limit)]
  assert hashlib.sha256(piece).digest()==cd[hash_off+i*hsize:hash_off+(i+1)*hsize],('codeslot',i)
 modules=[]
 for suffix,digest in [('ntdll.dll','e43ae5a9bc90a02c3c01e5a451e9ab7f5570b2735b93215284868a279254c27f'),('xtajit64.dll','74032ec4c38a6da83e3dbe07a0a0abc62a58a823c4a193cc75dd24716239bfdb')]:
  names=[n for n in z.namelist()if n.endswith('/'+suffix)and sha(z.read(n))==digest];assert names
  for name in names:
   p=bundle/name.removeprefix('Payload/Madeira.app/');assert sha(p.read_bytes())==digest;modules.append(name)
target=ROOT/'outputs/ipad-installers/Madeira-iPad-APC-Signal-v1-Signed.ipa'
with zipfile.ZipFile(target,'w',zipfile.ZIP_DEFLATED)as z:
 for p in sorted(bundle.rglob('*')):
  assert not p.is_symlink(),p
  if p.is_file():z.write(p,Path('Payload/Madeira.app')/p.relative_to(bundle))
report={'installed':True,'installer_exit_code':0,'installation_complete_log_confirmed':True,
 'signed_archive':str(target),'signed_archive_sha256':sha(target.read_bytes()),'signed_native_sha256':sha(actual),
 'retained_installer_staging_bundle_archived':True,'every_native_macho_section_unchanged_by_signing':True,
 'native_sha256_code_directory_pages_verified':n_code,'preserved_cpu_tls_members':modules,
 'verification_limit':'Checks retained signed staging payload and native code hashes; does not extract the app binary back from iPad or independently establish CMS trust. Installer completion is the device installation evidence.',
 'cli_output_copy_not_used_as_signed_archive':True,'device_test_pending':True,'cef_fixed':False}
(H/'install-report.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report,indent=2))
