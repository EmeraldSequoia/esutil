//
//  ESUtil_Cocoa.mm
//
//  Created by Steve Pucci 26 Jan 2011
//  Copyright Emerald Sequoia LLC 2011. All rights reserved.
//

#include "ESUtil.hpp"

/*static*/ std::string 
ESUtil::localeCountryCode() {
    NSString *locale = [[NSLocale currentLocale] objectForKey:NSLocaleCountryCode];
    if (locale) {
        return [locale UTF8String];
    }
    return "";
}

// According to the Cocoa docs, if I'm going to have pthreads but no NSThread, I need
// to make an NSThread to tell Cocoa I'm multithreaded so its protections will work
// properly, but that the thread doesn't actually need to do anything.
@interface MyDummyThread : NSThread {
}
-(void)main;
@end
@implementation MyDummyThread
-(void)main {
}
@end

/*static*/ void 
ESUtil::initPlatformSpecific() {
    if (![NSThread isMultiThreaded]) {  // But don't bother doing it if Cocoa already knows...
        printf("Starting dummy thread\n");
        static MyDummyThread *dummyThread = [[MyDummyThread alloc] init];
        [dummyThread start];
    }
    _deviceID = "ImNotReallyUnique";  // Nothing we have so far depends on this actually *being* unique.
}

/*static*/ std::string 
ESUtil::stackTrace() {
    NSArray *stackTraceArray = [NSThread callStackSymbols];
    ESAssert(stackTraceArray);
    int length = 0;
    for (NSString *str in stackTraceArray) {
        length += [str lengthOfBytesUsingEncoding:NSUTF8StringEncoding] + 1;
    }
    char *buf = new char[length];
    char *ptr = buf;
    for (NSString *str in stackTraceArray) {
        strcpy(ptr, [str UTF8String]);
        size_t len = strlen(ptr);
        ESAssert(len == [str lengthOfBytesUsingEncoding:NSUTF8StringEncoding]);
        ptr[len] = '\n';
        ptr += len + 1;
    }
    *--ptr = '\0';
    std::string s = buf;
    delete[] buf;
    return s;
}

/*static*/ std::string 
ESUtil::appIdentifier() {
    return [[[NSBundle mainBundle] bundleIdentifier] UTF8String];
}

