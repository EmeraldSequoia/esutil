//
//  ESUtil.cpp
//
//  Created by Steve Pucci 12 Jan 2011
//  Copyright Emerald Sequoia LLC 2011. All rights reserved.
//

#include <errno.h>
#include <stdio.h>
#include <sys/resource.h>
#include <sys/time.h>

#include <list>

#include "ESUtil.hpp"
#include "ESLock.hpp"
#include "ESThread.hpp"
#include "ESErrorReporter.hpp"

// Class static variables
/*static*/ std::string ESUtil::_primaryBundleID = "";
/*static*/ std::string ESUtil::_deviceID = "";

// File static variables
static std::list<ESUtilMemoryWarningObserver *> *memoryWarningObservers;
static std::list<ESUtilSleepWakeObserver *> *sleepWakeObservers;
static std::list<ESUtilSignificantTimeChangeObserver *> *significantTimeChangeObservers;

static ESUtilNoterOfTimeAtPhase noterOfTimeAtPhase = NULL;

/*static*/ void
ESUtil::init() {
    initPlatformSpecific();
    ESAssert(sizeof(void*) == sizeof(ESPointerSizedInt));
}


/*static*/ void 
ESUtil::setPrimaryBundleID(const char *bundleID) {
    _primaryBundleID = bundleID;
}

/*static*/ void 
ESUtil::goingToSleep() {
    if (!sleepWakeObservers) {
        return;
    }
    // Call observers "safely" with respect to removing the observer we're on
    ESUtilSleepWakeObserver *observer;
    std::list<ESUtilSleepWakeObserver *>::iterator end = sleepWakeObservers->end();
    std::list<ESUtilSleepWakeObserver *>::iterator iter = sleepWakeObservers->begin();
    if (iter != end) {
        observer = *iter;
        iter++;
        while (iter != end) {
            observer->goingToSleep();  // Last one
            observer = *iter;
            iter++;
        }
        observer->goingToSleep();  // Last one
    }
}

 /*static*/ void 
ESUtil::wakingUp() {
    if (!sleepWakeObservers) {
        return;
    }
    // Call observers "safely" with respect to removing the observer we're on
    ESUtilSleepWakeObserver *observer;
    std::list<ESUtilSleepWakeObserver *>::iterator end = sleepWakeObservers->end();
    std::list<ESUtilSleepWakeObserver *>::iterator iter = sleepWakeObservers->begin();
    if (iter != end) {
        observer = *iter;
        iter++;
        while (iter != end) {
            observer->wakingUp();  // Last one
            observer = *iter;
            iter++;
        }
        observer->wakingUp();  // Last one
    }
}

 /*static*/ void 
ESUtil::enteringBackground() {
    if (!sleepWakeObservers) {
        return;
    }
    // Call observers "safely" with respect to removing the observer we're on
    ESUtilSleepWakeObserver *observer;
    std::list<ESUtilSleepWakeObserver *>::iterator end = sleepWakeObservers->end();
    std::list<ESUtilSleepWakeObserver *>::iterator iter = sleepWakeObservers->begin();
    if (iter != end) {
        observer = *iter;
        iter++;
        while (iter != end) {
            observer->enteringBackground();  // Last one
            observer = *iter;
            iter++;
        }
        observer->enteringBackground();  // Last one
    }
}

 /*static*/ void 
ESUtil::leavingBackground() {
    if (!sleepWakeObservers) {
        return;
    }
    // Call observers "safely" with respect to removing the observer we're on
    ESUtilSleepWakeObserver *observer;
    std::list<ESUtilSleepWakeObserver *>::iterator end = sleepWakeObservers->end();
    std::list<ESUtilSleepWakeObserver *>::iterator iter = sleepWakeObservers->begin();
    if (iter != end) {
        observer = *iter;
        iter++;
        while (iter != end) {
            observer->leavingBackground();  // Last one
            observer = *iter;
            iter++;
        }
        observer->leavingBackground();  // Last one
    }
}

