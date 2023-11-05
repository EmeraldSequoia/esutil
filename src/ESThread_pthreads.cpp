//
//  ESThread.cpp
//
//  Created by Steve Pucci 08 Jan 2011
//  Copyright Emerald Sequoia LLC 2011. All rights reserved.
//

#include "ESThread.hpp"

#if !ES_PTHREADS
error "Don't include this file in non-pthreads platform builds";
#endif

#include <stdio.h>
#include <strings.h>
#include <assert.h>
#include <errno.h>

#include "ESErrorReporter.hpp"
#undef ESTRACE
#include "ESTrace.hpp"

ESChildThread::ESChildThread(const std::string         &name,
                             ESChildThreadExitStrategy exitStrategy)
:   ESThread(name),
    _exitStrategy(exitStrategy),
    _waitingOnSocket(false)
{        
    bzero(&_pthread, sizeof(_pthread));  // For deterministic behavior; we shouldn't use this before initializing it later, though
}

ESChildThread::~ESChildThread() {
    ESAssert(!inThisThread());
    ESAssert(_parentThread);  // There's no way to delete a thread that hasn't been started, since threads are only destroyed as part of ESChildThread::joinGlue()
    ESAssert(!_parentThread || _parentThread->inThisThread());
    ESAssert(_exitStrategy != ESChildThreadNeverExits);
}

void *
ESThreadStarter(void *param) {
    ESChildThread *thread = reinterpret_cast<ESChildThread *>(param);
    thread->initializeInThread();
    void *st = thread->main();
    ESAssert(thread->exitStrategy() == ESChildThreadExitsOnlyWhenFinished);
    thread->cleanupInThread();
    return st;
}

void
ESChildThread::start() {
    if (!_mainThread) {
        setMainThreadToThisOne();
    }
    _parentThread = currentThread();
    pthread_attr_t threadAttr;
    pthread_attr_init(&threadAttr);
    int st = pthread_attr_setstacksize(&threadAttr, 10 * 1024 * 1024);  // Each thread has 10MB stack
    ESErrorReporter::checkAndLogSystemError("ESThread", st, "thread attr setstacksize");
// [stevep 15 May 2011: The following was an attempt to keep the join from getting ESRCH
//      but it had no effect whatsoever.  Changing to PTHREAD_CREATE_DETACHED made us get
//      ESRCH all of the time. ]
//    pthread_attr_t attr;
//    int st = pthread_attr_init(&attr);
//    ESErrorReporter::checkAndLogSystemError("ESThread", st, "thread attr init");
//    st = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
//    ESErrorReporter::checkAndLogSystemError("ESThread", st, "thread attr setdetachstate");
    st = pthread_create(&_pthread, &threadAttr, ESThreadStarter, this);
    ESErrorReporter::checkAndLogSystemError("ESThread", st, "thread create");
}

void *
ESChildThread::join() {
    assert(_parentThread);  // set in start()
    assert(_parentThread->inThisThread());
    void *retval = NULL;
    int st = pthread_join(_pthread, &retval);
    tracePrintf2("join of child thread 0x%08x returned status %d\n", (unsigned long int)this, st);
    ESErrorReporter::checkAndLogSystemError("ESThread", st, "thread join");
    return retval;
}

void
ESChildThread::exit() {
    cleanupInThread();
    pthread_exit(NULL);
}

ESMainThread::ESMainThread()
:   ESThread("Main")
{
    _pthread = pthread_self();
}
