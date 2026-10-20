#!/usr/bin/env python3
"""Execute the production registry view, bounded query replies and write protection."""
from pathlib import Path
import os,platform,shutil,subprocess,tempfile,json
root=Path(__file__).resolve().parents[2]
header=root/'build/ntdll-unix/ios_graphics_preferences.h'
wrapper=(root/'build/ntdll-unix/registry_ios.c').read_text()
assert wrapper.index('madeira_registry_query_raw(key, name, cls, info, length, needed)')<wrapper.index('return madeira_graphics_query(')
fixture=r'''
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
typedef int32_t NTSTATUS;typedef uint16_t WCHAR;typedef uint32_t DWORD,ULONG;typedef uintptr_t HANDLE;
#define WINAPI
#define STATUS_SUCCESS ((NTSTATUS)0)
#define STATUS_OBJECT_NAME_NOT_FOUND ((NTSTATUS)0xc0000034)
#define STATUS_ACCESS_DENIED ((NTSTATUS)0xc0000022)
#define STATUS_BUFFER_TOO_SMALL ((NTSTATUS)0xc0000023)
#define STATUS_BUFFER_OVERFLOW ((NTSTATUS)0x80000005)
#define STATUS_INVALID_PARAMETER ((NTSTATUS)0xc000000d)
#define STATUS_NO_MORE_ENTRIES ((NTSTATUS)0x8000001a)
#define REG_DWORD 4
#define KEY_SET_VALUE 2
#define FIELD_OFFSET(t,f) offsetof(t,f)
typedef struct{uint16_t Length,MaximumLength;WCHAR *Buffer;}UNICODE_STRING;
typedef enum{KeyFullInformation=2,KeyNameInformation=3,KeyCachedInformation=4}KEY_INFORMATION_CLASS;
typedef enum{KeyValueBasicInformation,KeyValueFullInformation,KeyValuePartialInformation,Unused3,KeyValuePartialInformationAlign64}KEY_VALUE_INFORMATION_CLASS;
typedef struct{ULONG NameLength;WCHAR Name[1];}KEY_NAME_INFORMATION;
typedef struct{DWORD TitleIndex,Type,DataLength;unsigned char Data[4];}KEY_VALUE_PARTIAL_INFORMATION;
typedef struct{DWORD TitleIndex,Type,DataOffset,DataLength,NameLength;WCHAR Name[1];}KEY_VALUE_FULL_INFORMATION;
typedef struct{uint64_t LastWriteTime;DWORD TitleIndex,ClassOffset,ClassLength,SubKeys,MaxNameLen,MaxClassLen,Values,MaxValueNameLen,MaxValueDataLen;WCHAR Class[1];}KEY_FULL_INFORMATION;
typedef struct{uint64_t LastWriteTime;DWORD TitleIndex,SubKeys,MaxNameLen,Values,MaxValueNameLen,MaxValueDataLen,NameLength;}KEY_CACHED_INFORMATION;
typedef struct{DWORD GrantedAccess;}OBJECT_BASIC_INFORMATION;
enum{ObjectBasicInformation=0};
static int active=1,missing,read_denied,key_denied,object_denied,bad_type;
static DWORD saved=3,granted=KEY_SET_VALUE;static unsigned raw_calls,key_calls;
static unsigned enum_count=3;static int enum_denied;
static const char*enum_names[]={"otherPref","shadowQualityIndex_h534661776","antiAliasingIndex_h3690569407"};
static char key_path[512]="\\Registry\\User\\S-1-5-21-1000\\Software\\Curve Animation\\Liar's Bar";
static UNICODE_STRING wide(const char *s,WCHAR *b){unsigned n=0;while(*s)b[n++]=(unsigned char)*s++;return(UNICODE_STRING){n*2,n*2,b};}
static NTSTATUS NtQueryKey(HANDLE key,KEY_INFORMATION_CLASS cls,void *out,ULONG length,ULONG *needed){
 (void)key;++key_calls;if(key_denied)return STATUS_ACCESS_DENIED;
 if(cls==KeyFullInformation){*needed=FIELD_OFFSET(KEY_FULL_INFORMATION,Class);if(length<*needed)return STATUS_BUFFER_TOO_SMALL;
  memset(out,0,*needed);((KEY_FULL_INFORMATION*)out)->Values=enum_count;return 0;}
 assert(cls==KeyNameInformation);
 unsigned n=(unsigned)strlen(key_path);*needed=4+2*n;if(length<*needed)return STATUS_BUFFER_TOO_SMALL;
 KEY_NAME_INFORMATION *p=out;p->NameLength=n*2;for(unsigned i=0;i<n;++i)p->Name[i]=(unsigned char)key_path[i];return 0;
}
static NTSTATUS NtQueryObject(HANDLE key,int cls,void*out,ULONG length,ULONG*needed){
 (void)key;assert(cls==ObjectBasicInformation&&length==sizeof(OBJECT_BASIC_INFORMATION));
 if(object_denied)return STATUS_ACCESS_DENIED;*(OBJECT_BASIC_INFORMATION*)out=(OBJECT_BASIC_INFORMATION){granted};*needed=length;return 0;
}
static WCHAR image_text[128];
// Exact field types are supplied by Wine in production. The test supplies
// an ordinary per-thread image and environment to execute the same gate.
typedef struct{UNICODE_STRING ImagePathName;} MGPParams;
typedef struct{MGPParams *ProcessParameters;} MGPPeb;
typedef struct{MGPPeb *Peb;} MGPTEB;
static MGPParams params2;static MGPPeb peb2={&params2};static MGPTEB teb2={&peb2};
#define NtCurrentTeb() (&teb2)
static char *test_env(const char *key){assert(!strcmp(key,"MADEIRA_GAME_GRAPHICS_PROFILE"));return active?"liarsbar-542782a6-v1":NULL;}
#define getenv test_env
#include "ios_graphics_preferences.h"
#include "ios_graphics_enumeration.h"
static NTSTATUS raw(HANDLE key,const UNICODE_STRING*name,KEY_VALUE_INFORMATION_CLASS cls,void*out,DWORD length,DWORD*needed){
 (void)key;(void)name;++raw_calls;if(read_denied)return STATUS_ACCESS_DENIED;if(missing)return STATUS_OBJECT_NAME_NOT_FOUND;
 assert(cls==KeyValuePartialInformation);*needed=16;if(length<16)return STATUS_BUFFER_TOO_SMALL;
 KEY_VALUE_PARTIAL_INFORMATION*p=out;p->TitleIndex=0;p->Type=bad_type?1:REG_DWORD;p->DataLength=4;memcpy(p->Data,&saved,4);return 0;
}
static NTSTATUS query(const char *name,KEY_VALUE_INFORMATION_CLASS cls,void *out,DWORD length,DWORD *needed){
 WCHAR b[100];UNICODE_STRING n=wide(name,b);
 return madeira_graphics_query(STATUS_OBJECT_NAME_NOT_FOUND,1,&n,cls,out,length,needed,raw);
}
static int preserve(const char *name,DWORD value){WCHAR b[100];UNICODE_STRING n=wide(name,b);return madeira_graphics_preserve(1,&n,REG_DWORD,&value,4,raw);}
static NTSTATUS raw_enum(HANDLE key,ULONG index,KEY_VALUE_INFORMATION_CLASS cls,void*out,DWORD length,DWORD*needed){
 (void)key;if(enum_denied)return STATUS_ACCESS_DENIED;if(index>=enum_count)return STATUS_NO_MORE_ENTRIES;
 unsigned char bytes[512]={0};const char*text=enum_names[index];unsigned n=(unsigned)strlen(text);DWORD*head=(void*)bytes;
 head[1]=REG_DWORD;unsigned fixed;
 if(cls==KeyValueFullInformation){head[2]=fixed=20+n*2;head[3]=4;head[4]=n*2;for(unsigned i=0;i<n;++i)((WCHAR*)(bytes+20))[i]=(unsigned char)text[i];}
 else if(cls==KeyValueBasicInformation){fixed=12+n*2;head[2]=n*2;for(unsigned i=0;i<n;++i)((WCHAR*)(bytes+12))[i]=(unsigned char)text[i];}
 else if(cls==KeyValuePartialInformation){fixed=12;head[2]=4;}else return STATUS_INVALID_PARAMETER;
 *needed=fixed+(cls==KeyValueBasicInformation?0:4);if(cls!=KeyValueBasicInformation)memcpy(bytes+fixed,&saved,4);
 if(length)memcpy(out,bytes,length<*needed?length:*needed);return length<*needed?STATUS_BUFFER_OVERFLOW:0;
}
static NTSTATUS enumerate(ULONG index,KEY_VALUE_INFORMATION_CLASS cls,void*out,DWORD length,DWORD*needed){
 NTSTATUS s=raw_enum(1,index,cls,out,length,needed);return madeira_graphics_enumerate(s,1,index,cls,out,length,needed,raw_enum,raw);
}
int main(void){
 params2.ImagePathName=wide("C:\\Games\\Liar's Bar.exe",image_text);
 const char*keys[]={"shadowQualityIndex","shadowQualityIndex_h534661776","antiAliasingIndex","antiAliasingIndex_h3690569407"};
 for(unsigned k=0;k<4;++k)for(unsigned cls=0;cls<=4;++cls){if(cls==3)continue;
  unsigned fixed=cls==0?12+2*(unsigned)strlen(keys[k]):cls==1?20+2*(unsigned)strlen(keys[k]):cls==4?8:12;
  unsigned minimum=cls==0?12:cls==1?20:cls==4?8:12,needed_expected=fixed+(cls==0?0:4);
  for(unsigned length=0;length<160;++length){
   unsigned char bytes[192];memset(bytes,0xa7,sizeof(bytes));DWORD needed=0;
   NTSTATUS r=query(keys[k],cls,bytes,length,&needed);
   assert(needed==needed_expected);assert(r==(length<minimum?STATUS_BUFFER_TOO_SMALL:length<needed?STATUS_BUFFER_OVERFLOW:0));
   for(unsigned i=length;i<sizeof(bytes);++i)assert(bytes[i]==0xa7);
   if(cls!=0&&length>fixed){DWORD cap=1;assert(!memcmp(bytes+fixed,&cap,length-fixed<4?length-fixed:4));}
  }
 }
 for(unsigned value=0;value<8;++value){saved=value;DWORD out[40],needed=0;memset(out,0xa7,sizeof(out));
  NTSTATUS r=query(keys[0],KeyValuePartialInformation,out,sizeof(out),&needed);
  assert((r==0)==(value==2||value==3));if(r==0)assert(out[3]==1);
  assert(preserve(keys[0],1)==(value==2||value==3));assert(!preserve(keys[0],0));assert(!preserve(keys[0],2));
 }
 saved=3;missing=1;DWORD out[40],needed=0;
 assert(!query(keys[0],KeyValuePartialInformation,out,sizeof(out),&needed)&&out[3]==1);
 assert(preserve(keys[0],1));missing=0;
 const char*other[]={"textureIndex","resolutionIndex","qualityIndex","bloomIndex","antiAliasingQualityIndex","shadowQualityIndex_h0000","notshadowQualityIndex"};
 for(unsigned i=0;i<sizeof(other)/sizeof(*other);++i){unsigned before=raw_calls;assert(query(other[i],2,out,sizeof(out),&needed)==STATUS_OBJECT_NAME_NOT_FOUND&&raw_calls==before);assert(!preserve(other[i],1));}
 active=0;unsigned before=key_calls;assert(query(keys[0],2,out,sizeof(out),&needed)==STATUS_OBJECT_NAME_NOT_FOUND&&key_calls==before);active=1;
 params2.ImagePathName=wide("C:\\Steam\\steam.exe",image_text);assert(query(keys[0],2,out,sizeof(out),&needed)==STATUS_OBJECT_NAME_NOT_FOUND);
 params2.ImagePathName=wide("C:\\Games\\LIAR'S BAR.EXE",image_text);assert(!query(keys[0],2,out,sizeof(out),&needed));
 strcpy(key_path,"\\Registry\\Machine\\Software\\Curve Animation\\Liar's Bar");assert(query(keys[0],2,out,sizeof(out),&needed)==STATUS_OBJECT_NAME_NOT_FOUND);
 strcpy(key_path,"\\Registry\\User\\S-1-5-21-1000\\Software\\Other\\Liar's Bar");assert(query(keys[0],2,out,sizeof(out),&needed)==STATUS_OBJECT_NAME_NOT_FOUND);
 strcpy(key_path,"\\REGISTRY\\USER\\S-1-5-21-1000\\SOFTWARE\\CURVE ANIMATION\\LIAR'S BAR");assert(!query(keys[0],2,out,sizeof(out),&needed));
 key_denied=1;assert(query(keys[0],2,out,sizeof(out),&needed)==STATUS_OBJECT_NAME_NOT_FOUND&&!preserve(keys[0],1));key_denied=0;
 read_denied=1;assert(query(keys[0],2,out,sizeof(out),&needed)==STATUS_OBJECT_NAME_NOT_FOUND&&!preserve(keys[0],1));read_denied=0;
 bad_type=1;assert(query(keys[0],2,out,sizeof(out),&needed)==STATUS_OBJECT_NAME_NOT_FOUND&&!preserve(keys[0],1));bad_type=0;
 granted=0;assert(!preserve(keys[0],1));granted=KEY_SET_VALUE;object_denied=1;assert(!preserve(keys[0],1));object_denied=0;
 WCHAR b[100];UNICODE_STRING n=wide(keys[0],b);n.Length--;assert(!madeira_graphics_preserve(1,&n,REG_DWORD,&saved,4,raw));
 n=wide(keys[0],b);assert(madeira_graphics_query(STATUS_ACCESS_DENIED,1,&n,2,out,sizeof(out),&needed,raw)==STATUS_ACCESS_DENIED);
 // A simulated save of the effective cap leaves raw storage high; a lower
 // player choice is allowed through, and disabling the profile removes the view.
 assert(saved==3&&preserve(keys[0],1));active=0;assert(!preserve(keys[0],1));active=1;
 // Unity may load existing PlayerPrefs by enumeration rather than by named
 // queries. Both access patterns must see the same cap and original storage.
 for(unsigned cls=0;cls<3;++cls)for(unsigned length=0;length<160;++length){
  unsigned char buffer[192];memset(buffer,0xa7,sizeof(buffer));DWORD need=0;
  NTSTATUS s=enumerate(1,cls,buffer,length,&need);
  assert(s==(length<need?STATUS_BUFFER_OVERFLOW:0));
  for(unsigned j=length;j<sizeof(buffer);++j)assert(buffer[j]==0xa7);
  unsigned at=cls==0?0:cls==1?20+2*(unsigned)strlen(enum_names[1]):12;
  if(cls&&length>=at+4){DWORD v;memcpy(&v,buffer+at,4);assert(v==1);}
 }
 enum_count=1;missing=1;
 KEY_FULL_INFORMATION full={0};full.Values=enum_count;
 madeira_graphics_key_info(0,1,KeyFullInformation,&full,sizeof(full),raw);assert(full.Values==3&&enum_count==1&&full.MaxValueDataLen==4);
 KEY_CACHED_INFORMATION cached={0};cached.Values=1;
 madeira_graphics_key_info(0,1,KeyCachedInformation,&cached,sizeof(cached),raw);assert(cached.Values==3);
 for(unsigned i=1;i<=2;++i){DWORD need=0;unsigned char buffer[256];
  assert(!enumerate(i,KeyValueFullInformation,buffer,sizeof(buffer),&need));
  KEY_VALUE_FULL_INFORMATION*p=(void*)buffer;assert(p->Type==REG_DWORD&&p->DataLength==4);
  DWORD value;memcpy(&value,buffer+p->DataOffset,4);assert(value==1);
  assert(mgp_ascii(p->Name,p->NameLength/2,mgp_virtual_names[i-1]));}
 assert(enumerate(3,KeyValueFullInformation,out,sizeof(out),&needed)==STATUS_NO_MORE_ENTRIES);
 active=0;assert(enumerate(1,KeyValueFullInformation,out,sizeof(out),&needed)==STATUS_NO_MORE_ENTRIES);active=1;
 enum_denied=1;assert(enumerate(1,KeyValueFullInformation,out,sizeof(out),&needed)==STATUS_ACCESS_DENIED);enum_denied=0;
 assert(saved==3);missing=0;
 puts("PASS: production game preference view, exact two keys, all reply sizes/canaries, saved-original protection, lower settings, opt-out, path/type/access/error preservation");
}
'''
cc=os.environ.get('CC') or shutil.which('clang');assert cc,'clang required'
with tempfile.TemporaryDirectory(prefix='madeira-game-prefs-')as tmp:
    folder=Path(tmp);c=folder/'test.c';exe=folder/('check.exe'if os.name=='nt'else'check');c.write_text(fixture)
    flags=['-fsanitize=address,undefined']if platform.system()=='Darwin'else[]
    subprocess.run([cc,'-std=c11','-O1','-g','-Wall','-Wextra','-Werror',*flags,'-I'+str(header.parent),str(c),'-o',str(exe)],check=True)
    subprocess.run([str(exe)],check=True,timeout=20,
        env=dict(os.environ,UBSAN_OPTIONS='halt_on_error=1:print_stacktrace=1'))
    if os.environ.get('BITRISE_DEPLOY_DIR'):
        (Path(os.environ['BITRISE_DEPLOY_DIR'])/'game-graphics-preferences-validation.json').write_text(json.dumps({
            'productionPreferencePolicyExecuted':True,'boundedNativeReplies':True,'savedOriginalProtected':True,
            'resolutionAndTextureUntouched':True,'physicalGameFPSMeasured':False},indent=2)+'\n')
