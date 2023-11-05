//
//  ESPlatform.h
//
//  Created by Steve Pucci on 11/19/2010
//  Copyright 2010 Emerald Sequoia LLC. All rights reserved.
//
//  This file contains defines, constants, small utility methods that are different on different platforms

#ifndef _ESPLATFORM_H
#define _ESPLATFORM_H

#include <stdint.h>
typedef int32_t  ESINT32;
typedef int64_t  ESINT64;
typedef uint32_t ESUINT32;
typedef uint64_t ESUINT64;

#if (defined(ES_IOS) && ES_IOS)
#define ES_MACOS 0
#define ES_ANDROID 0
#elif (defined(ES_MACOS) && ES_MACOS)
#define ES_IOS 0
#define ES_ANDROID 0
#elif (defined(ES_ANDROID) && ES_ANDROID)
#define ES_IOS 0
#define ES_MACOS 0
#else
#error "Build system (project file or makefile) needs to define ES_IOS=1, ES_MACOS=1, or ES_ANDROID=1"
#endif

#if ES_IOS || ES_MACOS
#define ES_COCOA 1
#else
#define ES_COCOA 0
#endif

#if ES_COCOA || ES_ANDROID
#define ES_PTHREADS 1
#else
#define ES_PTHREADS 0
#endif

#if ES_COCOA
#include <libkern/OSAtomic.h>  // For OSMemoryBarrier()

# if __LP64__ || NS_BUILD_32_LIKE_64
typedef long ESPointerSizedInt;
# else
typedef int ESPointerSizedInt;
# endif
#elif ES_ANDROID
# if __SIZEOF_POINTER__ == 8
typedef int64_t ESPointerSizedInt;
# elif __SIZEOF_POINTER__ == 4
typedef int32_t ESPointerSizedInt;
# else
#  error "Android macros failed to find pointer size"
# endif
#else
typedef int ESPointerSizedInt; // Until this doesn't work...
#endif

#if ES_COCOA
#define ESOSMemoryBarrier() (OSMemoryBarrier())
#else
#define ESOSMemoryBarrier()
#endif

#if ES_ANDROID
#define nan(str) __builtin_nan(str)
#endif

// Snippet to declare an opaque @class even if not compiling under ObjC
#ifdef __OBJC__
#define ES_OPAQUE_OBJC(className) @class className
#else
#define ES_OPAQUE_OBJC(className) class className  // Not really but it doesn't matter because we can't use it without ObjC anyway
#endif

// Snippet to declare an opaque class even if not compiling under C++  (though strictly speaking this shouldn't be necessary; always use .mm files)
#ifdef __cplusplus
#define ES_OPAQUE_CPLUSPLUS(className) class className
#else
#define ES_OPAQUE_CPLUSPLUS(className) @class className  // Not really but it doesn't matter because we can't use it without C++ anyway
#endif

#endif // _ESPLATFORM_H
