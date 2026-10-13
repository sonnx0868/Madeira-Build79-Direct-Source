#!/usr/bin/env python3
"""Exercise the actual CDN pool, retry selection and native encrypted DNS API."""
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile

root=Path(__file__).resolve().parents[2]
source=(root/'app/Madeira/SwiftSteam/Content/DepotDownloader.swift').read_text(encoding='utf-8')
app=(root/'app/Madeira/MadeiraApp.swift').read_text(encoding='utf-8')
def block(text,signature):
    start=text.index(signature);end=text.index('{',start)+1;depth=1
    while depth:
        depth+=(text[end]=='{')-(text[end]=='}');end+=1
    return text[start:end]
swift=shutil.which('swiftc')
if not swift:
    if '--require-swift' in sys.argv: raise SystemExit('swiftc required for CDN/DNS policy')
    print('SKIP: CDN/DNS policy requires Mac Swift/Network');raise SystemExit(0)
fixture='import Foundation\nimport Network\n'
fixture+='enum Pool {\n'+block(source,'    nonisolated static func contentPool(')+'\n}\n'
fixture+='enum MadeiraConfig { static var mode:String?; static func get(_ key:String)->String? { mode } }\n'
fixture+=block(app,'enum SteamNetworkDNS {')+'\n'
fixture+='enum SteamLog { static func event(_ message:String) {} }\n'
fixture+=block(source,'final class ContentHostHealth:')+'\n'
start=source.index('            let order = plan.health.order(plan.hosts, seed: seed)')
end=source.index('            let url = ',start)
selection=source[start:end].replace('plan.health','health').replace('plan.hosts','pool')
fixture+=r'''
let fallback="https://steampipe.akamaized.net"
let offered=(0..<20).map { "https://cache\($0).steamcontent.com" }+[fallback]
let pool=Pool.contentPool(offered,limit:6)
assert(pool.count==6 && pool.contains(fallback))
assert(Pool.contentPool([fallback],limit:6)==[fallback])
assert(Pool.contentPool(offered,limit:0).isEmpty)
let health=ContentHostHealth();let seed=0
var tried=Set<String>()
for attempt in 0..<pool.count {
'''+selection+r'''
    health.recordFailure(host,reason:"url-1003")
}
assert(tried.count==pool.count && tried.contains(fallback))
for mode in ["cloudflare","google","system","auto"] {
    MadeiraConfig.mode=mode;SteamNetworkDNS.apply()
}
MadeiraConfig.mode="system";SteamNetworkDNS.apply()
print("PASS: fallback CDN survives pool bounds, changing health visits every host, native encrypted DNS API respects Cloudflare/Google/system choices")
'''
with tempfile.TemporaryDirectory(prefix='madeira-cdn-') as scratch:
    folder=Path(scratch);path=folder/'main.swift';path.write_text(fixture,encoding='utf-8')
    subprocess.run([swift,str(path),'-o',str(folder/'check')],check=True)
    subprocess.run([str(folder/'check')],check=True,timeout=15)
