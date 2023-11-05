//
//  ESFile.hpp
//
//  Created by Steven Pucci 27 Jul 2011
//  Copyright Emerald Sequoia LLC 2011. All rights reserved.
//

#ifndef _ESFILE_HPP_
#define _ESFILE_HPP_

#include <string>

#include "ESPlatform.h"
#if ES_ANDROID
#include "ESJNIDefs.hpp"
#define ES_SIMPLE_RESOURCE 0
#else
#define ES_SIMPLE_RESOURCE 1
#endif

enum ESFilePathType {
    ESFilePathTypeRelativeToResourceDir,
    ESFilePathTypeRelativeToDocumentDir,
    ESFilePathTypeRelativeToAppSupportDir,
    ESFilePathTypeRelativeToAppSupportThenResourceDir,
};

// Interface class which handles the closing of a file if necessary.
class ESFileCloser {
  public:
    virtual                 ~ESFileCloser() {}
    virtual void            closeAndDie() = 0;
};

/*! Utility file functions */
class ESFile {
  public:
    static const char *     pathTypeString(ESFilePathType pathType);

    static unsigned         readSingleUnsignedFromFile(const char     *path,
                                                       ESFilePathType pathType);

    static int              getFDPointingAtFile(const char     *path,
                                                ESFilePathType pathType,
                                                bool           missingOK,
                                                size_t         *fileSizeReturn,
                                                ESFileCloser   **fileCloser);

    static int              getFDPointingAtFileInDirectory(const char        *path,
                                                           const std::string &dir,
                                                           bool              missingOK,
                                                           size_t            *fileSizeReturn);

    static int              getFDPointingAtResource(const char   *resourcePath,
                                                    bool         missingOK,
                                                    size_t       *resourceSizeReturn,
                                                    ESFileCloser **fileCloser);

    static char             *getFileContentsInMallocdArray(const char     *path,
                                                           ESFilePathType pathType,
                                                           bool           missingOK,
                                                           size_t         *fileSizeReturn);

    static bool             fileExistsAtPath(const char *path);

    static void             removeFileAtPath(const char *path);
    static void             removeFileAtPathIfPresent(const char *path);

    /** Write the given buffer to the given file.
     *  @return true iff the write was successful. */
    static bool             writeArrayToFile(const void *buf,
                                             size_t     buflen,
                                             const char *path,
                                             ESFilePathType pathType);  // Must be ESFilePathTypeRelativeToAppSupportDir

    static std::string      ensureDirectoryExistsInAppSupportDirectoryWithRelativePath(const char *relativePath);
    static std::string      ensureDirectoryExists(const std::string &absolutePath);

    /** The directory in which to place documents visible to the user */
    static std::string      documentDirectory();  // without trailing slash

    /** The directory in which to place documents the user doesn't need to see (~/Library/Application Support/<appname>) */
    static std::string      appSupportDirectory();  // without trailing slash

#if ES_SIMPLE_RESOURCE
    static std::string      resourceDirectory();  // without trailing slash
#endif

    static void             setFileNotBackedUp(const char *filename);

    static void             setFileNotBackedUp(const char     *relativePath,
                                               ESFilePathType pathType);  // Must be ESFilePathTypeRelativeToAppSupportDir
};

#endif  // _ESFILE_HPP_
