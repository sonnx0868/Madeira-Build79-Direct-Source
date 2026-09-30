#import "Platform.h"
#include "AudioDependency.h"
extern int open(const char *, int, ...);
extern unsigned int arc4random(void);
extern void madeira_resolution_trace(NSString *message);

void MadeiraAudioProvision(void) {
    static BOOL finished;
    if(finished) return;
    NSString *documents=NSSearchPathForDirectoriesInDomains(9,1,YES)[0];
    int root=open(documents.UTF8String,MDA_DIR);
    if(root<0) return;
    int directory=mda_system_directory(root);
    close(root);
    if(directory<0) return; /* Fresh prefix is populated from its template. */
    NSString *system=[documents stringByAppendingPathComponent:@"wine/drive_c/windows/system32"];
    NSArray<NSString *> *entries=[[NSFileManager defaultManager] contentsOfDirectoryAtPath:system error:0];
    if(!entries) {close(directory);return;}
    for(NSString *name in entries) {
        if([name.lowercaseString isEqualToString:@"xaudio2_7.dll"] &&
           ![name isEqualToString:@"xaudio2_7.dll"]) {
            madeira_resolution_trace(@"[audio-dependency] Existing differently cased XAudio DLL preserved");
            close(directory);finished=YES;return;
        }
    }
    /* Locate our actual dylib's app directory, including LiveContainer guests;
     * never use a host app's mainBundle to select the dependency. */
    NSString *app=nil;
    for(unsigned i=0;i<_dyld_image_count();i++) {
        NSString *path=[NSString stringWithUTF8String:_dyld_get_image_name(i)];
        if([path.lastPathComponent isEqualToString:@"MadeiraIPadUI.dylib"])
            app=path.stringByDeletingLastPathComponent.stringByDeletingLastPathComponent;
    }
    NSData *data=app?[NSData dataWithContentsOfFile:[app stringByAppendingPathComponent:@"AudioSupport/xaudio2_7.dll"]]:nil;
    if(!mda_payload_valid(data.bytes,data.length)) {
        madeira_resolution_trace(@"[audio-dependency] Bundled x64 XAudio resource unavailable or invalid");
        close(directory);finished=YES;return;
    }
    NSString *temporary=[NSString stringWithFormat:@".madeira-audio-%08x.part",arc4random()];
    int result=mda_publish(directory,temporary.UTF8String,data.bytes,data.length);
    close(directory);
    madeira_resolution_trace([NSString stringWithFormat:@"[audio-dependency] XAudio 2.7 provisioning=%d (0=installed, 1=matches, 2=conflict preserved, 3=error)",result]);
    if(result!=MDA_ERROR) finished=YES;
}
