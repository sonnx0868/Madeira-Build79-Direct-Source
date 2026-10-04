#import <Foundation/Foundation.h>
#import <Metal/Metal.h>
#include <stdint.h>

uint64_t madeira_mtl_new_library_source(uint64_t device_handle,
                                        const char *source,
                                        uint64_t source_length,
                                        uint64_t *error_out)
{
    id<MTLDevice> device = (id<MTLDevice>)(uintptr_t)device_handle;
    NSString *text = [[NSString alloc] initWithBytes:source
                                              length:(NSUInteger)source_length
                                            encoding:NSUTF8StringEncoding];
    MTLCompileOptions *options = [[MTLCompileOptions alloc] init];
    options.languageVersion = MTLLanguageVersion3_1;

    NSError *error = nil;
    id<MTLLibrary> library = text ? [device newLibraryWithSource:text
                                                         options:options
                                                           error:&error] : nil;
    [options release];
    [text release];

    if (error_out) *error_out = (uint64_t)(uintptr_t)error;
    return (uint64_t)(uintptr_t)library;
}

