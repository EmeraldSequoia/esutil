//
//  ESFile_android.cpp
//
//  Created by Steve Pucci 06 Aug 2011
//  Copyright Emerald Sequoia LLC 2011. All rights reserved.
//

#include "ESFile.hpp"
#include "ESUtil.hpp"
#include "ESJNIDefs.hpp"
#include "ESErrorReporter.hpp"

#include <string>

#include <unistd.h>
#include <errno.h>

#include "jni.h"

static int
rawFDFromJavaFileDescriptor(const ESJNI_java_io_FileDescriptor &jfd,
                            off_t                              off,
                            const char                         *pathForErrorMsgs,
                            JNIEnv                             *jniEnv) {
    static bool initialized = false;
    static enum {useFD, useDescriptor} whichWay;
    if (!initialized) {
        if (ESJNI_java_io_FileDescriptor::fdFieldValid(jniEnv)) {
            // ESErrorReporter::logInfo("ESFile", "fd field is valid\n");
            whichWay = useFD;
        } else if (ESJNI_java_io_FileDescriptor::descriptorFieldValid(jniEnv)) {
            // ESErrorReporter::logInfo("ESFile", "descriptor field is valid\n");
            whichWay = useDescriptor;
        } else {
            ESAssert(false);
        }
        initialized = true;
    }
    int fd =
        (whichWay == useFD)
          ? jfd.fdField(jniEnv)
          : jfd.descriptorField(jniEnv);
    off_t st2 = lseek(fd, off, SEEK_CUR);
    if (st2 < 0) {
        ESErrorReporter::checkAndLogSystemError("ESFileArray", errno, ESUtil::stringWithFormat("Trouble seeking to position %d of file %s\n",
                                                                                               off, pathForErrorMsgs).c_str());
        close(fd);
        ESAssert(false);
        return -1;
    }
    return fd;
}

/*static*/ std::string 
ESFile::documentDirectory() {
    static std::string cacheDir;
    if (cacheDir.length() == 0) {
        JNIEnv *jniEnv = ESUtil::jniEnv();
        ESJNI_java_lang_String jstr = ESUtil::theAndroidContextWrapper().getFilesDir(jniEnv).getPath(jniEnv);
        cacheDir = jstr.toESString(jniEnv);
        jstr.DeleteLocalRef(jniEnv);
    }
    return cacheDir;
}

/*static*/ std::string 
ESFile::appSupportDirectory() {
    return documentDirectory();
}

class ESAndroidResourceFileCloser : public ESFileCloser {
  public:
                            ESAndroidResourceFileCloser(ESJNI_android_content_res_AssetFileDescriptor afd,
                                                        int fd, // For logging
                                                        JNIEnv *jniEnv)
    :   _afd(afd.getRetainedCopy(jniEnv)),
        _fd(fd)
    {}
    virtual void            closeAndDie() {
        JNIEnv *jniEnv = ESUtil::jniEnv();
        _afd.close(jniEnv);
        _afd.release(jniEnv);
        // ESErrorReporter::logInfo("ESAndroidResourceFileCloser::closeAndDie", "Closing Java wrapper for fd %d", _fd);
        delete this;
    }

  private:
    ESJNI_android_content_res_AssetFileDescriptor _afd;
    int                     _fd;
};

/*static*/ int 
ESFile::getFDPointingAtResource(const char   *resourcePath,
                                bool         missingOK,
                                size_t       *resourceSizeReturn,
                                ESFileCloser **fileCloser) {
    ESAssert(resourcePath);
    ESAssert(*resourcePath == '/');
    resourcePath++;  // Move past leading '/'
    std::string uncompressedPath = std::string(resourcePath) + ".png";  // We always use .png extensions to keep Android from compressing...
    JNIEnv *jniEnv = ESUtil::jniEnv();
    ESJNI_java_lang_String jstr = uncompressedPath;
    jstr.toJObject(jniEnv);  // Force creation of jstring here so we can delete its local ref later.
    ESJNI_android_content_res_AssetFileDescriptor afd = ESUtil::theAndroidAssetManager().openFd(jniEnv, jstr);  // for uncompressed files
    if (jniEnv->ExceptionCheck()) {
        jniEnv->ExceptionClear();
    }
    jstr.DeleteLocalRef(jniEnv);
    if (afd.isNull()) {
        if (!missingOK) {
            ESErrorReporter::logError("ESFile", "Asset manager failed to find resource: %s", resourcePath);
        }
        return -1;
    }
    long offset = afd.getStartOffset(jniEnv);
    long length = afd.getLength(jniEnv);
    ESJNI_java_io_FileDescriptor jfd = afd.getFileDescriptor(jniEnv);
    *resourceSizeReturn = (size_t)length;
    int fd = rawFDFromJavaFileDescriptor(jfd, offset, resourcePath, jniEnv);
    jfd.DeleteLocalRef(jniEnv);
    *fileCloser = new ESAndroidResourceFileCloser(afd, fd, jniEnv);
    afd.DeleteLocalRef(jniEnv);
    // ESErrorReporter::logInfo("ESFile::getFDPointingAtResource", "returning fd %d", fd);
    return fd;
}

/*static*/ void 
ESFile::setFileNotBackedUp(const char *filename) {
    // Do nothing for now
}
