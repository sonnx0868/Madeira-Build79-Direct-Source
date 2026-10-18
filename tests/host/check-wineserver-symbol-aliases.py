#!/usr/bin/env python3
"""Check the production C linkage map without macro-renaming Wine structures."""
from pathlib import Path
import argparse, subprocess, tempfile, sys

root=Path(__file__).resolve().parents[2]
p=argparse.ArgumentParser()
p.add_argument('--cc',default='clang');p.add_argument('--nm',default='nm');a=p.parse_args()
with tempfile.TemporaryDirectory(prefix='madeira-server-alias-') as tmp:
    folder=Path(tmp); source=folder/'provider.c'; obj=folder/'provider.o'
    source.write_text('struct get_window_thread_request { int number; };\nint user_shared_data=27;\nint get_window_thread(struct get_window_thread_request *r){return r->number+user_shared_data;}\n')
    flags=['-target','arm64-apple-ios17.0'] if sys.platform=='win32' else []
    subprocess.run([a.cc,*flags,'-Werror=unknown-pragmas','-Werror=ignored-pragmas',
                    '-include',str(root/'build/wineserver/ios_symbol_aliases.h'),'-c',str(source),'-o',str(obj)],check=True)
    symbols=subprocess.check_output([a.nm,'-g',str(obj)],text=True)
    names={line.split()[-1] for line in symbols.splitlines() if line.split()}
    assert '_ws_get_window_thread' in names and '_ws_user_shared_data' in names,names
    assert '_get_window_thread' not in names and '_user_shared_data' not in names,names
    if sys.platform=='darwin':
        client=folder/'client.c';binary=folder/'client'
        client.write_text('struct get_window_thread_request { int number; };\nextern int ws_user_shared_data;\nextern int ws_get_window_thread(struct get_window_thread_request*);\nint main(void){struct get_window_thread_request r={5};return ws_user_shared_data==27 && ws_get_window_thread(&r)==32 ? 0 : 1;}\n')
        subprocess.run([a.cc,str(obj),str(client),'-o',str(binary)],check=True)
        subprocess.run([str(binary)],check=True)
    print('PASS production Clang symbol aliases preserve Wine request identifiers and native linkage')
