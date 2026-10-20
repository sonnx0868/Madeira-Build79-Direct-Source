/* SPDX-License-Identifier: LGPL-2.1-or-later
 * A transient view of two verified Unity PlayerPrefs. Keep the saved original
 * when Unity writes the applied cap back on exit. No handle/value cache, disk
 * registry rewrite, game-code patch, resolution or texture override. */
#pragma once
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>

#define MADEIRA_LIARS_GRAPHICS_PROFILE "liarsbar-542782a6-v1"
typedef NTSTATUS (WINAPI *MadeiraRegistryQuery)(HANDLE, const UNICODE_STRING *,
    KEY_VALUE_INFORMATION_CLASS, void *, DWORD, DWORD *);
#ifndef MGP_QUERY_KEY
#define MGP_QUERY_KEY NtQueryKey
#endif

static int mgp_ascii(const WCHAR *wide, unsigned count, const char *text)
{
    unsigned i;
    for (i=0; text[i]; ++i) {
        unsigned a, b=(unsigned char)text[i];
        if (i>=count) return 0;
        a=wide[i]; if(a>='A' && a<='Z') a+=32; if(b>='A' && b<='Z') b+=32;
        if (a!=b) return 0;
    }
    return i==count;
}

#ifndef MGP_SCOPE_ACTIVE
static int mgp_scope_active(void)
{
    const char *mode=getenv("MADEIRA_GAME_GRAPHICS_PROFILE");
    const UNICODE_STRING *image;
    unsigned n, start=0;
    if (!mode || strcmp(mode,MADEIRA_LIARS_GRAPHICS_PROFILE)) return 0;
    if (!NtCurrentTeb()->Peb || !NtCurrentTeb()->Peb->ProcessParameters) return 0;
    image=&NtCurrentTeb()->Peb->ProcessParameters->ImagePathName;
    if (!image->Buffer || (image->Length&1) || image->Length>4096) return 0;
    n=image->Length/sizeof(WCHAR);
    for(unsigned i=0;i<n;++i) if(image->Buffer[i]=='\\' || image->Buffer[i]=='/') start=i+1;
    return mgp_ascii(image->Buffer+start,n-start,"Liar's Bar.exe") ||
           mgp_ascii(image->Buffer+start,n-start,"LiarsBar.exe");
}
#define MGP_SCOPE_ACTIVE() mgp_scope_active()
#endif

static int mgp_preference(const UNICODE_STRING *name)
{
    unsigned n;
    if(!name || !name->Buffer || (name->Length&1)) return 0;
    n=name->Length/sizeof(WCHAR);
    if(mgp_ascii(name->Buffer,n,"shadowQualityIndex") ||
       mgp_ascii(name->Buffer,n,"shadowQualityIndex_h534661776")) return 1;
    if(mgp_ascii(name->Buffer,n,"antiAliasingIndex") ||
       mgp_ascii(name->Buffer,n,"antiAliasingIndex_h3690569407")) return 2;
    return 0;
}

static int mgp_game_key(HANDLE key)
{
    union { uint64_t align; unsigned char bytes[1024]; } storage;
    KEY_NAME_INFORMATION *info=(void *)storage.bytes;
    ULONG needed=0;unsigned n, start;
    if(MGP_QUERY_KEY(key,KeyNameInformation,info,sizeof(storage.bytes),&needed)) return 0;
    if(needed<sizeof(ULONG) || needed>sizeof(storage.bytes) ||
       (info->NameLength&1) || info->NameLength>needed-sizeof(ULONG)) return 0;
    n=info->NameLength/sizeof(WCHAR);
    if(n<16 || !mgp_ascii(info->Name,15,"\\Registry\\User\\")) return 0;
    start=15;
    while(start<n && info->Name[start]!='\\') ++start;
    return start<n && mgp_ascii(info->Name+start,n-start,"\\Software\\Curve Animation\\Liar's Bar");
}

/* 1 means that a higher saved value (or the game's known default) needs the
 * cap. Lower settings, invalid values and non-DWORD preferences are preserved. */
