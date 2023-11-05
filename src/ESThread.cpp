//
//  ESThread.cpp
//
//  Created by Steve Pucci 30 Jan 2011
//  Copyright Emerald Sequoia LLC 2011. All rights reserved.
//

#include "ESUtil.hpp"
#include "ESThread.hpp"
#include "ESThreadLocalStorage.hpp"
#include "ESErrorReporter.hpp"
#define ESTRACE
#include "ESTrace.hpp"

#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>

/*static*/ ESThread *ESThread::_mainThread = NULL;

static ESThreadLocalStorageScalar<bool> *exitingThreadHasBeenJoined = NULL;
static ESThreadLocalStoragePtr<ESThread> *currentThreadTLS = NULL;

struct ESInterThreadPacket {
    ESInterThreadFn         fn;
    void                    *obj;
    void                    *param;
};

ESThread::ESThread(const std::string &name)
:   _name(name)
{
    if (!currentThreadTLS) {
        initStatics();
    }
    int fds[2];
    int st = socketpair(AF_UNIX, SOCK_STREAM, 0, fds);  // Could possibly use SOCK_DGRAM here, but it can fail in certain low-resource situations
    if (st == 0) {
        _myInterThreadSocket            = fds[0];
        _correspondentInterThreadSocket = fds[1];
        // ESErrorReporter::logInfo("ESThread::ESThread", "Created socketpair for thread %s with fds %d, %d", 
        //                          name.c_str(), fds[0], fds[1]);
#if !ES_ANDROID
        size_t optval = sizeof(ESInterThreadPacket);
        unsigned int optval_sz = sizeof(optval);
        st = setsockopt(_correspondentInterThreadSocket, SOL_SOCKET, SO_SNDLOWAT, &optval, optval_sz);
        if (st != 0) {
            ESErrorReporter::checkAndLogSystemError("ESThread", errno, "Inter-thread socket SNDLOWAT setsockopt");
            _myInterThreadSocket            = -1;
            _correspondentInterThreadSocket = -1;
            ESAssert(false);
        }
        st = setsockopt(_myInterThreadSocket, SOL_SOCKET, SO_RCVLOWAT, &optval, optval_sz);
        if (st != 0) {
            ESErrorReporter::checkAndLogSystemError("ESThread", errno, "Inter-thread socket RCVLOWAT setsockopt");
            _myInterThreadSocket            = -1;
            _correspondentInterThreadSocket = -1;
            ESAssert(false);
        }
#endif
    } else {
        ESErrorReporter::checkAndLogSystemError("ESThread", errno, "Inter-thread socket creation");
        _myInterThreadSocket            = -1;
        _correspondentInterThreadSocket = -1;
        ESAssert(false);  // This is too bad to try to continue from
    }
}

/*virtual*/
ESThread::~ESThread() {
    // ESErrorReporter::logInfo("ESThread dtor", "closing thread %s with sockets %d, %d",
    //                          _name.c_str(), _myInterThreadSocket, _correspondentInterThreadSocket);
    close(_myInterThreadSocket);
    close(_correspondentInterThreadSocket);
}

/*static*/ void 
ESThread::initStatics() {
    currentThreadTLS = new ESThreadLocalStoragePtr<ESThread>;
    exitingThreadHasBeenJoined = new ESThreadLocalStorageScalar<bool>;
    *exitingThreadHasBeenJoined = true;  // Required for assert to pass in requestExitAndWaitForJoin() the first time it is called
    platformSpecificInit();
}

/*static*/ void
ESThread::setMainThreadToThisOne() {
    ESAssert(osDirectInMainThread());  // Just a sanity check...
    if (_mainThread) {
        if (!currentThreadTLS) {
            initStatics();  // Must be restarting after shutdown()
        } else {
            ESAssert(_mainThread->inThisThread());
        }
    } else {
        _mainThread = new ESMainThread();
    }
    _mainThread->initializeInThread();
    ESAssert(_mainThread->inThisThread());
}

/*static*/ void
ESThread::shutdown() {
    ESAssert(false);  // Not ready for prime time yet.
    delete exitingThreadHasBeenJoined;
    exitingThreadHasBeenJoined = NULL;
    delete currentThreadTLS;
    currentThreadTLS = NULL;
}

void
ESThread::initializeInThread() {
    *currentThreadTLS = this;
    platformSpecificThreadInitialization();
}

/*static*/ bool 
ESThread::inMainThread() {
    return mainThread()->inThisThread();
}

/*static*/ ESThread *
ESThread::mainThread() {
    if (!_mainThread) {
        setMainThreadToThisOne();
    }
    return _mainThread;
}

bool 
ESThread::inThisThread() {
    return *currentThreadTLS == this;
}

/*static*/ ESThread *
ESThread::currentThread() {
    if (!_mainThread) {
        setMainThreadToThisOne();
    }
    return *currentThreadTLS;
}

