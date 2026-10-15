/* Offline prefix placement of the verified, bundled Wine Mono runtime. */
#import <Foundation/Foundation.h>

static NSString *madeira_prepare_mono_prefix(NSString *bundle, NSString *prefix)
{
    NSFileManager *fm = NSFileManager.defaultManager;
    NSString *source = [bundle stringByAppendingPathComponent:@"mono/wine-mono-11.0.0"];
    if (![fm fileExistsAtPath:[source stringByAppendingPathComponent:@"bin/libmono-2.0-x86.dll"]] ||
        ![fm fileExistsAtPath:[source stringByAppendingPathComponent:@"lib/mono/4.5/mscorlib.dll"]]) return @"missing-bundle";
    NSString *parent = [prefix stringByAppendingPathComponent:@"drive_c/windows/mono"];
    NSString *target = [parent stringByAppendingPathComponent:@"mono-2.0"];
    NSString *link = [fm destinationOfSymbolicLinkAtPath:target error:nil];
    if (link && [link isEqualToString:source]) return @"already-linked";
    if (link && [link rangeOfString:@".app/mono/wine-mono-"].location == NSNotFound) return @"existing-runtime";
    // Preserve a user's physical MSI-installed runtime. Only replace our
    // bundle symlink, whose destination changes whenever iOS reinstalls IPA.
    if (!link && [fm fileExistsAtPath:target]) return @"existing-runtime";
    if (![fm createDirectoryAtPath:parent withIntermediateDirectories:YES attributes:nil error:nil]) return @"failed";
    if (link && ![fm removeItemAtPath:target error:nil]) return @"failed";
    return [fm createSymbolicLinkAtPath:target withDestinationPath:source error:nil] ? @"linked" : @"failed";
}
