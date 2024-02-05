/*
 * Copyright 2017 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

// AL proc address retrieval

#include <string.h>
#include <stdlib.h>
#include <emscripten.h>
#include <emscripten/console.h>

#include <AL/alc.h>
#include <AL/al.h>


// ALC extensions
void emscripten_alcDevicePauseSOFT(ALCdevice *device);
void emscripten_alcDeviceResumeSOFT(ALCdevice *device);
const ALCchar *emscripten_alcGetStringiSOFT(ALCdevice *device, ALCenum paramName, ALCsizei index);
ALCboolean emscripten_alcResetDeviceSOFT(ALCdevice *device, const ALCint *attrList);


void* alcGetProcAddress(ALCdevice *device, const ALCchar *name) {
  // Validate the input.
  if (EM_ASM_INT({
    if (!$0) {
      AL.alcErr = 0xA004 /* ALC_INVALID_VALUE */;
      return 1;
    }
  }, name)) {
    return NULL;
  }

  // Base API
  if (!strcmp(name, "alcCreateContext")) { return alcCreateContext; }
  else if (!strcmp(name, "alcMakeContextCurrent")) { return alcMakeContextCurrent; }
  else if (!strcmp(name, "alcProcessContext")) { return alcProcessContext; }
  else if (!strcmp(name, "alcSuspendContext")) { return alcSuspendContext; }
  else if (!strcmp(name, "alcDestroyContext")) { return alcDestroyContext; }
  else if (!strcmp(name, "alcGetCurrentContext")) { return alcGetCurrentContext; }
  else if (!strcmp(name, "alcGetContextsDevice")) { return alcGetContextsDevice; }
  else if (!strcmp(name, "alcOpenDevice")) { return alcOpenDevice; }
  else if (!strcmp(name, "alcCloseDevice")) { return alcCloseDevice; }
  else if (!strcmp(name, "alcGetError")) { return alcGetError; }
  else if (!strcmp(name, "alcIsExtensionPresent")) { return alcIsExtensionPresent; }
  else if (!strcmp(name, "alcGetProcAddress")) { return alcGetProcAddress; }
  else if (!strcmp(name, "alcGetEnumValue")) { return alcGetEnumValue; }
  else if (!strcmp(name, "alcGetString")) { return alcGetString; }
  else if (!strcmp(name, "alcGetIntegerv")) { return alcGetIntegerv; }
  else if (!strcmp(name, "alcCaptureOpenDevice")) { return alcCaptureOpenDevice; }
  else if (!strcmp(name, "alcCaptureCloseDevice")) { return alcCaptureCloseDevice; }
  else if (!strcmp(name, "alcCaptureStart")) { return alcCaptureStart; }
  else if (!strcmp(name, "alcCaptureStop")) { return alcCaptureStop; }
  else if (!strcmp(name, "alcCaptureSamples")) { return alcCaptureSamples; }

  // Extensions
  else if (!strcmp(name, "alcDevicePauseSOFT")) { return emscripten_alcDevicePauseSOFT; }
  else if (!strcmp(name, "alcDeviceResumeSOFT")) { return emscripten_alcDeviceResumeSOFT; }
  else if (!strcmp(name, "alcGetStringiSOFT")) { return emscripten_alcGetStringiSOFT; }
  else if (!strcmp(name, "alcResetDeviceSOFT")) { return emscripten_alcResetDeviceSOFT; }

  emscripten_errf("bad name in alcGetProcAddress: %s", name);
  return 0;
}


