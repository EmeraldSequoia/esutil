//
//  ESUtil_android.cpp
//
//  Created by Steve Pucci 06 Feb 2011
//  Copyright Emerald Sequoia LLC 2011. All rights reserved.
//

#include "ESUtil.hpp"
#include "ESThreadLocalStorage.hpp"

#include <jni.h>

#include <assert.h>

/*static*/ ESThreadLocalStoragePtr<JNIEnv> *ESUtil::_jniEnv;
/*static*/ JavaVM           *ESUtil::_javaVM;
/*static*/ jobject          ESUtil::_theMainThreadMessageHandler;

static ESJNI_android_content_ContextWrapper theContextWrapper;
static ESJNI_android_content_res_AssetManager theAssetManager;
static ESJNI_java_util_Locale defaultLocale;

static std::string versionName;

/*static*/ void 
ESUtil::initPlatformSpecific() {
    assert(_javaVM);  // Call initAndroid() first...
    //JNIEnv *jniEnv = *_jniEnv;
}

/*static*/ void 
ESUtil::setJNIEnvInNewThread(const std::string &threadName) {
    // Initialize JNIEnv (good only for this thread)
    JavaVMAttachArgs attachArgs;
    attachArgs.version = JNI_VERSION_1_2;
    attachArgs.name = threadName.c_str();
    attachArgs.group = NULL;
    JNIEnv *env;
    _javaVM->AttachCurrentThread(&env, &attachArgs);
    *_jniEnv = env;
}

/*static*/ void
ESUtil::detachFromJavaThread() {
    _javaVM->DetachCurrentThread();
}

/*static*/ std::string 
ESUtil::localeCountryCode() { // Returns "" if uninitialized, never NULL
    JNIEnv *jniEnv = *_jniEnv;
    assert(jniEnv);  // Make sure ESUtil::init is called first
    ESJNI_java_util_Locale defaultLocale = ESJNI_java_util_Locale::getDefault(jniEnv);
    ESJNI_java_lang_String jstr = defaultLocale.getCountry(jniEnv);
    return jstr.toESString(jniEnv);
}

/*static*/ void 
ESUtil::initAndroid(JNIEnv  *jniEnv,
                    jobject contextWrapper,
                    jobject mainThreadMessageHandler) {
    // ESErrorReporter::logInfo("ESUtil", "initAndroid");
    ESJNI_initAndRetainAll(jniEnv);  // Gotta do this before calling glue code on anything
    // ESErrorReporter::logInfo("ESUtil", "Finished initializing JNI wrappers");

    theContextWrapper = ESJNI_android_content_ContextWrapper(contextWrapper).getRetainedCopy(jniEnv);
    theAssetManager = theContextWrapper.getAssets(jniEnv).getRetainedCopy(jniEnv);
    _theMainThreadMessageHandler = jniEnv->NewGlobalRef(mainThreadMessageHandler);

    // Initialize JavaVM static (good for all threads)
    int st = jniEnv->GetJavaVM(&_javaVM);
    assert(st == 0);

    _jniEnv = new ESThreadLocalStoragePtr<JNIEnv>;
    *_jniEnv = jniEnv;

    ESJNI_android_content_pm_PackageManager pm = theContextWrapper.getPackageManager(jniEnv);
    ESJNI_java_lang_String packageNameJstr = theContextWrapper.getPackageName(jniEnv);
    ESJNI_android_content_pm_PackageInfo pi = pm.getPackageInfo(jniEnv, packageNameJstr, 0);
    pm.DeleteLocalRef(jniEnv);
    packageNameJstr.DeleteLocalRef(jniEnv);
    ESJNI_java_lang_String versionNameJstr = pi.versionNameField(jniEnv);
    versionName = versionNameJstr.toESString(jniEnv);
    versionNameJstr.DeleteLocalRef(jniEnv);

    ESJNI_java_lang_String jDeviceIDName = ESJNI_android_provider_Settings_Secure::ANDROID_IDField(jniEnv);
    ESJNI_android_content_ContentResolver contentResolver = theContextWrapper.getContentResolver(jniEnv);
    ESJNI_java_lang_String jDeviceID = ESJNI_android_provider_Settings_Secure::getString(jniEnv, contentResolver, jDeviceIDName);
    jDeviceIDName.DeleteLocalRef(jniEnv);
    contentResolver.DeleteLocalRef(jniEnv);
    _deviceID = jDeviceID.toESString(jniEnv);
    jDeviceID.DeleteLocalRef(jniEnv);

    // ESErrorReporter::logInfo("initAndroid", "Starting up version %s for device id %s", versionName.c_str(), deviceID().c_str());
    ESErrorReporter::logInfo("initAndroid", "Starting up version %s", versionName.c_str());
}

/*static*/ void 
ESUtil::reinitAndroid(JNIEnv  *jniEnv,
                      jobject contextWrapper) {
    ESAssert(theContextWrapper.toJObject());
    theContextWrapper.release(jniEnv);
    theContextWrapper = ESJNI_android_content_ContextWrapper(contextWrapper).getRetainedCopy(jniEnv);
}

/*static*/ void
ESUtil::shutdownAndroid(JNIEnv *jniEnv) {
    ESAssert(false);  // This mechanism doesn't fully work yet, and will leave things in an inconsistent state.

    // NOTE: The ordering of calls within this function is important!
    
    ESErrorReporter::logInfo("ESUtil", "shutdownAndroid entry");
    
    // Notify all clients that we're going away.  This should shut down
    // all client threads if clients do their job right in their dtors
    // and notification callbacks.
    ESUtil::shutdown();

    // Now do local cleanup.  Nothing outside here should still be alive.
    jniEnv->DeleteGlobalRef(_theMainThreadMessageHandler);
    _theMainThreadMessageHandler = NULL;

    theContextWrapper.release(jniEnv);
    theAssetManager.release(jniEnv);

    _javaVM = NULL;

    delete _jniEnv;

    ESJNI_shutdownAll(jniEnv);
    ESErrorReporter::logInfo("ESUtil", "shutdownAndroid complete");
}

/*static*/ ESJNI_android_content_ContextWrapper
ESUtil::theAndroidContextWrapper() {
    assert(_javaVM);  // call initAndroid() first
    return theContextWrapper;
}

/*static*/ ESJNI_android_content_res_AssetManager 
ESUtil::theAndroidAssetManager() {
    assert(_javaVM);  // call initAndroid() first
    return theAssetManager;
}

/*static*/ bool
ESUtil::isTablet() {
    // TODO(spucci): Return the right answer here
    return false;
}

/*static*/ ESBackgroundTaskID 
ESUtil::requestMoreTimeForBackgroundTask(ESBGTaskExpirationHandler expirationHandler) {
    // TODO(spucci): See if Android wants us to call something here
    return 0;
}

/*static*/ void 
ESUtil::declareBackgroundTaskFinished(ESBackgroundTaskID taskID) {
    // TODO(spucci): See if Android wants us to call something here
}

/*static*/ std::string
ESUtil::appVersion() {
    return versionName;
}
