//
//  ESFile.cpp
//
//  Created by Steven Pucci 27 Jul 2011
//  Copyright Emerald Sequoia LLC 2011. All rights reserved.
//

#include "ESFile.hpp"
#include "ESFilePvt.hpp"
#include "ESErrorReporter.hpp"

#include <strings.h>  // For bzero
#include <fcntl.h>
#include <sys/errno.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>

/*static*/ const char * 
ESFile::pathTypeString(ESFilePathType pathType) {
    switch(pathType) {
      case ESFilePathTypeRelativeToResourceDir:
        return "resource";
      case ESFilePathTypeRelativeToDocumentDir:
        return "document";
      case ESFilePathTypeRelativeToAppSupportDir:
        return "Application Support";
      case ESFilePathTypeRelativeToAppSupportThenResourceDir:
        return "Application Support then resource";
      default:
        return "unknown path type";
    }
}

/*static*/ unsigned 
ESFile::readSingleUnsignedFromFile(const char     *path,
                                   ESFilePathType pathType) {
    size_t fileSize;
    ESFileCloser *fileCloser;
    int fd = getFDPointingAtFile(path, pathType, false/* !missingOK*/, &fileSize, &fileCloser);
    if (fd < 0) {
        return 0;
    }
    if (fileSize < 4) {
	ESErrorReporter::logError("ESFile", "Binary file %s (%s) unexpectedly small: %zd < 4", path, pathTypeString(pathType), fileSize);
        if (fileCloser) {
            fileCloser->closeAndDie();
        }
        return 0;
    }
    unsigned int storage;
    size_t bytesRead = read(fd, &storage, sizeof(unsigned int));
    if (bytesRead != sizeof(unsigned int)) {
	ESErrorReporter::logError("ESFile", "Error reading binary file %s (%s): %s", path, pathTypeString(pathType), strerror(errno));
        if (fileCloser) {
            fileCloser->closeAndDie();
        }
        return 0;
    }
    if (fileCloser) {
        // ESErrorReporter::logInfo("ESFile::readSingleUnsignedFromFile", "Closing fd %d", fd);
        fileCloser->closeAndDie();
    }
    return storage;
}

/*static*/ int 
ESFile::getFDPointingAtFile(const char     *path,
                            ESFilePathType pathType,
                            bool           missingOK,
                            size_t         *fileSizeReturn,
                            ESFileCloser   **fileCloser) {
    int fd;
    switch(pathType) {
      case ESFilePathTypeRelativeToResourceDir:
        return getFDPointingAtResource(path, missingOK, fileSizeReturn, fileCloser);
      case ESFilePathTypeRelativeToDocumentDir:
        fd = getFDPointingAtFileInDirectory(path, documentDirectory(), missingOK, fileSizeReturn);
        *fileCloser = new ESStaticFileCloser(fd);
        return fd;
      case ESFilePathTypeRelativeToAppSupportDir:
        fd = getFDPointingAtFileInDirectory(path, appSupportDirectory(), missingOK, fileSizeReturn);
        *fileCloser = new ESStaticFileCloser(fd);
        return fd;
      case ESFilePathTypeRelativeToAppSupportThenResourceDir:
        fd = getFDPointingAtFileInDirectory(path, appSupportDirectory(), true/*missingOK*/, fileSizeReturn);
        if (fd >= 0) {
            *fileCloser = new ESStaticFileCloser(fd);
            return fd;
        }
        return getFDPointingAtResource(path, missingOK, fileSizeReturn, fileCloser);
      default:
	ESErrorReporter::logError("ESFile", "Improper path type %d (%s) for path %s", (int)pathType, pathTypeString(pathType), path);
        *fileSizeReturn = 0;
        *fileCloser = NULL;
        return -1;
    }
}

/*static*/ char *
ESFile::getFileContentsInMallocdArray(const char     *path,
                                      ESFilePathType pathType,
                                      bool           missingOK,
                                      size_t         *fileSizeReturn) {
    size_t fileSize;
    ESFileCloser *fileCloser;
    int fd = ESFile::getFDPointingAtFile(path, pathType, missingOK, &fileSize, &fileCloser);
    if (fd < 0) {
        return NULL;
    }
    if (fileSize <= 0) {
        ESErrorReporter::logError("ESFile", "Apparently empty %s file [%s]", ESFile::pathTypeString(pathType), path);
        return NULL;
    }
    // ESErrorReporter::logInfo("ESFile", "Successful open of file at %s: length %d, fd %d\n", path, fileSize, fd);
    char *bytes = (char *)malloc(fileSize + 1);
    //ESTime::noteTimeAtPhase(ESUtil::stringWithFormat("File read start: reading %s %s", ESFile::pathTypeString(pathType), path));
    size_t bytesRead = read(fd, bytes, fileSize);
    if (fileCloser) {
        // ESErrorReporter::logInfo("ESFile::getFileContentsInMallocdArray", "Closing fd %d", fd);
        fileCloser->closeAndDie();
    }
    if (bytesRead != fileSize) {
        ESErrorReporter::logError("ESFile", "Failed to read entire file [%s]", ESFile::pathTypeString(pathType), path);
        free(bytes);
        *fileSizeReturn = 0;
        return NULL;
    }
    //ESTime::noteTimeAtPhase(ESUtil::stringWithFormat("File read finish: read %d bytes", fileSize));
    *fileSizeReturn = bytesRead;
    return bytes;
}

