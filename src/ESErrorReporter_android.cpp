//
//  ESErrorReporter_android.cpp
//
//  Created by Steve Pucci 17 Feb 2011
//  Copyright Emerald Sequoia LLC 2011. All rights reserved.
//

#include "ESErrorReporter.hpp"
#include "ESUtil.hpp"
#include "ESOfflineLogger.hpp"

#include <android/log.h>

/*static*/ void 
ESErrorReporter::logErrorImpl(const char *where,
                              const char *str) {
    if (where) {
        //fprintf(stderr, "%s: ", where);
        __android_log_print(ANDROID_LOG_ERROR, where, "%s", str);
    } else {
        __android_log_print(ANDROID_LOG_ERROR, "EmeraldSequoia", "%s", str);
    }
}

/*static*/ void 
ESErrorReporter::logInfoImpl(const char *where,
                             const char *str) {
    if (where) {
        //fprintf(stderr, "%s: ", where);
        __android_log_print(ANDROID_LOG_INFO, where, "%s", str);
    } else {
        __android_log_print(ANDROID_LOG_INFO, "EmeraldSequoia", "%s", str);
    }
}


/*static*/ void 
ESErrorReporter::_abortOnAssert(const char *cond,
                                const char *file,
                                int        lineno) {
    __android_log_assert(cond, "assert", "Assert failure at %s:%d: %s", file, lineno, cond);
}

/*static*/ void 
ESErrorReporter::logOffline(const std::string &text) {
    ESOfflineLogger::log(text);
}
