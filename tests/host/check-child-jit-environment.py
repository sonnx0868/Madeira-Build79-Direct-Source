#!/usr/bin/env python3
"""Exercise Wine's actual environment builder with Chromium-style child blocks."""
from pathlib import Path
import os
import shutil
import subprocess
import tempfile

root = Path(__file__).resolve().parents[2]
source = (root / 'build/ntdll-unix/env_ios.c').read_text(encoding='utf-8')
baseline = subprocess.check_output(['git', '-C', str(root), 'show',
    'd784eea:build/ntdll-unix/env_ios.c'], text=True)


def function(text, signature):
    start = text.index(signature)
    opening = text.index('{', start)
    depth = 1
    end = opening + 1
    while depth:
        depth += (text[end] == '{') - (text[end] == '}')
        end += 1
    return text[start:end]


assert source.count('add_ios_jit_environment( env, pos, size );') == 1
assert 'add_dynamic_environment( &env, &env_pos, &env_size );' in source
fixture = r'''
#include <assert.h>
#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef uint16_t WCHAR;
typedef size_t SIZE_T;
typedef int BOOL;
#define FALSE 0
#define TRUE 1
#define WINE_IOS 1
#define CP_UTF8 65001
#define FILE_OPEN 1
#define OS_LOG_DEFAULT 0
#define os_log(...) ((void)0)
#define max(a,b) ((a)>(b)?(a):(b))
static size_t u16len(const WCHAR *p) {size_t n=0;while(p[n])++n;return n;}
static int u16icmp(const WCHAR *a,const WCHAR *b,size_t n) {
    for(size_t i=0;i<n;++i) {
        int d=tolower(a[i])-tolower(b[i]);if(d || !a[i] || !b[i])return d;
    }
    return 0;
}
#define wcslen u16len
#define wcsnicmp u16icmp
static void ascii_to_unicode(WCHAR *d,const char *s,size_t n) {for(size_t i=0;i<n;++i)d[i]=(unsigned char)s[i];}
static void ntdll_umbstowcs(const char *s,size_t n,WCHAR *d,size_t cap) {assert(n<=cap);ascii_to_unicode(d,s,n);}
static const char *build_dir,*data_dir,*home_dir,*config_dir,*wineloader;
static WCHAR *nt_build_dir,*nt_data_dir;
static const char *dll_paths[]={NULL};
static const char *user_name="mobile",*user_locale="en-US";
static struct {unsigned CodePage;} unix_cp={CP_UTF8};
static int unix_to_nt_file_name(const char *a,WCHAR **b,int c) {(void)a;(void)b;(void)c;return 1;}
static void add_system_dll_path_var(WCHAR **e,SIZE_T *p,SIZE_T *s) {(void)e;(void)p;(void)s;}
/*PRODUCTION*/
static void host(const char *key,const char *value) {
#ifdef _WIN32
    assert(!_putenv_s(key,value?value:""));
#else
    if(value)assert(!setenv(key,value,1));else assert(!unsetenv(key));
#endif
}
static char *value(WCHAR *env,SIZE_T pos,const char *key) {
    static char out[128];WCHAR name[64];ascii_to_unicode(name,key,strlen(key)+1);
    WCHAR *p=find_env_var(env,pos,name,strlen(key));if(!p)return NULL;
    p+=strlen(key)+1;size_t n=u16len(p);assert(n<sizeof(out));
    for(size_t i=0;i<=n;++i)out[i]=(char)p[i];return out;
}
static void assert_value(WCHAR *e,SIZE_T p,const char *k,const char *v) {
    char *got=value(e,p,k);if(v)assert(got && !strcmp(got,v));else assert(!got);
}
int main(void) {
    host("WINE_IOS_JIT_RW","7000000000");
    host("WINE_IOS_JIT_RX","148000000");
    host("WINE_IOS_JIT_SIZE","27000000");
    for(unsigned iteration=0;iteration<2000;++iteration) {
        SIZE_T cap=2,pos=0;WCHAR *env=calloc(cap,sizeof(*env));
        append_envA(&env,&pos,&cap,"CHROME_CRASHPAD_PIPE_NAME","original");
        append_envA(&env,&pos,&cap,"PATH","C:\\windows");
        if(iteration&1) {
            append_envA(&env,&pos,&cap,"wine_ios_jit_rw","dead");
            append_envA(&env,&pos,&cap,"WINE_IOS_JIT_RX","beef");
        }
        add_dynamic_environment(&env,&pos,&cap);
#ifdef BASELINE
        assert_value(env,pos,"WINE_IOS_JIT_RW",NULL);
        puts("REPRODUCED: production926 environment builder leaves a restricted child without JIT mapping values");
        free(env);return 0;
#else
        assert_value(env,pos,"WINE_IOS_JIT_RW","7000000000");
        assert_value(env,pos,"WINE_IOS_JIT_RX","148000000");
        assert_value(env,pos,"WINE_IOS_JIT_SIZE","27000000");
        assert_value(env,pos,"CHROME_CRASHPAD_PIPE_NAME","original");
        assert_value(env,pos,"PATH","C:\\windows");
        // A later pool must replace stale guest values, including case variants.
        host("WINE_IOS_JIT_SIZE","18000000");
        add_dynamic_environment(&env,&pos,&cap);
        assert_value(env,pos,"WINE_IOS_JIT_SIZE","18000000");
        host("WINE_IOS_JIT_SIZE","27000000");
        // Missing host data must remove guest-supplied mapping addresses.
        host("WINE_IOS_JIT_RX",NULL);add_dynamic_environment(&env,&pos,&cap);
        assert_value(env,pos,"WINE_IOS_JIT_RX",NULL);
        host("WINE_IOS_JIT_RX","148000000");
        free(env);
#endif
    }
    puts("PASS: production Wine environment rebuild, restricted/inherited/stale/case-insensitive blocks, host authority, absent values and2000 repeated children");
}
'''
names = ['static WCHAR *find_env_var(', 'static void set_env_var(', 'static void append_envW(',
         'static void append_envA(', 'static void add_path_var(']
compiler = os.environ.get('CC') or shutil.which('clang') or shutil.which('cc')
assert compiler, 'C compiler required'
with tempfile.TemporaryDirectory(prefix='madeira-child-jit-') as temp:
    work = Path(temp)
    for old in (True, False):
        text = baseline if old else source
        production = '\n'.join(function(text, name) for name in names)
        if not old:
            production += '\n' + function(text, 'static void add_ios_jit_environment(')
        production += '\n' + function(text, 'static void add_dynamic_environment(')
        c = work / ('baseline.c' if old else 'fixed.c')
        exe = c.with_suffix('.exe' if os.name == 'nt' else '')
        c.write_text(fixture.replace('/*PRODUCTION*/', production), encoding='utf-8')
        subprocess.run([compiler, '-std=c11', '-D_POSIX_C_SOURCE=200809L', '-O2', '-Wall', '-Wextra',
                        '-Werror', *([] if os.name == 'nt' else ['-fsanitize=address,undefined']),
                        *(['-DBASELINE'] if old else []), str(c), '-o', str(exe)], check=True)
        subprocess.run([str(exe)], check=True, timeout=30)
print('NOT MEASURED: complete Windows Chrome execution, Sync or extensions on iPad')
