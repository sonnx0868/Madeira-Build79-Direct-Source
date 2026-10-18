// Offline diagnostic only. Use the installed Apple archive reader/tracer;
// this executable and these private Xcode APIs are never linked into Madeira.
#import <Foundation/Foundation.h>
#import <objc/message.h>
#import <Metal/Metal.h>
#import <dlfcn.h>

@interface NSObject (AppleArchiveReader)
- (id)initWithURL:(NSURL *)url options:(NSInteger)options error:(NSError **)error;
- (BOOL)isReadOnly;
- (NSUInteger)countOfFilenames;
- (NSString *)objectInFilenamesAtIndex:(NSUInteger)index;
- (NSData *)copyDataForFilename:(NSString *)name error:(NSError **)error;
- (id)openFileWithFilename:(NSString *)name error:(NSError **)error;
- (id)initWithCaptureStore:(id)archive;
- (id)deviceInfo;
- (id)deviceProfiles;
- (id)initWithCaptureArchive:(id)archive withFilePath:(NSString *)path withFunctionTracer:(id)tracer;
- (id)performTrace;
- (void)visitFunctionStreamFile:(id)file;
- (NSError *)error;
- (void)closeFile;
- (void)close;
@end

static id jsonValue(id value) {
    if (!value) return NSNull.null;
    if ([value isKindOfClass:NSString.class] || [value isKindOfClass:NSNumber.class]) return value;
    if ([value isKindOfClass:NSDictionary.class]) {
        NSMutableDictionary *result=[NSMutableDictionary dictionary];
        for(id key in value) result[[key description]]=jsonValue(value[key]);
        return result;
    }
    if ([value isKindOfClass:NSArray.class]) {
        NSMutableArray *result=[NSMutableArray array];for(id item in value) [result addObject:jsonValue(item)];return result;
    }
    if ([value isKindOfClass:NSData.class]) {
        id plist=[NSPropertyListSerialization propertyListWithData:value options:0 format:nil error:nil];
        if(plist) return jsonValue(plist);
        return @{ @"class":NSStringFromClass([value class]), @"bytes":@([value length]) };
    }
    return @{ @"class":NSStringFromClass([value class]), @"description":[value description] };
}
int main(int argc,char **argv) { @autoreleasepool {
    if(argc<4) return 2;
    NSString *frameworks=@(argv[1]),*path=@(argv[2]),*out=@(argv[3]);
    // Diagnostic process only: initialize the system Metal API metadata too.
    [MTLCaptureManager sharedCaptureManager];
    [[NSBundle bundleWithPath:@"/System/Library/PrivateFrameworks/MetalTools.framework"] load];
    for(NSString *name in @[@"MTLTools",@"MTLToolsServices",@"MTLToolsAnalysisEngine"]) {
        NSError *error=nil;
        BOOL loaded=[[NSBundle bundleWithPath:[frameworks stringByAppendingPathComponent:[name stringByAppendingString:@".framework"]]] loadAndReturnError:&error];
        if(!loaded) { NSLog(@"Reader framework load failed: %@",error);return 3; }
    }
    NSError *error=nil;
    id archive=[[NSClassFromString(@"DYCaptureArchive") alloc] initWithURL:[NSURL fileURLWithPath:path] options:0 error:&error];
    if(!archive) { NSLog(@"Archive open failed: %@",error);return 4; }
    NSMutableArray *names=[NSMutableArray array];
    for(NSUInteger i=0;i<[archive countOfFilenames];i++) [names addObject:[archive objectInFilenamesAtIndex:i]];
    id info=[[NSClassFromString(@"DYMTLCaptureSessionInfo") alloc] initWithCaptureStore:archive];
    NSMutableDictionary *report=[@{@"files":names,@"readOnly":@([archive isReadOnly]),@"deviceInfo":jsonValue([info deviceInfo]),@"deviceProfiles":jsonValue([info deviceProfiles])} mutableCopy];
    for(NSString *name in @[@"(device profile)",@"(device info)",@"(control device info)"]) {
        NSData *data=[archive copyDataForFilename:name error:&error];
        if(data) report[name]=jsonValue(data);
    }
    NSData *data=[NSJSONSerialization dataWithJSONObject:report options:NSJSONWritingPrettyPrinted|NSJSONWritingSortedKeys error:&error];
    if(!data || ![data writeToFile:[out stringByAppendingString:@"-apple-info.json"] options:NSDataWritingAtomic error:&error]) {NSLog(@"Report failed: %@",error);return 5;}
    id tracer=[[NSClassFromString(@"DYFunctionTracer") alloc] init];
    for(NSString *selector in @[@"setCompact:",@"setEmitErrors:",@"setPrependReceiverArgument:"]) {
        SEL sel=NSSelectorFromString(selector);
        if([tracer respondsToSelector:sel]) ((void(*)(id,SEL,BOOL))objc_msgSend)(tracer,sel,YES);
    }
    id visitor=[[NSClassFromString(@"DYCaptureArchiveTraceToFile") alloc] initWithCaptureArchive:archive withFilePath:[out stringByAppendingString:@"-apple-api.txt"] withFunctionTracer:tracer];
    id result=[visitor performTrace];
    [visitor closeFile];
    NSLog(@"Apple archive reader files=%lu readonly=%d trace=%@ error=%@",(unsigned long)names.count,[archive isReadOnly],result,[visitor error]);
    id frames=[[NSClassFromString(@"DYCaptureArchiveTraceToFile") alloc] initWithCaptureArchive:archive withFilePath:[out stringByAppendingString:@"-apple-frames.txt"] withFunctionTracer:tracer];
    id capture=[archive openFileWithFilename:@"capture" error:&error];
    if(capture) [frames visitFunctionStreamFile:capture];
    [frames closeFile];
    const char *(*lookup)(uint32_t)=dlsym(RTLD_DEFAULT,"GetFuncEnumAsString");
    NSMutableDictionary *enumNames=[NSMutableDictionary dictionary];
    NSData *stream=[NSData dataWithContentsOfFile:[path stringByAppendingPathComponent:@"capture"]];
    const uint8_t *bytes=stream.bytes;NSUInteger position=8;
    while(position+8<=stream.length) {
        uint32_t length=0,code=0;memcpy(&length,bytes+position,4);memcpy(&code,bytes+position+4,4);
        if(length<8 || position+length>stream.length) return 7;
        const char *name=lookup?lookup(code):NULL;
        if(name && name[0]>=32 && name[0]<127) {
            NSString *value=[[NSString alloc] initWithBytes:name length:strnlen(name,1024) encoding:NSUTF8StringEncoding];
            if(value) enumNames[[NSString stringWithFormat:@"%d",(int32_t)code]]=value;
        }
        position+=length;
    }
    NSData *mapping=[NSJSONSerialization dataWithJSONObject:enumNames options:NSJSONWritingPrettyPrinted|NSJSONWritingSortedKeys error:&error];
    [mapping writeToFile:[out stringByAppendingString:@"-apple-function-names.json"] atomically:YES];
    NSLog(@"Apple function enum labels=%lu streamBytes=%lu framedEnd=%lu",(unsigned long)enumNames.count,(unsigned long)stream.length,(unsigned long)position);
    // Descriptors may be embedded in the archive index instead of loose files.
    // Export only hashed shader/PSO metadata, never texture/buffer contents.
    NSString *metadataOut=[out stringByAppendingString:@"-shader-metadata"];
    [[NSFileManager defaultManager] createDirectoryAtPath:metadataOut withIntermediateDirectories:YES attributes:nil error:nil];
    NSRegularExpression *hashName=[NSRegularExpression regularExpressionWithPattern:@"^[0-9A-F]{1,16}$" options:0 error:nil];
    NSUInteger exportedBytes=0, exportedFiles=0;
    for(NSString *name in names) {
        if(![hashName firstMatchInString:name options:0 range:NSMakeRange(0,name.length)]) continue;
        NSData *blob=[archive copyDataForFilename:name error:nil];
        if(blob.length<8 || blob.length>4*1024*1024) continue;
        const char *magic=blob.bytes;
        if(memcmp(magic,"bplist00",8) && memcmp(magic,"MTLB",4)) continue;
        if(exportedBytes+blob.length>128*1024*1024) return 8;
        if(![blob writeToFile:[metadataOut stringByAppendingPathComponent:name] atomically:YES]) return 9;
        exportedBytes+=blob.length;exportedFiles++;
    }
    NSLog(@"Shader metadata files=%lu bytes=%lu",(unsigned long)exportedFiles,(unsigned long)exportedBytes);
    // Inspect the reserved capture metadata for embedded timing/profile data.
    // No environment or account information is exported.
    for(NSString *name in @[@"gttrace-dump",@"MTLCaptureDescriptor"]) {
        uint64_t infoWords[4]={0};
        SEL selector=NSSelectorFromString(@"getInfo:forFilename:error:");
        BOOL available=((BOOL(*)(id,SEL,void*,id,NSError**))objc_msgSend)(archive,selector,infoWords,name,&error);
        NSLog(@"Archive metadata %@ info=%d fields=%llu,%llu,%llu,%llu",name,available,infoWords[0],infoWords[1],infoWords[2],infoWords[3]);
        if(available && infoWords[0]<8*1024*1024 && infoWords[1]<8*1024*1024) {
            NSData *blob=[archive copyDataForFilename:name error:&error];
            if(blob && blob.length<8*1024*1024) [blob writeToFile:[out stringByAppendingFormat:@"-%@.bin",name] atomically:YES];
        }
    }
    [archive close];
    return [visitor error]?6:0;
} }
