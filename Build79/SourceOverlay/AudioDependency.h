#ifndef MADEIRA_AUDIO_DEPENDENCY_H
#define MADEIRA_AUDIO_DEPENDENCY_H
#include <stddef.h>
#include <stdint.h>
/* Public POSIX calls; SDK-independent declarations for the Linux cross build. */
extern int openat(int, const char *, int, ...), close(int), fsync(int);
extern int linkat(int, const char *, int, const char *, int);
extern int unlinkat(int, const char *, int);
extern long read(int, void *, unsigned long), write(int, const void *, unsigned long);
#ifdef __APPLE__
typedef long long MDAOffset;
#define MDA_DIR (0x0100 | 0x100000 | 0x01000000)
#define MDA_READ (0x0100 | 0x0004 | 0x01000000)
#define MDA_CREATE (0x0001 | 0x0200 | 0x0800 | 0x0100 | 0x01000000)
#else
#include <fcntl.h>
typedef long MDAOffset;
#define MDA_DIR (O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC)
#define MDA_READ (O_RDONLY | O_NONBLOCK | O_NOFOLLOW | O_CLOEXEC)
#define MDA_CREATE (O_WRONLY | O_CREAT | O_EXCL | O_NOFOLLOW | O_CLOEXEC)
#endif
extern MDAOffset lseek(int, MDAOffset, int);
enum { MDA_CREATED, MDA_MATCHES, MDA_PRESERVED, MDA_ERROR, MDA_NOT_READY };
static int mda_payload_valid(const unsigned char *p, size_t n) {
    if (!p || n != 518488 || p[0]!='M' || p[1]!='Z') return 0;
    uint32_t pe = (uint32_t)p[60] | ((uint32_t)p[61]<<8) |
        ((uint32_t)p[62]<<16) | ((uint32_t)p[63]<<24);
    return pe <= n-6 && p[pe]=='P' && p[pe+1]=='E' &&
        !p[pe+2] && !p[pe+3] && p[pe+4]==0x64 && p[pe+5]==0x86;
}
/* No following symlinked prefix components or altering directory contents. */
static int mda_system_directory(int documents) {
    const char *parts[] = {"wine", "drive_c", "windows", "system32"};
    int current = documents;
    for (unsigned i=0;i<4;i++) {
        int next = openat(current, parts[i], MDA_DIR);
        if (current != documents) close(current);
        if (next < 0) return -1;
        current = next;
    }
    return current;
}
static int mda_matches(int directory, const char *name, const unsigned char *p, size_t n) {
    int fd = openat(directory, name, MDA_READ);
    if (fd < 0) return 0;
    int matches = lseek(fd, 0, 2)==(long long)n && lseek(fd, 0, 0)==0;
    unsigned char buffer[4096];
    size_t pos=0;
    while (matches && pos<n) {
        size_t count=n-pos; if (count>sizeof buffer) count=sizeof buffer;
        long got=read(fd,buffer,count);
        if (got<=0) {matches=0;break;}
        for (long i=0;i<got;i++) if(buffer[i]!=p[pos+(size_t)i]) {matches=0;break;}
        pos+=(size_t)got;
    }
    if (close(fd)) matches=0;
    return matches;
}
/* All names are fixed or generated locally. linkat cannot replace an existing
 * destination, including a dangling symlink. A failed stage is never a DLL. */
static int mda_publish(int directory, const char *temporary, const unsigned char *p, size_t n) {
    if (!mda_payload_valid(p,n)) return MDA_ERROR;
    const char *destination="xaudio2_7.dll";
    if (mda_matches(directory,destination,p,n)) return MDA_MATCHES;
    int fd=openat(directory,temporary,MDA_CREATE,0600);
    if (fd<0) return MDA_ERROR;
    int ok=1;size_t pos=0;
    while (pos<n) {
        long count=write(fd,p+pos,n-pos);
        if(count<=0) {ok=0;break;}
        pos+=(size_t)count;
    }
    if(ok && fsync(fd)) ok=0;
    if(close(fd)) ok=0;
    if(ok && !mda_matches(directory,temporary,p,n)) ok=0;
    int result=MDA_ERROR;
    if(ok) {
        if(linkat(directory,temporary,directory,destination,0))
            result=mda_matches(directory,destination,p,n)?MDA_MATCHES:MDA_PRESERVED;
        else result=mda_matches(directory,destination,p,n)?MDA_CREATED:MDA_ERROR;
    }
    unlinkat(directory,temporary,0); /* Only our exclusively created stage. */
    return result;
}
#endif
