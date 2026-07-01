#include <emscripten/webaudio.h>
#include <assert.h>

// This test showcases posting messages (function calls) between the main thread
// and the Audio Worklet thread using the
// emscripten_audio_worklet_post_function_*() API.

// This test consists of two steps
//   1. post invocation from main thread to audio worklet thread
//   2. post invocation from audio worklet thread to main thread
_Atomic uint32_t callbackCount = 0;

EMSCRIPTEN_WEBAUDIO_T context;

void do_exit() {
  emscripten_out("do_exit");
  assert(callbackCount == 16);
  emscripten_destroy_audio_context(context);
  emscripten_force_exit(0);
}

// [v, vi, vii, viii, vd, vdd, vddd, viiiiiidddddd] * [audio_worklet, main]
// 8 * 2 callbacks invoked, each callbacks check the correctness of arguments
void v_received_on_audio_worklet() {
  assert(emscripten_current_thread_is_audio_worklet());
  emscripten_out("v_on_audio_worklet");
  ++callbackCount;
}

void vi_received_on_audio_worklet(int i) {
  assert(emscripten_current_thread_is_audio_worklet());
  emscripten_out("vi_on_audio_worklet");
  assert(i == 1);
  ++callbackCount;
}

void vii_received_on_audio_worklet(int i, int j) {
  assert(emscripten_current_thread_is_audio_worklet());
  emscripten_out("vii_on_audio_worklet");
  assert(i == 2);
  assert(j == 3);
  ++callbackCount;
}

void viii_received_on_audio_worklet(int i, int j, int k) {
  assert(emscripten_current_thread_is_audio_worklet());
  emscripten_out("viii_on_audio_worklet");
  assert(i == 4);
  assert(j == 5);
  assert(k == 6);
  ++callbackCount;
}

void vd_received_on_audio_worklet(double i) {
  assert(emscripten_current_thread_is_audio_worklet());
  emscripten_out("vd_on_audio_worklet");
  assert(i == 1.5);
  ++callbackCount;
}

void vdd_received_on_audio_worklet(double i, double j) {
  assert(emscripten_current_thread_is_audio_worklet());
  emscripten_out("vdd_on_audio_worklet");
  assert(i == 2.5);
  assert(j == 3.5);
  ++callbackCount;
}

void vddd_received_on_audio_worklet(double i, double j, double k) {
  assert(emscripten_current_thread_is_audio_worklet());
  emscripten_out("vddd_on_audio_worklet");
  assert(i == 4.5);
  assert(j == 5.5);
  assert(k == 6.5);
  ++callbackCount;
}

void viiiiiidddddd_received_on_audio_worklet(int a, int b, int c, int d, int e, int f, double g, double h, double i, double j, double k, double l) {
  assert(emscripten_current_thread_is_audio_worklet());
  emscripten_out("viiiiiidddddd_on_audio_worklet");
  assert(a == 10);
  assert(b == 11);
  assert(c == 12);
  assert(d == 13);
  assert(e == 14);
  assert(f == 15);
  assert(g == 16.5);
  assert(h == 17.5);
  assert(i == 18.5);
  assert(j == 19.5);
  assert(k == 20.5);
  assert(l == 21.5);
  ++callbackCount;
}

void v_received_on_main() {
  assert(!emscripten_current_thread_is_audio_worklet());
  emscripten_out("v_on_main");
  ++callbackCount;
}

void vi_received_on_main(int i) {
  assert(!emscripten_current_thread_is_audio_worklet());
  emscripten_out("vi_on_main");
  assert(i == 1);
  ++callbackCount;
}

void vii_received_on_main(int i, int j) {
  assert(!emscripten_current_thread_is_audio_worklet());
  emscripten_out("vii_on_main");
  assert(i == 2);
  assert(j == 3);
  ++callbackCount;
}

void viii_received_on_main(int i, int j, int k) {
  assert(!emscripten_current_thread_is_audio_worklet());
  emscripten_out("viii_on_main");
  assert(i == 4);
  assert(j == 5);
  assert(k == 6);
  ++callbackCount;
}

void vd_received_on_main(double i) {
  assert(!emscripten_current_thread_is_audio_worklet());
  emscripten_out("vd_on_main");
  assert(i == 1.5);
  ++callbackCount;
}

void vdd_received_on_main(double i, double j) {
  assert(!emscripten_current_thread_is_audio_worklet());
  emscripten_out("vdd_on_main");
  assert(i == 2.5);
  assert(j == 3.5);
  ++callbackCount;
}

