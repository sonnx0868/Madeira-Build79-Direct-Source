/* Included after trace(): isolated UI integration, no startup constructor. */
#include "ControllerInput.h"
#include "OverrideMerge.h"
#include "ControllerRetry.h"
#include "ControllerGameDetect.h"
@interface NSData (ControllerFileAccess)
+ (instancetype)dataWithContentsOfFile:(NSString *)path;
- (BOOL)isEqualToData:(NSData *)data;
@end
@interface NSBundle : NSObject
+ (instancetype)mainBundle;
@property(readonly) NSString *bundlePath;
@end
@interface NSFileManager (ControllerFileAccess)
- (BOOL)removeItemAtPath:(NSString *)path error:(NSError **)error;
- (NSString *)destinationOfSymbolicLinkAtPath:(NSString *)path error:(NSError **)error;
@end

extern int open(const char *, int, ...), close(int), fsync(int);
extern int link(const char *, const char *), unlink(const char *);
extern int rename(const char *, const char *);
extern long pread(int, void *, unsigned long, long long);
extern long write(int, const void *, unsigned long);
extern unsigned int arc4random(void);

static BOOL controllerSessionEnabled, controllerEnvironmentReady, controllerNativeStarted;
static BOOL controllerDeploying;
static unsigned controllerScanTicks;
static MCRetry controllerRetry;
static NSString *controllerStateLabel = @"Off";
static NSString *controllerDeploymentLabel = @"";
static NSArray<NSString *> *controllerDLLNames(void) {
    return @[@"xinput1_3.dll", @"xinput1_4.dll", @"xinput9_1_0.dll"];
}
static NSString *controllerResource(NSString *name) {
    return [[[NSBundle mainBundle].bundlePath stringByAppendingPathComponent:@"ControllerSupport"] stringByAppendingPathComponent:name];
}
static NSString *controllerLegacyResource(NSString *name) {
    return controllerResource([@"Legacy" stringByAppendingPathComponent:name]);
}
static NSString *controllerGamesPath(void) {
    return [documents() stringByAppendingPathComponent:@"wine/drive_c/Games"];
}

/* Publish a fully written, verified same-directory temporary inode with link(2).
 * Unlike replacing rename(), link fails if the destination already exists.
 * A crash during writing can leave only a .tmp file, never a partial game DLL. */
static BOOL controllerInstallVerified(NSData *data, NSString *destination, NSData *previous) {
    NSString *temporary = nil;
    int fd = -1;
    for (unsigned attempt = 0; attempt < 8 && fd < 0; ++attempt) {
        temporary = [destination.stringByDeletingLastPathComponent stringByAppendingPathComponent:
            [NSString stringWithFormat:@".madeira-controller-%08x.tmp", arc4random()]];
        // Darwin O_WRONLY | O_CREAT | O_EXCL | O_NOFOLLOW | O_CLOEXEC.
        fd = open(temporary.UTF8String, 0x0001 | 0x0200 | 0x0800 | 0x0100 | 0x01000000, 0600);
    }
    if (fd < 0) return NO;
    const unsigned char *bytes = data.bytes;
    NSUInteger written = 0;
    BOOL complete = YES;
    while (written < data.length) {
        long count = write(fd, bytes + written, data.length - written);
        if (count <= 0) { complete = NO; break; }
        written += (NSUInteger)count;
    }
    if (complete && fsync(fd)) complete = NO;
    if (close(fd)) complete = NO;
    if (complete && ![[NSData dataWithContentsOfFile:temporary] isEqualToData:data]) complete = NO;
    if (complete && previous) {
        // Upgrade only exact bundled R6 bytes, preserving the old inode as a
        // same-directory backup. Foreign DLLs/symlinks/backup collisions stay.
        NSFileManager *fm = [NSFileManager defaultManager];
        NSString *backup = [destination stringByAppendingString:@".madeira-r6-controller-backup"];
        if ([fm destinationOfSymbolicLinkAtPath:destination error:0] ||
            [fm destinationOfSymbolicLinkAtPath:backup error:0] ||
            ![[NSData dataWithContentsOfFile:destination] isEqualToData:previous]) complete = NO;
        if (complete && ![fm fileExistsAtPath:backup isDirectory:0] &&
            link(destination.UTF8String, backup.UTF8String)) complete = NO;
        if (complete && (![[NSData dataWithContentsOfFile:backup] isEqualToData:previous] ||
            ![[NSData dataWithContentsOfFile:destination] isEqualToData:previous])) complete = NO;
        if (complete && rename(temporary.UTF8String, destination.UTF8String)) complete = NO;
        if (complete) trace([NSString stringWithFormat:@"Legacy game XInput upgraded with backup: %@", destination.lastPathComponent]);
    } else if (complete && link(temporary.UTF8String, destination.UTF8String)) complete = NO;
    unlink(temporary.UTF8String);
    return complete;
}

