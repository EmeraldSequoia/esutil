//
//  ESThreadLocalStorage.hpp
//
//  Created by Steve Pucci 30 Jan 2011
//  Copyright Emerald Sequoia LLC 2011. All rights reserved.
//

#ifndef _ESTHREADLOCALSTORAGE_HPP_
#define _ESTHREADLOCALSTORAGE_HPP_

#include "ESPlatform.h"

#if ES_PTHREADS
#include <pthread.h>
#else
#error "Need a Windows implementation (use TLSAlloc and friends)";
#endif

class ESThreadLocalStorageBase {
  protected:
                            ESThreadLocalStorageBase();
                            ~ESThreadLocalStorageBase();
    void                    setValue(void *v);
    void                    *getValue();
#if ES_PTHREADS
    pthread_key_t           _key;
#endif
    
};

/*! Thread-local variables */
template<class PointerType>
class ESThreadLocalStoragePtr : public ESThreadLocalStorageBase {
  public:
                            ESThreadLocalStoragePtr() {}
                            ~ESThreadLocalStoragePtr() {}
    operator                PointerType *() { return reinterpret_cast<PointerType *>(getValue()); }
    void                    operator=(PointerType *p) { setValue(p); }
    bool                    operator==(PointerType *p) { return reinterpret_cast<PointerType *>(getValue()) == p; }
};

template<class scalarType>
class ESThreadLocalStorageScalar : public ESThreadLocalStorageBase {
  public:
                            ESThreadLocalStorageScalar() {}
                            ~ESThreadLocalStorageScalar() {}
    operator                scalarType() { return (scalarType)(ESPointerSizedInt)(getValue()); }
    void                    operator=(scalarType t) { setValue((void*)t); }
    bool                    operator==(scalarType t) { return (scalarType)(getValue()) == t; }
};

#if ES_PTHREADS
#include "ESThreadLocalStorageInl_pthreads.hpp"
#endif

#endif  // _ESTHREADLOCALSTORAGE_HPP_
