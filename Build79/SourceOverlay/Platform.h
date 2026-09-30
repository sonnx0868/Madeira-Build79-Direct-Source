// Minimal declarations of public Apple APIs used by this local compatibility UI.
// Enables cross-compilation on Linux; no Apple SDK implementation is included.
#if __OBJC_BOOL_IS_BOOL
typedef _Bool BOOL;
#else
typedef signed char BOOL;
#endif
typedef long NSInteger;
typedef unsigned long NSUInteger;
typedef double CGFloat;
typedef struct { CGFloat x, y; } CGPoint;
typedef struct { CGFloat width, height; } CGSize;
typedef struct { CGPoint origin; CGSize size; } CGRect;
static inline CGRect CGRectMake(CGFloat x, CGFloat y, CGFloat w, CGFloat h) { return (CGRect){{x,y},{w,h}}; }
#define YES ((BOOL)1)
#define NO ((BOOL)0)
#define nil ((id)0)
typedef struct { unsigned long state; id __unsafe_unretained *itemsPtr; unsigned long *mutationsPtr; unsigned long extra[5]; } NSFastEnumerationState;
@protocol NSObject @end
@protocol NSFastEnumeration
- (NSUInteger)countByEnumeratingWithState:(NSFastEnumerationState *)state objects:(id __unsafe_unretained *)buffer count:(NSUInteger)len;
@end
@class NSString, NSArray, NSDictionary, NSError, NSData, NSURL, NSBundle;
// Match NSObject's object-header layout. Clang uses fixed ivar offsets for our
// direct NSObject subclasses, so omitting isa would place their first fields
// over the real object's class pointer.
__attribute__((objc_root_class)) @interface NSObject <NSObject> { Class isa; }
+ (id)alloc; + (id)new; + (Class)class;
- (id)init; - (BOOL)respondsToSelector:(SEL)s; - (BOOL)isKindOfClass:(Class)c;
@end
@interface NSString : NSObject
+ (instancetype)stringWithUTF8String:(const char *)s;
+ (instancetype)stringWithFormat:(NSString *)format, ...;
+ (instancetype)stringWithContentsOfFile:(NSString *)path encoding:(NSUInteger)encoding error:(NSError **)error;
- (NSString *)stringByAppendingPathComponent:(NSString *)s;
- (NSString *)stringByAppendingString:(NSString *)s;
- (NSString *)stringByReplacingOccurrencesOfString:(NSString *)old withString:(NSString *)replacement;
- (NSData *)dataUsingEncoding:(NSUInteger)encoding;
- (NSString *)stringByDeletingLastPathComponent;
- (NSString *)stringByDeletingPathExtension;
- (NSString *)lastPathComponent;
- (NSString *)pathExtension;
- (NSString *)lowercaseString;
- (BOOL)isEqualToString:(NSString *)s;
- (BOOL)containsString:(NSString *)s;
- (BOOL)hasSuffix:(NSString *)s;
- (NSString *)substringFromIndex:(NSUInteger)i;
- (NSString *)substringToIndex:(NSUInteger)i;
- (BOOL)writeToFile:(NSString *)path atomically:(BOOL)a encoding:(NSUInteger)e error:(NSError **)error;
@property(readonly) NSUInteger length;
@property(readonly) const char *UTF8String;
@end
@interface NSData : NSObject
+ (instancetype)dataWithContentsOfFile:(NSString *)path;
@property(readonly) NSUInteger length;
@property(readonly) const void *bytes;
- (BOOL)isEqualToData:(NSData *)data;
- (BOOL)writeToFile:(NSString *)path atomically:(BOOL)atomic;
@end
@interface NSMutableData : NSData
- (void)appendBytes:(const void *)bytes length:(NSUInteger)length;
- (void)appendData:(NSData *)data;
@end
@interface NSArray<ObjectType> : NSObject <NSFastEnumeration>
+ (instancetype)arrayWithObjects:(const id [])objects count:(NSUInteger)count;
@property(readonly) NSUInteger count;
- (ObjectType)objectAtIndexedSubscript:(NSUInteger)i;
- (NSArray *)sortedArrayUsingSelector:(SEL)s;
@end
@interface NSMutableArray<ObjectType> : NSArray<ObjectType>
- (void)addObject:(ObjectType)obj;
@end
@interface NSFileManager : NSObject
+ (instancetype)defaultManager;
- (NSArray<NSString *> *)contentsOfDirectoryAtPath:(NSString *)path error:(NSError **)error;
- (BOOL)fileExistsAtPath:(NSString *)path isDirectory:(BOOL *)directory;
- (BOOL)createDirectoryAtPath:(NSString *)path withIntermediateDirectories:(BOOL)intermediate attributes:(NSDictionary *)attributes error:(NSError **)error;
- (BOOL)removeItemAtPath:(NSString *)path error:(NSError **)error;
@end
@interface NSUserDefaults : NSObject
+ (instancetype)standardUserDefaults;
- (id)objectForKey:(NSString *)key;
- (NSInteger)integerForKey:(NSString *)key;
- (BOOL)boolForKey:(NSString *)key;
- (void)setInteger:(NSInteger)value forKey:(NSString *)key;
- (void)setBool:(BOOL)value forKey:(NSString *)key;
@end
@interface NSTimer : NSObject
+ (NSTimer *)scheduledTimerWithTimeInterval:(double)t repeats:(BOOL)r block:(void (^)(NSTimer *))block;
@end
@interface NSProcessInfo : NSObject
+ (NSProcessInfo *)processInfo;
@property(readonly) NSInteger thermalState;
@property(readonly, getter=isLowPowerModeEnabled) BOOL lowPowerModeEnabled;
@end
@interface NSURL : NSObject
+ (instancetype)fileURLWithPath:(NSString *)path;
@end
extern NSArray<NSString *> *NSSearchPathForDirectoriesInDomains(NSUInteger, NSUInteger, BOOL);
@class UIView, UIWindow, UIWindowScene, UIViewController, UIColor, UIFont, UIEvent, CALayer, UIGestureRecognizer;
@interface UIColor : NSObject
+ (instancetype)clearColor; + (instancetype)whiteColor;
+ (instancetype)labelColor; + (instancetype)systemBackgroundColor;
- (UIColor *)resolvedColorWithTraitCollection:(id)traits;
+ (instancetype)colorWithWhite:(CGFloat)white alpha:(CGFloat)alpha;
+ (instancetype)colorWithRed:(CGFloat)r green:(CGFloat)g blue:(CGFloat)b alpha:(CGFloat)a;
@property(readonly) void *CGColor;
@end
@interface NSObject (GlassConfigurationRuntime)
+ (id)glassButtonConfiguration;
+ (id)clearGlassButtonConfiguration;
- (void)setTitle:(NSString *)title;
- (void)setBaseForegroundColor:(UIColor *)color;
- (void)setCornerStyle:(NSInteger)style;
@end
@interface UIFont : NSObject
+ (instancetype)systemFontOfSize:(CGFloat)size weight:(CGFloat)weight;
@end
@interface CALayer : NSObject
@property CGFloat cornerRadius;
@property CGFloat borderWidth;
@property void *borderColor;
@property void *shadowColor;
@property float shadowOpacity;
@property CGFloat shadowRadius;
@property CGSize shadowOffset;
@end
@interface NSObject (MetalLayerAccess)
@property(readonly) CGSize drawableSize;
@end
@interface UIView : NSObject
- (instancetype)initWithFrame:(CGRect)frame;
- (void)addSubview:(UIView *)v;
- (void)addGestureRecognizer:(UIGestureRecognizer *)g;
- (void)removeGestureRecognizer:(UIGestureRecognizer *)g;
- (UIView *)hitTest:(CGPoint)p withEvent:(UIEvent *)e;
- (void)layoutSubviews;
- (CGSize)sizeThatFits:(CGSize)size;
- (BOOL)becomeFirstResponder; - (BOOL)resignFirstResponder;
- (void)insertText:(NSString *)text;
@property CGRect frame;
@property CGRect bounds;
@property(retain) UIColor *backgroundColor;
@property CGFloat alpha;
@property(getter=isHidden) BOOL hidden;
@property(getter=isUserInteractionEnabled) BOOL userInteractionEnabled;
@property BOOL clipsToBounds;
@property NSUInteger autoresizingMask;
@property(readonly) UIWindow *window;
@property(readonly) NSArray<UIView *> *subviews;
@property(readonly) CALayer *layer;
@property(readonly) BOOL isFirstResponder;
@end
@interface UIVisualEffect : NSObject @end
@interface UIVisualEffectView : UIView
- (instancetype)initWithEffect:(UIVisualEffect *)effect;
@property(readonly) UIVisualEffect *effect;
@end
@interface NSObject (GlassEffectRuntime)
+ (id)effectWithStyle:(NSInteger)style;
- (void)setTintColor:(UIColor *)color;
@end
@interface UIApplication : NSObject
+ (instancetype)sharedApplication;
@property(readonly) NSArray<UIWindow *> *windows;
@end
@interface UIViewController : NSObject
@property(retain) UIView *view;
@property(readonly) id traitOverrides;
@property(readonly) id traitCollection;
@property(readonly) UIViewController *presentedViewController;
- (BOOL)prefersStatusBarHidden;
- (BOOL)prefersHomeIndicatorAutoHidden;
- (UIViewController *)childViewControllerForStatusBarHidden;
- (UIViewController *)childViewControllerForHomeIndicatorAutoHidden;
- (void)setNeedsStatusBarAppearanceUpdate;
- (void)setNeedsUpdateOfHomeIndicatorAutoHidden;
- (void)presentViewController:(UIViewController *)vc animated:(BOOL)animated completion:(void (^)(void))completion;
- (void)dismissViewControllerAnimated:(BOOL)a completion:(void (^)(void))completion;
@end
@interface NSObject (TraitAccess)
@property NSInteger verticalSizeClass;
@property(readonly) NSInteger userInterfaceStyle;
@end
@interface UIStatusBarManager : NSObject
@property(readonly, getter=isStatusBarHidden) BOOL statusBarHidden;
@end
@interface UIWindowScene : NSObject
@property(readonly) NSArray<UIWindow *> *windows;
@property(readonly) UIStatusBarManager *statusBarManager;
@end
@interface UIGestureRecognizer : NSObject
- (instancetype)initWithTarget:(id)target action:(SEL)action;
@property BOOL cancelsTouchesInView;
@property BOOL delaysTouchesBegan;
@property BOOL delaysTouchesEnded;
@property(weak) id delegate;
@end
@interface UITapGestureRecognizer : UIGestureRecognizer
@property NSUInteger numberOfTouchesRequired;
@property NSUInteger numberOfTapsRequired;
@end
@interface UIWindow : UIView
- (instancetype)initWithWindowScene:(UIWindowScene *)scene;
@property(retain) UIViewController *rootViewController;
@property(readonly) UIWindowScene *windowScene;
@property CGFloat windowLevel;
@end
@interface UILabel : UIView
@property(retain) UIFont *font;
@property(copy) NSString *text;
@property(retain) UIColor *textColor;
@end
@interface UIButton : UIView
+ (instancetype)buttonWithType:(NSInteger)type;
- (void)setTitle:(NSString *)title forState:(NSUInteger)state;
- (void)setTitleColor:(UIColor *)color forState:(NSUInteger)state;
- (void)addTarget:(id)target action:(SEL)action forControlEvents:(NSUInteger)events;
@property(retain) id configuration;
@property(readonly) UILabel *titleLabel;
@property(copy) NSString *accessibilityLabel;
@end
@interface UITextField : UIView
@property(copy) NSString *text;
@property(copy) NSString *placeholder;
@property NSInteger keyboardType;
@end
@interface UIScreen : NSObject
+ (instancetype)mainScreen;
@property(readonly) CGRect nativeBounds;
@end
@interface UIAlertAction : NSObject
+ (instancetype)actionWithTitle:(NSString *)title style:(NSInteger)style handler:(void (^)(UIAlertAction *))handler;
@end
@interface UIAlertController : UIViewController
@property(copy) NSString *message;
+ (instancetype)alertControllerWithTitle:(NSString *)title message:(NSString *)message preferredStyle:(NSInteger)style;
- (void)addAction:(UIAlertAction *)action;
- (void)addTextFieldWithConfigurationHandler:(void (^)(UITextField *))handler;
@property(readonly) NSArray<UITextField *> *textFields;
@end
@interface UIActivityViewController : UIViewController
- (instancetype)initWithActivityItems:(NSArray *)items applicationActivities:(NSArray *)activities;
@property(readonly) id popoverPresentationController;
@property(copy) void (^completionWithItemsHandler)(NSString *, BOOL, NSArray *, NSError *);
@end
@interface NSObject (PopoverAccess)
@property(retain) UIView *sourceView;
@property CGRect sourceRect;
@end
extern const void *_NSConcreteStackBlock;
extern void dispatch_async(void *, void (^)(void));
extern void *dispatch_get_global_queue(long, unsigned long);
extern void dispatch_once(long *, void (^)(void));
extern int open(const char *, int, ...), close(int);
extern long write(int, const void *, unsigned long);
extern void dispatch_after(unsigned long long, void *, void (^)(void));
extern unsigned long long dispatch_time(unsigned long long, long long);
extern char _dispatch_main_q;
extern id objc_getClass(const char *);
extern id objc_msgSend(id, SEL, ...);
extern Class object_getClass(id);
extern const char *class_getName(Class);
extern BOOL class_addMethod(Class, SEL, void *, const char *);
extern void *imp_implementationWithBlock(id);
extern void *class_getInstanceMethod(Class, SEL);
extern void *method_getImplementation(void *);
extern const char *method_getTypeEncoding(void *);
extern void *class_replaceMethod(Class, SEL, void *, const char *);
extern long _dyld_get_image_vmaddr_slide(unsigned int);
extern unsigned int _dyld_image_count(void);
extern const char *_dyld_get_image_name(unsigned int);
extern unsigned long os_proc_available_memory(void);
extern char *getenv(const char *);
extern int setenv(const char *, const char *, int);
extern void *dlsym(void *, const char *);
extern int printf(const char *, ...);
