#!/usr/bin/env python3
"""Validate an exported comparison and obtain Apple's GPU debugger inventories.

Run on a folder extracted from Madeira's ZIP. No undocumented trace parser,
replay-derived device clock estimate, or original-device timing is fabricated.
"""
from pathlib import Path
import argparse, json, platform, shutil, statistics, subprocess

def inspect(folder):
    folder=folder.resolve()
    assert not (folder/'manifest.json').is_symlink(),'manifest symlink rejected'
    info=json.loads((folder/'manifest.json').read_text(encoding='utf-8'))
    assert info.get('format')=='madeira-gpu-comparison-v1','unrecognized comparison format'
    summary={'context':info.get('context',{}),'referenceFPS':info.get('referenceFPS'),'samples':{},
             'limits':['Capture changes execution timing. Pre-capture FPS is the unrecorded game window.',
                       'Replay timing on a Mac does not establish the original iPad GPU clock or utilization.',
                       'Matching draw/pass counts do not prove an identical scene. Compare resources and commands too.']}
    for kind in ('smooth','slow'):
        sample=info.get(kind,{})
        assert sample.get('complete') is True and sample.get('frames')==2,f'{kind}: no two-frame completed capture'
        assert sample.get('file')==kind+'.gputrace',f'{kind}: unexpected trace filename'
        trace=folder/(kind+'.gputrace')
        assert trace.exists() and not trace.is_symlink(),f'{kind}: missing trace or symlink'
        files=[trace] if trace.is_file() else list(trace.rglob('*'))
        assert len(files)<=100000,'trace file count limit'
        assert not any(p.is_symlink() for p in files),'trace symlink rejected'
        sizes=[p.stat().st_size for p in files if p.is_file()]
        assert sum(sizes)>0,f'{kind}: empty trace'
        fps=[x['fps'] for x in info.get(kind+'BeforeCapture',[]) if isinstance(x.get('fps'),(int,float))]
        summary['samples'][kind]={'bytes':sum(sizes),'files':len(sizes),'buffers':sample.get('buffers'),
             'captureUptime':sample.get('captureUptime'),'preCaptureFPS':fps,
             'preCaptureFPSMedian':statistics.median(fps[-5:]) if fps else None}
    return summary

def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('folder',type=Path);p.add_argument('--output',type=Path,required=True)
    p.add_argument('--replay',action='store_true',help='Use Apple gpudebug on macOS; may take several minutes per trace')
    a=p.parse_args();result=inspect(a.folder);a.output.mkdir(parents=True,exist_ok=True)
    tool=shutil.which('gpudebug')
    if not tool and platform.system()=='Darwin':
        found=subprocess.run(['xcrun','--find','gpudebug'],capture_output=True,text=True)
        if found.returncode==0:tool=found.stdout.strip()
    result['gpudebugAvailable']=bool(tool)
    if a.replay:
        if not tool:raise SystemExit('Apple gpudebug is required for replay analysis. Use a compatible macOS/Xcode runner.')
        for kind in ('smooth','slow'):
            # The documented self-describing tree is the starting point. Keep
            # complete output to inspect its advertised performance actions.
            command=[tool,'--oneshot','-t',str((a.folder/(kind+'.gputrace')).resolve()),
                     '-c','list','-c','go performance','-c','list --all']
            with (a.output/(kind+'-gpudebug.txt')).open('w',encoding='utf-8') as log:
                run=subprocess.run(command,stdout=log,stderr=subprocess.STDOUT,timeout=300)
            result['samples'][kind]['gpudebugExitCode']=run.returncode
    (a.output/'comparison.json').write_text(json.dumps(result,indent=2)+'\n',encoding='utf-8')
    print(json.dumps({'validated':True,'replayRequested':a.replay,'output':str(a.output.resolve()),
                      'smoothBytes':result['samples']['smooth']['bytes'],'slowBytes':result['samples']['slow']['bytes']}))

if __name__=='__main__':main()