/*static*/ bool 
ESFile::writeArrayToFile(const void *buf,
                         size_t     buflen,
                         const char *path,
                         ESFilePathType pathType) {  // Must be ESFilePathTypeRelativeToDocumentDir or ESFilePathRelativeToAppSupportDir
    ESAssert(pathType == ESFilePathTypeRelativeToDocumentDir ||
             pathType == ESFilePathTypeRelativeToAppSupportDir);  // Can't write to resource directory
    int fd = open(((pathType == ESFilePathTypeRelativeToDocumentDir ? ESFile::documentDirectory() : ESFile::appSupportDirectory()) + "/" + path).c_str(),
                  O_CREAT|O_TRUNC|O_RDWR, 0777);
    if (fd < 0) {
	ESErrorReporter::logError("ESFile", "Error opening binary %s file %s [%s] for write: %s", ESFile::pathTypeString(pathType), path, (ESFile::documentDirectory() + path).c_str(), strerror(errno));
        ESAssert(false);
        return false;
    }
    bool success = true;
    ssize_t bytesWritten = write(fd, buf, buflen);
    // ESErrorReporter::logInfo("ESFile::writeArrayToFile", "Closing fd %d", fd);
    close(fd);
    if (bytesWritten != buflen) {
	ESErrorReporter::logError("ESFile", "Failed to write entire %s file %s: %s", ESFile::pathTypeString(pathType), path, strerror(errno));
        success = false;
        unlink((ESFile::documentDirectory() + path).c_str());  // Unlink it so we don't have a partially written file confusing future reads
    }
    return success;
}

/*static*/ int 
ESFile::getFDPointingAtFileInDirectory(const char        *path,
                                       const std::string &dir,
                                       bool              missingOK,
                                       size_t            *fileSizeReturn) {
    int fd = open((dir + "/" + path).c_str(), O_RDONLY);
    if (fd < 0) {
        if (!missingOK) {
            ESErrorReporter::logError("ESFile", "Error opening binary file %s: %s", path, strerror(errno));
        }
        *fileSizeReturn = 0;
	return -1;
    }
    struct stat buf;
    int st = fstat(fd, &buf);
    if (st != 0) {
	ESErrorReporter::logError("ESFile", "Error running fstat on file %s: %s", path, strerror(errno));
        close(fd);
        *fileSizeReturn = 0;
        return -1;
    }
    *fileSizeReturn = (size_t)buf.st_size;
    return fd;
}

/*static*/ std::string
ESFile::ensureDirectoryExists(const std::string &fullPath) {
    struct stat buf;
    int st = stat(fullPath.c_str(), &buf);
    if (st == 0) {
        // Here we know it exists, make sure it's a directory
        if ((buf.st_mode & S_IFDIR) == 0) {
            ESErrorReporter::logError("ESFile", "File exists and is not a directory: %s", fullPath.c_str());
            ESAssert(false);
            return NULL;  // Ensure a crash downstream rather than mucking things up further
        }
    } else {
        if (errno != ENOENT) {
            // Something other than "not existent"
            ESErrorReporter::logError("ESFile", "Error running stat on supposed directory %s: %s", fullPath.c_str(), strerror(errno));
            ESAssert(false);
            return NULL;  // Ensure a crash downstream rather than mucking things up further
        }
        st = mkdir(fullPath.c_str(), 0777);
        if (st != 0) {
            ESErrorReporter::logError("ESFile", "Error creating directory %s: %s", fullPath.c_str(), strerror(errno));
            ESAssert(false);
            return NULL;  // Ensure a crash downstream rather than mucking things up further
        }
    }
    return fullPath;
}

/*static*/ bool 
ESFile::fileExistsAtPath(const char *path) {
    struct stat statBuffer;
    return stat(path, &statBuffer) == 0;
}

/*static*/ std::string
ESFile::ensureDirectoryExistsInAppSupportDirectoryWithRelativePath(const char *relativePath) {
    return ensureDirectoryExists(appSupportDirectory() + "/" + relativePath);
}

/*static*/ void 
ESFile::removeFileAtPath(const char *path) {
    int st = unlink(path);
    ESErrorReporter::checkAndLogSystemError("ESFile::removeFileAtPath", st, path);
}

/*static*/ void 
ESFile::removeFileAtPathIfPresent(const char *path) {
    int st = unlink(path);
    if (st != 0 && st != ENOENT) {
        ESErrorReporter::checkAndLogSystemError("ESFile::removeFileAtPath", st, path);
    }
}

/*static*/ void 
ESFile::setFileNotBackedUp(const char     *relativePath,
                           ESFilePathType pathType) {  // Must be ESFilePathTypeRelativeToAppSupportDir or ESFilePathTypeRelativeToDocumentDir
    ESAssert(pathType == ESFilePathTypeRelativeToAppSupportDir ||
             pathType == ESFilePathTypeRelativeToDocumentDir);  // Can't write to resource directory
    if (pathType == ESFilePathTypeRelativeToDocumentDir) {
        setFileNotBackedUp((ESFile::documentDirectory() + "/" + relativePath).c_str());
    } else {
        setFileNotBackedUp((ESFile::appSupportDirectory() + "/" + relativePath).c_str());
    }
}

