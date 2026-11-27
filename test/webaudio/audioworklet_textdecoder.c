#include <emscripten/webaudio.h>
#include <emscripten/em_js.h>
#include <assert.h>
#include <uchar.h>

// Tests that an Audio Worklet starts up in a -sTEXTDECODER=2 build of a
// program that uses UTF16ToString().
//
// TextDecoder does not exist in AudioWorkletGlobalScope, so the runtime must
// not unconditionally construct one when the script is loaded there: the
// exception from that leaves everything that the script defines after that
// point undefined in the Audio Worklet scope.

EM_JS_DEPS(deps, "$UTF16ToString");

EM_JS(int, utf16_to_string_works, (const char16_t* str), {
  return UTF16ToString(str) == 'hello from the main thread';
});

// With -sTEXTDECODER=2 strings can not be converted on the Audio Worklet
// thread, since there is no TextDecoder there. So on that thread only check
// that the script was loaded up to the point where UTF16ToString() is defined.
EM_JS(int, utf16_to_string_is_defined, (), {
  return typeof UTF16ToString == 'function';
});

EMSCRIPTEN_WEBAUDIO_T context;

void report_result(int is_defined) {
  assert(!emscripten_current_thread_is_audio_worklet());
  assert(is_defined && "UTF16ToString is not defined in the Audio Worklet scope");
  emscripten_destroy_audio_context(context);
  emscripten_force_exit(is_defined ? 0 : 1);
}

void run_in_audio_worklet() {
  assert(emscripten_current_thread_is_audio_worklet());
  emscripten_audio_worklet_post_function_vi(EMSCRIPTEN_AUDIO_MAIN_THREAD, report_result, utf16_to_string_is_defined());
}

void WebAudioWorkletThreadInitialized(EMSCRIPTEN_WEBAUDIO_T audioContext, bool success, void *userData) {
  assert(success);
  emscripten_audio_worklet_post_function_v(audioContext, run_in_audio_worklet);
}

uint8_t wasmAudioWorkletStack[4096];

int main() {
  // On the main thread TextDecoder exists and UTF16ToString() works.
  assert(utf16_to_string_works(u"hello from the main thread"));

  context = emscripten_create_audio_context(0);
  emscripten_start_wasm_audio_worklet_thread_async(context, wasmAudioWorkletStack, sizeof(wasmAudioWorkletStack), WebAudioWorkletThreadInitialized, 0);
  emscripten_exit_with_live_runtime();
}
