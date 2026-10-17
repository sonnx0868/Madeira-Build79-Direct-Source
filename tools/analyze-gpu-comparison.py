#!/usr/bin/env python3
"""Validate an exported comparison and obtain Apple's GPU debugger inventories.

Run on a folder extracted from Madeira's ZIP. No undocumented trace parser,
replay-derived device clock estimate, or original-device timing is fabricated.
"""
from pathlib import Path
import argparse, collections, hashlib, json, platform, shutil, statistics, struct, subprocess

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

def command_inventory(folder, apple_output, kind):
    """Pair Apple's decoded arguments/enum names with the original records.

    This inspects command structure; it does not estimate GPU execution times.
    Reject a version/reader mismatch instead of silently guessing an API label.
    """
    trace=folder/(kind+'.gputrace')
    raw=(trace/'capture').read_bytes()
    assert raw[:4]==b'MTSP' and len(raw)<=32*1024*1024,'unsupported command stream'
    names=json.loads((apple_output/(kind+'-apple-function-names.json')).read_text(encoding='utf-8'))
    lines=(apple_output/(kind+'-apple-frames.txt')).read_text(encoding='utf-8').splitlines()
    position=8;codes=[]
    while position<len(raw):
        assert position+8<=len(raw),'truncated command header'
        size,code=struct.unpack_from('<Ii',raw,position)
        assert size>=8 and position+size<=len(raw),'invalid command boundary'
        assert str(code) in names and names[str(code)].startswith('kDYFE'),'missing Apple API label'
        codes.append(names[str(code)][5:]);position+=size
    assert position==len(raw) and len(codes)==len(lines),'Apple listing/record counts differ'
    counts=collections.Counter(codes);named=[];pipelines={};dispatches=[];indices=0
    for name,line in zip(codes,lines):
        named.append(name+' '+line)
        if name=='MTLComputeCommandEncoder_setComputePipelineState':
            values=line.split('(null)(',1)[1].rstrip(')').split(', ')
            pipelines[values[0]]=values[1]
        if name=='MTLComputeCommandEncoder_dispatchThreadgroups_threadsPerThreadgroup':
            receiver,arguments=line.split('(null)(',1)[1].rstrip(')').split(', ',1)
            assert receiver in pipelines,'compute dispatch has no pipeline in this capture'
            dispatches.append([pipelines[receiver],arguments])
        if name.startswith('MTLRenderCommandEncoder_drawIndexedPrimitives_indexCount_'):
            values=line.split('(null)(',1)[1].rstrip(')').split(', ')
            indices+=int(values[2].removesuffix('ul'))*int(values[6].removesuffix('ul'))
    inventory={'commands':len(codes),'renderEncoders':counts['MTLCommandBuffer_renderCommandEncoderWithDescriptor'],
               'drawCalls':sum(n for name,n in counts.items() if '_draw' in name),
               'indexedIndicesTimesInstances':indices,'computeDispatches':len(dispatches),
               'computePipelineIDs':len({row[0] for row in dispatches}),
               'computeSignatureSHA256':hashlib.sha256(json.dumps(dispatches).encode()).hexdigest(),
               'presentCalls':counts['MTLCommandBuffer_presentDrawable'],
               'commandBufferCommits':counts['MTLCommandBuffer_commit'],
               'GPUWaitEventOrFenceCalls':sum(n for name,n in counts.items() if 'encodeWait' in name),
               'CPUWaitUntilCompletedCalls':counts['MTLCommandBuffer_waitUntilCompleted'],
               'APIHistogram':dict(sorted(counts.items())),
               'timingMeaning':'Static API structure; no original-device per-pass timing or hardware utilization inferred'}
    return inventory,named

def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('folder',type=Path);p.add_argument('--output',type=Path,required=True)
    p.add_argument('--replay',action='store_true',help='Use Apple gpudebug on macOS; may take several minutes per trace')
    p.add_argument('--apple-output',type=Path,help='Private Mac artifacts containing Apple enum labels and capture-only API listings')
    a=p.parse_args();result=inspect(a.folder);a.output.mkdir(parents=True,exist_ok=True)
    if a.apple_output:
        for kind in ('smooth','slow'):
            commands,lines=command_inventory(a.folder,a.apple_output,kind)
            result['samples'][kind]['commands']=commands
            (a.output/(kind+'-commands-named.txt')).write_text('\n'.join(lines)+'\n',encoding='utf-8')
        result['computeSequenceIdentical']=result['samples']['smooth']['commands']['computeSignatureSHA256']==result['samples']['slow']['commands']['computeSignatureSHA256']
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
