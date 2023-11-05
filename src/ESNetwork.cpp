//
//  ESNetwork.cpp
//
//  Created by Steve Pucci 16 May 2011
//  Copyright Emerald Sequoia LLC 2011. All rights reserved.
//

#include "ESNetwork.hpp"
#include "ESThread.hpp"
#define ESTRACE
#include "ESTrace.hpp"

/*static*/ std::list<ESInterThreadObserver *> *ESNetworkInternetObserver::_observersList = NULL;

ESNetworkInternetObserver::ESNetworkInternetObserver(bool currentReachability)
:   _lastReachabilityStatus(currentReachability),
    ESInterThreadObserver(ESThread::mainThread())
{
    callInterThreadInitialize();
}

/*virtual*/ 
ESNetworkInternetObserver::~ESNetworkInternetObserver() {  // The destruction is done elsewhere via release() and friends
    ESAssert(_notificationThread->inThisThread());
}

/*virtual*/ std::list<ESInterThreadObserver *> *&
ESNetworkInternetObserver::observersList() {  // will be called only in generating thread -- return a reference to the list pointer
    ESAssert(ESThread::inMainThread());  // To avoid race conditions with add/delete/iterate
    return _observersList;
}

/*virtual*/ void 
ESNetworkInternetObserver::notify(void *param) { // will be called in notification thread
    ESAssert(_notificationThread->inThisThread());
    bool nowReachable = (bool)param;
    if (nowReachable != _lastReachabilityStatus) {
        if (nowReachable) {
            internetIsNowReachable();
            tracePrintf("internet now reachable");
        } else {
            internetIsNowUnreachable();
            tracePrintf("internet now unreachable");
        }
        _lastReachabilityStatus = nowReachable;
    }
}

/*virtual*/ void 
ESNetworkInternetObserver::possiblyNotifyChangeDuringInit() {  // make sure to notify about changes that happen between when observer is created and it is installed
    ESAssert(_generatingThread->inThisThread());
    bool nowReachable = ESNetwork::internetIsReachable();
    if (_generatingThread == _notificationThread) {
        notify((void*)nowReachable);
    } else {
        _notificationThread->callInThread(notificationGlue, this, (void*)nowReachable);
    }
}

static int activeNetworkCount = 0;

/*static*/ void 
ESNetwork::startNetworkActivityGlue(void *obj,
                                    void *param) {
    startNetworkActivityIndicatorForRealInMainThread();
}

/*static*/ void 
ESNetwork::startNetworkActivityIndicator() {                  // call in any thread; may be nested
    if (activeNetworkCount++ == 0) {
        if (ESThread::inMainThread()) {
            startNetworkActivityIndicatorForRealInMainThread();
        } else {
            ESThread::callInMainThread(startNetworkActivityGlue, NULL, NULL);
        }
    }
}

/*static*/ void 
ESNetwork::stopNetworkActivityGlue(void *obj,
                                   void *param) {
    stopNetworkActivityIndicatorForRealInMainThread();
}

/*static*/ void 
ESNetwork::stopNetworkActivityIndicator() {                   // call in any thread; may be nested
    if (--activeNetworkCount == 0) {
        if (ESThread::inMainThread()) {
            stopNetworkActivityIndicatorForRealInMainThread();
        } else {
            ESThread::callInMainThread(stopNetworkActivityGlue, NULL, NULL);
        }
    }
}

