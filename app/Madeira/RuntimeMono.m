#import "WineMonoPrefix.h"
int madeira_runtime_prepare_mono(const char *bundle, const char *prefix) {
    @autoreleasepool {
        if (!bundle || !prefix) return 1;
        NSString *state = madeira_prepare_mono_prefix([NSString stringWithUTF8String:bundle], [NSString stringWithUTF8String:prefix]);
        return [@[@"linked", @"already-linked", @"existing-runtime"] containsObject:state] ? 0 : 2;
    }
}
