"""Run only the small lifecycle diagnostic in a unique Linux Wine prefix."""
from pathlib import Path
import json, os, shutil, subprocess, uuid
H=Path(__file__).resolve().parent
run=H/'probe-host'/uuid.uuid4().hex;run.mkdir(parents=True)
prefix=run/'prefix';folder=prefix/'drive_c/MadeiraDiagnostics/BeamNG034';folder.mkdir(parents=True)
shutil.copy2(H/'build/MadeiraAsyncCancelV2.exe',folder/'MadeiraAsyncCancelV2.exe')
env={**os.environ,'WINEPREFIX':str(prefix),'WINEARCH':'win64','DISPLAY':'','WINEDEBUG':'-all',
     'WINEDLLOVERRIDES':'winemenubuilder.exe=d'}
for key in ['MADEIRA_VULKAN_UNIXLIB','SteamAppId','SteamGameId','SteamAppPath']:env.pop(key,None)
try:
    boot=subprocess.run(['wineboot','-u'],env=env,capture_output=True,text=True,timeout=45)
    (run/'wineboot.log').write_text(boot.stdout+boot.stderr)
    assert boot.returncode==0
    test=subprocess.run(['wine',r'C:\MadeiraDiagnostics\BeamNG034\MadeiraAsyncCancelV2.exe'],env=env,capture_output=True,text=True,timeout=180)
    (run/'wine.log').write_text(test.stdout+test.stderr)
    log=(folder/'async-cancel-v2-test.log').read_text()
    assert test.returncode==0 and 'PASS all 48 rounds: 192 cancellations and 48 subsequent byte transfers.' in log
    assert log.count('PASS round ')==48 and 'FAIL' not in log
    report={'result':'PASS','48_cancellation_rounds':True,'log':log,'scope':'Linux Wine only, unique prefix; no game/device execution.','run':str(run)}
    (H/'probe-host-report.json').write_text(json.dumps(report,indent=2)+'\n')
    print(json.dumps(report,indent=2))
finally:
    subprocess.run(['wineserver','-k'],env=env,capture_output=True,timeout=8)
    subprocess.run(['wineserver','-w'],env=env,capture_output=True,timeout=8)
