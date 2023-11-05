//
//  ESLock.hpp
//
//  Created by Steve Pucci 26 Jan 2011
//  Copyright Emerald Sequoia LLC 2011. All rights reserved.
//

#ifndef _ESLOCK_HPP_
#define _ESLOCK_HPP_

#include "ESPlatform.h"  // Must be first

#if ES_PTHREADS
#include <pthread.h>
#endif

/*! class description */
class ESLock {
  public:
                            ESLock();
    virtual                 ~ESLock();
    void                    lock();
    void                    unlock();

  private:
#if ES_PTHREADS
    pthread_mutex_t        _mutex;  // Consider using spinlock here (but note that on a uniprocessor it will be worse)
#else
error "Need a non-pthreads solution on Windows";
#endif
};

#endif  // _ESLOCK_HPP_
