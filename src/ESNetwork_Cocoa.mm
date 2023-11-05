//
//  ESNetwork_iOS.mm
//
//  Created by Steve Pucci 16 May 2011
//  Copyright Emerald Sequoia LLC 2011. All rights reserved.
//

#include "ESNetwork.hpp"
#include "ESErrorReporter.hpp"
#include "ESThread.hpp"
#define ESTRACE
#include "ESTrace.hpp"

#include <SystemConfiguration/SCNetworkReachability.h>

// File static variables
static SCNetworkReachabilityRef networkReachabilityRef = NULL;

// =============================
// Direct queries
// =============================

/*static*/ bool 
ESNetwork::internetIsReachable() {
    struct sockaddr_in zeroAddress;
    bzero(&zeroAddress, sizeof(zeroAddress));
    zeroAddress.sin_len = sizeof(zeroAddress);
    zeroAddress.sin_family = AF_INET;
    bool IPv4_ok = hostIsReachable((const sockaddr *)&zeroAddress);  // Does this work if all we have is an IPv6 interface?
#if 0 // Can't seem to construct a reachability ref for an IPv6 address...
    if (IPv4_ok) {
        return true;
    }
    printf("4 no good, trying 6\n");
    zeroAddress.sin_family = AF_INET6;
    return hostIsReachable((const sockaddr *)&zeroAddress);  // This might be redundant.
#endif
    return IPv4_ok;
}

/*static*/ bool 
ESNetwork::hostIsReachable(const struct sockaddr *address) {
    SCNetworkReachabilityRef reachability = SCNetworkReachabilityCreateWithAddress(kCFAllocatorDefault, address);
    if (reachability != NULL) {
        SCNetworkReachabilityFlags flags;
        BOOL st = SCNetworkReachabilityGetFlags(reachability, &flags);
        CFRelease(reachability);
        if (st) {
            if (flags & (kSCNetworkReachabilityFlagsReachable | kSCNetworkReachabilityFlagsConnectionOnTraffic)) {
                return true;
            } else {
                return false;
            }
        } else {
            ESErrorReporter::logError("ESNetwork", "Can't get flags from reachability reference");
            // fall through: The flags could not be retrieved, so we don't know the state
        }
    } else {
        ESErrorReporter::logError("ESNetwork", "Can't create reachability reference");
    }
    // Unknown:  return true to be safe (i.e., to allow a client to continue trying)
    return true;
}

// =============================
// Internet observers
// =============================

/*static*/ void
ESNetworkInternetObserver::networkReachabilityCallback(SCNetworkReachabilityRef   reachabilityRef,
                                                       SCNetworkReachabilityFlags flags,
                                                       void                       *info) {
    ESAssert(ESThread::inMainThread());  // That's our generatingThread
    if (flags & (kSCNetworkReachabilityFlagsReachable | kSCNetworkReachabilityFlagsConnectionOnTraffic)) {
        tracePrintf("iOS callback saying it's reachable now");
        callInterThreadNotifyObservers(_observersList, (void*)true);
    } else {
        tracePrintf("iOS callback saying it's NOT reachable now");
        callInterThreadNotifyObservers(_observersList, (void*)false);
    }
}

void 
ESNetworkInternetObserver::initialize() {  // The base class will take care of adding to observer list (*after* this method completes); we just need to make sure OS observer is set up
    ESAssert(ESThread::inMainThread());   // all use and setting of observer list must be done in main thread to avoid race conditions
    if (!networkReachabilityRef) {
        struct sockaddr_in zeroAddress;
        bzero(&zeroAddress, sizeof(zeroAddress));
        zeroAddress.sin_len = sizeof(zeroAddress);
        zeroAddress.sin_family = AF_INET;   // We should worry about IPv6 here (someday) in case IPv4 connectivity stays down while IPv6 comes up, probably by creating another reachability object
        networkReachabilityRef = SCNetworkReachabilityCreateWithAddress(kCFAllocatorDefault, (const sockaddr*)&zeroAddress);
        if (!networkReachabilityRef) {
            ESErrorReporter::logError("ESNetwork", "Can't create reachability reference for zero address in observer");
            ESAssert(false);
            return;
        }
        SCNetworkReachabilityContext context = {0, NULL/*info*/, NULL, NULL, NULL};
        BOOL st = SCNetworkReachabilitySetCallback(networkReachabilityRef, networkReachabilityCallback, &context);
        if (!st) {
            ESErrorReporter::logError("ESNetwork", "Can't set reachability callback for zero address in observer");
            ESAssert(false);
            CFRelease(networkReachabilityRef);
            networkReachabilityRef = NULL;
            return;
        }
        st = SCNetworkReachabilityScheduleWithRunLoop(networkReachabilityRef, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
        if (!st) {
            ESErrorReporter::logError("ESNetwork", "Can't schedule reachability with runloop for zero address in observer");
            ESAssert(false);
            CFRelease(networkReachabilityRef);
            networkReachabilityRef = NULL;
            return;
        }
        tracePrintf("iOS setup seems to have worked...");
    }
}

