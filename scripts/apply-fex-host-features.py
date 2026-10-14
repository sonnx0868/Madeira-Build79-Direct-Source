#!/usr/bin/env python3
"""Integrate the existing upstream Windows/iOS host-feature implementation."""
import argparse, subprocess
from pathlib import Path
root = Path(__file__).resolve().parents[1]
parser = argparse.ArgumentParser()
parser.add_argument('--source', type=Path, default=root/'FEX')
args = parser.parse_args()
patch = root/'patches/fex-upstream-host-features.patch'
command = ['git', '-C', str(args.source), 'apply']
if subprocess.run(command+['--reverse','--check',str(patch)],capture_output=True).returncode == 0:
    print('Upstream FEX host features already integrated')
elif subprocess.run(command+['--check',str(patch)],capture_output=True).returncode == 0:
    subprocess.run(command+[str(patch)],check=True)
    print('Integrated upstream FEX CPU feature selection: be778d7 / aaf73a9')
else:
    raise SystemExit('FEX host-feature source drift; refusing to mix implementations')
