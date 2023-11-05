//
//  ESNameResolver.hpp
//
//  Created by Steve Pucci 13 May 2011
//  Copyright Emerald Sequoia LLC 2011. All rights reserved.
//

#ifndef _ESNAMERESOLVER_HPP_
#define _ESNAMERESOLVER_HPP_

#include <string>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

// Forward declarations
class ESNameResolver;

// Opaque declarations
class ESThread;
class ESNameResolverThread;

/*! Abstract observer base class -- derive from this to be notified when the name resolution is complete. */
class ESNameResolverObserver {
  public:
    virtual void            notifyNameResolutionComplete(ESNameResolver *resolver) = 0;
    virtual void            notifyNameResolutionFailed(ESNameResolver *resolver,
                                                       int            failureStatus) = 0;
};

/*! Objects of this class resolve a single name in a separate thread, notifying the observer when the
 *  resolution is complete (or has failed).
 *  Once activated, the resolver must not be directly destroyed by clients, because it may be in use
 *  in the separate thread.  Instead, call release() when you want to destroy it (just like an ESTimer).
 */
class ESNameResolver {
  public:
                            ESNameResolver(ESNameResolverObserver *observer,
                                           const std::string      &name,
                                           const std::string      &portAsString,
                                           int                    hintsFlags,
                                           int                    hintsProtocol);
    void                    release();  // Call this in place of 'delete resolver' when you want to destroy it, and only in the thread used to construct it

    std::string             requestedName() const { return _requestedName; }
    std::string             portAsString() const { return _portAsString; }
    struct addrinfo         *result0() const { return _result0; }   // valid until release() is called on this object

  private:
                            ~ESNameResolver();
    void                    *threadMain();
    
    static void             resolutionCompleteGlue(void *obj,
                                                   void *param);
    static void             resolutionFailedGlue(void *obj,
                                                 void *param);

    void                    deliverNotifyNameResolutionComplete();
    void                    deliverNotifyNameResolutionFailed(int status);

    ESNameResolverObserver  *_observer;
    ESNameResolverThread    *_resolverThread;
    ESThread                *_notificationThread;
    bool                    _released;       // Don't try to use this outside the notification thread
    bool                    _readyForDelete; // Don't try to use this outside the notification thread
    std::string             _requestedName;
    std::string             _portAsString;
    struct addrinfo         _hints;
    struct addrinfo         *_result0;

friend class ESNameResolverThread;
};

#endif  // _ESNAMERESOLVER_HPP_