/*static*/ void 
ESUtil::shutdown() {
    ESThread::shutdown();
}

 /*static*/ void 
ESUtil::significantTimeChange() {
    if (!significantTimeChangeObservers) {
        return;
    }
    // Call observers "safely" with respect to removing the observer we're on
    ESUtilSignificantTimeChangeObserver *observer;
    std::list<ESUtilSignificantTimeChangeObserver *>::iterator end = significantTimeChangeObservers->end();
    std::list<ESUtilSignificantTimeChangeObserver *>::iterator iter = significantTimeChangeObservers->begin();
    if (iter != end) {
        observer = *iter;
        iter++;
        while (iter != end) {
            observer->significantTimeChange();  // Last one
            observer = *iter;
            iter++;
        }
        observer->significantTimeChange();  // Last one
    }
}

 /*static*/ void 
ESUtil::didReceiveMemoryWarning() {
    if (!memoryWarningObservers) {
        return;
    }
    // Call observers "safely" with respect to removing the observer we're on
    ESUtilMemoryWarningObserver *observer;
    std::list<ESUtilMemoryWarningObserver *>::iterator end = memoryWarningObservers->end();
    std::list<ESUtilMemoryWarningObserver *>::iterator iter = memoryWarningObservers->begin();
    if (iter != end) {
        observer = *iter;
        iter++;
        while (iter != end) {
            observer->memoryWarning();  // Last one
            observer = *iter;
            iter++;
        }
        observer->memoryWarning();  // Last one
    }
}

// 
// Methods called by general clients
// 

/*static*/ void 
ESUtil::registerSleepWakeObserver(ESUtilSleepWakeObserver *observer) {
    if (!sleepWakeObservers) {
        sleepWakeObservers = new std::list<ESUtilSleepWakeObserver *>;
    }
    sleepWakeObservers->push_back(observer);
}

/*static*/ void 
ESUtil::registerSignificantTimeChangeObserver(ESUtilSignificantTimeChangeObserver *observer) {
    if (!significantTimeChangeObservers) {
        significantTimeChangeObservers = new std::list<ESUtilSignificantTimeChangeObserver *>;
    }
    significantTimeChangeObservers->push_back(observer);
}

/*static*/ void 
ESUtil::unregisterSleepWakeObserver(ESUtilSleepWakeObserver *observer) {
    sleepWakeObservers->remove(observer);
}

/*static*/ void 
ESUtil::unregisterSignificantTimeChangeObserver(ESUtilSignificantTimeChangeObserver *observer) {
    significantTimeChangeObservers->remove(observer);
}

/*static*/ void 
ESUtil::registerMemoryWarningObserver(ESUtilMemoryWarningObserver *observer) {
    if (!memoryWarningObservers) {
        memoryWarningObservers = new std::list<ESUtilMemoryWarningObserver *>;
    }
    memoryWarningObservers->push_back(observer);
}

 /*static*/ void 
ESUtil::unregisterMemoryWarningObserver(ESUtilMemoryWarningObserver *observer) {
    memoryWarningObservers->remove(observer);
}

/*static*/ void 
ESUtil::registerNoteTimeAtPhaseCapability(ESUtilNoterOfTimeAtPhase aNoterOfTimeAtPhase) {
    noterOfTimeAtPhase = aNoterOfTimeAtPhase;
}

static ESLock *printfLock;
static double lastTimeNoted = -1;
static double firstTimeNoted = 0;

/*static*/ void 
ESUtil::noteTimeAtPhase(const char  *phaseName) {
    if (noterOfTimeAtPhase) {
        (*noterOfTimeAtPhase)(phaseName);
    } else {  // If ESTime isn't built in, provide a rudimentary alternative using system time via gettimeofday():
        if (!printfLock) {
            printfLock = new ESLock;
        }
        printfLock->lock();
        struct timeval tv;
        gettimeofday(&tv, NULL);
        double t = tv.tv_sec + tv.tv_usec / 1000000.;
        if (lastTimeNoted < 0) {
            printf("Phase time      Total Description\n");
            printf("%10.4f %10.4f: ", 0.0, 0.0);
            firstTimeNoted = t;
        } else {
            printf("%10.4f %10.4f: ", t - lastTimeNoted, t - firstTimeNoted);
        }
        printf("%s\n", phaseName);
        lastTimeNoted = t;
        printfLock->unlock();
    }
}

