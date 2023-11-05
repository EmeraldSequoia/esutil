//
//  ESThread_android.cpp
//
//  Created by Steve Pucci 06 Feb 2011
//  Copyright Emerald Sequoia LLC 2011. All rights reserved.
//

// No MainThread cleanup because the main thread never dies

#include "ESThread.hpp"
#include "ESUtil.hpp"
#include "ESErrorReporter.hpp"
#include "jni.h"

static jclass Message_class = NULL;
static jfieldID Message_arg1Field = NULL;
static jfieldID Message_arg2Field = NULL;
static jfieldID Message_whatField = NULL;
static jmethodID Message_obtainMethod = NULL;

static jmethodID Handler_sendMessageMethod = NULL;

// Implementation note:
// For Android passing messages to the main thread, we make use of the android.os.Message
// class to deliver messages to the main loop of the main thread.  However, in a 64-bit
// build, since Message has Java "int" fields (32-bit) and we need to supply three 64-bit
// pointer values, we need some additional storage outside of the Message field itself.
//
// So in callInThread, we "new" a 3-pointer struct, and pass the pointer to the struct
// (a 64-bit value) via arg1 and arg2 (32-bit each).  Then in dispatchMethodInThread, we
// reconstruct the struct, use the values, and then "delete" the struct.
//
// Strictly speaking, we don't need to do this in a 32-bit build.  But having the code work
// the same for both kinds of builds is advantageous when testing.

struct ESInterThreadMessage {
    ESInterThreadFn         function;
    void                    *object;
    void                    *parameter;
                            ESInterThreadMessage(ESInterThreadFn fn, void* obj, void *param)
    :   function(fn), object(obj), parameter(param) {
    }
};

void
ESMainThread::platformSpecificThreadInitialization() {
    // Do nothing.
    // TODO(spucci): Consider switching to a scheme in which we use sockets as when sending
    // messages to threads other than the main one, but with registration of the reading of
    // that socket in the Android main loop, as is done with iOS.  But don't get your hopes
    // up; the main loop is Java, after all. :)
}

void 
ESChildThread::platformSpecificThreadInitialization() {
    ESUtil::setJNIEnvInNewThread(name());  // Also attaches thread to Java
}

void 
ESChildThread::platformSpecificThreadCleanup() {
    ESUtil::detachFromJavaThread();
}

void 
ESChildThread::preInterThreadFunction() {
    JNIEnv *jniEnv = ESUtil::jniEnv();
    jniEnv->PushLocalFrame(512);  // 512 because that's what we get by default on Android, it seems
    ESJNI_CHECK_AND_LOG_EXCEPTION(jniEnv);
}

void 
ESChildThread::postInterThreadFunction() {
    JNIEnv *jniEnv = ESUtil::jniEnv();
    jniEnv->PopLocalFrame(NULL);
    ESJNI_CHECK_AND_LOG_EXCEPTION(jniEnv);
}

/*static*/ bool 
ESThread::osDirectInMainThread() {
    return true;  // Legal (though not ideal), because this is only to be used in ASSERTS and it always returns true in release mode anyway
}

void 
ESThread::platformSpecificInit() {
    JNIEnv *jniEnv = ESUtil::jniEnv();
    ESAssert(jniEnv);

    Message_class = (jclass)jniEnv->NewGlobalRef(jniEnv->FindClass("android/os/Message"));
    ESAssert(Message_class);

    Message_arg1Field = jniEnv->GetFieldID(Message_class, "arg1", "I");
    ESAssert(Message_arg1Field);
    Message_arg2Field = jniEnv->GetFieldID(Message_class, "arg2", "I");
    ESAssert(Message_arg2Field);
    Message_whatField = jniEnv->GetFieldID(Message_class, "what", "I");
    ESAssert(Message_whatField);

    Message_obtainMethod = jniEnv->GetStaticMethodID(Message_class, "obtain", "()Landroid/os/Message;");
    ESAssert(Message_obtainMethod);

    jclass Handler_class = jniEnv->FindClass("android/os/Handler");
    Handler_sendMessageMethod = jniEnv->GetMethodID(Handler_class, "sendMessage", "(Landroid/os/Message;)Z");
    ESAssert(Handler_sendMessageMethod);
}

