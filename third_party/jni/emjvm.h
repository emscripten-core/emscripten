
#include "jni.h"

extern "C" {
  extern JavaVM* emscripten_get_jvm();
  extern JNIEnv* emscripten_get_jni_env();
}

