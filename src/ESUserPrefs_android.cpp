//
//  ESUserPrefs_android.cpp
//
//  Created by Steve Pucci 14 Feb 2011
//  Copyright Emerald Sequoia LLC 2011. All rights reserved.
//

#include "ESUserPrefs.hpp"
#include "ESUtil.hpp"
#include "ESJNIDefs.hpp"
#include "android_content_Context.h"

#include "ESErrorReporter.hpp"

#include <map>
#include <string>

#include <stdlib.h>

#include "jni.h"

static std::map<std::string, bool> *boolDefaultPrefs;
static std::map<std::string, int> *intDefaultPrefs;
static std::map<std::string, double> *doubleDefaultPrefs;
static std::map<std::string, std::string> *stringDefaultPrefs;

static ESJNI_android_content_SharedPreferences *sharedPreferencesObj = NULL;
static ESJNI_android_content_SharedPreferences *sharedPreferencesObject(JNIEnv *env) {
    if (sharedPreferencesObj == NULL) {
        ESJNI_java_lang_String sharedPrefsName = ESJNI_com_emeraldsequoia_esutil_ESUserPrefs::SHARED_PREFS_NAMEField(env);
        sharedPreferencesObj = new ESJNI_android_content_SharedPreferences(ESUtil::theAndroidContextWrapper().getSharedPreferences(env, sharedPrefsName, android_content_Context_MODE_PRIVATE).getRetainedCopy(env));
        sharedPrefsName.DeleteLocalRef(env);
    }
    return sharedPreferencesObj;
}

// These methods are used to obtain the value of a pref.
/*static*/ bool 
ESUserPrefs::boolPref(const char *name) {
    bool defaultValue = boolDefaultPrefs ? (*boolDefaultPrefs)[name] : false;
    JNIEnv *jniEnv = ESUtil::jniEnv();
    ESJNI_android_content_SharedPreferences *sharedPrefs = sharedPreferencesObject(jniEnv);
    ESJNI_java_lang_String prefName(name);
    prefName.toJObject(jniEnv);
    ESJBool val = sharedPrefs->getBoolean(jniEnv, prefName, ESJBool((defaultValue ? JNI_TRUE : JNI_FALSE)));
    prefName.DeleteLocalRef(jniEnv);
    return val.toBool();
}

/*static*/ int
ESUserPrefs::intPref(const char *name) {
    int defaultValue = intDefaultPrefs ? (*intDefaultPrefs)[name] : false;
    JNIEnv *jniEnv = ESUtil::jniEnv();
    ESJNI_android_content_SharedPreferences *sharedPrefs = sharedPreferencesObject(jniEnv);
    ESJNI_java_lang_String prefName(name);
    prefName.toJObject(jniEnv);
    jlong val = sharedPrefs->getLong(jniEnv, prefName, defaultValue);
    prefName.DeleteLocalRef(jniEnv);
    return (int) val;
}

static jlong reinterpretDoubleAsLong(jdouble jd) {
    return *reinterpret_cast<jlong*>(&jd);
}

static jdouble reinterpretLongAsDouble(jlong jl) {
    return *reinterpret_cast<jdouble*>(&jl);
}

/*static*/ double 
ESUserPrefs::doublePref(const char *name) {
    jdouble defaultValue = doubleDefaultPrefs ? (*doubleDefaultPrefs)[name] : 0.0;
    // "Hey Johnny Android -- can you think of a reason why someone would want to store a double
    //    in a user preference value?  We've already got int, boolean, float, string, long, ..."
    // "No, Jimmmy, I can't.  And if we can't think of one, it must not be important enough to
    //    add the API, no matter how simple it is to implement."
    // "Wait, I just thought of one -- these are mobile devices, right?  The longitude is a double."
    // "Sorry, Jimmy.  My mind is made up.  Don't confuse me with facts."
    JNIEnv *jniEnv = ESUtil::jniEnv();
    ESJNI_android_content_SharedPreferences *sharedPrefs = sharedPreferencesObject(jniEnv);
    ESJNI_java_lang_String prefName(name);
    prefName.toJObject(jniEnv);
    jlong valueAsLong = sharedPrefs->getLong(jniEnv, prefName, reinterpretDoubleAsLong(defaultValue));
    prefName.DeleteLocalRef(jniEnv);
    return reinterpretLongAsDouble(valueAsLong);
}
 
/*static*/ std::string 
ESUserPrefs::stringPref(const char *name) {
    std::string defaultValue = stringDefaultPrefs ? (*stringDefaultPrefs)[name] : "";
    JNIEnv *jniEnv = ESUtil::jniEnv();
    ESJNI_android_content_SharedPreferences *sharedPrefs = sharedPreferencesObject(jniEnv);
    ESJNI_java_lang_String prefName(name);
    prefName.toJObject(jniEnv);
    ESJNI_java_lang_String val = sharedPrefs->getString(jniEnv, prefName, defaultValue);
    prefName.DeleteLocalRef(jniEnv);
    std::string str = val.toESString(jniEnv);
    val.DeleteLocalRef(jniEnv);
    return str;
}

