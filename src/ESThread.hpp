//
//  ESThread.hpp
//
//  Created by Steve Pucci 08 Jan 2011
//  Copyright Emerald Sequoia LLC 2011. All rights reserved.
//

#ifndef _ESTHREAD_HPP
#define _ESTHREAD_HPP

#include "ESPlatform.h"  // Must be first

#include <unistd.h>

#include <string>

#if ES_PTHREADS
#include <pthread.h>
#endif

#if ES_ANDROID
#include "jni.h"
#endif

ES_OPAQUE_OBJC(NSAutoreleasePool);

typedef void (*ESInterThreadFn)(void *object, void *param);

enum ESChildThreadExitStrategy {
    ESChildThreadExitsOnlyByParentRequest,
    ESChildThreadExitsOnlyWhenFinished,
    ESChildThreadNeverExits
};

// To define a new kind of child thread, do something like
//class MyThread : public ESChildThread {
//  public:
//                            MyThread(...);
//                            ~MyThread();
//    void                    *main();
//}
//
// void createAndStartMyThread() {
//     MyThread *myThread = new MyThread(...);
//     myThread->start();
// }
//
// Strictly speaking, this is all you need to do.  Your thread's main()
// method will get called in the thread's context after it has been started.
// If you want to also communicate with this thread (other than by global state)
// they you may also want to examine the inter-thread methods defined below
// so your thread's main loop can accept inter-thread calls from other threads.


// Ending threads:
//
// There are two ways a thread can exit:
//  - The parent thread can instruct the thread to exit or return.  In such a case
//    the (child) thread must run a message-handling loop.
//  - The child thread can run out of things to do and want to exit on its own.

// The actual destruction of an ESThread happens in the parent thread after the parent
// has joined the thread.  This happens unconditionally to avoid race conditions, so it
// requires that the parent thread no longer have a pointer to the thread after the thread
// exits.  This requires the following convention:
//  - When a parent thread instructs the child to exit, it must then treat the thread
//    as destroyed (removing all references to it) by the time it gets back to the main loop
//  - When the child thread quits on its own, it must first inform the parent thread
//    (this notification may be implicit in its final functional message), and when
//    the parent thread receives this notification it must treat the thread as destroyed
//    (removing all references to it) by the time it gets back to its main loop.
//
// BOTH KINDS OF EXIT IN THE SAME THREAD:  NOT CURRENTLY SUPPORTED, BUT HERE'S HOW IT MIGHT BE:
// 
// When both kinds of exit can happen in the same thread, the child must not be allowed to
// simply send a message and quit, to avoid having that message cross with a parent thread
// sending the child thread a message.  In this case the right approach is for the child
// thread to send an "I'm done" message to the parent, *and then continue to wait and read
// messages*.  (Note that if the parent never is allowed to request an exit, as with the case
// of the ESNameResolver thread, this wait-read loop at the end is unnecessary).

// Further, the parent, when sending its exit request, must safely handle any following
// child's "I'm done" message even though it may never get such a message and must (by
// the convention above) not retain any reference to the child after sending the exit
// request.  In particular, the parent thread is not allowed to *itself* exit (*ever* --
// this could be circumvented by maintaining a count of unjoined child threads in each
// thread and only allowing exit when the count goes to zero).

// So to handle this two-exit-mechanism case: If the parent wants the child to exit, it does
// the requestExit() as it would in the other case, but first setting _parentDone in the
// thread.  This flag allows any inbound messages from the child thread that get
// delivered after this point to be properly ignored.  If the child wants to exit on its
// own, it (as with the single-mechanism case) is required to notify the parent with a
// message to remove all references to the thread (and do any other cleanup that is
// required); when this message is received in the parent thread,
//  - if _parentDone is set the message is ignored
//  - if _parentDone is not set the message is handled as normal, i.e., by removing references
//       to the thread.

// Note that this checking of _parentDone (ESThread::parentDone()) is not easily done by
// the thread infrastructure.  The message-passing infrastructure does not currently have
// a pointer to the sending thread, so when the parent thread receives the message it
// cannot in general determine whether the message should be ignored because it comes
// from a child thread which the parent is done with (that is, it doesn't know which
// thread to call parentDone() on, and in fact it doesn't even know that the message is
// even coming from a child thread rather than a parent or peer thread).  I suppose the
// calling thread could know that it is calling its parent (by making an otherwise
// gratuitous which-thread-am-i call and then checking that thread's parent field), and
// in that case pass an extra hidden parameter in the message, which the message handler
// in the receiver can than receive, interpret as a child thread (since that's the only
// time it will appear), and then check the _parentDone flag to ignore.  Is this extra
// what-thread-am-i overhead *on every message send* worth it, or should we just avoid
// this situation by disallowing threads that can exit in two ways?  Note that we can't
// let the parent-thread code do *anything* other than examine the message itself,
// because the parent is not allowed to keep a pointer to the thread so it can check for
// the parentDone flag.  Perhaps the special code to send the calling thread if it is a
// child can be done only for threads which declare that they have both kinds of exit;
// but that flag would have to be on the parent (recipient) thread to be of use in
// circumventing the call to what-thread-am-i

// Or perhaps this is best handled with a semaphore somehow.  Let's worry about it when
// it happens, which is probably never... In the meantime, a child thread's owner must pick
// which mechanism it will use and stick with it.

// An ESThread has communication and identity but no start/stop, so that it can
// be used for the main thread also
class ESThread {
  public:
                            ESThread(const std::string &name);

