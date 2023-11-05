//
//  ESOfflineLogger.cpp
//
//  Created by Steve Pucci 10 Nov 2017
//  Copyright Emerald Sequoia LLC 2017. All rights reserved.
//

#include "ESOfflineLogger.hpp"

#include "ESErrorReporter.hpp"
#include "ESThread.hpp"
#include "ESFile.hpp"
#include "ESUtil.hpp"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h> 
#include <unistd.h>

static ESSimpleWorkerThread *workerThread = NULL;

static const char *OFFLINE_LOG_NAME = "OfflineLog.txt";
static const char *PREV_OFFLINE_LOG_NAME = "OfflineLog-previous.txt";
static std::string offlineLogPathName;
static const off_t MAX_LOG_SIZE (25000000);

// Once each session, check the size of the offline log and rename if it's too big.
static void checkRename(void *object, void *param) {
    struct stat st;
    if(stat(offlineLogPathName.c_str(), &st) == 0) {
        if (st.st_size > MAX_LOG_SIZE) {
            // File is large, do rename.
            std::string prevName = ESFile::documentDirectory() + "/" + PREV_OFFLINE_LOG_NAME;
            if (unlink(prevName.c_str()) != 0) {
                if (errno != ENOENT) {
                    ESErrorReporter::checkAndLogSystemError("OfflineLogger init", errno, 
                                                            ESUtil::stringWithFormat("Trouble removing prev file '%s'", 
                                                                                     prevName.c_str()).c_str());
                }   
            }
            if (rename(offlineLogPathName.c_str(), prevName.c_str()) != 0) {
                ESErrorReporter::checkAndLogSystemError("OfflineLogger init", errno, 
                                                        ESUtil::stringWithFormat("Trouble renaming '%s' to '%s'", 
                                                                                 offlineLogPathName.c_str(), 
                                                                                 prevName.c_str()).c_str());
            }
            ESErrorReporter::logInfo("OfflineLogger init", ESUtil::stringWithFormat("Renamed log to '%s'", 
                                                                                    prevName.c_str()).c_str());
        } else {
            ESErrorReporter::logInfo("OfflineLogger init", "Offline log exists but small enough, ignoring");
        }
    } else {  // trouble reading file.  Check why
        if (errno != ENOENT) {
            ESErrorReporter::checkAndLogSystemError("OfflineLogger init", errno, 
                                                    ESUtil::stringWithFormat("Trouble reading log file '%s'", 
                                                                             offlineLogPathName.c_str()).c_str());
        } else {
            ESErrorReporter::logInfo("OfflineLogger init", "Offline log doesn't exist");
        }
    }
}

#define ENABLE_OFFLINE_LOGGER 0

/*static*/ void 
ESOfflineLogger::initialize() {
#if ENABLE_OFFLINE_LOGGER
    ESAssert(!workerThread);
    offlineLogPathName = ESFile::documentDirectory() + "/" + OFFLINE_LOG_NAME;
    // ESErrorReporter::logInfo("ESOfflineLogger::initialize", "log path name %s", offlineLogPathName.c_str());
    workerThread = new ESSimpleWorkerThread("ESOfflineLogger", ESChildThreadExitsOnlyByParentRequest);
    workerThread->start();
    workerThread->callInThread(checkRename, NULL, NULL);
#endif
}

static void doLog(const char *text) {
    ESAssert(workerThread);
    ESAssert(workerThread->inThisThread());
    int fd = open(offlineLogPathName.c_str(), O_WRONLY | O_APPEND | O_CREAT, 0600);
    if (fd < 0) {
        ESErrorReporter::logError("ESOfflineLogger::doLog", "Couldn't append to offline log file '%s'", offlineLogPathName.c_str());
        ESErrorReporter::checkAndLogSystemError("ESOfflineLogger::doLog", errno, "errno from file append");
    }
    // ESErrorReporter::logInfo("ESOfflineLogger::doLog", "opened file for append, fd %d", fd);
    size_t length = strlen(text);
    ssize_t written = write(fd, text, length);
    if (written != length) {
        ESErrorReporter::logError("ESOfflineLogger::doLog", "Wrote %d instead of expected %d bytes to fd %d", (int)written, (int)length, fd);
        ESErrorReporter::checkAndLogSystemError("ESOfflineLogger::doLog", errno, "errno from file write");
    }
    if (close(fd) != 0) {
        ESErrorReporter::logError("ESOfflineLogger::doLog", "Close of fd %d failed", fd);
        ESErrorReporter::checkAndLogSystemError("ESOfflineLogger::doLog", errno, "errno from file close");
    }
}

