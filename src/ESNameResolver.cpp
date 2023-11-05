//
//  ESNameResolver.cpp
//
//  Created by Steve Pucci 13 May 2011
//  Copyright Emerald Sequoia LLC 2011. All rights reserved.
//

#include "ESNameResolver.hpp"
#include "ESThread.hpp"
#include "ESErrorReporter.hpp"
#undef ESTRACE
#include "ESTrace.hpp"

class ESNameResolverThread : public ESChildThread {
  public:
                            ESNameResolverThread(ESNameResolver *resolver);

    /*virtual*/ void        *main();  // redefine main loop of thread

  private:
    ESNameResolver          *_resolver;
};


ESNameResolverThread::ESNameResolverThread(ESNameResolver *resolver)
:   _resolver(resolver),
    ESChildThread(std::string("resolve(") + resolver->requestedName() + ")", ESChildThreadExitsOnlyWhenFinished)
{
}

/*virtual*/ void *
ESNameResolverThread::main() {  // redefine main loop of thread
    ESAssert(inThisThread());
    return _resolver->threadMain();
}

ESNameResolver::ESNameResolver(ESNameResolverObserver *observer,
                               const std::string      &name,
                               const std::string      &portAsString,
                               int                    hintsFlags,
                               int                    hintsProtocol)
:   _observer(observer),
    _requestedName(name),
    _result0(NULL),
    _released(false),
    _readyForDelete(false),
    _portAsString(portAsString)
{
    _hints.ai_flags = hintsFlags;
    _hints.ai_family = PF_UNSPEC;
    _hints.ai_socktype = SOCK_DGRAM;
    _hints.ai_protocol = hintsProtocol;
    _hints.ai_addrlen = 0;
    _hints.ai_addr = NULL;
    _hints.ai_canonname = NULL;
    _hints.ai_next = NULL;
    _notificationThread = ESThread::currentThread();
    tracePrintf1("will start thread resolve(%s)", name.c_str());
    _resolverThread = new ESNameResolverThread(this);
    tracePrintf3("ESNameResolver ctor of 0x%08x created thread 0x%08x whose parent thread is 0x%08x\n", (unsigned long int)this, (unsigned long int)_resolverThread, (unsigned long int)_resolverThread->parentThread());
    _resolverThread->start();
}

ESNameResolver::~ESNameResolver() {
    ESAssert(_notificationThread->inThisThread());
    ESAssert(_released);
    ESAssert(_readyForDelete);  // Only delete things via release()
}

void *
ESNameResolver::threadMain() {
    ESAssert(_resolverThread->inThisThread());
    tracePrintf1("will call getaddrinfo(%s)", _requestedName.c_str());
    int st = getaddrinfo(_requestedName.c_str(), _portAsString.c_str(), &_hints, &_result0);
    tracePrintf2("back from getaddrinfo(%s), st %d", _requestedName.c_str(), st);
    if (st == 0) {
        deliverNotifyNameResolutionComplete();
    } else {
        deliverNotifyNameResolutionFailed(st);
    }
    return NULL;
}

void 
ESNameResolver::release() {
    tracePrintf2("release of resolver 0x%08x asserting we're in the thread 0x%08x\n", (unsigned long int)this, (unsigned long int)_notificationThread);
    ESAssert(_notificationThread->inThisThread());
    _released = true;
    if (_readyForDelete) {
        delete this;
    }
}

/*static*/ void 
ESNameResolver::resolutionCompleteGlue(void *obj,
                                       void *param) {
    ESNameResolver *resolver = (ESNameResolver *)obj;
    ESAssert(resolver->_notificationThread->inThisThread());
    if (resolver->_released) {
        tracePrintf1("no delivery for released resolver(%s)", resolver->requestedName().c_str());
        resolver->_readyForDelete = true;
        delete resolver;
    } else {
        tracePrintf1("will notify good resolve(%s)", resolver->requestedName().c_str());
        resolver->_observer->notifyNameResolutionComplete(resolver);
        resolver->_readyForDelete = true;
    }
}

void 
ESNameResolver::deliverNotifyNameResolutionComplete() {
    ESAssert(_resolverThread->inThisThread());
    tracePrintf1("will notify good resolve(%s)", _requestedName.c_str());
    _notificationThread->callInThread(resolutionCompleteGlue, this, NULL);
}

/*static*/ void 
ESNameResolver::resolutionFailedGlue(void *obj,
                                     void *param) {
    ESNameResolver *resolver = (ESNameResolver *)obj;
    ESAssert(resolver->_notificationThread->inThisThread());
    int failureStatus = (int)(ESPointerSizedInt)param;
    if (resolver->_released) {
        tracePrintf1("no FAILED delivery for released resolver(%s)", resolver->requestedName().c_str());
        resolver->_readyForDelete = true;
        delete resolver;
    } else {
        tracePrintf1("will notify FAILED resolve(%s)", resolver->requestedName().c_str());
        resolver->_observer->notifyNameResolutionFailed(resolver, failureStatus);
        resolver->_readyForDelete = true;
    }
}

void 
ESNameResolver::deliverNotifyNameResolutionFailed(int status) {
    ESAssert(_resolverThread->inThisThread());
    tracePrintf1("will notify FAILED resolve(%s)", _requestedName.c_str());
    _notificationThread->callInThread(resolutionFailedGlue, this, (void*)(long)status);
}

