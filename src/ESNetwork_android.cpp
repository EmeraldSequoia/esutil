//
//  ESNetwork_android.cpp
//
//  Created by Steve Pucci 22 May 2011
//  Copyright Emerald Sequoia LLC 2011. All rights reserved.
//

#include "ESNetwork.hpp"
#include "ESThread.hpp"
#include "ESErrorReporter.hpp"

/*static*/ bool 
ESNetwork::internetIsReachable() {                            // call in any thread
    return true; // FIX FIX
}

/*static*/ bool 
ESNetwork::hostIsReachable(const struct sockaddr *address) {  // call in any thread
    return true; // FIX FIX
}

void 
ESNetworkInternetObserver::initialize() {  // The base class will take care of adding to observer list (*after* this method completes); we just need to make sure OS observer is set up
    ESAssert(ESThread::inMainThread());   // all use and setting of observer list must be done in main thread to avoid race conditions
    // FIX FIX -- need to do something to observe network changes
}

/*static*/ void 
ESNetwork::startNetworkActivityIndicatorForRealInMainThread() {
    // Do nothing -- no network activity indicator on Android
}

/*static*/ void 
ESNetwork::stopNetworkActivityIndicatorForRealInMainThread() {
    // Do nothing -- no network activity indicator on Android
}
