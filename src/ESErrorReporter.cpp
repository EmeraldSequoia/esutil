//
//  ESErrorReporter.cpp
//
//  Created by Steve Pucci 29 Jan 2011
//  Copyright Emerald Sequoia LLC 2011. All rights reserved.
//

#include "ESErrorReporter.hpp"
#include "ESFileArray.hpp"
#include "ESUserString.hpp"

#include <stdarg.h>

#include <map>
#include <string>

/*static*/ void 
ESErrorReporter::logErrorWithCode(const char *where,
                                  int        st,
                                  const char * (*stringForCodeFn)(int st),
                                  const char *msg) {
    if (msg) {
        logError(where, "%s: %s", msg, (*stringForCodeFn)(st));
    } else {
        logErrorImpl(where, (*stringForCodeFn)(st));
    }
}

/*static*/ void 
ESErrorReporter::logErrorWithCode(const char *where,
                                  int        st,
#if ES_ANDROID
                                  char       *(*stringForCodeFn)(int st, char *buf, size_t buflen),
#else
                                  int        (*stringForCodeFn)(int st, char *buf, size_t buflen),
#endif                                  
                                  const char *msg) {
    char buf[4096];
    (*stringForCodeFn)(st, buf, sizeof(buf));
    if (msg) {
        logError(where, "%s: %s", msg, buf);
    } else {
        logErrorImpl(where, buf);
    }
}

/*static*/ void 
ESErrorReporter::logError(const char *where,
                          const char *format,
                          ...) {
    va_list args;
    va_start(args, format);
    char buf[4096];
    int charsToBePrinted = vsnprintf(buf, sizeof(buf), format, args);
    va_end(args);
    logErrorImpl(where, buf);
    if (charsToBePrinted >= sizeof(buf)) {
        logErrorImpl("ESErrorReporter", "Previous error message was truncated");
    }
}


/*static*/ void 
ESErrorReporter::logInfo(const char *where,
                         const char *format,
                         ...) {
    va_list args;
    va_start(args, format);
    char buf[4096];
    int charsToBePrinted = vsnprintf(buf, sizeof(buf), format, args);
    va_end(args);
    logInfoImpl(where, buf);
    if (charsToBePrinted >= sizeof(buf)) {
        logInfoImpl("ESErrorReporter", "Previous error message was truncated");
    }
}

typedef std::map<std::string, std::string> ViewMessageQueue;
typedef std::map<std::string, ViewMessageQueue *> MessageQueueByView;

static MessageQueueByView *queuedMessages = NULL;
static ESUserErrorReporter *userErrorReporter = NULL;

static const char messageQueueFilePath[] = "message_queue.txt";

// Copy string into *bufptr, advancing bufptr past terminating NULL.
// 'string' must be nonempty and NULL terminated, and must contain no internal NULLs.
static void copyStringIntoBuffer(char **bufptr, const std::string &str) {
    size_t stringLength = str.length();
    ESAssert(stringLength > 0);
    ESAssert(strlen(str.c_str()) == stringLength);
    strcpy(*bufptr, str.c_str());
    *bufptr += stringLength + 1;
}

static const char *readStringAndAdvance(const char **bufptr, size_t *bufRemaining) {
    ESErrorReporter::logInfo("readStringAndAdvance", "start, bufRemaining %d", (int) *bufRemaining);
    if (*bufRemaining == 0) {
        ESErrorReporter::logInfo("readStringAndAdvance", "end, EOF");
        return NULL;  // EOF, OK at string boundary.
    }
    const char *returnString = *bufptr;
    while (**bufptr && *bufRemaining) {
        (*bufptr)++;
        (*bufRemaining)--;
    }
    if (!*bufRemaining) {
        // We ran out of buffer before the terminating NULL.
        ESAssert(false);  // In real life we might have a truncated file for some reason.
        return NULL;
    }
    (*bufptr)++;  // the terminating NULL.
    (*bufRemaining)--;
    ESErrorReporter::logInfo("readStringAndAdvance", "end, returning '%s', bufRemaining %d", returnString, (int) *bufRemaining);
    return returnString;
}