void 
ESThread::callInThread(ESInterThreadFn fn,
                       void            *object,
                       void            *param,
                       bool            forceUseSocket) {
    ESAssert(!inThisThread());  // otherwise don't go through this overhead; caller should check or just know
    ESInterThreadPacket packet;
    packet.fn = fn;
    packet.obj = object;
    packet.param = param;
    ssize_t bytesWritten = write(_correspondentInterThreadSocket, &packet, sizeof(packet));
    if (bytesWritten != sizeof(packet)) {
        ESErrorReporter::logError("ESThread::callInThread", "bytesWritten (%d) not expected (%d)",
                                  (int)bytesWritten, (int)sizeof(packet));
        ESErrorReporter::checkAndLogSystemError("ESThread", errno, 
                                                ESUtil::stringWithFormat("Inter-thread socket write to fd %d",
                                                                         _correspondentInterThreadSocket)
                                                .c_str());
#ifdef ESTRACE
#ifndef ES_ANDROID
        size_t bufsz = 0;
        unsigned int bufsz_sz = sizeof(bufsz);
        int st = getsockopt(_correspondentInterThreadSocket, SOL_SOCKET, SO_NWRITE,
                            &bufsz, &bufsz_sz);
        printf("remaining bytes to send (st %d): %lu\n", st, bufsz);
#endif
#endif        
        ESAssert(false);
    }
}

/*static*/ void 
ESThread::callInMainThread(ESInterThreadFn fn,
                           void            *object,
                           void            *param) {
    mainThread()->callInThread(fn, object, param);
}

int
ESThread::_setBitsForSelect(fd_set *fdset) {
    FD_SET(_myInterThreadSocket, fdset);
    return _myInterThreadSocket;
}

/*static*/ int
ESThread::setBitsForSelect(fd_set *fdset) {
    return currentThread()->_setBitsForSelect(fdset);
}

void
ESThread::readAndExecuteInterThreadFunction() {
    ESInterThreadPacket packet;
    ssize_t bytesRead = read(_myInterThreadSocket, &packet, sizeof(packet));
    if (bytesRead == sizeof(packet)) {
        preInterThreadFunction();
        (*packet.fn)(packet.obj, packet.param);
        postInterThreadFunction();
    } else {
        ESErrorReporter::checkAndLogSystemError("ESThread", errno, "Inter-thread socket read");
        ESErrorReporter::logInfo("ESThread", ".... from socket %d on thread %s", 
                                 _myInterThreadSocket, _name.c_str());
        ESAssert(false);
    }
}

// Brain-dead standards people decided to make this a function that takes a non-const ptr, when
// converting from a macro, so with const this crashes on Android as of NDK 15.0.
void
ESThread::_processInterThreadMessages(fd_set *fdset) {
    if (FD_ISSET(_myInterThreadSocket, const_cast<fd_set *>(fdset))) {
        readAndExecuteInterThreadFunction();
   }
}

/*static*/ void
ESThread::processInterThreadMessages(fd_set *fdset) {
    currentThread()->_processInterThreadMessages(fdset);
}

/*static*/ void 
ESThread::waitForAndProcessInterThreadMessages() {
    fd_set readers;
    FD_ZERO(&readers);
    ESThread *thread = currentThread();
    int highestThreadFD = thread->_setBitsForSelect(&readers);
    int nfds = highestThreadFD + 1;
    select(nfds, &readers, NULL/*writers*/, NULL, NULL);
    thread->_processInterThreadMessages(&readers);
}

/*static*/ int 
ESThread::myInterThreadSocket() {             // this thread reads from this socket
    return currentThread()->_myInterThreadSocket;
}

int 
ESThread::correspondentInterThreadSocket() {  // other threads write to this socket
    return _correspondentInterThreadSocket;
}

/*static*/ void
ESThread::verifyThreadSocketWithPeek(const char *msg) {
    ESThread *thread = currentThread();
    ESAssert(thread->inThisThread());
    char buf[4];
    ssize_t bytesRead = recv(thread->_myInterThreadSocket, buf, 4, MSG_DONTWAIT | MSG_PEEK);
    if (bytesRead < 0 && errno != EAGAIN) {
        ESErrorReporter::checkAndLogSystemError("verifyThreadSocketWithPeek", errno, 
                                                ESUtil::stringWithFormat("Couldn't peek at socket (fd %d), errno %d",
                                                                         thread->_myInterThreadSocket,
                                                                         errno).c_str());
        if (msg && *msg) {
            ESErrorReporter::logInfo("verifyThreadSocketWithPeek", "FAIL ...%s", msg);
        }
        ESUtil::showFDInfo();
        ESAssert(false);
    } else {
        if (msg && *msg) {
            ESErrorReporter::logInfo("verifyThreadSocketWithPeek", "OK %s", msg);
        }
    }
}

