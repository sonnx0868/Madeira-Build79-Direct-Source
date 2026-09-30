#define _GNU_SOURCE
#include "AudioDependency.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
static unsigned char payload[518488];
int main(void) {
    char path[]="/tmp/madeira-audio-test-XXXXXX";
    assert(mkdtemp(path));
    int root=open(path,MDA_DIR);assert(root>=0);
    payload[0]='M';payload[1]='Z';payload[60]=128;
    payload[128]='P';payload[129]='E';payload[132]=0x64;payload[133]=0x86;
    assert(mda_payload_valid(payload,sizeof payload));
    assert(!mda_payload_valid(NULL,sizeof payload));
    assert(!mda_payload_valid(payload,sizeof payload-1));
    assert(mda_system_directory(root)<0);
    assert(mda_publish(root,".stage",payload,sizeof payload)==MDA_CREATED);
    assert(mda_publish(root,".stage",payload,sizeof payload)==MDA_MATCHES);
    assert(faccessat(root,".stage",F_OK,0)<0);
    assert(mda_matches(root,"xaudio2_7.dll",payload,sizeof payload));
    assert(!unlinkat(root,"xaudio2_7.dll",0));
    int fd=openat(root,"xaudio2_7.dll",O_WRONLY|O_CREAT|O_EXCL,0600);
    assert(fd>=0 && write(fd,"foreign",7)==7);close(fd);
    assert(mda_publish(root,".stage",payload,sizeof payload)==MDA_PRESERVED);
    fd=openat(root,"xaudio2_7.dll",O_RDONLY);char foreign[7];
    assert(fd>=0 && read(fd,foreign,7)==7 && !memcmp(foreign,"foreign",7));close(fd);
    assert(!unlinkat(root,"xaudio2_7.dll",0));
    assert(!symlinkat("absent",root,"xaudio2_7.dll"));
    assert(mda_publish(root,".stage",payload,sizeof payload)==MDA_PRESERVED);
    char link[64];assert(readlinkat(root,"xaudio2_7.dll",link,sizeof link)==6);
    assert(!unlinkat(root,"xaudio2_7.dll",0));
    assert(!mkfifoat(root,"xaudio2_7.dll",0600));
    assert(mda_publish(root,".stage",payload,sizeof payload)==MDA_PRESERVED);
    assert(!unlinkat(root,"xaudio2_7.dll",0));
    fd=openat(root,".stage",O_WRONLY|O_CREAT|O_EXCL,0600);assert(fd>=0);close(fd);
    assert(mda_publish(root,".stage",payload,sizeof payload)==MDA_ERROR);
    assert(!unlinkat(root,".stage",0));
    payload[132]=0;assert(mda_publish(root,".stage",payload,sizeof payload)==MDA_ERROR);
    assert(faccessat(root,"xaudio2_7.dll",F_OK,0)<0);payload[132]=0x64;
    assert(!symlinkat(".",root,"wine"));assert(mda_system_directory(root)<0);
    assert(!unlinkat(root,"wine",0));
    const char *parts[]={"wine","drive_c","windows","system32"};
    int directories[5]={root};
    for(unsigned i=0;i<4;i++) {
        assert(!mkdirat(directories[i],parts[i],0700));
        directories[i+1]=openat(directories[i],parts[i],MDA_DIR);assert(directories[i+1]>=0);
    }
    fd=mda_system_directory(root);assert(fd>=0);close(fd);
    for(unsigned i=4;i>0;i--) {close(directories[i]);assert(!unlinkat(directories[i-1],parts[i-1],AT_REMOVEDIR));}
    close(root);assert(!rmdir(path));
    puts("PASS: audio missing-only publication, exact readback, repeats, foreign file/FIFO/symlink preservation, stage collisions, invalid payload and no-symlink prefix traversal");
}