//  The implementation of the following stringWithFormat* methods is
//  based on the idea behind http://www.senzee5.com/2006/05/c-formatting-stdstring.html
/*static*/ std::string 
ESUtil::stringWithFormatV(const char *fmt, va_list args) {
    if (!fmt) {
        return "";
    }
    int  result = -1;
#define LOCAL_STACK_LENGTH 128
    int  length = LOCAL_STACK_LENGTH;
    char localBuf[LOCAL_STACK_LENGTH];
    char *buffer = localBuf;
    while (result < 0) {
        if (buffer != localBuf) {
            delete [] buffer;
        }
        if (length != LOCAL_STACK_LENGTH) {
            buffer = new char [length];
        }
        result = vsnprintf(buffer, length, fmt, args);
        if (result < 0) {
            break;
        }
        if (result >= length) {
            result = -1;
        }
        length *= 2;
    }
    std::string s(buffer);
    if (buffer != localBuf) {
        delete [] buffer;
    }
    return s;
}

/*static*/ std::string 
ESUtil::stringWithFormat(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    std::string s = stringWithFormatV(fmt, args);
    va_end(args);
    return s;
}

#if 0  // XCode 5.1 compiler complains about undefined behavior with va_start and reference types
/*static*/ std::string 
ESUtil::stringWithFormat(const std::string &fmt, ...) {
    va_list args;
    va_start(args, fmt);
    std::string s = stringWithFormatV(fmt.c_str(), args);
    va_end(args);
    return s;
}
#endif

/*static*/ bool 
ESUtil::firstRunOfThisVersion() {
    ESAssert(false);  // Put some code here
    return false;
}

/*static*/ std::string 
ESUtil::angleString(double angleInRadians) {
    if (isnan(angleInRadians)) {
        return "            NAN (\"\")                                                                              ";
    }
    int sign = angleInRadians < 0 ? -1 : 1;
    double absAngle = fabs(angleInRadians);
    int degrees = sign * (int)(((long long int)floor(absAngle * 180/M_PI)));
    int arcMinutes = (int)(((long long int)floor(absAngle * 180/M_PI * 60)) % 60);
    int arcSeconds = (int)(((long long int)floor(absAngle * 180/M_PI * 3600)) % 60);
    int arcSecondHundredths = (int)(((long long int)floor(absAngle * 180/M_PI * 360000)) % 100);
    int hours = sign * (int)(((long long int)floor(absAngle * 12/M_PI)));
    int          minutes  = (int)(((long long int)floor(absAngle * 12/M_PI * 60)) % 60);
    int minuteThousandths = (int)(((long long int)floor(absAngle * 12/M_PI * 60000)) % 1000);
    int          seconds = (int)(((long long int)floor(absAngle * 12/M_PI * 3600)) % 60);
    int secondHundredths = (int)(((long long int)floor(absAngle * 12/M_PI * 360000)) % 100);
    return stringWithFormat("%32.24fr %16.8fd %5do%02d'%02d.%02d\" %16.8fh %5dh%02dm%02d.%02ds %5dh%02d.%03dm",
                            angleInRadians,
                            angleInRadians * 180 / M_PI,
                            degrees,
                            arcMinutes,
                            arcSeconds,
                            arcSecondHundredths,
                            angleInRadians * 12 / M_PI,
                            hours,
                            minutes,
                            seconds,
                            secondHundredths,
                            hours,
                            minutes,
                            minuteThousandths);
}

