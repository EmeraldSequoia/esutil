//
//  ESErrorReporter.hpp
//
//  Created by Steve Pucci 08 Jan 2011
//  Copyright Emerald Sequoia LLC 2011. All rights reserved.
//

#ifndef _ESERRORREPORTER_HPP_
#define _ESERRORREPORTER_HPP_

#include "ESPlatform.h"

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <netdb.h>

#include <string>

class ESUserString;

#define ESErr ESErrorReporter

// Interface (abstract class) which can display errors to the user, and which knows
// which view is currently visible.
class ESUserErrorReporter {
  public:
    virtual                 ~ESUserErrorReporter() {}
    // Display 'msg' to the user, iff 'viewName' is currently visible.  Returns
    // whether the message was delivered.
    virtual bool            displayErrorIfVisible(const std::string &viewName,
                                                  const std::string &msg) = 0;
};

class ESErrorReporter {
  public:
    static void             logError(const char *where,  // simple module or class name (e.g., "ESUtil" or "ESNTPDriver")
                                     const char *format,
                                     ...);
    static void             logInfo(const char *where,  // simple module or class name (e.g., "ESUtil" or "ESNTPDriver")
                                    const char *format,
                                    ...);
    static void             logErrorWithCode(const char *where,  // simple module or class name (e.g., "ESUtil" or "ESNTPDriver")
                                             int        st,
                                             const char * (*stringForCodeFn)(int st),
                                             const char *msg = NULL);
    static void             logErrorWithCode(const char *where,  // simple module or class name (e.g., "ESUtil" or "ESNTPDriver")
                                             int        st,
#if ES_ANDROID
                                             char       *(*stringForCodeFn)(int st, char *buf, size_t buflen),
#else
                                             int        (*stringForCodeFn)(int st, char *buf, size_t buflen),
#endif
                                             const char *msg = NULL);
    static void             checkAndLogSystemError(const char *where,  // simple module or class name (e.g., "ESUtil" or "ESNTPDriver")
                                                   int        st,
                                                   const char *msg = NULL);
    static void             checkAndLogGAIError(const char *where,  // simple module or class name (e.g., "ESUtil" or "ESNTPDriver")
                                                int        st,
                                                const char *msg = NULL);

    static void             reportErrorToUser(ESUserString &fmt,
                                              ...);

    // Register a mechanism for displaying errors to the user.
    static void             registerUserErrorReporter(ESUserErrorReporter *reporter);
    static void             unregisterUserErrorReporter(ESUserErrorReporter *reporter);

    // Report a message to the user when the view associated with 'viewName' (e.g., a watch so named) is visible
    // on the screen, for context to the user.  If the given view is visible now, displays the message immediately.
    // If the given view is *not* visible now, it will be displayed the next time the given view is visible.
    // The 'msgKey' is provided so that only the last of a series of messages (perhaps the same, perhaps distinct) 
    // will be displayed (so if the condition triggering the message has changed or been removed, the message will
    // not be displayed in that case.
    static void             reportErrorToUserWhenVisible(const std::string &viewName,
                                                         const std::string &msgKey,
                                                         const ESUserString &msg);

    // Displays all queued messages for the given view.  Typically called when a watch face becomes visible, for example.
    // Messages are only returned once; the internal structure is cleared for the given view after each call.
    static void             displayAndClearMessagesForView(const std::string &viewName);

    // Remove the message associated with 'msgKey' from the queue for 'viewName'.  Typically called if the condition
    // triggering the message has gone away.
    static void             removeMessageForView(const std::string &viewName,
                                                 const std::string &msgKey);

    // Creates an entry in an offline log file that can be retrieved later.  Only implemented on Android so far (is no-op on other platforms).
    static void             logOffline(const std::string &text);

    // Essentially private, but called only from a macro so needs to be public
    static void             _abortOnAssert(const char *cond,
                                           const char *file,
                                           int        lineno);
  private:
    static void             logErrorImpl(const char *where,  // simple module or class name (e.g., "ESUtil" or "ESNTPDriver")
                                         const char *msg);
    static void             logInfoImpl(const char *where,  // simple module or class name (e.g., "ESUtil" or "ESNTPDriver")
                                        const char *msg);

};

inline /*static*/ void 
ESErrorReporter::checkAndLogSystemError(const char *where,
                                        int        st,
                                        const char *msg) {
    if (st != 0) {
        logErrorWithCode(where, st, strerror_r, msg);
    }
}

inline /*static*/ void 
ESErrorReporter::checkAndLogGAIError(const char *where,
                                     int        st,
                                     const char *msg) {
    if (st != 0) {
        logErrorWithCode(where, st, gai_strerror, msg);
    }
}

#ifdef NDEBUG
#define ESAssert(cond) {}
#else
#define ESAssert(cond) ((cond) ? (0) : (ESErrorReporter::_abortOnAssert(#cond, __FILE__, __LINE__), 0))
#endif

#endif // _ESERRORREPORTER_HPP_
