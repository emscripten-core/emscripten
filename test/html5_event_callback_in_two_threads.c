#include <emscripten.h>
#include <emscripten/html5.h>
#include <emscripten/threading.h>
#include <emscripten/key_codes.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <assert.h>

pthread_t application_thread_id = 0;
pthread_t main_runtime_thread_id = 0;

volatile int saw_keydown_event_on_enter_key_on_application_thread = 0;
volatile int saw_keydown_event_on_enter_key_on_main_runtime_thread = 0;
volatile int saw_keypress_event_on_enter_key = 0;

void ReportResult(int code)
{
  printf("Test finished with code: %d\n", code);
#ifdef REPORT_RESULT
  REPORT_RESULT(code);
#endif
  exit(code);
}

EM_BOOL keydown_callback_on_application_thread(int eventType, const EmscriptenKeyboardEvent *e, void *userData)
{
  int dom_pk_code = emscripten_compute_dom_pk_code(e->code);
  printf("keydown_callback_on_application_thread received on pthread: %p, application_thread_id: %p, dom_pk_code: %s\n", pthread_self(), application_thread_id, emscripten_dom_pk_code_to_string(dom_pk_code));
  assert(pthread_self() == application_thread_id);

  if (dom_pk_code == DOM_PK_ENTER) saw_keydown_event_on_enter_key_on_application_thread = 1;
  return 0;
}

EM_BOOL keydown_callback_on_main_runtime_thread(int eventType, const EmscriptenKeyboardEvent *e, void *userData)
{
  int dom_pk_code = emscripten_compute_dom_pk_code(e->code);
  printf("keydown_callback_on_main_runtime_thread received on pthread: %p, main_runtime_thread_id; %p, dom_pk_code: %s\n", pthread_self(), main_runtime_thread_id, emscripten_dom_pk_code_to_string(dom_pk_code));
  assert(pthread_self() == main_runtime_thread_id);

#if __EMSCRIPTEN_PTHREADS__
  EmscriptenKeyboardEvent *duplicatedEventStruct = malloc(sizeof(*e));
  memcpy(duplicatedEventStruct, e, sizeof(*e));
  emscripten_dispatch_to_thread(application_thread_id, EM_FUNC_SIG_IIII, keydown_callback_on_application_thread, duplicatedEventStruct, eventType, duplicatedEventStruct, userData);
#else
  keydown_callback_on_application_thread(eventType, e, userData);
#endif

  if (dom_pk_code == DOM_PK_ENTER) saw_keydown_event_on_enter_key_on_main_runtime_thread = 1;

  return dom_pk_code == DOM_PK_ENTER; // Suppress default event handling for the enter/return key so that it should not generate the keypress event.
}


EM_BOOL keypress_callback_on_application_thread(int eventType, const EmscriptenKeyboardEvent *e, void *userData)
{
  int dom_pk_code = emscripten_compute_dom_pk_code(e->code);
  printf("keypress_callback_on_application_thread received on pthread: %p, application_thread_id; %p, dom_pk_code: %s\n", pthread_self(), application_thread_id, emscripten_dom_pk_code_to_string(dom_pk_code));
  assert(pthread_self() == application_thread_id);

  if (dom_pk_code == DOM_PK_ENTER)
  {
    saw_keypress_event_on_enter_key = 1;
    printf("Test failed! KeyPress event came through even though it was suppressed in KeyDown handler!\n");
    ReportResult(12345); // FAIL
  }
  return 0;
}

EM_BOOL keyup_callback_on_application_thread(int eventType, const EmscriptenKeyboardEvent *e, void *userData)
{
  int dom_pk_code = emscripten_compute_dom_pk_code(e->code);
  printf("keyup_callback_on_application_thread received on pthread: %p, application_thread_id; %p, dom_pk_code: %s\n", pthread_self(), application_thread_id, emscripten_dom_pk_code_to_string(dom_pk_code));
  assert(pthread_self() == application_thread_id);

  if (dom_pk_code == DOM_PK_ENTER)
  {
    if (!saw_keydown_event_on_enter_key_on_application_thread)
    {
      printf("Test failed! KeyUp event came through, but a KeyDown event should have first been processed on the application thread!\n");
      ReportResult(12346); // FAIL
    }
    if (!saw_keydown_event_on_enter_key_on_main_runtime_thread)
    {
      printf("Test failed! KeyUp event came through, but a KeyDown event should have first been processed on the main runtime thread!\n");
      ReportResult(12347); // FAIL
    }
    if (saw_keypress_event_on_enter_key)
    {
      printf("Test failed! KeyUp event came through, but a KeyPress event was first seen, suppressing it did not work!\n");
      ReportResult(12348); // FAIL
    }
    printf("Test passed!\n");
    ReportResult(1); // PASS
  }
  return 0;
}

int main()
{
  main_runtime_thread_id = emscripten_main_runtime_thread_id();
  assert(main_runtime_thread_id);
  application_thread_id = pthread_self();
  assert(application_thread_id);

  printf("Main runtime thread ID: %p, application thread ID: %p\n", main_runtime_thread_id, application_thread_id);

  emscripten_set_keydown_callback_on_thread(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, keydown_callback_on_main_runtime_thread, EM_CALLBACK_THREAD_CONTEXT_MAIN_RUNTIME_THREAD);
  emscripten_set_keypress_callback_on_thread(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, keypress_callback_on_application_thread, EM_CALLBACK_THREAD_CONTEXT_CALLING_THREAD);
  emscripten_set_keyup_callback_on_thread(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, keyup_callback_on_application_thread, EM_CALLBACK_THREAD_CONTEXT_CALLING_THREAD);

  printf("Please press the Enter key.\n");

  emscripten_exit_with_live_runtime();
}
