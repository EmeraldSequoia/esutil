//
//  ESLock_pthreads.cpp
//
//  Created by Steve Pucci 26 Jan 2011
//  Copyright Emerald Sequoia LLC 2011. All rights reserved.
//

#include "ESLock.hpp"
#include "ESErrorReporter.hpp"

#if !ES_PTHREADS
error "Don't include this file in non-pthreads platform builds";
#endif

ESLock::ESLock() {
    int st = pthread_mutex_init(&_mutex, NULL);  // Consider calling pthread_mutexattr_init() for fast, recursive, errorcheck, especially ifndef NDEBUG
    ESErrorReporter::checkAndLogSystemError("ESLock", st, "mutex init");
}

/*virtual*/ 
ESLock::~ESLock() {
    int st = pthread_mutex_destroy(&_mutex);
    ESErrorReporter::checkAndLogSystemError("ESLock", st, "mutex destroy");
}

void 
ESLock::lock() {
    int st = pthread_mutex_lock(&_mutex);
    ESErrorReporter::checkAndLogSystemError("ESLock", st, "mutex lock");
}

void 
ESLock::unlock() {
    int st = pthread_mutex_unlock(&_mutex);
    ESErrorReporter::checkAndLogSystemError("ESLock", st, "mutex unlock");
}

