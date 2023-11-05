//
//  ESThreadLocalStorageInl_pthreads.cpp
//
//  Created by Steve Pucci 30 Jan 2011
//  Copyright Emerald Sequoia LLC 2011. All rights reserved.
//

#include <pthread.h>
#include "ESThreadLocalStorage.hpp"
#include "ESErrorReporter.hpp"

inline
ESThreadLocalStorageBase::ESThreadLocalStorageBase()
{
    int st = pthread_key_create(&_key, NULL/*destructor*/);
    ESErrorReporter::checkAndLogSystemError("ESUtil", st, "ESThreadLocalStorage ctor key creation");
}

inline
ESThreadLocalStorageBase::~ESThreadLocalStorageBase() {
    int st = pthread_key_delete(_key);
    ESErrorReporter::checkAndLogSystemError("ESUtil", st, "ESThreadLocalStorage ctor key destruction");
}

inline void 
ESThreadLocalStorageBase::setValue(void *v) {
    pthread_setspecific(_key, v);
}

inline void *
ESThreadLocalStorageBase::getValue() {
    return pthread_getspecific(_key);
}

