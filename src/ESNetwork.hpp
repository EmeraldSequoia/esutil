//
//  ESNetwork.hpp
//
//  Created by Steve Pucci 16 May 2011
//  Copyright Emerald Sequoia LLC 2011. All rights reserved.
//

#ifndef _ESNETWORK_HPP_
#define _ESNETWORK_HPP_

#include "ESInterThreadObserver.hpp"

#include <string>

#if ES_COCOA
#include <SystemConfiguration/SCNetworkReachability.h>
#endif

// Opaque declarations
class ESThread;

// ==========================

/*! Class which can return some information about whether the network is up */
class ESNetwork {
  public:
    static bool             internetIsReachable();                            // call in any thread
    static bool             hostIsReachable(const struct sockaddr *address);  // call in any thread
    static void             startNetworkActivityIndicator();                  // call in any thread; may be nested
    static void             stopNetworkActivityIndicator();                   // call in any thread; may be nested

  private:
    // To be implemented in OS-specific code (or not)
    static void             startNetworkActivityIndicatorForRealInMainThread();
    static void             stopNetworkActivityIndicatorForRealInMainThread();

    static void             startNetworkActivityGlue(void *obj,
                                                     void *param);
    static void             stopNetworkActivityGlue(void *obj,
                                                    void *param);
};

// ==========================

/*! Abstract class to allow clients to be notified when the reachability of the network as a whole changes */
class ESNetworkInternetObserver : public ESInterThreadObserver {
  public:
                            ESNetworkInternetObserver(bool currentReachability);  // As known to client.  Call in notification thread
    virtual void            internetIsNowReachable() = 0;
    virtual void            internetIsNowUnreachable() = 0;

  protected:
    virtual                 ~ESNetworkInternetObserver();   // Do not delete directly; call release() (it's defined on the base class)

    // ESInterThreadObserver overrides
    /*virtual*/ void        initialize();        // will be called in generating thread
    /*virtual*/ void        notify(void *param); // will be called in notification thread
    /*virtual*/ std::list<ESInterThreadObserver *> *&observersList();  // will be called only in generating thread -- return a reference to the list pointer
                                                                       // that contains a list of this class's observers
    /*virtual*/ void        possiblyNotifyChangeDuringInit();  // make sure to notify about changes that happen between when observer is created and it is installed

  private:
#if ES_COCOA
    static void             networkReachabilityCallback(SCNetworkReachabilityRef   reachabilityRef,
                                                        SCNetworkReachabilityFlags flags,
                                                        void                       *info);
#endif
    bool                    _lastReachabilityStatus;

    static std::list<ESInterThreadObserver *> *_observersList;
};

#endif  // _ESNETWORK_HPP_
