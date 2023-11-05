//
//  ESJNI.hpp
//
//  Created by Steve Pucci 08 Jun 2011
//  Copyright Emerald Sequoia LLC 2011. All rights reserved.
//

#ifndef _ESJNI_HPP_
#define _ESJNI_HPP_

#include <jni.h>

#include <string>

// An ESJNI handle glue class (one which begins with ESJNI) is in many respects a "smart
// pointer" to a Java reference.  There are two flavors of ESJNI handles:  Handles whose
// underlying pointers are "retained" (converted into Java GlobalRefs) and pointers which
// are not (and thus are invalid once we return to the Java main loop).

// Note that the client of these classes must take care about the lifetime of retained
// objects, by understanding what is actually happening internally.  In particular, the
// "retainedness" is associated with the actual underlying pointer, not the glue-handle.
// Thus a copy of a glue-handle object which has been retained also points at the same
// retained object, and it is illegal to call retain() on *either* glue-handle object
// after that point.  If it is desired to release() the underlying pointer, this action
// may be done from either object as well; the glue-handle objects are not distinguished
// nor do they have reference counting (reference counting is difficult because the dtor
// might be the last one out the door and it doesn't have access to the JNIEnv pointer
// without accessing thread-local storage).

// Note:  The intent of these classes is *NOT* to provide a safe comfortable environment
// for working with Java classes.  The client programmer must retain (no pun intented)
// some knowledge of the lifetime of the underlying objects.  The intent is to provide
// some "syntactic sugar" to avoid the clunky sequence of C calls associated with the
// traditional JNI syntax for calling back into Java.

#define ESJNI_LOG_EXCEPTIONS 1

#if ESJNI_LOG_EXCEPTIONS
# define ESJNI_CHECK_AND_LOG_EXCEPTION(jniEnv) ESJNI::checkAndLogException(jniEnv)
#else
# define ESJNI_CHECK_AND_LOG_EXCEPTION(jniEnv)
#endif

// Utility function class (really a namespace)
class ESJNI {
  public:
    static void             checkAndLogException(JNIEnv *jniEnv);
};

/*! The base class of all generated ES Java glue classes */
class ESJNIBase {
  public:
    void                    release(JNIEnv *jniEnv);  // retain must be implemented in derived classes due to return value
#ifndef NDEBUG
    virtual std::string     className() const = 0;  // Because all of this code is generated, we can do this easily...
#endif
    bool                    isNull() const { return _javaObject == NULL; }
    void                    DeleteLocalRef(JNIEnv *jniEnv);
    jobject                 toJObject() const { return _javaObject; }
  protected:
                            ESJNIBase();
                            ESJNIBase(jobject jobj);
                            ESJNIBase(jobject jobj,
                                      bool    retained);
                            ESJNIBase(const ESJNIBase &other);
    jobject                 _javaObject;
    bool                    _retained;
};

/*! A glue class between bool and jboolean */
class ESJBool {
  public:
                            ESJBool(bool b,
                                    bool isBool) : _jb(b ? JNI_TRUE : JNI_FALSE) {}
                            ESJBool(jboolean jb) : _jb(jb) {}
    bool                    toBool() { return _jb ? true : false; }
    jboolean                toJBool() { return _jb; }
                            operator jboolean() { return _jb; }
  private:
    jboolean                _jb;
};

class ESJNI_java_lang_CharSequence;

/*! A glue class between std::string and jstring */
class ESJNI_java_lang_String {
  public:
                            ESJNI_java_lang_String();
                            ~ESJNI_java_lang_String();
                            ESJNI_java_lang_String(jobject js);  // We presume the object is a string
                            ESJNI_java_lang_String(jstring js);
                            ESJNI_java_lang_String(const std::string &str);
                            ESJNI_java_lang_String(const char *str);
                            ESJNI_java_lang_String(const ESJNI_java_lang_String &other);

    jobject                 toJObject(JNIEnv *jniEnv);
    jstring                 toJString(JNIEnv *jniEnv);
    std::string             toESString(JNIEnv *jniEnv);
    std::string             convertAndConsume(JNIEnv *jniEnv); // Like DeleteLocalRef, this changes the underlying object, thus implicitly invalidating any other wrappers pointing to it.
    void                    retainJString(JNIEnv *jniEnv);
    void                    releaseJString(JNIEnv *jniEnv);
    void                    DeleteLocalRef(JNIEnv *jniEnv);  // Changes the underlying object, thus implicitly invaliding any other wrappers pointing to it.
    // size_t                  length();
    jsize                   jlength(JNIEnv *jniEnv); // length as in Java, which counts characters rather than bytes
    ESJNI_java_lang_String  &operator=(ESJNI_java_lang_String &other);
    ESJNI_java_lang_String  &operator=(const std::string &str);
    ESJNI_java_lang_String  &operator=(const char *str);
    ESJNI_java_lang_String  &operator=(jstring js);
    ESJNI_java_lang_String  &operator=(jobject js);

    ESJNI_java_lang_CharSequence toCharSequence(JNIEnv *jniEnv);

  private:
    jstring                 _javaString;
    std::string             _esString;
    bool                    _esStringInitialized;
    bool                    _javaStringRetained;
};

class ESJNI_ByteArray : public ESJNIBase {
  public:
                            ESJNI_ByteArray();
                            ESJNI_ByteArray(jbyteArray ja);  // We presume the object is a byte[]
                            ESJNI_ByteArray(const ESJNI_ByteArray &other);
                            ~ESJNI_ByteArray();

    // Creates a Java Byte[] array and wraps it.
    static ESJNI_ByteArray  create(JNIEnv *jniEnv, long size);

    std::string             className() const { return "ESJNI_ByteArray"; }
    ESJNI_ByteArray         getRetainedCopy(JNIEnv *jniEnv) const;

    // Returns an array that should be delete'd later; length gets the size of the array
    const unsigned char     *getCopyOfBytes(jint *length, JNIEnv *jniEnv) const;

    // Copy starts at offset of the array.  *length remains the size of the copied array.
    const unsigned char     *getCopyOfBytes(jint *length, jint offset, JNIEnv *jniEnv) const;
    
  private:
                            ESJNI_ByteArray(jbyteArray ja,
                                            bool       retained);
    jbyteArray              ja() const { return static_cast<jbyteArray>(toJObject()); }
};

#endif  // _ESJNI_HPP_