/*static*/ void 
ESChildThread::joinGlue(void *obj,
                        void *param) {
#ifndef NDEBUG    
    // ESErrorReporter::logInfo("ESChildThread::joinGlue", "enter");
    ESThread *parentThread = (ESThread *)obj;
    ESAssert(parentThread->inThisThread());
#endif
    ESChildThread *childThread = (ESChildThread *)param;
    ESAssert(!childThread->inThisThread());
    ESAssert(childThread->_parentThread == parentThread);
    tracePrintf2("joining child '%s', child pthread id is %x", childThread->name().c_str(), childThread->_pthread);
    childThread->join();
    ESAssert(exitingThreadHasBeenJoined);
    // ESAssert(!*exitingThreadHasBeenJoined);  // Can't assert this, because not set by requestExit() but its caller so other paths here that don't clear the flag.
    *exitingThreadHasBeenJoined = true;
    delete childThread;
}

void 
ESChildThread::requestJoin() {  // Sends message to creating thread to join immediately prior to exit
    ESAssert(inThisThread());
    // ESErrorReporter::logInfo("ESChildThread::requestJoin", "child thread requesting join (parent %s, child %s)",
    //                          _parentThread->name().c_str(), name().c_str());
    _parentThread->callInThread(joinGlue, _parentThread, this, _waitingOnSocket);
}

static void requestExitGlue(void *obj,
                            void *param) {
    ESAssert(((ESChildThread *)obj)->inThisThread());
    ((ESChildThread *)obj)->exit();
}

void 
ESChildThread::requestExit() {
    ESAssert(!inThisThread());
    ESAssert(_parentThread);
    ESAssert(_parentThread->inThisThread());
    ESAssert(_exitStrategy == ESChildThreadExitsOnlyByParentRequest);
    callInThread(requestExitGlue, this, NULL);
}

void
ESChildThread::requestExitAndWaitForJoin() {
    ESAssert(!inThisThread());
    ESAssert(_parentThread);
    ESAssert(_parentThread->inThisThread());
    // We assume a given parent thread is only waiting for one thread to join at a time here,
    // so we can use a single TLS bool to determine whether it's happened yet or not.
    ESAssert(exitingThreadHasBeenJoined);
    ESAssert(*exitingThreadHasBeenJoined);
    *exitingThreadHasBeenJoined = false;
    // ESErrorReporter::logInfo("ESChildThread::requestExitAndWaitForJoin", "thread %s waiting for thread %s to join",
    //                          _parentThread->name().c_str(), name().c_str());
    _waitingOnSocket = true;
    requestExit();
    while (!*exitingThreadHasBeenJoined) {
        // ESErrorReporter::logInfo("ESChildThread::requestExitAndWaitForJoin", "in loop, still waiting");
        // The logic here is a little tricky.  It's not safe to call the following method if the thread has been
        // deleted, which can happen more or less at any time.  But note that the TLS variable and the destruction
        // of the class happens in *this* thread, and in the same function (joinGlue above), so it the TLS value
        // is false, we know the thread has not been deleted.
        // There is a small theoretical hole in that as part of processing some (other) inter-thread message sent
        // to us, the parent thread, we attempt to shut down a different thread, thus updating the value of the
        // TLS bool, but the assert above should catch that when we re-enter this routine in that case.
        waitForAndProcessInterThreadMessages();
    }
    // ESErrorReporter::logInfo("ESChildThread::requestExitAndWaitForJoin", "Found join true, exiting loop");
}

void
ESChildThread::cleanupInThread() {
    tracePrintf2("will be joined by '%s', child (my) pthread id is %x", _parentThread->name().c_str(), pthread_self());

    requestJoin();

    // Note:  We have already sent off a request to join here, so at first glance this might seem
    // dangerous:  The thread might have been joined, and after that, this thread will be destroyed!  
    // But really, joining a thread waits for the thread to exit, so we're going to get to finish here
    // before join() returns and the parent thread goes on to delete 'this' thread.
    // Also, although it's moot in this case, Android doesn't do anything in platformSpecificThreadCleanup()
    // that depends on 'this' thread staying around (it depends on _javaVM, and that's it).
    platformSpecificThreadCleanup();
}

ESMainThread::~ESMainThread() {
    ESAssert(false);  // Never destroy the main thread.
}

ESSimpleWorkerThread::ESSimpleWorkerThread(const std::string         &name,
                                           ESChildThreadExitStrategy exitStrategy)
:   ESChildThread(name, exitStrategy)
{
}

/*virtual*/ void *
ESSimpleWorkerThread::main() {
    while (true) {
        fd_set readers;
        FD_ZERO(&readers);
        int highestThreadFD = _setBitsForSelect(&readers);
        int nfds = highestThreadFD + 1;
        select(nfds, &readers, NULL/*writers*/, NULL, NULL);
        _processInterThreadMessages(&readers);
    }
    return NULL;
}