static void logGlue(void *object, void *param) {
    char *txt = static_cast<char *>(object);
    doLog(txt);

    // Delete the copy that was created in ESOfflineLogger::log().
    delete [] txt;
}

/*static*/ void 
ESOfflineLogger::log(const std::string &txt) {
#if ENABLE_OFFLINE_LOGGER
    ESAssert(workerThread);

    struct timeval tv;
    gettimeofday(&tv, NULL);

    struct tm tm;
    localtime_r(&tv.tv_sec, &tm);

    // Make a copy of the string.  We have to anyway, and this lets us record the time in the
    // calling thread before sending to the subthread (and add a trailing newline, if necessary).

    // First create the format string we're going to use to create 'buf', by running strftime
    // on formatFormat:
    static const char formatFormat[] = "%m-%d %H:%M:%S.%%03u";
    char format[64];
    strftime(format, sizeof format, formatFormat, &tm);

    // Now 'format' contains the string we can append the milliseconds to:
    size_t buflen = txt.length() + 32;  // conservative
    char *buf = new char[buflen + 1];  // allocated on heap, because we're passing to another thread
    snprintf(buf, buflen, format, tv.tv_usec / 1000);

    strcat(buf, " ");
    strcat(buf, txt.c_str());
    int len = strlen(buf);
    if (buf[len - 1] != '\n') {
        buf[len] = '\n';
        buf[len + 1] = '\0';
    }
    workerThread->callInThread(logGlue, buf, NULL);
#endif
}

static void doWriteScreenCaptureFile(const char *filename, const char *textToWrite) {
    ESAssert(workerThread);
    ESAssert(workerThread->inThisThread());
    int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0600);
    if (fd < 0) {
        ESErrorReporter::logError("ESOfflineLogger::doWriteScreenCaptureFile", "Couldn't create screen capture request file '%s'", filename);
        ESErrorReporter::checkAndLogSystemError("ESOfflineLogger::doWriteScreenCaptureFile", errno, "errno from file create");
    }
    // ESErrorReporter::logInfo("ESOfflineLogger::doWriteScreenCaptureFile", "opened file for create, fd %d", fd);
    size_t length = strlen(textToWrite);
    ssize_t written = write(fd, textToWrite, length);
    if (written != length) {
        ESErrorReporter::logError("ESOfflineLogger::doWriteScreenCaptureFile", "Wrote %d instead of expected %d bytes to fd %d", (int)written, (int)length, fd);
        ESErrorReporter::checkAndLogSystemError("ESOfflineLogger::doWriteScreenCaptureFile", errno, "errno from file write");
    }
    if (close(fd) != 0) {
        ESErrorReporter::logError("ESOfflineLogger::doWriteScreenCaptureFile", "Close of fd %d failed", fd);
        ESErrorReporter::checkAndLogSystemError("ESOfflineLogger::doWriteScreenCaptureFile", errno, "errno from file close");
    }
    ESErrorReporter::logInfo("ESOfflineLogger::doWriteScreenCaptureFile", "Wrote '%s' into capture request file %s", textToWrite, filename);
}

static void writeScreenCaptureGlue(void *object, void *param) {
    const char *filename = static_cast<const char *>(object);
    const char *textToWrite = static_cast<const char *>(param);

    doWriteScreenCaptureFile(filename, textToWrite);

    delete [] filename;
    delete [] textToWrite;
}

/*static*/ void 
ESOfflineLogger::writeScreenCaptureRequestFile(const std::string &filename,
                                               const std::string &textToWrite) {
#if ENABLE_OFFLINE_LOGGER
    ESAssert(workerThread);

    size_t filenamebuflen = filename.length() + 32;  // conservative
    char *filenamebuf = new char[filenamebuflen + 1];  // allocated on heap, because we're passing to another thread
    strcpy(filenamebuf, filename.c_str());

    size_t textbuflen = textToWrite.length() + 32;  // conservative
    char *textbuf = new char[textbuflen + 1];  // allocated on heap, because we're passing to another thread
    strcpy(textbuf, textToWrite.c_str());

    workerThread->callInThread(writeScreenCaptureGlue, filenamebuf, textbuf);
#else
    ESAssert(false);
#endif
}
