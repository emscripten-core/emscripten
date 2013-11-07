
// Emscripten shims for JVM support

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "emjvm.h"

// invoke interface

jint DestroyJavaVM(JavaVM*) { exit(55); return NULL; }
jint AttachCurrentThread(JavaVM*, JNIEnv**, void*) { exit(66); return NULL; }
jint DetachCurrentThread(JavaVM*) { exit(77); return NULL; }
jint GetEnv(JavaVM*, void** env, jint); // forward def
jint AttachCurrentThreadAsDaemon(JavaVM*, JNIEnv**, void*) { exit(88); return NULL; }

// env - some of these are externs that are implemented in JS

jobject EMJVM_NewGlobalRef(JNIEnv*, jobject obj) {
  return obj; // XXX no global refcounting, we just keep global singletons alive etc.
}

extern "C" {
  extern jstring emjvm_newString(const jchar *chars, jsize len);
}
jstring EMJVM_NewString(JNIEnv*, const jchar* unicodeChars, jsize len) {
  return emjvm_newString(unicodeChars, len);
}

extern "C" {
  extern jclass emjvm_getObjectClass(JNIEnv*, jobject obj);
}

extern "C" {
  extern jmethodID emjvm_getMethodID(jclass, const char*, const char*);
}
extern jmethodID EMJVM_GetMethodID(JNIEnv*, jclass clazz, const char* methodName, const char* sig) {
  return emjvm_getMethodID(clazz, methodName, sig);
}

extern "C" {
  extern jobject emjvm_callObjectMethod(JNIEnv*, jobject, jmethodID, va_list);

  extern void emjvm_deleteLocalRef(JNIEnv*, jobject);

  jsize emjvm_getArrayLength(JNIEnv*, jarray);

  void emjvm_getByteArrayRegion(JNIEnv*, jbyteArray, jsize, jsize, jbyte*);

  jclass emjvm_findClass(JNIEnv*, const char*);

  jmethodID emjvm_getStaticMethodID(JNIEnv*, jclass, const char*, const char*);

  jobject emjvm_callStaticObjectMethod(JNIEnv*, jclass, jmethodID, va_list);

  jboolean emjvm_callBooleanMethod(JNIEnv*, jobject, jmethodID, va_list);
  jboolean emjvm_callStaticBooleanMethod(JNIEnv*, jclass, jmethodID, va_list);

  void emjvm_callVoidMethod(JNIEnv*, jobject, jmethodID, va_list);

  jint emjvm_callIntMethod(JNIEnv*, jobject, jmethodID, va_list);

  const char* emjvm_getStringUTFChars(JNIEnv*, jstring, jboolean*);
  jsize emjvm_getStringUTFLength(JNIEnv*, jstring);
  void emjvm_releaseStringUTFChars(JNIEnv*, jstring, const char*);
}

// JVM

struct EmJVM {
  JavaVM jvm;
  JNIInvokeInterface jvmFunctions;

  JNIEnv env;
  JNINativeInterface envFunctions;

  EmJVM() {
    // jvm
    jvm.functions = &jvmFunctions;

    jvmFunctions.DestroyJavaVM = DestroyJavaVM;
    jvmFunctions.AttachCurrentThread = AttachCurrentThread;
    jvmFunctions.DetachCurrentThread = DetachCurrentThread;
    jvmFunctions.GetEnv = GetEnv;
    jvmFunctions.AttachCurrentThreadAsDaemon = AttachCurrentThreadAsDaemon;

    // env
    memset(&envFunctions, 0, sizeof(envFunctions));

    env.functions = &envFunctions;

    envFunctions.NewGlobalRef = EMJVM_NewGlobalRef;
    envFunctions.NewString = EMJVM_NewString;
    envFunctions.GetObjectClass = emjvm_getObjectClass;
    envFunctions.GetMethodID = EMJVM_GetMethodID;
    envFunctions.CallObjectMethodV = emjvm_callObjectMethod;
    envFunctions.DeleteLocalRef = emjvm_deleteLocalRef;
    envFunctions.GetArrayLength = emjvm_getArrayLength;
    envFunctions.GetByteArrayRegion = emjvm_getByteArrayRegion;
    envFunctions.FindClass = emjvm_findClass;
    envFunctions.GetStaticMethodID = emjvm_getStaticMethodID;
    envFunctions.CallStaticObjectMethodV = emjvm_callStaticObjectMethod;
    envFunctions.CallBooleanMethodV = emjvm_callBooleanMethod;
    envFunctions.CallStaticBooleanMethodV = emjvm_callStaticBooleanMethod;
    envFunctions.CallVoidMethodV = emjvm_callVoidMethod;
    envFunctions.CallIntMethodV = emjvm_callIntMethod;
    envFunctions.GetStringUTFChars = emjvm_getStringUTFChars;
    envFunctions.GetStringUTFLength = emjvm_getStringUTFLength;
    envFunctions.ReleaseStringUTFChars = emjvm_releaseStringUTFChars;
  }
};

EmJVM emJVM;

// implement forward defs

jint GetEnv(JavaVM*, void** env, jint) {
  *env = &emJVM.env;
  return 0;
}

// external access from JS

extern "C" {

JavaVM* emscripten_get_jvm() { return &emJVM.jvm; }

JNIEnv* emscripten_get_jni_env() { return &emJVM.env; }

}

