#!/usr/bin/env python3
"""Execute the production DNS binding with missing APIs, retries, threads and real Darwin BIND9."""
from pathlib import Path
import os,shutil,subprocess,tempfile
root=Path(__file__).resolve().parents[2]
s=(root/'build/ntdll-unix/dnsapi_unixlib_ios.c').read_text()
start=s.index('typedef int (*ios_res_ninit_fn)');end=s.index('/***********************************************************************',start)
body=s[start:end]
fixture=r"""
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
typedef int pthread_once_t,pthread_key_t;
#define PTHREAD_ONCE_INIT 0
#define STDERR_FILENO 2
#define RTLD_NOW 1
#define RTLD_LOCAL 0
static void *slot;
static void (*cleanup)(void *);
static int pthread_key_create(int *key,void(*fn)(void *)){*key=1;cleanup=fn;return 0;}
static void *pthread_getspecific(int key){assert(key==1);return slot;}
static int pthread_setspecific(int key,const void *p){assert(key==1);slot=(void *)p;return 0;}
static void pthread_once(int *done,void(*fn)(void)){if(!*done){fn();*done=1;}}
#define dprintf(...) ((void)0)
#else
#include <unistd.h>
#include <pthread.h>
#include <dlfcn.h>
#endif
#ifdef REAL_DNS
#include <netinet/in.h>
#include <arpa/nameser.h>
#include <resolv.h>
#include <netdb.h>
#else
#define RES_INIT 1
#define TRY_AGAIN 2
struct __res_state {unsigned long options;int res_h_errno;void *resource;};
static int mode,allocations,destroys,queries,state_getter_calls;
static struct __res_state borrowed;
static struct __res_state *foreign_state(void){++state_getter_calls;return &borrowed;}
static int real_init(struct __res_state *s){assert(s && s!=&borrowed && !s->resource);s->resource=malloc(16);assert(s->resource);__atomic_add_fetch(&allocations,1,__ATOMIC_RELAXED);s->options=mode==2?0:RES_INIT;return mode==2?-1:0;}
static void real_destroy(struct __res_state *s){assert(s && s!=&borrowed);if(s->resource){free(s->resource);s->resource=NULL;__atomic_add_fetch(&destroys,1,__ATOMIC_RELAXED);}s->options=0;}
static int real_query(struct __res_state *s,const char *name,int cls,int type,unsigned char *answer,int length){assert(s && s!=&borrowed && s->options&RES_INIT && cls==1 && type==1 && length>=4);__atomic_add_fetch(&queries,1,__ATOMIC_RELAXED);if(!strcmp(name,"fail")){s->res_h_errno=TRY_AGAIN;return -1;}memset(answer,0x5a,4);return 4;}
static void *fake_open(const char *name,int flags){(void)flags;assert(!strcmp(name,"/usr/lib/libresolv.9.dylib"));return mode==1?NULL:(void *)1;}
static void *fake_sym(void *lib,const char *name){assert(lib==(void *)1);if(!strcmp(name,"res_9_ninit"))return mode==3?NULL:real_init;if(!strcmp(name,"res_9_nquery"))return mode==4?NULL:real_query;if(!strcmp(name,"res_9_ndestroy"))return mode==5?NULL:real_destroy;if(!strcmp(name,"__res_9_state")||!strcmp(name,"__res_state"))return foreign_state;assert(!"unexpected or mixed ABI symbol");return NULL;}
#define dlopen fake_open
#define dlsym fake_sym
#endif
static int madeira_steam_dns_should_override(const char *name){return !strcmp(name,"steam.test");}
static int madeira_steam_dns_query(const char *name,int c,int t,unsigned char *out,int len){(void)name;(void)c;(void)t;assert(len>=4);memset(out,0x4b,4);return 4;}
/*PRODUCTION*/
#ifndef _WIN32
static pthread_mutex_t lock=PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t ready=PTHREAD_COND_INITIALIZER;
static int count,go;
static void *states[8];
#endif
static void work(unsigned index){
 unsigned char answer[16];struct __res_state *s=ios_resolv_state();assert(s);
#ifndef _WIN32
 pthread_mutex_lock(&lock);states[index]=s;++count;pthread_cond_broadcast(&ready);while(!go)pthread_cond_wait(&ready,&lock);pthread_mutex_unlock(&lock);
#else
 (void)index;
#endif
#ifdef REAL_DNS
 assert(s!=&ios_resolv_null_state);assert(!ios_resolv_init());assert(s->options&RES_INIT);assert(ios_resolv_state()==s);
#else
 assert(!state_getter_calls);
 if(!mode){assert(s!=&borrowed && s!=&ios_resolv_null_state);assert(!ios_resolv_init());assert(s->options&RES_INIT);}
 for(int i=0;i<50;++i){int r=ios_resolv_query("ok",1,1,answer,sizeof answer);assert(r==(mode?-1:4));if(mode)assert(ios_resolv_h_errno==TRY_AGAIN);else assert(answer[0]==0x5a);}
 assert(ios_resolv_query("steam.test",1,1,answer,sizeof answer)==4 && answer[0]==0x4b);
 if(!mode){assert(ios_resolv_query("fail",1,1,answer,sizeof answer)==-1);assert(ios_resolv_h_errno==TRY_AGAIN);}
#endif
}
#ifndef _WIN32
static void *worker(void *p){work((uintptr_t)p);return NULL;}
#endif
int main(int argc,char **argv){
#ifdef REAL_DNS
 (void)argc;(void)argv;
#else
 mode=argc>1?atoi(argv[1]):0;
#endif
#ifndef _WIN32
 pthread_t threads[8];for(uintptr_t i=0;i<8;++i)assert(!pthread_create(&threads[i],NULL,worker,(void *)i));
 pthread_mutex_lock(&lock);while(count<8)pthread_cond_wait(&ready,&lock);
 for(int i=0;i<8;++i)for(int j=0;j<i;++j)assert(states[i]!=states[j]);
 go=1;pthread_cond_broadcast(&ready);pthread_mutex_unlock(&lock);
 for(int i=0;i<8;++i)assert(!pthread_join(threads[i],NULL));
#else
 for(unsigned i=0;i<8;++i){work(i);if(slot){cleanup(slot);slot=NULL;}}
#endif
#ifndef REAL_DNS
 assert(!state_getter_calls && !borrowed.options && !borrowed.resource);assert(allocations==destroys);assert(queries==(mode?0:408));
 puts("PASS: production DNS uses only owned state and one BIND9 family; missing library/APIs, failed init/retry, query errors, DoH and thread cleanup");
#else
 puts("PASS: real Darwin BIND9 ninit/ndestroy on eight separate pthread-owned states; no borrowed system resolver storage");
#endif
 return 0;
}
"""
compiler=os.environ.get('CC') or shutil.which('clang') or shutil.which('cc');assert compiler
with tempfile.TemporaryDirectory(prefix='madeira-owned-dns-') as tmp:
 c=Path(tmp)/'test.c';c.write_text(fixture.replace('/*PRODUCTION*/',body))
 for real in ([False,True] if hasattr(os,'uname') and os.uname().sysname=='Darwin' else [False]):
  exe=Path(tmp)/('real' if real else 'model');exe=exe.with_suffix('.exe') if os.name=='nt' else exe
  flags=['-DREAL_DNS'] if real else []
  if os.name!='nt':flags+=['-pthread','-fsanitize=address,undefined']
  subprocess.run([compiler,'-std=c11','-O2','-Wall','-Wextra','-Werror','-Wno-unused-function','-Wno-unused-variable',*flags,str(c),'-o',str(exe)],check=True)
  for mode in ([0] if real else range(6)):subprocess.run([str(exe),str(mode)],check=True,timeout=30)
print('NOT PROVEN: physical Chrome browsing, renderer/GPU startup or Google Sync')