/* Check all members before copying any; case-insensitive collision detection
 * reflects Windows loading even on a case-sensitive host filesystem. */
static BOOL controllerDeployFolder(NSString *directory, BOOL enabled) {
    NSFileManager *fm = [NSFileManager defaultManager];
    NSMutableArray<NSString *> *names = [NSMutableArray new];
    for (NSString *name in controllerDLLNames()) [names addObject:name];
    NSArray<NSString *> *existing = [fm contentsOfDirectoryAtPath:directory error:0];
    for (NSString *name in names) {
        NSData *ours = [NSData dataWithContentsOfFile:controllerResource(name)];
        NSData *legacy = [NSData dataWithContentsOfFile:controllerLegacyResource(name)];
        if (!ours.length) { trace(@"Controller resource missing; deployment skipped"); return NO; }
        for (NSString *item in existing) {
            if (![item.lowercaseString isEqualToString:name.lowercaseString]) continue;
            NSString *path = [directory stringByAppendingPathComponent:item];
            BOOL symlink = [fm destinationOfSymbolicLinkAtPath:path error:0] != nil;
            NSData *current = [NSData dataWithContentsOfFile:path];
            if (symlink || (![current isEqualToData:ours] && !(legacy.length && [current isEqualToData:legacy]))) {
                if (enabled) {
                    trace([NSString stringWithFormat:@"Controller conflict preserved: %@/%@", directory.lastPathComponent, item]);
                    return NO;
                }
                continue; // Disabling must never delete a foreign DLL.
            }
            if (!enabled) [fm removeItemAtPath:path error:0];
        }
    }
    if (!enabled) return YES;
    if (![fm createDirectoryAtPath:directory withIntermediateDirectories:YES attributes:nil error:0]) return NO;
    for (NSString *name in names) {
        BOOL present = NO;
        NSData *ours = [NSData dataWithContentsOfFile:controllerResource(name)];
        NSData *legacy = [NSData dataWithContentsOfFile:controllerLegacyResource(name)];
        for (NSString *item in existing) {
            if (![item.lowercaseString isEqualToString:name.lowercaseString]) continue;
            present = YES;
            NSString *path = [directory stringByAppendingPathComponent:item];
            NSData *current = [NSData dataWithContentsOfFile:path];
            if (![current isEqualToData:ours] &&
                (!legacy.length || ![current isEqualToData:legacy] || !controllerInstallVerified(ours, path, legacy))) return NO;
        }
        if (!present && !controllerInstallVerified([NSData dataWithContentsOfFile:controllerResource(name)],
            [directory stringByAppendingPathComponent:name], nil)) return NO;
    }
    return YES;
}

static int controllerReadPE(void *context,uint32_t offset,void *bytes,size_t length) {
    // Limit I/O per candidate, never read a complete game executable or archive.
    int *state=context;
    if (state[1]<=0 || length>4096) return 0;
    --state[1];
    return pread(state[0],bytes,length,offset)==(long)length;
}
static BOOL controllerExecutableSupported(NSString *path) {
    int fd=open(path.UTF8String,0x0100 | 0x01000000); // RDONLY, NOFOLLOW, CLOEXEC
    if(fd<0) return NO;
    int state[2]={fd,4096};
    BOOL supported=mc_pe_game_uses_xinput(controllerReadPE,state)!=0;
    close(fd);
    return supported;
}

/* Bounded, no-symlink traversal of Games. Recognize real x64 XInput imports
 * (normal and delay), not arbitrary installers or EXEs of other architectures.
 * Keep the verified BeamNG path, which may use dynamic loading. */
static void controllerScanFolder(NSString *root, unsigned depth, unsigned *budget,
                                  unsigned *prepared, unsigned *conflicts) {
    if (!*budget || depth > 5) return;
    --*budget;
    NSFileManager *fm = [NSFileManager defaultManager];
    if ([fm destinationOfSymbolicLinkAtPath:root error:0]) return;
    NSArray<NSString *> *entries = [fm contentsOfDirectoryAtPath:root error:0];
    for (NSString *name in entries) {
        if (!*budget) break;
        if (![name.lowercaseString hasSuffix:@".exe"]) continue;
        --*budget;
        NSString *exe = [root stringByAppendingPathComponent:name];
        BOOL directory = NO;
        if (![fm destinationOfSymbolicLinkAtPath:exe error:0] &&
            [fm fileExistsAtPath:exe isDirectory:&directory] && !directory &&
            (controllerExecutableSupported(exe) ||
             [name.lowercaseString isEqualToString:@"beamng.drive.x64.exe"])) {
            if (controllerDeployFolder(root, YES)) ++*prepared; else ++*conflicts;
            break; // One deployment per folder, even with multiple game EXEs.
        }
    }
    for (NSString *name in entries) {
        if (!*budget) break;
        if (!name.length || [name hasSuffix:@".tmp"]) continue;
        NSString *child = [root stringByAppendingPathComponent:name];
        BOOL directory = NO;
        if ([fm fileExistsAtPath:child isDirectory:&directory] && directory)
            controllerScanFolder(child, depth + 1, budget, prepared, conflicts);
    }
}