    // The following routines (callIn*Thread) depend on the thread watching a particular
    // socket for messages.  This is set up for reception in the main thread automatically,
    // but the implementor of each thread's main() method must ensure that the thread's main
    // loop will select on (or at least poll) the socket for available messages to read.
    // If the thread's main loop has a select() anyway, then the simple thing is for that
    // main loop to add checking for the thread's socket in its select processing; if this
    // approach is used, there are convenience functions to do that (in the block following).
#if ES_ANDROID
    virtual
#endif
    void                    callInThread(ESInterThreadFn fn,
                                         void            *object,
                                         void            *param,
                                         bool            forceUseSocket = false);
    static void             callInMainThread(ESInterThreadFn fn,
                                             void            *object,
                                             void            *param);

    // Convenience functions for inter-thread communication
    // Call these in the thread's select loop
    // The methods below are static, but return different values in different threads
    static int              setBitsForSelect(fd_set *fdset);  // returns highest bit set
    static void             processInterThreadMessages(fd_set *fdset);

    /** Wait for at least one message to come in, handle all that have come in, and return.
     *  So this routine will not return until it has processed at least one message.
     *  Note:  No other input processing is done in this method, so if this is the main thread,
     *  no UI events will be handled, and in other threads, no other sockets will be examined.
     *  It is intended for short-term waits while calculations finish. */
    static void             waitForAndProcessInterThreadMessages();

    // If you don't have a select loop, find some way of getting the main loop to check for
    // input on this socket
    static int              myInterThreadSocket();             // this thread reads from this socket
    int                     correspondentInterThreadSocket();  // other threads write to this socket

    // If the first ESThread is not started in the main thread, or if
    // isMainThread() or mainThread() are called in some non-main
    // thread before starting the first ESThread, use this function
    // first *in the main thread* to declare which one it is.
    static void             setMainThreadToThisOne();

    bool                    inThisThread();
    static bool             inMainThread();

    static ESThread         *mainThread();
    static ESThread         *currentThread();

    // Utility method for callback
    void                    readAndExecuteInterThreadFunction();

    static void             verifyThreadSocketWithPeek(const char *msg = NULL);

    std::string             name() { return _name; }

    int                     _setBitsForSelect(fd_set *fdset);
    void                    _processInterThreadMessages(fd_set *fdset);

    static void             shutdown();  // init happens during the first thread construction

    // Useful to set up "per-event-loop" style setup/cleanup.
    virtual void            preInterThreadFunction() {}
    virtual void            postInterThreadFunction() {}

  protected:
    virtual                 ~ESThread();  // Don't delete a thread directly; parent threads should call requestExit(), and child threads should just return from main() or call exit()

    static bool             osDirectInMainThread();  // For debug assert only; returns true always in release mode

    static ESThread         *_mainThread;

    static void             initStatics();
    void                    initializeInThread();
    virtual void            platformSpecificThreadInitialization() {}
    virtual void            platformSpecificThreadCleanup() {}

    static void             platformSpecificInit();

    int                     _myInterThreadSocket;

#if ES_PTHREADS
    pthread_t               _pthread;
#else
error "Need a non-pthreads solution on Windows";
#endif

  private:
    std::string             _name;  // For debugging only

    int                     _correspondentInterThreadSocket;
};

// An ESChildThread is what clients create and redefine
class ESChildThread : public ESThread {
  public:
    // Methods called in the calling thread:
                            ESChildThread(const std::string         &name,          // Primarily for debug
                                          ESChildThreadExitStrategy exitStrategy);  // Entirely for debug
    void                    start();
    void                    requestExit();
    void                    requestExitAndWaitForJoin();  // Must be called by parent thread; will block parent thread until child completes, so make sure that happens quickly

    // Methods called in this thread:
    virtual void            exit();  // Override if you need to do any cleanup before the thread exits as a result of requestExit(); be sure to call superclass method when finished

    // Methods required to be overridden in derived classes
    virtual void            *main() = 0;

    // e.g., NSAutoreleasePool init/release
    /*virtual*/ void        platformSpecificThreadInitialization();
    /*virtual*/ void        platformSpecificThreadCleanup();

    void                    cleanupInThread();

    ESThread                *parentThread() const { return _parentThread; }

    ESChildThreadExitStrategy exitStrategy() const { return _exitStrategy; }

  protected:
    virtual                 ~ESChildThread();  // Don't delete a thread directly; parent threads should call requestExit(), and child threads should just return from main() or call exit()

    void                    requestJoin();  // Sends message to creating thread to join immediately prior to exit
    void                    *join();  // Join is called automatically in parent thread upon receipt of the message that says the thread is complete
    static void             joinGlue(void *obj,
                                     void *param);

    void                    preInterThreadFunction();
    void                    postInterThreadFunction();

  private:
    ESThread                *_parentThread;
    ESChildThreadExitStrategy _exitStrategy;
    bool                    _waitingOnSocket;

#if ES_COCOA
    NSAutoreleasePool       *_pool;
#endif
friend void *ESThreadStarter(void *);
};

class ESMainThread : public ESThread {
  public:
                            ESMainThread();
#if ES_ANDROID
    /*virtual*/ void        callInThread(ESInterThreadFn fn,
                                         void            *object,
                                         void            *param,
                                         bool            forceUseSocket = false);
    static void             dispatchMethodInThread(JNIEnv  *jniEnv,
                                                   jobject activity,
                                                   jobject message);
#endif

  private:
                            ~ESMainThread();
#if ES_COCOA || ES_ANDROID
    /*virtual*/ void        platformSpecificThreadInitialization();
#endif
};

/** This thread does nothing (and can do nothing) except respond to callInThread() messages. */
class ESSimpleWorkerThread: public ESChildThread {
  public:
                            ESSimpleWorkerThread(const std::string         &name,
                                                 ESChildThreadExitStrategy exitStrategy);
    virtual void            *main();
};

#endif // ESTHREAD_HPP
