#include <emscripten/webaudio.h>
#include <assert.h>
#include <stdint.h>

uint8_t audio_worklet_stack[4096];
EMSCRIPTEN_WEBAUDIO_T audio_context;

void do_exit() {
  emscripten_destroy_audio_context(audio_context);
  emscripten_force_exit(0);
}

void in_audio_worklet() {
  assert(emscripten_current_thread_is_audio_worklet());
  emscripten_audio_worklet_post_function_v(EMSCRIPTEN_AUDIO_MAIN_THREAD, do_exit);
}

void worklet_init(EMSCRIPTEN_WEBAUDIO_T ctx, bool success, void* userData) {
  assert(success);
  EM_ASM({
    var audio = emscriptenGetAudioObject($0);
    assert(audio.audioWorklet.port instanceof MessagePort);
    assert(!audio.audioWorklet.bootstrapMessage);
  }, ctx);
  emscripten_audio_worklet_post_function_v(ctx, in_audio_worklet);
}

int main() {
  audio_context = emscripten_create_audio_context(0);
  emscripten_start_wasm_audio_worklet_thread_async(
    audio_context, audio_worklet_stack, sizeof(audio_worklet_stack), worklet_init, 0);
  emscripten_exit_with_live_runtime();
}
