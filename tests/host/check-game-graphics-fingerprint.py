#!/usr/bin/env python3
"""Run the production fingerprint gate, file digest cache and launch cleanup."""
from pathlib import Path
import os,shutil,subprocess,sys,tempfile,json
root=Path(__file__).resolve().parents[2]
s=(root/'app/Madeira/Library.swift').read_text()
a=s.index('enum GameGraphicsPreferences {');b=s.index('{',a);end=b+1;depth=1
while depth:depth+=(s[end]=='{')-(s[end]=='}');end+=1
production=s[a:end]
manifest=json.loads((root/'build/game-profiles/liarsbar-hdrp-v1.json').read_text())
assert manifest['profile'] in production and manifest['gameAssemblySHA256'] in production and manifest['metadataSHA256'] in production
assert manifest['profile'] in (root/'build/ntdll-unix/ios_graphics_preferences.h').read_text()
fixture=r'''
let fm=FileManager.default
let folder=URL(fileURLWithPath:NSTemporaryDirectory()).appendingPathComponent(UUID().uuidString)
try fm.createDirectory(at:folder.appendingPathComponent("Liar's Bar_Data"),withIntermediateDirectories:true)
defer {try? fm.removeItem(at:folder)}
func env()->String?{getenv("MADEIRA_GAME_GRAPHICS_PROFILE").map{String(cString:$0)}}
var identities=["GameAssembly.dll":GameGraphicsPreferences.dllDigest,"global-metadata.dat":GameGraphicsPreferences.metadataDigest]
var reads=0
func read(_ url:URL)->String?{reads+=1;return identities[url.lastPathComponent]}
GameGraphicsPreferences.apply(enabled:true,appID:3097560,location:folder,readDigest:read)
assert(env()==GameGraphicsPreferences.profile && reads==2)
GameGraphicsPreferences.apply(enabled:false,appID:3097560,location:folder,readDigest:read)
assert(env()=="0" && reads==2)
GameGraphicsPreferences.apply(enabled:true,appID:nil,location:folder.appendingPathComponent("Liar's Bar.exe"),readDigest:read)
assert(env()==GameGraphicsPreferences.profile)
GameGraphicsPreferences.apply(enabled:true,appID:20,location:folder,readDigest:read)
assert(env()=="0")
identities["global-metadata.dat"]="unknown"
GameGraphicsPreferences.apply(enabled:true,appID:3097560,location:folder,readDigest:read)
assert(env()=="0")
identities["GameAssembly.dll"]="unknown";identities["global-metadata.dat"]=GameGraphicsPreferences.metadataDigest
GameGraphicsPreferences.apply(enabled:true,appID:3097560,location:folder,readDigest:read)
assert(env()=="0")
let file=folder.appendingPathComponent("fixture.bin")
try Data("abc".utf8).write(to:file)
assert(GameGraphicsPreferences.digest(file)=="ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad")
assert(GameGraphicsPreferences.digest(file)=="ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad")
try Data("changed size".utf8).write(to:file)
assert(GameGraphicsPreferences.digest(file) != "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad")
assert(GameGraphicsPreferences.digest(folder.appendingPathComponent("missing"))==nil)
print("PASS: production graphics fingerprint gate, exact pair, imported/Steam paths, opt-out, unknown version, digest cache invalidation and cross-game cleanup")
'''
compiler=shutil.which('swiftc')
if not compiler:
    if '--require-swift'in sys.argv:raise SystemExit('Swift required')
    print('SKIP: graphics fingerprint behavior requires macOS Swift/CryptoKit');raise SystemExit(0)
with tempfile.TemporaryDirectory(prefix='madeira-graphics-fingerprint-')as temp:
    folder=Path(temp);source=folder/'main.swift';exe=folder/'check'
    source.write_text('import Foundation\nimport CryptoKit\n'+production+'\n'+
      'final class LogStore{static let shared=LogStore();func log(_ s:String){}}\n'+fixture)
    subprocess.run([compiler,str(source),'-o',str(exe)],check=True)
    subprocess.run([str(exe)],check=True)
