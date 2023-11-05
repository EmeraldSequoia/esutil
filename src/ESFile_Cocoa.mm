//
//  ESFile_Cocoa.mm
//
//  Created by Steven Pucci 27 Jul 2011
//  Copyright Emerald Sequoia LLC 2011. All rights reserved.
//

#include <sys/xattr.h>

#include "ESFile.hpp"
#include "ESUtil.hpp"
#include "ESErrorReporter.hpp"

/*static*/ std::string
ESFile::resourceDirectory() {
    static std::string cacheDir;
    if (cacheDir.length() == 0) {
        NSBundle *bundle;
        std::string primaryBundleID = ESUtil::primaryBundleID();
        if (primaryBundleID.length() > 0) {
            NSLog(@"Looking for bundle with id %@", [NSString stringWithUTF8String:primaryBundleID.c_str()]);
            bundle = [NSBundle bundleWithIdentifier:[NSString stringWithUTF8String:primaryBundleID.c_str()]];
        } else {
            bundle = [NSBundle mainBundle];
        }
        ESAssert(bundle);
        cacheDir = [[bundle resourcePath] UTF8String];
    }
    return cacheDir;
}

/*static*/ std::string 
ESFile::documentDirectory() {
    static std::string cacheDir;
    if (cacheDir.length() == 0) {
        NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
        cacheDir = [[paths objectAtIndex:0] UTF8String];
    }
    return cacheDir;
}

/*static*/ std::string 
ESFile::appSupportDirectory() {  // without trailing slash
    static std::string cacheDir;
    if (cacheDir.length() == 0) {
        NSArray *paths = NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES);
        std::string parent = [[paths objectAtIndex:0] UTF8String];
        ensureDirectoryExists(parent);
        cacheDir = parent + "/" + ESUtil::appIdentifier();
        ensureDirectoryExists(cacheDir);
    }
    return cacheDir;
}

/*static*/ void 
ESFile::setFileNotBackedUp(const char *filename) {
    const char* attrName = "com.apple.MobileBackup";
    u_int8_t attrValue = 1;
 
    int result = setxattr(filename, attrName, &attrValue, sizeof(attrValue), 0, 0);
    if (result == 0) {
        //printf("Set attribute successfully on %s\n", filename);
    } else {
        printf("Set attribute FAILED on %s\n", filename);
        perror("mobile backup attribute set via setxattr");
    }
}


