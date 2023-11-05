//
//  ESInterThreadObserver.cpp
//
//  Created by Steve Pucci 17 May 2011
//  Copyright Emerald Sequoia LLC 2011. All rights reserved.
//

#include "ESInterThreadObserver.hpp"

#include "ESThread.hpp"
#include "ESErrorReporter.hpp"

/*static*/ ESLock ESInterThreadObserver::_creationLock;

ESInterThreadObserver::ESInterThreadObserver(ESThread *generatingThread) 
:   _released(false),
    _generatingThread(generatingThread),
    _notificationThread(ESThread::currentThread())
{
}

void 
ESInterThreadObserver::callInterThreadInitialize() {   // Call this from the end of the leaf-class constructor
    ESAssert(_notificationThread->inThisThread());
    if (_notificationThread == _generatingThread) {  // If this happens all of the time this class is unnecessary but possibly the clients are distributed among threads
        initializeGlue(this, NULL);
    } else {
        _generatingThread->callInThread(initializeGlue, reinterpret_cast<void *>(this), NULL);
    }
}

/*virtual*/ 
ESInterThreadObserver::~ESInterThreadObserver() {   // Don't call directly; call release() in notification thread
    ESAssert(_notificationThread->inThisThread());
}

void 
ESInterThreadObserver::release() {          // Call in notification thread
    ESAssert(_notificationThread->inThisThread());
    _released = true;
    if (_notificationThread == _generatingThread) {
        releaseGlue(this, NULL);
    } else {
        _generatingThread->callInThread(releaseGlue, this, NULL);
    }
}

/*static*/ void 
ESInterThreadObserver::notificationGlue(void *obj, void *param) {
    ESInterThreadObserver *observer = (ESInterThreadObserver *)obj;
    ESAssert(observer->_notificationThread->inThisThread());
    if (!observer->_released) {
        observer->notify(param);
    }
}

/*static*/ void 
ESInterThreadObserver::callInterThreadNotifyObservers(const std::list<ESInterThreadObserver *> *observers,
                                                      void                                     *param) {
    ESAssert(observers);  // Should have been set up in ctor via initialize()
    std::list<ESInterThreadObserver *>::const_iterator end = observers->end();
    std::list<ESInterThreadObserver *>::const_iterator iter = observers->begin();
    while (iter != end) {
        ESInterThreadObserver *observer = *iter;
        ESAssert(observer->_generatingThread->inThisThread());
        if (observer->_notificationThread == observer->_generatingThread) {
            observer->notify(param);
        } else {
            observer->_notificationThread->callInThread(notificationGlue, observer, param);
        }
        iter++;
    }
}

/*static*/ void 
ESInterThreadObserver::initializeGlue(void *obj, void *param) {
    ESInterThreadObserver *observer = reinterpret_cast<ESInterThreadObserver *>(obj);
    ESAssert(observer->_generatingThread->inThisThread());
    _creationLock.lock();
    std::list<ESInterThreadObserver *> *&observers = observer->observersList();
    if (!observers) {
        observers = new std::list<ESInterThreadObserver *>;
    }
    observer->initialize();  // Call this before adding to observers list to avoid clients getting notification before initialization
    _creationLock.unlock();
    observers->push_back(observer);
    observer->possiblyNotifyChangeDuringInit();
}

/*static*/ void 
ESInterThreadObserver::releaseGlue(void *obj, void *param) {
    ESInterThreadObserver *observer = reinterpret_cast<ESInterThreadObserver *>(obj);
    ESAssert(observer->_generatingThread->inThisThread());
    ESAssert(observer->observersList());  // Presumably this happens after the ctor which was supposed to have called initialize
    observer->observersList()->remove(observer); // We'd like to check the return value but STL in its wisdom returns void here
    if (observer->_generatingThread == observer->_notificationThread) {
        observerRemovedGlue(observer, NULL);
    } else {
        observer->_notificationThread->callInThread(observerRemovedGlue, observer, NULL);
    }
}

/*static*/ void 
ESInterThreadObserver::observerRemovedGlue(void *obj, void *param) {
    ESInterThreadObserver *observer = reinterpret_cast<ESInterThreadObserver *>(obj);
    delete observer;
}

