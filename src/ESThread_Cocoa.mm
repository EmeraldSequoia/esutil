//
//  ESThread_Cocoa.mm
//
//  Created by Steve Pucci 29 Jan 2011
//  Copyright Emerald Sequoia LLC 2011. All rights reserved.
//

#include "ESThread.hpp"

/*static*/ bool 
ESThread::osDirectInMainThread() {
#ifdef NDEBUG    
    return true;   // Some platforms don't support this call, so we must not rely on it
#else
    return ([NSThread isMainThread] == YES);
#endif
}

void 
ESThread::platformSpecificInit() {
    // Nothing to do
}

static void mySocketCallback(CFSocketRef          s,
                             CFSocketCallBackType callbackType,
                             CFDataRef            address,
                             const void           *data,
                             void                 *info) {
    assert(callbackType == kCFSocketReadCallBack);
    ESMainThread *mainThread = (ESMainThread*)info;
    mainThread->readAndExecuteInterThreadFunction();
}

void
ESMainThread::platformSpecificThreadInitialization() {
    // Add _myInterThreadSocket observer to main loop
    CFSocketContext socketContext;
    bzero(&socketContext, sizeof(socketContext));
    socketContext.info = this;
    CFSocketRef cfsock = CFSocketCreateWithNative(NULL, _myInterThreadSocket, kCFSocketReadCallBack, mySocketCallback, &socketContext);
    CFRunLoopSourceRef runLoopSource = CFSocketCreateRunLoopSource(NULL, cfsock, 0);
    CFRunLoopAddSource(CFRunLoopGetMain(), runLoopSource, kCFRunLoopCommonModes);
    CFRelease(runLoopSource);
}

// No MainThread cleanup because the main thread never dies

void 
ESChildThread::platformSpecificThreadInitialization() {
    _pool = [[NSAutoreleasePool alloc] init];
}

void 
ESChildThread::platformSpecificThreadCleanup() {
    [_pool release];
}

void 
ESChildThread::preInterThreadFunction() {
    
}

void 
ESChildThread::postInterThreadFunction() {

}