void* alGetProcAddress(const ALchar *name) {
  // Validate the state and the input.
  if (EM_ASM_INT({
    if (!AL.currentCtx) {
      err("alGetProcAddress() called without a valid context");
      return 1;
    }
    if (!$0) {
      AL.currentCtx.err = 0xA003 /* AL_INVALID_VALUE */;
      return 1;
    }
  }, name)) {
    return NULL;
  }

  // Base API
  if (!strcmp(name, "alDopplerFactor")) { return alDopplerFactor; }
  else if (!strcmp(name, "alDopplerVelocity")) { return alDopplerVelocity; }
  else if (!strcmp(name, "alSpeedOfSound")) { return alSpeedOfSound; }
  else if (!strcmp(name, "alDistanceModel")) { return alDistanceModel; }
  else if (!strcmp(name, "alEnable")) { return alEnable; }
  else if (!strcmp(name, "alDisable")) { return alDisable; }
  else if (!strcmp(name, "alIsEnabled")) { return alIsEnabled; }
  else if (!strcmp(name, "alGetString")) { return alGetString; }
  else if (!strcmp(name, "alGetBooleanv")) { return alGetBooleanv; }
  else if (!strcmp(name, "alGetIntegerv")) { return alGetIntegerv; }
  else if (!strcmp(name, "alGetFloatv")) { return alGetFloatv; }
  else if (!strcmp(name, "alGetDoublev")) { return alGetDoublev; }
  else if (!strcmp(name, "alGetBoolean")) { return alGetBoolean; }
  else if (!strcmp(name, "alGetInteger")) { return alGetInteger; }
  else if (!strcmp(name, "alGetFloat")) { return alGetFloat; }
  else if (!strcmp(name, "alGetDouble")) { return alGetDouble; }
  else if (!strcmp(name, "alGetError")) { return alGetError; }
  else if (!strcmp(name, "alIsExtensionPresent")) { return alIsExtensionPresent; }
  else if (!strcmp(name, "alGetProcAddress")) { return alGetProcAddress; }
  else if (!strcmp(name, "alGetEnumValue")) { return alGetEnumValue; }
  else if (!strcmp(name, "alListenerf")) { return alListenerf; }
  else if (!strcmp(name, "alListener3f")) { return alListener3f; }
  else if (!strcmp(name, "alListenerfv")) { return alListenerfv; }
  else if (!strcmp(name, "alListeneri")) { return alListeneri; }
  else if (!strcmp(name, "alListener3i")) { return alListener3i; }
  else if (!strcmp(name, "alListeneriv")) { return alListeneriv; }
  else if (!strcmp(name, "alGetListenerf")) { return alGetListenerf; }
  else if (!strcmp(name, "alGetListener3f")) { return alGetListener3f; }
  else if (!strcmp(name, "alGetListenerfv")) { return alGetListenerfv; }
  else if (!strcmp(name, "alGetListeneri")) { return alGetListeneri; }
  else if (!strcmp(name, "alGetListener3i")) { return alGetListener3i; }
  else if (!strcmp(name, "alGetListeneriv")) { return alGetListeneriv; }
  else if (!strcmp(name, "alGenSources")) { return alGenSources; }
  else if (!strcmp(name, "alDeleteSources")) { return alDeleteSources; }
  else if (!strcmp(name, "alIsSource")) { return alIsSource; }
  else if (!strcmp(name, "alSourcef")) { return alSourcef; }
  else if (!strcmp(name, "alSource3f")) { return alSource3f; }
  else if (!strcmp(name, "alSourcefv")) { return alSourcefv; }
  else if (!strcmp(name, "alSourcei")) { return alSourcei; }
  else if (!strcmp(name, "alSource3i")) { return alSource3i; }
  else if (!strcmp(name, "alSourceiv")) { return alSourceiv; }
  else if (!strcmp(name, "alGetSourcef")) { return alGetSourcef; }
  else if (!strcmp(name, "alGetSource3f")) { return alGetSource3f; }
  else if (!strcmp(name, "alGetSourcefv")) { return alGetSourcefv; }
  else if (!strcmp(name, "alGetSourcei")) { return alGetSourcei; }
  else if (!strcmp(name, "alGetSource3i")) { return alGetSource3i; }
  else if (!strcmp(name, "alGetSourceiv")) { return alGetSourceiv; }
  else if (!strcmp(name, "alSourcePlayv")) { return alSourcePlayv; }
  else if (!strcmp(name, "alSourceStopv")) { return alSourceStopv; }
  else if (!strcmp(name, "alSourceRewindv")) { return alSourceRewindv; }
  else if (!strcmp(name, "alSourcePausev")) { return alSourcePausev; }
  else if (!strcmp(name, "alSourcePlay")) { return alSourcePlay; }
  else if (!strcmp(name, "alSourceStop")) { return alSourceStop; }
  else if (!strcmp(name, "alSourceRewind")) { return alSourceRewind; }
  else if (!strcmp(name, "alSourcePause")) { return alSourcePause; }
  else if (!strcmp(name, "alSourceQueueBuffers")) { return alSourceQueueBuffers; }
  else if (!strcmp(name, "alSourceUnqueueBuffers")) { return alSourceUnqueueBuffers; }
  else if (!strcmp(name, "alGenBuffers")) { return alGenBuffers; }
  else if (!strcmp(name, "alDeleteBuffers")) { return alDeleteBuffers; }
  else if (!strcmp(name, "alIsBuffer")) { return alIsBuffer; }
  else if (!strcmp(name, "alBufferData")) { return alBufferData; }
  else if (!strcmp(name, "alBufferf")) { return alBufferf; }
  else if (!strcmp(name, "alBuffer3f")) { return alBuffer3f; }
  else if (!strcmp(name, "alBufferfv")) { return alBufferfv; }
  else if (!strcmp(name, "alBufferi")) { return alBufferi; }
  else if (!strcmp(name, "alBuffer3i")) { return alBuffer3i; }
  else if (!strcmp(name, "alBufferiv")) { return alBufferiv; }
  else if (!strcmp(name, "alGetBufferf")) { return alGetBufferf; }
  else if (!strcmp(name, "alGetBuffer3f")) { return alGetBuffer3f; }
  else if (!strcmp(name, "alGetBufferfv")) { return alGetBufferfv; }
  else if (!strcmp(name, "alGetBufferi")) { return alGetBufferi; }
  else if (!strcmp(name, "alGetBuffer3i")) { return alGetBuffer3i; }
  else if (!strcmp(name, "alGetBufferiv")) { return alGetBufferiv; }

  // Extensions

  emscripten_errf("bad name in alGetProcAddress: %s", name);
  return 0;
}
