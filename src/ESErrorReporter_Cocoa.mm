//
//  ESErrorReporter_Cocoa.mm
//
//  Created by Steve Pucci 17 Feb 2011
//  Copyright Emerald Sequoia LLC 2011. All rights reserved.
//

#include "ESErrorReporter.hpp"

#include <stdlib.h>

/*static*/ void 
ESErrorReporter::logErrorImpl(const char *where,
                              const char *str) {
    if (where) {
        //fprintf(stderr, "%s: ", where);
        NSLog(@"%s: %s", where, str);
    } else {
        NSLog(@"%s", str);
    }
}

/*static*/ void 
ESErrorReporter::logInfoImpl(const char *where,
                             const char *str) {
    if (where) {
        //fprintf(stderr, "%s: ", where);
        NSLog(@"%s: %s", where, str);
    } else {
        NSLog(@"%s", str);
    }
}

/*static*/ void 
ESErrorReporter::_abortOnAssert(const char *cond,
                                const char *file,
                                int        lineno) {
    NSLog(@"Assert failure at %s:%d: %s\n", file, lineno, cond);
    abort();
}

/*static*/ void 
ESErrorReporter::logOffline(const std::string &text) {
    // Do nothing for now.
}
