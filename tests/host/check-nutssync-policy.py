#!/usr/bin/env python3
"""Execute the Swift and native sync selectors against the same migration cases."""
from pathlib import Path
import os, subprocess, tempfile

root=Path(__file__).resolve().parents[2]
source=(root/'app/Madeira/Library.swift').read_text()
start=source.index('enum SyncEngine:'); end=source.index('\nstruct RuntimeMemorySyncSettings',start)
engine=source[start:end]
cases=[('', 'nutssync'), ('inproc-sync=1\n','madsync'), ('inproc-sync=0\n','wine'),
       ('env.MADEIRA_FASTSYNC=auto\n','nutssync'), ('env.MADEIRA_FASTSYNC=1\n','fastsync'),
       ('env.MADEIRA_FASTSYNC=0\n','wine'), ('inproc-sync=1\nenv.MADEIRA_FASTSYNC=auto\n','madsync'),
       ('sync-engine=nutssync\ninproc-sync=0\nenv.MADEIRA_FASTSYNC=0\n','nutssync'),
       ('sync-engine=fastsync\nenv.MADEIRA_FASTSYNC=auto\n','fastsync'),
       ('sync-engine=wine\ninproc-sync=1\n','wine'), ('sync-engine=unknown\n','nutssync')]
with tempfile.TemporaryDirectory(prefix='madeira-sync-policy-') as tmp:
    folder=Path(tmp); docs=folder/'Documents'; docs.mkdir()
    c=folder/'policy.c'; c.write_text('#include <stdio.h>\n#include "madeira_cfg.h"\nint main(void){printf("%d\\n",madeira_cfg_sync_engine());}\n')
    binary=folder/'policy'
    subprocess.run(['clang',str(c),'-I',str(root/'build'),'-o',str(binary)],check=True)
    swift=folder/'policy.swift'
    swift.write_text((root/'app/Madeira/MadeiraConfig.swift').read_text()+'\n'+engine+'\nprint(SyncEngine.current.rawValue)\n')
    env=dict(os.environ,HOME=str(folder),CFFIXED_USER_HOME=str(folder),MADEIRA_DOCS_DIR=str(docs))
    for config,expected in cases:
        (docs/'madeira.cfg').write_text(config)
        native=subprocess.check_output([str(binary)],text=True,env=env).strip()
        ui=subprocess.check_output(['swift',str(swift)],text=True,env=env).strip()
        assert ui==expected, (config,ui,expected)
        assert native==str({'madsync':0,'fastsync':1,'wine':2,'nutssync':3}[expected]), (config,native,expected)
    print('PASS eleven native/Swift migration cases; automatic NutsSync and explicit engine isolation')