// These methods are used to initialize the defaults (logically)
// before reading the prefs from the last session.  note that through
// C++ overloading the names are all the same, unlike the "get"
// methods which cannot overload based on return type
/*static*/ void 
ESUserPrefs::initDefaultPref(const char *name,
                             bool       value) {
    if (!boolDefaultPrefs) {
        boolDefaultPrefs = new std::map<std::string, bool>;
    }
    (*boolDefaultPrefs)[name] = value;
}

/*static*/ void 
ESUserPrefs::initDefaultPref(const char *name,
                             int        value) {
    if (!intDefaultPrefs) {
        intDefaultPrefs = new std::map<std::string, int>;
    }
    (*intDefaultPrefs)[name] = value;
}

/*static*/ void 
ESUserPrefs::initDefaultPref(const char *name,
                             double     value) {
    if (!doubleDefaultPrefs) {
        doubleDefaultPrefs = new std::map<std::string, double>;
    }
    (*doubleDefaultPrefs)[name] = value;
}

/*static*/ void 
ESUserPrefs::initDefaultPref(const char *name,
                             const char *value) {
    if (!stringDefaultPrefs) {
        stringDefaultPrefs = new std::map<std::string, std::string>;
    }
    (*stringDefaultPrefs)[name] = value;
}

// These methods are used to change the value during a session
/*static*/ void 
ESUserPrefs::setPref(const char *name,
                     bool       value) {
    JNIEnv *jniEnv = ESUtil::jniEnv();
    ESJNI_android_content_SharedPreferences *sharedPrefs = sharedPreferencesObject(jniEnv);
    ESJNI_android_content_SharedPreferences_Editor editor = sharedPrefs->edit(jniEnv);
    ESAssert(!editor.isNull());
    ESJNI_java_lang_String prefName(name);
    prefName.toJObject(jniEnv);
    ESJNI_android_content_SharedPreferences_Editor return_editor = editor.putBoolean(jniEnv, prefName, ESJBool(value ? JNI_TRUE : JNI_FALSE));
    return_editor.DeleteLocalRef(jniEnv);
    prefName.DeleteLocalRef(jniEnv);
    editor.apply(jniEnv);
    editor.DeleteLocalRef(jniEnv);
}

/*static*/ void 
ESUserPrefs::setPref(const char *name,
                     int        value) {
    JNIEnv *jniEnv = ESUtil::jniEnv();
    ESJNI_android_content_SharedPreferences *sharedPrefs = sharedPreferencesObject(jniEnv);
    ESJNI_android_content_SharedPreferences_Editor editor = sharedPrefs->edit(jniEnv);
    ESAssert(!editor.isNull());
    ESJNI_java_lang_String prefName(name);
    prefName.toJObject(jniEnv);
    ESJNI_android_content_SharedPreferences_Editor return_editor = editor.putLong(jniEnv, prefName, value);
    return_editor.DeleteLocalRef(jniEnv);
    prefName.DeleteLocalRef(jniEnv);
    editor.apply(jniEnv);
    editor.DeleteLocalRef(jniEnv);
}

/*static*/ void 
ESUserPrefs::setPref(const char *name,
                     double     value) {
    JNIEnv *jniEnv = ESUtil::jniEnv();
    ESJNI_android_content_SharedPreferences *sharedPrefs = sharedPreferencesObject(jniEnv);
    ESJNI_android_content_SharedPreferences_Editor editor = sharedPrefs->edit(jniEnv);
    ESAssert(!editor.isNull());
    ESJNI_java_lang_String prefName(name);
    prefName.toJObject(jniEnv);
    ESJNI_android_content_SharedPreferences_Editor return_editor = editor.putLong(jniEnv, prefName, reinterpretDoubleAsLong(value));
    return_editor.DeleteLocalRef(jniEnv);
    prefName.DeleteLocalRef(jniEnv);
    editor.apply(jniEnv);
    editor.DeleteLocalRef(jniEnv);
}

/*static*/ void 
ESUserPrefs::setPref(const char *name,
                     const char *value) {
    JNIEnv *jniEnv = ESUtil::jniEnv();
    ESJNI_android_content_SharedPreferences *sharedPrefs = sharedPreferencesObject(jniEnv);
    ESJNI_android_content_SharedPreferences_Editor editor = sharedPrefs->edit(jniEnv);
    ESAssert(!editor.isNull());
    ESJNI_java_lang_String prefName(name);
    prefName.toJObject(jniEnv);
    ESJNI_java_lang_String prefValue(value);
    prefValue.toJObject(jniEnv);
    ESJNI_android_content_SharedPreferences_Editor return_editor = editor.putString(jniEnv, prefName, prefValue);
    return_editor.DeleteLocalRef(jniEnv);
    prefName.DeleteLocalRef(jniEnv);
    prefValue.DeleteLocalRef(jniEnv);
    editor.apply(jniEnv);
    editor.DeleteLocalRef(jniEnv);
}

