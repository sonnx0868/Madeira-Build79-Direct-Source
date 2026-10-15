#!/usr/bin/env python3
"""Exercise the real pre-Wine parser and compare it with Windows UCRT."""
from pathlib import Path
import os, random, shutil, subprocess, tempfile

root = Path(__file__).resolve().parents[2]
cc = os.environ.get('CC') or shutil.which('clang') or shutil.which('cc')
if not cc: raise SystemExit('C compiler required')
bridge = (root/'app/Madeira/WineProcessBridge.m').read_text(encoding='utf-8')
native = bridge.split('// Parse once using Windows quoting.',1)[1].split('/* iOS-Madeira: chdir',1)[0]
assert 'madeira_windows_arguments_init(&launch_args, exe_path, madeira_args)' in native
assert 'strtok' not in native and 'extra_argv' not in native
assert 'getenv("_MADEIRA_PRIVATE_ARGS") ? "<private store argument>" : argv[i]' in native
assert 'wine_launch_arguments_status(launchWindowsPath, launchArguments)' in (root/'app/Madeira/Library.swift').read_text(encoding='utf-8')
assert 'entry.launchArguments.utf8.count < 1024' not in (root/'app/Madeira/ContentView.swift').read_text(encoding='utf-8')

fixture = r'''
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static int fail_at, calls, live;
static void *allocate(size_t size) { if(++calls==fail_at)return NULL;void *p=malloc(size);if(p)++live;return p; }
static void *allocate_zero(size_t n,size_t size) { if(++calls==fail_at)return NULL;void *p=calloc(n,size);if(p)++live;return p; }
static void release(void *p) { if(p)--live;free(p); }
#define malloc allocate
#define calloc allocate_zero
#define free release
#include "WindowsArguments.h"
#undef malloc
#undef calloc
#undef free
int main(int argc,char **argv) {
    assert(argc==3);fail_at=atoi(argv[2]);
    FILE *f=fopen(argv[1],"rb");assert(f);assert(!fseek(f,0,SEEK_END));long n=ftell(f);assert(n>=0);rewind(f);
    char *text=malloc((size_t)n+1);assert(text);assert(fread(text,1,(size_t)n,f)==(size_t)n);fclose(f);text[n]=0;
    char executable[]="C:\\Program Files\\Game\\game.exe";
    struct madeira_windows_arguments args;
    int status=madeira_windows_arguments_init(&args,executable,text);
    printf("%d\n",status);
    if(!status) {
        memset(executable,'x',strlen(executable));memset(text,'z',(size_t)n);
        assert(!strcmp(args.argv[0],"wine"));assert(!strcmp(args.argv[1],"C:\\Program Files\\Game\\game.exe"));
        assert(args.argv[args.argc]==NULL);
        for(int i=2;i<args.argc;i++) { for(unsigned char *p=(unsigned char *)args.argv[i];*p;p++)printf("%02x",*p);putchar('\n'); }
    } else assert(!args.storage&&!args.argv&&!args.argc);
    madeira_windows_arguments_dispose(&args);madeira_windows_arguments_dispose(&args);assert(live==0);free(text);
    return 0;
}
'''
crt = r'''
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
int wmain(int argc,wchar_t **argv) {
    for(int i=1;i<argc;i++) {
        int n=WideCharToMultiByte(CP_UTF8,0,argv[i],-1,NULL,0,NULL,NULL);char *text=malloc(n);if(!text)return 2;
        WideCharToMultiByte(CP_UTF8,0,argv[i],-1,text,n,NULL,NULL);
        for(unsigned char *p=(unsigned char *)text;*p;p++)printf("%02x",*p);putchar('\n');free(text);
    }return 0;
}
'''
official = [
    ('"a b c" d e',['a b c','d','e']),
    (r'"ab\"c" "\\" d',['ab"c','\\','d']),
    (r'a\\\b d"e f"g h',[r'a\\\b','de fg','h']),
    (r'a\\\"b c d',['a\\"b','c','d']),
    (r'a\\\\"b c" d e',[r'a\\b c','d','e']),
    ('a"b"" c d',['ab" c d']),
    ('"" "" x',['','','x']), ('a\tb "c\td"',['a','b','c\td']),
    ('-path "C:\\Games\\Folder Name\\\\" -flag',['-path','C:\\Games\\Folder Name\\','-flag']),
    ('-option=\"value with spaces\"',['-option=value with spaces']),
    ('"unclosed quoted value',['unclosed quoted value']),
    ('& | > < %PATH% $HOME ; ^',['&','|','>','<','%PATH%','$HOME',';','^']),
]
rng=random.Random(705)
cases=list(official)
alphabet='ab09 \\"\táế日🙂'
for _ in range(128):
    expected=[''.join(rng.choice(alphabet) for _ in range(rng.randrange(25))) for _ in range(rng.randrange(1,12))]
    cases.append((subprocess.list2cmdline(expected),expected))
