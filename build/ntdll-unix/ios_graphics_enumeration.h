/* SPDX-License-Identifier: LGPL-2.1-or-later
 * The same transient preferences when a Unity version loads its registry by
 * enumeration. Virtual defaults exist only in this view, never in user.reg. */
#pragma once
typedef NTSTATUS (WINAPI *MadeiraRegistryEnumerate)(HANDLE,ULONG,KEY_VALUE_INFORMATION_CLASS,void *,DWORD,DWORD *);
static const char *mgp_virtual_names[]={"shadowQualityIndex_h534661776","antiAliasingIndex_h3690569407"};
static const char *mgp_plain_names[]={"shadowQualityIndex","antiAliasingIndex"};
static UNICODE_STRING mgp_wide(const char *s,WCHAR *buffer)
{
    unsigned n=0;while(s[n]) {buffer[n]=(unsigned char)s[n];++n;}
    UNICODE_STRING result={n*sizeof(WCHAR),n*sizeof(WCHAR),buffer};return result;
}
static unsigned mgp_missing_preferences(HANDLE key,MadeiraRegistryQuery raw,unsigned missing[2])
{
    unsigned count=0;WCHAR name[64];union{uint64_t align;unsigned char bytes[32];}data;DWORD needed;
    for(unsigned i=0;i<2;++i) {
        UNICODE_STRING n=mgp_wide(mgp_virtual_names[i],name);
        if(raw(key,&n,KeyValuePartialInformation,&data,sizeof(data),&needed)!=STATUS_OBJECT_NAME_NOT_FOUND) continue;
        n=mgp_wide(mgp_plain_names[i],name);
        if(raw(key,&n,KeyValuePartialInformation,&data,sizeof(data),&needed)==STATUS_OBJECT_NAME_NOT_FOUND) missing[count++]=i;
    }
    return count;
}
static NTSTATUS madeira_graphics_enumerate(NTSTATUS status,HANDLE key,ULONG index,
    KEY_VALUE_INFORMATION_CLASS cls,void *info,DWORD length,DWORD *needed,
    MadeiraRegistryEnumerate raw_enum,MadeiraRegistryQuery raw_query)
{
    union{uint64_t align;unsigned char bytes[512];} storage;
    KEY_VALUE_FULL_INFORMATION *full=(void *)storage.bytes;
    UNICODE_STRING name;DWORD required=0;
    if(status!=STATUS_SUCCESS && status!=STATUS_BUFFER_TOO_SMALL && status!=STATUS_BUFFER_OVERFLOW &&
       status!=STATUS_NO_MORE_ENTRIES) return status;
    if(!MGP_SCOPE_ACTIVE() || !mgp_game_key(key)) return status;
    NTSTATUS fetched=raw_enum(key,index,KeyValueFullInformation,full,sizeof(storage.bytes),&required);
    if(fetched==STATUS_SUCCESS || fetched==STATUS_BUFFER_OVERFLOW) {
        if(required<FIELD_OFFSET(KEY_VALUE_FULL_INFORMATION,Name) ||
           (full->NameLength&1) || full->NameLength>sizeof(storage.bytes)-FIELD_OFFSET(KEY_VALUE_FULL_INFORMATION,Name)) return status;
        name=(UNICODE_STRING){full->NameLength,full->NameLength,full->Name};
        NTSTATUS adjusted=madeira_graphics_query(status,key,&name,cls,info,length,needed,raw_query);
        return adjusted==STATUS_BUFFER_TOO_SMALL?STATUS_BUFFER_OVERFLOW:adjusted;
    }
    if(fetched==STATUS_NO_MORE_ENTRIES) {
        KEY_FULL_INFORMATION *key_info=(void *)storage.bytes;WCHAR buffer[64];unsigned missing[2];
        NTSTATUS read=MGP_QUERY_KEY(key,KeyFullInformation,key_info,sizeof(storage.bytes),&required);
        if((read!=STATUS_SUCCESS && read!=STATUS_BUFFER_OVERFLOW) ||
           required<FIELD_OFFSET(KEY_FULL_INFORMATION,Class) || index<key_info->Values) return status;
        unsigned offset=index-key_info->Values,count=mgp_missing_preferences(key,raw_query,missing);
        if(offset>=count) return status;
        name=mgp_wide(mgp_virtual_names[missing[offset]],buffer);
        NTSTATUS adjusted=madeira_graphics_query(STATUS_OBJECT_NAME_NOT_FOUND,key,&name,cls,info,length,needed,raw_query);
        return adjusted==STATUS_BUFFER_TOO_SMALL?STATUS_BUFFER_OVERFLOW:adjusted;
    }
    return status;
}
static void madeira_graphics_key_info(NTSTATUS status,HANDLE key,KEY_INFORMATION_CLASS cls,
    void *info,DWORD length,MadeiraRegistryQuery raw)
{
    unsigned minimum=cls==KeyFullInformation?FIELD_OFFSET(KEY_FULL_INFORMATION,Class):
                     cls==KeyCachedInformation?sizeof(KEY_CACHED_INFORMATION):0;
    unsigned missing[2],count;
    if(!minimum || length<minimum || (status!=STATUS_SUCCESS && status!=STATUS_BUFFER_OVERFLOW) ||
       !MGP_SCOPE_ACTIVE() || !mgp_game_key(key)) return;
    count=mgp_missing_preferences(key,raw,missing);
    if(!count) return;
    ULONG *values,*max_name,*max_data;
    if(cls==KeyFullInformation) {
        KEY_FULL_INFORMATION *p=info;values=&p->Values;max_name=&p->MaxValueNameLen;max_data=&p->MaxValueDataLen;
    } else {
        KEY_CACHED_INFORMATION *p=info;values=&p->Values;max_name=&p->MaxValueNameLen;max_data=&p->MaxValueDataLen;
    }
    if(*values>UINT32_MAX-count) return;
    *values+=count;
    for(unsigned i=0;i<count;++i) {
        ULONG n=strlen(mgp_virtual_names[missing[i]])*sizeof(WCHAR);
        if(*max_name<n) *max_name=n;
    }
    if(*max_data<4) *max_data=4;
}
