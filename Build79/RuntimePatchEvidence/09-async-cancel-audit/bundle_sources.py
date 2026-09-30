from pathlib import Path
import zipfile
ROOT=Path(__file__).resolve().parents[3];H=Path(__file__).resolve().parent
files=[p for p in H.iterdir()if p.is_file()and p.suffix in ['.py','.c','.h','.patch','.md','.json','.asm']]
files += [p for p in (H/'native').iterdir()if p.is_file()and p.suffix in ['.py','.S','.md','.json','.asm']]
files += [p for p in (H/'build').iterdir()if p.is_file()and p.suffix in ['.c','.h','.json','.log']]
files += [p for p in (H/'probe').iterdir()if p.is_file()and p.suffix in ['.json','.def']]
files += [ROOT/'work/vulkan/build-audit/wine-source'/p for p in ['server/async.c','server/thread.c','server/fd.c','include/wine/list.h']]
files += [ROOT/'work/beamng/guest-exit-fix/build.py',ROOT/'work/beamng/legacy/content-session-launcher/build.py']
with zipfile.ZipFile(ROOT/'outputs/ipad-ui-source/ASYNC-CANCEL-v1-source.zip','w',zipfile.ZIP_DEFLATED)as z:
 for p in sorted(set(files)):z.write(p,p.relative_to(ROOT))
print('Source bundle refreshed:',len(files),'files')