static void controllerPrepareGames(void) {
    if (controllerDeploying) return;
    controllerDeploying = YES;
    NSString *games = controllerGamesPath();
    dispatch_async(dispatch_get_global_queue(0, 0), ^{
        @autoreleasepool {
            unsigned budget = 1024, prepared = 0, conflicts = 0;
            controllerScanFolder(games, 0, &budget, &prepared, &conflicts);
            NSString *label = [NSString stringWithFormat:@"%u game folders ready · %u conflicts preserved%@",
                prepared, conflicts, budget ? @"" : @" · scan limit reached"];
            dispatch_async(&_dispatch_main_q, ^{
                controllerDeploying = NO;
                if (![controllerDeploymentLabel isEqualToString:label]) trace(label);
                controllerDeploymentLabel = label;
            });
        }
    });
}

static BOOL controllerEnabled(void) {
    // New installs work by default; retain an explicitly saved Off preference.
    return ![preferences() objectForKey:@"MadeiraIPad.controllersEnabled"] ||
        [preferences() boolForKey:@"MadeiraIPad.controllersEnabled"];
}

static void controllerConfigureEnvironment(void) {
    controllerSessionEnabled = controllerEnabled();
    if (wine_running() || getenv("WINEPREFIX")) {
        controllerStateLabel = @"Wine was already initialized · reopen Madeira once to prepare game input";
        return; // Never pretend setenv can change Wine's cached load-order rules.
    }
    // Prepare native-first loading even when input is Off: Off neutralizes
    // input, not DLL availability. Subsequent On/Off changes need no restart.
    char overrides[32768];
    if (!mc_merge_overrides(getenv("WINEDLLOVERRIDES"), overrides, sizeof(overrides)) ||
        setenv("WINEDLLOVERRIDES", overrides, 1)) {
        controllerStateLabel = @"Game input environment could not be prepared";
        return;
    }
    controllerEnvironmentReady = YES;
    controllerStateLabel = @"Preparing controller input";
    controllerPrepareGames();
    trace(@"Automatic controller load order prepared before Wine startup");
}

static void controllerTryStart(void) {
    // The native writer only needs drive_c, not Wine's .update-timestamp,
    // system.reg, or the c: symlink. Failed starts are recoverable, not latched.
    if (!controllerEnvironmentReady) return;
    if (++controllerScanTicks >= 15) {
        controllerScanTicks = 0;
        controllerPrepareGames();
    }
    if (controllerNativeStarted || !mc_retry_ready(&controllerRetry)) return;
    NSFileManager *fm = [NSFileManager defaultManager];
    NSString *drive = [documents() stringByAppendingPathComponent:@"wine/drive_c"];
    BOOL directory = NO;
    if (![fm fileExistsAtPath:drive isDirectory:&directory] || !directory) {
        controllerStateLabel = @"Start Wine Desktop once to create its game folder";
        return;
    }
    if ([fm destinationOfSymbolicLinkAtPath:controllerGamesPath() error:0]) {
        controllerStateLabel = @"Game input folder is a symlink · preserved; setup blocked";
        return;
    }
    controllerPrepareGames();
    NSString *path = [controllerGamesPath() stringByAppendingPathComponent:@"MadeiraController.bin"];
    int result = MadeiraControllerStart(path.UTF8String);
    controllerNativeStarted = result == 0;
    if (controllerNativeStarted) MadeiraControllerSetInputEnabled(0);
    if (!result) mc_retry_reset(&controllerRetry); else mc_retry_failed(&controllerRetry);
    controllerStateLabel = result == 0 ? @"Game input bridge ready" :
        [NSString stringWithFormat:@"Game input start failed (%d) · retrying automatically", result];
    trace(controllerStateLabel);
}

static NSString *controllerSummary(void) {
    unsigned detected = MadeiraControllerDetectedCount();
    if (!controllerSessionEnabled) return [NSString stringWithFormat:@"Off · %u detected", detected];
    if (!detected) return @"On · waiting for controller";
    return [NSString stringWithFormat:@"%u connected · %@", detected,
        controllerNativeStarted ? @"bridge ready" : @"preparing game input"];
}
