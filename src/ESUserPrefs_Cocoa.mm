//
//  ESUserPrefs_Cocoa.mm
//
//  Created by Steve Pucci 25 Jan 2011
//  Copyright Emerald Sequoia LLC 2011. All rights reserved.
//

#include "ESPlatform.h"
#include "ESUserPrefs.hpp"
#include "ESThread.hpp"
#include "ESUtil.hpp"
#include "ESErrorReporter.hpp"

// We really don't want to be linking in the screensaver on all MacOS builds.  But we also don't really want to build two kinds of libraries,
// one with screensaver support and one without (because the number of configurations can quickly proliferate exponentially if there are other similar options).
// So for now we go ahead and link in the screen saver on all MacOS builds.  If we want to change this in the future, one possibility would be to provide a
// default implementation of properUserDefaults() which simply returns the standard defaults, but which can be overridden by supplying a registration mechanism
// for a screensaver-defaults-aware implementation.  This implementation can live in a separate .o file which would then only be linked in when referenced by
// the app (which would therefore presumably need to make a direct call to something in that .o file, though I suspect that could be done via an inline method
// in ESUtil (e.g., setPrimaryBundleID).  Someday maybe.
//#ifdef ES_SCREENSAVER
#if ES_MACOS
#import <ScreenSaver/ScreenSaver.h>
#endif

static NSMutableDictionary *pendingRegistrations;

static inline NSUserDefaults *
properUserDefaults() {
//#ifdef ES_SCREENSAVER
#if ES_MACOS
    std::string primaryBundleID = ESUtil::primaryBundleID();
    if (primaryBundleID.length() > 0) {
        // A bit hacky, this:  We're depending on only screensavers setting this id
        //NSLog(@"Using screen saver defaults");
        NSUserDefaults *defaults = [ScreenSaverDefaults defaultsForModuleWithName:[NSString stringWithUTF8String:primaryBundleID.c_str()]];
        ESAssert(defaults);
        return defaults;
    }
#endif
    return [NSUserDefaults standardUserDefaults];
}

static inline void
doPendingRegistrations() {
    assert(ESThread::inMainThread());
    if (pendingRegistrations) {
        [properUserDefaults() registerDefaults:pendingRegistrations];
        [pendingRegistrations release];
        pendingRegistrations = nil;
    }
}

static inline void
setupPendingRegistrations() {
    assert(ESThread::inMainThread());
    if (!pendingRegistrations) {
        pendingRegistrations = [[NSMutableDictionary alloc] initWithCapacity:30];
    }
}

/*static*/ bool 
ESUserPrefs::boolPref(const char *name) {
    doPendingRegistrations();
    BOOL val = [properUserDefaults() boolForKey:[NSString stringWithUTF8String:name]];
    return val != 0;
}

/*static*/ int 
ESUserPrefs::intPref(const char *name) {
    doPendingRegistrations();
    int val = (int)[properUserDefaults() integerForKey:[NSString stringWithUTF8String:name]];
    return val;
}

/*static*/ double 
ESUserPrefs::doublePref(const char *name) {
    doPendingRegistrations();
    return [properUserDefaults() doubleForKey:[NSString stringWithUTF8String:name]];
}

/*static*/ std::string 
ESUserPrefs::stringPref(const char *name) {
    doPendingRegistrations();
    NSString *str = [properUserDefaults() stringForKey:[NSString stringWithUTF8String:name]];
    return str ? [str UTF8String] : "";
}

/*static*/ void 
ESUserPrefs::initDefaultPref(const char *name,
                             bool       value) {
    setupPendingRegistrations();
    [pendingRegistrations setObject:[NSNumber numberWithBool:(value ? YES : NO)] forKey:[NSString stringWithUTF8String:name]];
}

/*static*/ void 
ESUserPrefs::initDefaultPref(const char *name,
                             int        value) {
    setupPendingRegistrations();
    [pendingRegistrations setObject:[NSNumber numberWithInteger:value] forKey:[NSString stringWithUTF8String:name]];
}

/*static*/ void 
ESUserPrefs::initDefaultPref(const char *name,
                             double     value) {
    setupPendingRegistrations();
    [pendingRegistrations setObject:[NSNumber numberWithDouble:value] forKey:[NSString stringWithUTF8String:name]];
}

/*static*/ void 
ESUserPrefs::initDefaultPref(const char *name,
                             const char *value) {
    setupPendingRegistrations();
    [pendingRegistrations setObject:[NSString stringWithUTF8String:value] forKey:[NSString stringWithUTF8String:name]];
}

/*static*/ void 
ESUserPrefs::setPref(const char *name,
                     bool       value) {
    [properUserDefaults() setBool:(value ? YES : NO) forKey:[NSString stringWithUTF8String:name]];
}

/*static*/ void 
ESUserPrefs::setPref(const char *name,
                     int        value) {
    [properUserDefaults() setInteger:value forKey:[NSString stringWithUTF8String:name]];
}

/*static*/ void 
ESUserPrefs::setPref(const char *name,
                     double     value) {
    [properUserDefaults() setDouble:value forKey:[NSString stringWithUTF8String:name]];
}

/*static*/ void 
ESUserPrefs::setPref(const char *name,
                     const char *value) {
    [properUserDefaults() setObject:[NSString stringWithUTF8String:value] forKey:[NSString stringWithUTF8String:name]];
}

/*static*/ void 
ESUserPrefs::synchronize() {
    [properUserDefaults() synchronize];
}
