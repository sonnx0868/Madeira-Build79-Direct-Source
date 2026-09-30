/* Extra public API declarations for the native touch overlay (Linux build). */
@class UITouch;
@interface NSSet<ObjectType> : NSObject <NSFastEnumeration>
@property(readonly) NSUInteger count;
@end
@interface NSDictionary<KeyType, ObjectType> : NSObject
+ (instancetype)dictionaryWithObjects:(const id [])objects forKeys:(const id [])keys count:(NSUInteger)count;
@property(readonly) NSUInteger count;
- (ObjectType)objectForKeyedSubscript:(KeyType)key;
@end
@interface NSNumber : NSObject
+ (instancetype)numberWithDouble:(double)value;
+ (instancetype)numberWithBool:(BOOL)value;
+ (instancetype)numberWithInt:(int)value;
@property(readonly) double doubleValue;
@property(readonly) long long longLongValue;
@property(readonly) BOOL boolValue;
@end
@interface NSString (TouchStrings)
- (BOOL)isEqualToString:(NSString *)other;
@end
@interface NSJSONSerialization : NSObject
+ (id)JSONObjectWithData:(NSData *)data options:(NSUInteger)options error:(NSError **)error;
+ (NSData *)dataWithJSONObject:(id)object options:(NSUInteger)options error:(NSError **)error;
@end
@interface UIView (MadeiraTouchEvents)
@property(readonly) UIView *superview;
@property(getter=isMultipleTouchEnabled) BOOL multipleTouchEnabled;
@property(getter=isExclusiveTouch) BOOL exclusiveTouch;
- (void)removeFromSuperview;
- (void)insertSubview:(UIView *)view atIndex:(NSInteger)index;
- (BOOL)pointInside:(CGPoint)point withEvent:(UIEvent *)event;
- (void)touchesBegan:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event;
- (void)touchesMoved:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event;
- (void)touchesEnded:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event;
- (void)touchesCancelled:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event;
@end
@interface UILabel (MadeiraTouchLabel)
@property NSInteger textAlignment;
@property BOOL adjustsFontSizeToFitWidth;
@property CGFloat minimumScaleFactor;
@end
@interface NSUUID : NSObject
+ (instancetype)UUID;
@property(readonly) NSString *UUIDString;
- (instancetype)initWithUUIDString:(NSString *)string;
@end
@interface UIViewController (MadeiraTouchLayout)
- (void)viewDidLayoutSubviews;
@end
@interface UITouch : NSObject
- (CGPoint)locationInView:(UIView *)view;
@property(readonly) NSInteger phase;
@end
@interface UIEvent : NSObject
- (NSSet<UITouch *> *)touchesForWindow:(UIWindow *)window;
@end
@interface UIWindow (MadeiraTouchObservation)
- (void)sendEvent:(UIEvent *)event;
@end
@interface NSNotification : NSObject @end
@interface NSOperationQueue : NSObject
+ (instancetype)mainQueue;
@end
@interface NSNotificationCenter : NSObject
+ (instancetype)defaultCenter;
- (id)addObserverForName:(NSString *)name object:(id)object queue:(NSOperationQueue *)queue usingBlock:(void (^)(NSNotification *))block;
@end
@interface NSThread : NSObject
+ (BOOL)isMainThread;
@end
@interface UIApplication (MadeiraTouchActivity)
@property(readonly) NSInteger applicationState;
@end
extern NSString * const UIApplicationWillResignActiveNotification;
