//
//  ESUtil.hpp
//
//  Created by Steve Pucci 12 Jan 2011
//  Copyright Emerald Sequoia LLC 2011. All rights reserved.
//

#ifndef _ESUTIL_HPP_
#define _ESUTIL_HPP_

#include <math.h>

#include "ESPlatform.h"
#include "ESThreadLocalStorage.hpp"

#include <string>

#if ES_ANDROID
#include "jni.h"
#include "ESJNIDefs.hpp"
#endif

class ESUtilSleepWakeObserver {
  public:
    virtual                 ~ESUtilSleepWakeObserver() {}
    virtual void            goingToSleep() = 0;
    virtual void            wakingUp() = 0;
    virtual void            enteringBackground() = 0;
    virtual void            leavingBackground() = 0;
};

class ESUtilSignificantTimeChangeObserver {
  public:
    virtual                 ~ESUtilSignificantTimeChangeObserver() {}
    virtual void            significantTimeChange() = 0;
};

class ESUtilMemoryWarningObserver {
  public:
    virtual                 ~ESUtilMemoryWarningObserver() {}
    virtual void            memoryWarning() = 0;
};

// A global/static/class-static method to implement noteTimeAtPhase
typedef void (*ESUtilNoterOfTimeAtPhase)(const char *);

typedef void (*ESBGTaskExpirationHandler)(void);
typedef unsigned long ESBackgroundTaskID;

class ESUtil {
  public:
    // Methods to be called by the "app delegate"
    static void             init();  // Should be called before anything else in the ESUtil library
    static void             goingToSleep();
    static void             wakingUp();
    static void             significantTimeChange();
    static void             enteringBackground();
    static void             leavingBackground();
    static void             didReceiveMemoryWarning();
    static void             shutdown();

    /** Set the primary bundle id.  If set, this overrides, e.g., [NSBundle mainBundle] when searching for resources.
     *  Currently only used for screensaver bundles. */
    static void             setPrimaryBundleID(const char *bundleID);
    static std::string      primaryBundleID() { return _primaryBundleID; }

    // Methods called by general clients
    static void             registerMemoryWarningObserver(ESUtilMemoryWarningObserver *observer);
    static void             registerSleepWakeObserver(ESUtilSleepWakeObserver *observer);
    static void             registerSignificantTimeChangeObserver(ESUtilSignificantTimeChangeObserver *observer);
    static void             unregisterMemoryWarningObserver(ESUtilMemoryWarningObserver *observer);
    static void             unregisterSleepWakeObserver(ESUtilSleepWakeObserver *observer);
    static void             unregisterSignificantTimeChangeObserver(ESUtilSignificantTimeChangeObserver *observer);
    static void             registerNoteTimeAtPhaseCapability(ESUtilNoterOfTimeAtPhase noterOfTimeAtPhase);
    static ESBackgroundTaskID requestMoreTimeForBackgroundTask(ESBGTaskExpirationHandler expirationHandler);
    static void             declareBackgroundTaskFinished(ESBackgroundTaskID);

    // General utility methods
    static std::string      localeCountryCode(); // Returns "" if uninitialized, never NULL
    static bool             isTablet();
    static void             noteTimeAtPhase(const char  *phaseName);
    static void             noteTimeAtPhase(const std::string &phaseName) { noteTimeAtPhase(phaseName.c_str()); }
    static double           fmod(double arg1,
                                 double arg2)
    { return (arg1 - floor(arg1/arg2)*arg2); }
    static std::string      appVersion();
    static std::string      firstAppVersionRun();
    static std::string      previousAppVersionRun();
    static bool             firstRunOfThisVersion();
    static std::string      stackTrace();
    static std::string      appIdentifier();  /**< e.g., com.EmeraldSequoia.Observatory */
    static std::string      deviceID() { return _deviceID; }  // Unique per device + factory-reset
    static std::string      removeLastValidUTFCharacter(const std::string& str);

    // Following formatters shouldn't be used in performance-critical code without careful analysis
    static std::string      stringWithFormatV(const char *fmt, va_list args);
    static std::string      stringWithFormat(const char *fmt, ...);
#if 0  // XCode 5.1 doesn't like va_args with reference parameters...
    static std::string      stringWithFormat(const std::string &fmt, ...);
#endif

    static std::string      angleString(double angleInRadians);
    static void             printAngle(double     angleInRadians,
                                       const char *description);

    template<typename TYP>
    static TYP              min(TYP a, TYP b) { return a < b ? a : b; }
    template<typename TYP>
    static TYP              max(TYP a, TYP b) { return a > b ? a : b; }

    template<typename TYP>
    static TYP              min(TYP a, TYP b, TYP c) { return a < b ? (a < c ? a : c) : (b < c ? b : c); }
    template<typename TYP>
    static TYP              max(TYP a, TYP b, TYP c) { return a > b ? (a > c ? a : c) : (b > c ? b : c); }

    static double           unspecifiedNAN() { return nan("0"); }
    static bool             isUnspecifiedNAN(double n) {  // Note:  For most purposes, you can just say "isnan(n)" unless you are using multiple nan values
        if (isnan(n)) {
            double u = nan("0");
            return *((long long *)(&n)) == *((long long *)(&u));
        }
        return false;
    }
    static bool             nansEqual(double n1,
                                      double n2) {
        if (isnan(n1) && isnan(n2)) {
            return *((long long *)(&n1)) == *((long long *)(&n2));
        } else {
            return false;  // they're not both nans
        }
    }

    template<typename TYP>
    static bool             isOdd(TYP a) { return (a & 0x1) != 0; }
    template<typename TYP>
    static bool             isEven(TYP a) { return (a & 0x1) == 0; }

    static bool             isOdd(double a) { return isOdd((int)round(a)); }
    static bool             isOdd(float a) { return isOdd((int)round(a)); }
    static bool             isEven(double a) { return isOdd((int)round(a)); }
    static bool             isEven(float a) { return isOdd((int)round(a)); }


#if ES_ANDROID
    static JNIEnv           *jniEnv() { return *_jniEnv; }
    static JavaVM           *javaVM() { return _javaVM; }

    static void             setJNIEnvInNewThread(const std::string &threadName);
    static void             detachFromJavaThread();

    static ESJNI_android_content_ContextWrapper theAndroidContextWrapper();
    static ESJNI_android_content_res_AssetManager theAndroidAssetManager();

    static jobject          theMainThreadMessageHandler() { return _theMainThreadMessageHandler; }
    static void             initAndroid(JNIEnv  *jenv,
                                        jobject theContextWrapper,
                                        jobject mainThreadMessageHandler);
    static void             reinitAndroid(JNIEnv  *jniEnv,
                                          jobject theContextWrapper);
    static void             shutdownAndroid(JNIEnv *jenv);
#endif

    static void             showFDInfo();

  private:
    static void             initPlatformSpecific();

    static std::string      _primaryBundleID;
    static std::string      _deviceID;

#if ES_ANDROID
    static ESThreadLocalStoragePtr<JNIEnv> *_jniEnv;
    static JavaVM           *_javaVM;
    static jobject          _theMainThreadMessageHandler;
#endif    
};

#endif