void vddd_received_on_main(double i, double j, double k) {
  assert(!emscripten_current_thread_is_audio_worklet());
  emscripten_out("vddd_on_main");
  assert(i == 4.5);
  assert(j == 5.5);
  assert(k == 6.5);
  ++callbackCount;
}

void viiiiiidddddd_received_on_main(int a, int b, int c, int d, int e, int f, double g, double h, double i, double j, double k, double l) {
  assert(!emscripten_current_thread_is_audio_worklet());
  emscripten_out("viiiiiidddddd_on_main");
  assert(a == 10);
  assert(b == 11);
  assert(c == 12);
  assert(d == 13);
  assert(e == 14);
  assert(f == 15);
  assert(g == 16.5);
  assert(h == 17.5);
  assert(i == 18.5);
  assert(j == 19.5);
  assert(k == 20.5);
  assert(l == 21.5);
  ++callbackCount;
}

void lastCallbackOnAudioWorklet() {
  assert(emscripten_current_thread_is_audio_worklet());
  assert(callbackCount == 8);

  // These event will fire callbacks on main thread.
  emscripten_audio_worklet_post_function_v(EMSCRIPTEN_AUDIO_MAIN_THREAD, v_received_on_main);
  emscripten_audio_worklet_post_function_vi(EMSCRIPTEN_AUDIO_MAIN_THREAD, vi_received_on_main, 1);
  emscripten_audio_worklet_post_function_vii(EMSCRIPTEN_AUDIO_MAIN_THREAD, vii_received_on_main, 2, 3);
  emscripten_audio_worklet_post_function_viii(EMSCRIPTEN_AUDIO_MAIN_THREAD, viii_received_on_main, 4, 5, 6);
  emscripten_audio_worklet_post_function_vd(EMSCRIPTEN_AUDIO_MAIN_THREAD, vd_received_on_main, 1.5);
  emscripten_audio_worklet_post_function_vdd(EMSCRIPTEN_AUDIO_MAIN_THREAD, vdd_received_on_main, 2.5, 3.5);
  emscripten_audio_worklet_post_function_vddd(EMSCRIPTEN_AUDIO_MAIN_THREAD, vddd_received_on_main, 4.5, 5.5, 6.5);
  emscripten_audio_worklet_post_function_sig(EMSCRIPTEN_AUDIO_MAIN_THREAD, viiiiiidddddd_received_on_main, "iiiiiidddddd", 10, 11, 12, 13, 14, 15, 16.5, 17.5, 18.5, 19.5, 20.5, 21.5);
  emscripten_audio_worklet_post_function_v(EMSCRIPTEN_AUDIO_MAIN_THREAD, do_exit);
}

// This callback will fire when the audio worklet thread has been initialized.
void WebAudioWorkletThreadInitialized(EMSCRIPTEN_WEBAUDIO_T audioContext, bool success, void *userData) {
  emscripten_out("WebAudioWorkletThreadInitialized");

  // These event will fire callbacks on audio worklet thread.
  emscripten_audio_worklet_post_function_v(audioContext, v_received_on_audio_worklet );
  emscripten_audio_worklet_post_function_vi(audioContext, vi_received_on_audio_worklet , 1);
  emscripten_audio_worklet_post_function_vii(audioContext, vii_received_on_audio_worklet , 2, 3);
  emscripten_audio_worklet_post_function_viii(audioContext, viii_received_on_audio_worklet , 4, 5, 6);
  emscripten_audio_worklet_post_function_vd(audioContext, vd_received_on_audio_worklet , 1.5);
  emscripten_audio_worklet_post_function_vdd(audioContext, vdd_received_on_audio_worklet , 2.5, 3.5);
  emscripten_audio_worklet_post_function_vddd(audioContext, vddd_received_on_audio_worklet , 4.5, 5.5, 6.5);
  emscripten_audio_worklet_post_function_sig(audioContext, viiiiiidddddd_received_on_audio_worklet , "iiiiiidddddd", 10, 11, 12, 13, 14, 15, 16.5, 17.5, 18.5, 19.5, 20.5, 21.5);
  emscripten_audio_worklet_post_function_v(audioContext, lastCallbackOnAudioWorklet);
}

uint8_t wasmAudioWorkletStack[4096];

int main() {
  // Create an audio context
  context = emscripten_create_audio_context(0 /* use default constructor options */);

  // and kick off Audio Worklet scope initialization, which shares the Wasm
  // Module and Memory to the AudioWorklet scope and initializes its stack.
  emscripten_start_wasm_audio_worklet_thread_async(context, wasmAudioWorkletStack, sizeof(wasmAudioWorkletStack), WebAudioWorkletThreadInitialized, 0);

  emscripten_exit_with_live_runtime();
}
