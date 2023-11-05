LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := esutil
LOCAL_CFLAGS    := -Werror -DES_ANDROID
LOCAL_SRC_FILES := \
ESJNIDefs.cpp \
ESJNI.cpp \
../../src/ESErrorReporter.cpp \
../../src/ESErrorReporter_android.cpp \
../../src/ESFile.cpp \
../../src/ESFile_android.cpp \
../../src/ESFileArray.cpp \
../../src/ESInterThreadObserver.cpp \
../../src/ESLock_pthreads.cpp \
../../src/ESNameResolver.cpp \
../../src/ESNetwork.cpp \
../../src/ESNetwork_android.cpp \
../../src/ESThread.cpp \
../../src/ESThread_android.cpp \
../../src/ESThread_pthreads.cpp \
../../src/ESTrace.cpp \
../../src/ESUserPrefs_android.cpp \
../../src/ESUserString.cpp \
../../src/ESUserString_android.cpp \
../../src/ESUtil.cpp \
../../src/ESUtil_android.cpp \

# Leave a blank line before this one
LOCAL_LDLIBS    := -llog
LOCAL_C_INCLUDES := $(LOCAL_PATH)/../../src

include $(BUILD_STATIC_LIBRARY)