#define ESLowBitsSlot Message_arg1Field
#define ESHighBitsSlot Message_arg2Field

void *convertHighLowJintToAddress(jint low_bits, jint high_bits) {
#if __SIZEOF_POINTER__ == 8
    return (void *)((uint64_t)low_bits + ((uint64_t)high_bits << 32));
#elif __SIZEOF_POINTER__ == 4
    ESAssert(high_bits == 0);
    return (void *)low_bits;
#else
#error Macro is apparently not defined here
    ESAssert(false);
    return NULL;
#endif

}

void convertAddressToJInt(void *addr, jint *low_bits, jint *high_bits) {
#if __SIZEOF_POINTER__ == 8
    *low_bits  = (jint)(((uint64_t)addr) & 0xffffffff);
    *high_bits = (jint)(((uint64_t)addr) >> 32);
#elif __SIZEOF_POINTER__ == 4
    *low_bits  = (jint)addr;
    *high_bits = 0;
#else
#error Macro is apparently not defined here
    ESAssert(false);
#endif
    ESAssert(addr == convertHighLowJintToAddress(*low_bits, *high_bits));
}

/*virtual*/ void 
ESMainThread::callInThread(ESInterThreadFn fn,
                           void            *object,
                           void            *param,
                           bool            forceUseSocket) {
    if (forceUseSocket) {
        // If we're going to call waitForAndProcessInterThreadMessages() on Android,
        // then we need to send to sockets, not to the Android main loop message handler.
        ESThread::callInThread(fn, object, param, true);
        return;
    }

    ESAssert(sizeof(int) >= sizeof(void*));  // See dispatchMethodInThread comment
    ESAssert(!inThisThread());  // otherwise don't go through this overhead; caller should check or just know
    JNIEnv *jniEnv = ESUtil::jniEnv();
    ESAssert(jniEnv);
    ESAssert(Message_class);
    ESAssert(Message_obtainMethod);
    jobject msg = jniEnv->CallStaticObjectMethod(Message_class, Message_obtainMethod);
    ESAssert(msg);
    ESInterThreadMessage *message = new ESInterThreadMessage(fn, object, param);

    ESAssert(sizeof(message) <= 2 * sizeof(int));

    jint addr_low_bits;
    jint addr_high_bits;
    convertAddressToJInt(message, &addr_low_bits, &addr_high_bits);
    ESAssert(message == convertHighLowJintToAddress(addr_low_bits, addr_high_bits));

    jniEnv->SetIntField(msg, ESLowBitsSlot, addr_low_bits);
    jniEnv->SetIntField(msg, ESHighBitsSlot, addr_high_bits);

    jobject msgHandler = ESUtil::theMainThreadMessageHandler();
    ESAssert(msgHandler);
    jboolean st = jniEnv->CallBooleanMethod(msgHandler, Handler_sendMessageMethod, msg);
    ESAssert(st);

    jniEnv->DeleteLocalRef(msg);
}

/*static*/ void 
ESMainThread::dispatchMethodInThread(JNIEnv  *jniEnv,
                                     jobject ignoredContextWrapper,
                                     jobject msg) {
    ESAssert(inMainThread());

    jint low_bits  = jniEnv->GetIntField(msg, ESLowBitsSlot);
    jint high_bits = jniEnv->GetIntField(msg, ESHighBitsSlot);

    const ESInterThreadMessage *message =
        reinterpret_cast<const ESInterThreadMessage *>(convertHighLowJintToAddress(low_bits, high_bits));

    ESAssert(message->function);
    (*message->function)(message->object, message->parameter);

    delete message;
}
