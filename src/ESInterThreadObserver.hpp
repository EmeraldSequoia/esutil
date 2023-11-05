//
//  ESInterThreadObserver.hpp
//
//  Created by Steve Pucci 17 May 2011
//  Copyright Emerald Sequoia LLC 2011. All rights reserved.
//

#ifndef _ESINTERTHREADOBSERVER_HPP_
#define _ESINTERTHREADOBSERVER_HPP_

#include "ESLock.hpp"

#include <list>

// Opaque declarations
class ESThread;

/*! This abstract class handles interthread communication for a particular scenario in which
 *  one thread generates observable events and another thread is observing them.  */
class ESInterThreadObserver {
  public:
                            ESInterThreadObserver(ESThread *generatingThread);  // Called in notification thread

    void                    release();          // Call in notification thread

  protected:
    // Methods redefined by derived classes:
    virtual void            initialize() = 0;        // will be called in generating thread with _creationLock locked (leave it locked)
    virtual void            notify(void *param) = 0; // will be called in notification thread
    virtual std::list<ESInterThreadObserver *> *&observersList() = 0;  // will be called only in generating thread -- return a pointer to the list pointer
                                                                       // that contains a list of this class's observers
    virtual void            possiblyNotifyChangeDuringInit() = 0;  // make sure to notify about changes that happen between when observer is created and it is installed

    // Methods called by derived classes:
    void                    callInterThreadInitialize();                  // Call this from the end of the leaf-class constructor (in notification thread)
    static void             callInterThreadNotifyObservers(const std::list<ESInterThreadObserver *> *observers,
                                                           void                                     *param);
    static void             notificationGlue(void *obj, void *param);

    // Methods called only by internals
    virtual                 ~ESInterThreadObserver();   // Don't call directly; call release() in notification thread;

    bool                    _released;
    ESThread                *_generatingThread;
    ESThread                *_notificationThread;

  private:
    static void             initializeGlue(void *obj, void *param);
    static void             releaseGlue(void *obj, void *param);
    static void             observerRemovedGlue(void *obj, void *param);

    static ESLock           _creationLock;
};

#endif  // _ESINTERTHREADOBSERVER_HPP_