cases += [(subprocess.list2cmdline([f'-arg{i}' for i in range(96)]),[f'-arg{i}' for i in range(96)]),
          ('-AUTH_PASSWORD='+('a'*8192),['-AUTH_PASSWORD='+('a'*8192)])]
with tempfile.TemporaryDirectory(prefix='madeira-windows-arguments-') as temporary:
    folder=Path(temporary);c=folder/'parse.c';c.write_text(fixture,encoding='utf-8')
    parser=folder/('parse.exe' if os.name=='nt' else 'parse')
    subprocess.run([cc,'-std=c11','-O2','-Wall','-Wextra','-Werror','-I',str(root/'app/Madeira'),str(c),'-o',str(parser)],check=True)
    input_file=folder/'command.txt'
    def parse(text,fail=0):
        input_file.write_bytes(text.encode('utf-8') if isinstance(text,str) else text)
        lines=subprocess.check_output([str(parser),str(input_file),str(fail)],text=True).splitlines()
        return int(lines[0]),[bytes.fromhex(line).decode('utf-8') for line in lines[1:]]
    for text,expected in cases:
        actual=parse(text);assert actual==(0,expected),(text,actual,expected)
    assert parse('   \t  ')==(0,[])
    for fail in [1,2]: assert parse('a "b c"',fail)[0]==3
    for invalid in [b'\xff',b'\xc0\xaf',b'\xe2',b'\xed\xa0\x80',b'\xf4\x90\x80\x80']:
        assert parse(invalid)[0]==2
    base=len(r'C:\Program Files\Game\game.exe')+2
    for character,units in [('a',1),('日',1),('🙂',2)]:
        remaining=32765-base
        text='x'*(remaining%units)+character*(remaining//units)
        assert parse(text)==(0,[text]);assert parse(text+'x')[0]==1
    # An open quote is valid Windows syntax, but Wine must still have room
    # to serialize the resulting trailing backslashes before a closing quote.
    assert parse('"x '+('\\'*20000))[0]==1
    print(f'PASS: {len(cases)} Windows argument vectors, empty/tabs/quotes/backslashes, 96 arguments, long store tokens, UTF-16 limits, invalid UTF-8, owned strings and allocation failure cleanup',flush=True)
    if os.name=='nt':
        c=folder/'crt.c';c.write_text(crt,encoding='utf-8');probe=folder/'crt.exe'
        subprocess.run([cc,'-municode',str(c),'-o',str(probe)],check=True)
        for text,expected in cases:
            lines=subprocess.check_output(subprocess.list2cmdline([str(probe)])+' '+text,text=True).splitlines()
            actual=[bytes.fromhex(line).decode('utf-8') for line in lines]
            assert actual==expected,(text,actual,expected)
        print(f'PASS: all {len(cases)} vectors independently match the actual Windows UCRT command-line parser',flush=True)
