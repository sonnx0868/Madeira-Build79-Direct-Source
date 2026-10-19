/* SPDX-License-Identifier: MIT
 * Objective-C scalar arguments must cross the Swift boundary as scalars.
 * performSelector:withObject: passes the NSNumber pointer, not its value.
 * These optional layer selectors are private on iOS; check their runtime
 * signatures and leave unsupported or changed interfaces untouched. */
#pragma once
#import <Foundation/Foundation.h>
#import <objc/message.h>
#include <string.h>

static inline const char *madeira_layer_scalar_argument(id layer, SEL selector) {
    if (![layer respondsToSelector:selector]) return NULL;
    NSMethodSignature *signature = [layer methodSignatureForSelector:selector];
    if (!signature || signature.numberOfArguments != 3 ||
        strcmp(signature.methodReturnType, @encode(void))) return NULL;
    return [signature getArgumentTypeAtIndex:2];
}

static inline int madeira_layer_set_sync(id layer, BOOL enabled) {
    SEL selector = NSSelectorFromString(@"setDisplaySyncEnabled:");
    const char *type = madeira_layer_scalar_argument(layer, selector);
    if (!type) return 0;
    if (!strcmp(type, "B"))
        ((void (*)(id, SEL, _Bool))objc_msgSend)(layer, selector, !!enabled);
    else if (!strcmp(type, "c"))
        ((void (*)(id, SEL, signed char))objc_msgSend)(layer, selector, !!enabled);
    else return 0;
    return 1;
}

static inline int madeira_layer_set_nominal_rate(id layer, NSInteger hz) {
    if (hz < 1 || hz > 1000) return 0;
    SEL selector = NSSelectorFromString(@"setNominalFramesPerSecond:");
    const char *type = madeira_layer_scalar_argument(layer, selector);
    if (!type) return 0;
    if (!strcmp(type, "q"))
        ((void (*)(id, SEL, long long))objc_msgSend)(layer, selector, (long long)hz);
    else if (!strcmp(type, "Q"))
        ((void (*)(id, SEL, unsigned long long))objc_msgSend)(layer, selector, (unsigned long long)hz);
    else if (!strcmp(type, "i"))
        ((void (*)(id, SEL, int))objc_msgSend)(layer, selector, (int)hz);
    else if (!strcmp(type, "I"))
        ((void (*)(id, SEL, unsigned))objc_msgSend)(layer, selector, (unsigned)hz);
    else return 0;
    return 1;
}