// MessageQueue errors are written very simply as an array of NULL-terminated
// strings (NOT a ESFileStringArray, which allows random access to the individual
// strings, but a ESFileArray<char> which we can read and write in one operation).
// The sequence of strings is
//   viewName[0]
//   viewName[0].key[0]
//   viewName[0].msg[0]
//   viewName[0].key[1]
//   viewName[0].msg[1]
//   viewName[0].key[2]
//   viewName[0].msg[2]
//   ...
//   viewName[0].msg[n-1]
//   ""   // empty string marking the end of a view
//   viewName[1]
//   viewName[1].key[0]
//   viewName[1].msg[0]
//   viewName[1].key[1]
//   viewName[1].msg[1]
//   ...  etc.
// This imposes two requirements:
// 1) No message key may be the empty string, since an empty string in that slot signifies the end of that view.
// 2) All strings must be modified UTF-8 (that is, they may not include NULL characters as part of a Unicode sequence).
static void writeUserErrorQueue() {
    if (queuedMessages) {
        // First walk of the queue, determining size of buffer (and file) necessary
        size_t sizeNeeded = 0;
        for (MessageQueueByView::iterator viewIter = queuedMessages->begin();
             viewIter != queuedMessages->end();
             viewIter++) {
            const std::string &viewName = viewIter->first;
            sizeNeeded += viewName.length() + 1;  // include NULL terminator
            ViewMessageQueue *viewMessageQueue = viewIter->second;
            for (ViewMessageQueue::iterator msgIter = viewMessageQueue->begin();
                 msgIter != viewMessageQueue->end();
                 msgIter++) {
                const std::string &msgKey = msgIter->first;
                const std::string &msg = msgIter->second;
                sizeNeeded += msgKey.length() + msg.length() + 2; // include NULL terminators
            }
            sizeNeeded++;  // Include empty string signifying end of view
        }

        ESFileArray<char> messageFile(NULL /*pathToRead*/, ESFilePathTypeRelativeToAppSupportDir, false /*readInAtStartup*/);
        messageFile.setupForWriteWithNumElements((int)sizeNeeded);
        char *bufptr = messageFile.writableArray();

        // Now walk the queue again, filling the buffer
        for (MessageQueueByView::iterator viewIter = queuedMessages->begin();
             viewIter != queuedMessages->end();
             viewIter++) {
            const std::string &viewName = viewIter->first;
            ESAssert(viewName.length() > 0);
            copyStringIntoBuffer(&bufptr, viewName);

            ViewMessageQueue *viewMessageQueue = viewIter->second;
            for (ViewMessageQueue::iterator msgIter = viewMessageQueue->begin();
                 msgIter != viewMessageQueue->end();
                 msgIter++) {
                const std::string &msgKey = msgIter->first;
                const std::string &msg = msgIter->second;
                ESAssert(msgKey.length() > 0);
                copyStringIntoBuffer(&bufptr, msgKey);
                ESAssert(msg.length() > 0);
                copyStringIntoBuffer(&bufptr, msg);
            }
            *bufptr++ = '\0';  // signify end of view
        }
        ESAssert((bufptr - messageFile.writableArray()) == sizeNeeded);

        // Note: No error checking.  We already log errors in the underlying ESFile code, and there's not much to do
        // here if there's an error except try again, and then when do we do that.  So we ignore the error.
        messageFile.writeToPath(messageQueueFilePath, ESFilePathTypeRelativeToAppSupportDir);
    }
}

static void readUserErrorQueue() {
    // ESErrorReporter::logInfo("readUserErrorQueue", "start");
    ESFileArray<char> messageFile(messageQueueFilePath, ESFilePathTypeRelativeToAppSupportDir, true /*readInAtStartup*/);
    const char *bufptr = messageFile.array();
    const size_t buflen = messageFile.bytesRead();
    size_t bufRemaining = buflen;
    if (bufptr) {
        queuedMessages = new MessageQueueByView;
        while (true) {
            const char *viewName = readStringAndAdvance(&bufptr, &bufRemaining);
            if (viewName) {
                ESAssert(strlen(viewName) > 0);
                // ESErrorReporter::logInfo("readUserErrorQueue", "Processing messages for view '%s'", viewName);
                ViewMessageQueue *viewMessageQueue = new ViewMessageQueue;
                (*queuedMessages)[viewName] = viewMessageQueue;
                while (true) {
                    const char *msgKey = readStringAndAdvance(&bufptr, &bufRemaining);
                    if (!msgKey) {
                        ESErrorReporter::logError("readUserErrorQueue", "no msgKey in queue for %s", viewName);
                        return;  // Error condition
                    }
                    if (!*msgKey) {
                        ESErrorReporter::logInfo("readUserErrorQueue", "Found empty msgKey, must be end of view");
                        break;  // Must be end of view
                    }
                    const char *msg = readStringAndAdvance(&bufptr, &bufRemaining);
                    if (msg) {
                        (*viewMessageQueue)[msgKey] = msg;
                    } else {
                        ESErrorReporter::logError("readUserErrorQueue", "Found empty msg for msgKey '%s'", msgKey);
                        return;  // Error condition
                    }
                }
            } else {
                ESErrorReporter::logInfo("readUserErrorQueue", "Found EOF (aka NULL view name) at end of all views");
                break;  // EOF here is expected at the end of all views.
            }
        }
    } else {
        // ESErrorReporter::logInfo("readUserErrorQueue", "no bufptr");
    }
}