/*static*/ void 
ESUtil::printAngle(double     angleInRadians,
                   const char *description) {
    ESErrorReporter::logInfo("Angle", "%s  %s\n", angleString(angleInRadians).c_str(), description);
}

/*static*/ std::string 
ESUtil::removeLastValidUTFCharacter(const std::string& str) {
    const char *s = str.c_str();
    int lastAfterTruncation = (int) (str.length() - 2);  // len-1 is last char, len-2 is prev
    while (lastAfterTruncation >= 0 && (s[lastAfterTruncation] & 0xC0) == 0x80) {  // Check for unicode continuation character
        lastAfterTruncation--;
    }
    return str.substr(0, lastAfterTruncation + 1);  // +1 because s[lastAfterTruncation] is the last one we want to keep, so 0 thru lastAfterTruncation
}

// TODO: Find the definitions of symbols here in iOS include files, and remove following #if 
#if ES_ANDROID
static void showFDInfo( int fd )
{
   char buf[256];
 
   char path[256];
   sprintf( path, "/proc/self/fd/%d", fd );
 
   memset( &buf[0], 0, 256 );
   ssize_t s = readlink( path, &buf[0], 256 );
   if ( s == -1 )
   {
       ESErrorReporter::logInfo("showFDInfo", "%fd (%s) not available", fd, path);
       return;
   }
 
   int fd_flags = fcntl( fd, F_GETFD ); 
   if ( fd_flags == -1 ) return;
 
   int fl_flags = fcntl( fd, F_GETFL ); 
   if ( fl_flags == -1 ) return;
 
   std::string attrs;
   if ( fd_flags & FD_CLOEXEC )  attrs += "cloexec ";
 
   // file status
   if ( fl_flags & O_APPEND   )  attrs += "append ";
   if ( fl_flags & O_NONBLOCK )  attrs += "nonblock ";
 
   // acc mode
   if ( fl_flags & O_RDONLY   )  attrs += "read-only ";
   if ( fl_flags & O_RDWR     )  attrs += "read-write ";
   if ( fl_flags & O_WRONLY   )  attrs += "write-only ";
 
   if ( fl_flags & O_DSYNC    )  attrs += "dsync ";
//   if ( fl_flags & O_RSYNC    )  attrs += "rsync ";
   if ( fl_flags & O_SYNC     )  attrs += "sync ";
 
   struct flock fl;
   fl.l_type = F_WRLCK;
   fl.l_whence = 0;
   fl.l_start = 0;
   fl.l_len = 0;
   fcntl( fd, F_GETLK, &fl );
   if ( fl.l_type != F_UNLCK )
   {
      if ( fl.l_type == F_WRLCK )
         attrs += "write-locked ";
      else
         attrs += "read-locked ";
      attrs += ESUtil::stringWithFormat("(pid: %d)", fl.l_pid);
   }
   ESErrorReporter::logInfo("showFDInfo", "%d (%s): %s", fd, path, attrs.c_str());
}
#endif  // ES_ANDROID

/*static*/void 
ESUtil::showFDInfo()
{
// TODO: Find the definitions of symbols here in iOS include files, and remove following #if 
#if ES_ANDROID
    struct rusage rusage_info;
    int st = getrusage(RUSAGE_THREAD, &rusage_info);
    if (st < 0) {
        ESErrorReporter::checkAndLogSystemError("ESUtil::showFDInfo", errno, "getrusage error");
        return;
    }

    struct rlimit rlimit_info;
    st = getrlimit(RLIMIT_NOFILE, &rlimit_info);
    if (st < 0) {
        ESErrorReporter::checkAndLogSystemError("ESUtil::showFDInfo", errno, "getrlimit error");
        return;
    }
    int numHandles = rlimit_info.rlim_cur;
 
    for ( int i = 0; i < numHandles; i++ ) {
        int fd_flags = fcntl( i, F_GETFD ); 
        if ( fd_flags == -1 ) continue;
 
        ::showFDInfo( i );
    }
#endif  // ES_ANDROID
}