static int mgp_read_cap(HANDLE key,const UNICODE_STRING *name,MadeiraRegistryQuery raw,
                        DWORD *saved,int *missing)
{
    union { uint64_t align; unsigned char bytes[32]; } storage;
    KEY_VALUE_PARTIAL_INFORMATION *info=(void *)storage.bytes;
    DWORD needed=0,value=0;
    NTSTATUS status=raw(key,name,KeyValuePartialInformation,info,sizeof(storage.bytes),&needed);
    *missing=status==STATUS_OBJECT_NAME_NOT_FOUND;
    if(*missing) { *saved=0; return 1; }
    if(status || needed<FIELD_OFFSET(KEY_VALUE_PARTIAL_INFORMATION,Data)+sizeof(DWORD) ||
       info->Type!=REG_DWORD || info->DataLength!=sizeof(DWORD)) return 0;
    memcpy(&value,info->Data,sizeof(value));*saved=value;
    return value==2 || value==3;
}

static NTSTATUS madeira_graphics_query(NTSTATUS status,HANDLE key,const UNICODE_STRING *name,
    KEY_VALUE_INFORMATION_CLASS cls,void *info,DWORD length,DWORD *needed,MadeiraRegistryQuery raw)
{
    unsigned fixed,minimum,name_offset=0;DWORD saved;int missing;
    if(status!=STATUS_SUCCESS && status!=STATUS_OBJECT_NAME_NOT_FOUND &&
       status!=STATUS_BUFFER_TOO_SMALL && status!=STATUS_BUFFER_OVERFLOW) return status;
    if(!mgp_preference(name) || !MGP_SCOPE_ACTIVE() || !mgp_game_key(key)) return status;
    switch(cls) {
    case KeyValueBasicInformation: minimum=12;fixed=12+name->Length;name_offset=12;break;
    case KeyValueFullInformation: minimum=20;fixed=20+name->Length;name_offset=20;break;
    case KeyValuePartialInformation: minimum=fixed=12;break;
    case KeyValuePartialInformationAlign64: minimum=fixed=8;break;
    default:return status;
    }
    if(!mgp_read_cap(key,name,raw,&saved,&missing)) return status;
    if(name_offset && length>name_offset)
        memcpy((char *)info+name_offset,name->Buffer,
               length-name_offset<name->Length?length-name_offset:name->Length);
    DWORD header[5]={0,REG_DWORD,4,0,0};
    if(cls==KeyValuePartialInformationAlign64) {header[0]=REG_DWORD;header[1]=4;}
    else if(cls==KeyValueBasicInformation) header[2]=name->Length;
    else if(cls==KeyValueFullInformation) {header[2]=fixed;header[3]=4;header[4]=name->Length;}
    if(length) memcpy(info,header,length<minimum?length:minimum);
    *needed=fixed+(cls==KeyValueBasicInformation?0:4);
    if(cls!=KeyValueBasicInformation && length>fixed) {
        DWORD cap=1;
        memcpy((char *)info+fixed,&cap,length-fixed<4?length-fixed:4);
    }
    if(length<minimum) return STATUS_BUFFER_TOO_SMALL;
    if(length<*needed) return STATUS_BUFFER_OVERFLOW;
    {
        static unsigned reports;
        if(__atomic_fetch_add(&reports,1,__ATOMIC_RELAXED)<8)
            fprintf(stderr,"[game-effects-v1] preference=%s saved=%s%u effective=1 texture-and-resolution=unchanged\n",
                mgp_preference(name)==1?"shadow-quality":"anti-aliasing",missing?"default/":"",saved);
    }
    return STATUS_SUCCESS;
}

static int madeira_graphics_preserve(HANDLE key,const UNICODE_STRING *name,ULONG type,
    const void *data,ULONG length,MadeiraRegistryQuery raw)
{
    DWORD value=0,saved;int missing;OBJECT_BASIC_INFORMATION basic;ULONG needed;
    if(type!=REG_DWORD || length!=sizeof(value) || !data) return 0;
    memcpy(&value,data,sizeof(value));
    if(value!=1 || !mgp_preference(name) || !MGP_SCOPE_ACTIVE() || !mgp_game_key(key)) return 0;
    /* Do not turn a denied write into success. Other values always reach the
     * original setter, including a new lower setting chosen in the game. */
    if(NtQueryObject(key,ObjectBasicInformation,&basic,sizeof(basic),&needed) ||
       !(basic.GrantedAccess&KEY_SET_VALUE)) return 0;
    if(!mgp_read_cap(key,name,raw,&saved,&missing)) return 0;
    return 1;
}