/*static*/ void 
ESErrorReporter::reportErrorToUserWhenVisible(const std::string  &viewName,
                                              const std::string  &msgKey,
                                              const ESUserString &englishMsg) {
    ESErrorReporter::logInfo("reportErrorToUserWhenVisible", "view '%s', msgKey '%s', msg '%s'", 
                             viewName.c_str(), msgKey.c_str(), englishMsg.localizedValue().c_str());
    ESAssert(viewName.length() > 0);
    ESAssert(msgKey.length() > 0);
    if (!queuedMessages) {
        readUserErrorQueue();
    }
    std::string msg = englishMsg.localizedValue();
    ESAssert(msg.length() > 0);
    if (userErrorReporter && userErrorReporter->displayErrorIfVisible(viewName, msg)) {
        // Message delivered.  Also remove any prior messages in the queue.
        removeMessageForView(viewName, msgKey);
    } else {
        // Here we can't deliver it, so need to queue it.
        if (!queuedMessages) {
            queuedMessages = new MessageQueueByView;
        }
        MessageQueueByView::iterator it = queuedMessages->find(viewName);
        ViewMessageQueue *messages;
        if (it == queuedMessages->end()) {
            messages = new ViewMessageQueue;
            (*queuedMessages)[viewName] = messages;
        } else {
            messages = it->second;
        }
        (*messages)[msgKey] = msg;  // Overwrites if already present.
        writeUserErrorQueue();
    }
}

/*static*/ void
ESErrorReporter::displayAndClearMessagesForView(const std::string &viewName) {
    // ESErrorReporter::logInfo("displayAndClearMessagesForView", "view '%s', userErrorReporter %s", 
    //                          viewName.c_str(), userErrorReporter ? "present" : "not present");
    if (userErrorReporter && queuedMessages) {
        // ESErrorReporter::logInfo("displayAndClearMessagesForView", "... found queued messages map");
        MessageQueueByView::iterator it = queuedMessages->find(viewName);
        if (it != queuedMessages->end()) {
            // ESErrorReporter::logInfo("displayAndClearMessagesForView", "... ... found queued messages in this view");
            ViewMessageQueue *messages = it->second;
            ESAssert(messages);
            if (!messages->empty()) {
                // ESErrorReporter::logInfo("displayAndClearMessagesForView", 
                //                          "view '%s' has outstanding messages", viewName.c_str());
                bool allMessagesDeliveredSuccessfully = true;
                for (ViewMessageQueue::iterator it = messages->begin(); it != messages->end(); it++) {
                    if (!userErrorReporter->displayErrorIfVisible(viewName, it->second)) {
                        allMessagesDeliveredSuccessfully = false;
                    }
                }
                if (allMessagesDeliveredSuccessfully) {
                    // ESErrorReporter::logInfo("displayAndClearMessagesForView", 
                    //                          "clearing messages for view '%s'", viewName.c_str());
                    messages->clear();
                    writeUserErrorQueue();
                }
            }
        }
    }
}

/*static*/ void 
ESErrorReporter::removeMessageForView(const std::string &viewName,
                                      const std::string &msgKey) {
    if (queuedMessages) {
        MessageQueueByView::iterator iter = queuedMessages->find(viewName);
        if (iter != queuedMessages->end()) {
            ESAssert(iter->second);
            iter->second->erase(msgKey);
            writeUserErrorQueue();
        }
    }
}

/*static*/ void 
ESErrorReporter::registerUserErrorReporter(ESUserErrorReporter *reporter) {
    ESAssert(userErrorReporter == NULL);
    userErrorReporter = reporter;
    if (!queuedMessages) {
        readUserErrorQueue();
    }
}

/*static*/ void 
ESErrorReporter::unregisterUserErrorReporter(ESUserErrorReporter *reporter) {
    ESAssert(userErrorReporter != NULL);
    userErrorReporter = NULL;
}
