/*
 * Copyright 2025 The Emscripten Authors. All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <assert.h>
#include <emscripten.h>
#include <emscripten/html5.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

enum TestGoal {
  GoalUnspecified,
  GoalInputSingleLetter,
  GoalPasteText,
  GoalDragAndDropVeryLongText,
  GoalCheckboxChange,
};

enum TestGoal currentGoal = GoalUnspecified;

bool goalPassed_inputSingleLetter = false;
bool goalPassed_pasteText = false;
bool goalPassed_dropVeryLongText = false;
bool goalPassed_checkboxChange = false;

int testStatus() {
  if (goalPassed_inputSingleLetter && goalPassed_pasteText &&
      goalPassed_dropVeryLongText && goalPassed_checkboxChange) {
    return 0;
  } else {
    return 1;
  }
}

const char* emscripten_result_to_string(EMSCRIPTEN_RESULT result) {
  if (result == EMSCRIPTEN_RESULT_SUCCESS)
    return "EMSCRIPTEN_RESULT_SUCCESS";
  if (result == EMSCRIPTEN_RESULT_DEFERRED)
    return "EMSCRIPTEN_RESULT_DEFERRED";
  if (result == EMSCRIPTEN_RESULT_NOT_SUPPORTED)
    return "EMSCRIPTEN_RESULT_NOT_SUPPORTED";
  if (result == EMSCRIPTEN_RESULT_FAILED_NOT_DEFERRED)
    return "EMSCRIPTEN_RESULT_FAILED_NOT_DEFERRED";
  if (result == EMSCRIPTEN_RESULT_INVALID_TARGET)
    return "EMSCRIPTEN_RESULT_INVALID_TARGET";
  if (result == EMSCRIPTEN_RESULT_UNKNOWN_TARGET)
    return "EMSCRIPTEN_RESULT_UNKNOWN_TARGET";
  if (result == EMSCRIPTEN_RESULT_INVALID_PARAM)
    return "EMSCRIPTEN_RESULT_INVALID_PARAM";
  if (result == EMSCRIPTEN_RESULT_FAILED)
    return "EMSCRIPTEN_RESULT_FAILED";
  if (result == EMSCRIPTEN_RESULT_NO_DATA)
    return "EMSCRIPTEN_RESULT_NO_DATA";
  return "Unknown EMSCRIPTEN_RESULT!";
}

void printEvent(const EmscriptenInputEvent* const inputEvent,
                const char* const context) {
  printf("    INFO    data: %s (%s)\n"
         "    INFO    inputType: %s (%s)\n"
         "    INFO    isComposing: %d (%s)\n",
         inputEvent->data,
         context,
         inputEvent->inputType,
         context,
         inputEvent->isComposing,
         context);
}

void checkEqual(const EMSCRIPTEN_RESULT expected,
                const EMSCRIPTEN_RESULT actual,
                const char* const context) {
  if (expected != actual) {
    printf("expected %s but got %s (context: %s)\n",
           emscripten_result_to_string(expected),
           emscripten_result_to_string(actual),
           context);
  }
}

#define ASSERT_RESULT(e, a, c)                                                 \
  do {                                                                         \
    checkEqual(e, a, c);                                                       \
    assert(e == a);                                                            \
  } while (0)

EM_JS(void, createParagraphWithContent, (const char *str), {
  const para = document.createElement("p");
  para.innerHTML = UTF8ToString(str);
  document.body.appendChild( para );
} )

EM_JS(void, setInputAValue, (const char *str), {
  var target = document.getElementById('input-a');
  target.value = UTF8ToString(str);
} )

EM_JS(void, setInputBValue, (const char *str), {
  var target = document.getElementById('input-b');
  target.value = UTF8ToString(str);
} )

void testingDone() {
  emscripten_html5_remove_all_event_listeners();
  emscripten_runtime_keepalive_pop();
  emscripten_force_exit(testStatus());
}

bool inputCallbackWaitingForCheckboxChange(
  const int eventType,
  const EmscriptenInputEvent* const inputEvent __attribute__((nonnull)),
  void* const userData) {
  printEvent(inputEvent, __func__);

  assert(currentGoal == GoalCheckboxChange);

  const int expectedInputTypeLen = 0;
  const int receivedInputTypeLen = strlen(inputEvent->inputType);
  if (receivedInputTypeLen != expectedInputTypeLen) {
    printf("    FAIL    wrong type length %d, expected %d\n",
           receivedInputTypeLen,
           expectedInputTypeLen);
    return false;
  }

  const int expectedDataLen = 0;
  const int receivedDataLen = strlen(inputEvent->data);
  if (receivedDataLen != expectedDataLen) {
    printf("    FAIL    wrong data length %d, expected %d\n",
           receivedDataLen,
           expectedDataLen);
    return false;
  }

  printf("    SUCCESS\n");
  goalPassed_checkboxChange = true;
  emscripten_async_call(testingDone, 0, 5000);
  return true;
}

void startTestChangeCheckbox() {
  currentGoal = GoalCheckboxChange;
  emscripten_html5_remove_all_event_listeners();
  const EMSCRIPTEN_RESULT res = emscripten_set_input_callback(
    "#checkbox-c", 0, false, inputCallbackWaitingForCheckboxChange);
  ASSERT_RESULT(res,
                EMSCRIPTEN_RESULT_SUCCESS,
                "install callback on existing input element");

  setInputAValue("");
  setInputBValue("");
  printf("Please check checkbox C.\n");
}

const char* const longDropText =
  "This is an example for a long text which is longer than the previously "
  "pasted text. Hence, it will trigger a realloc() call to reserve more "
  "memory. The string ends at the following arrow tip -->";

bool inputCallbackWaitingForPasteOfLongText(
  const int eventType,
  const EmscriptenInputEvent* const inputEvent __attribute__((nonnull)),
  void* const userData) {
  printEvent(inputEvent, __func__);

  assert(currentGoal == GoalDragAndDropVeryLongText);

  if (strcmp(inputEvent->inputType, "insertFromDrop") != 0) {
    printf("    FAIL    inputType is not 'insertFromDrop': %s\n",
           inputEvent->inputType);
    return false;
  }

  if (strcmp(inputEvent->data, longDropText) != 0) {
    printf("    FAIL    wrong text received: %s\n", inputEvent->data);
    return false;
  }

  printf("    SUCCESS\n");
  goalPassed_dropVeryLongText = true;
  startTestChangeCheckbox();
  return true;
}

void startTestDropLongText() {
  currentGoal = GoalDragAndDropVeryLongText;
  emscripten_html5_remove_all_event_listeners();
  const EMSCRIPTEN_RESULT res = emscripten_set_input_callback(
    "#input-a", 0, false, inputCallbackWaitingForPasteOfLongText);
  ASSERT_RESULT(res,
                EMSCRIPTEN_RESULT_SUCCESS,
                "install callback on existing input element");

  setInputAValue("");
  setInputBValue(longDropText);
  printf("Please drag&drop all text from B to A (Ctrl+A, drag, drop).\n");
}

const char* const expectedPasteText = "I like emscripten.";

bool inputCallbackWaitingForPaste(const int eventType,
                                  const EmscriptenInputEvent* const inputEvent
                                  __attribute__((nonnull)),
                                  void* const userData) {
  printEvent(inputEvent, __func__);

  assert(currentGoal == GoalPasteText);

  if (strcmp(inputEvent->inputType, "insertFromPaste") != 0) {
    printf("    FAIL    inputType is not 'insertFromPaste': %s\n",
           inputEvent->inputType);
    return false;
  }

  if (strcmp(inputEvent->data, expectedPasteText) != 0) {
    printf("    FAIL    wrong text received: %s\n", inputEvent->data);
    return false;
  }

  printf("    SUCCESS\n");
  goalPassed_pasteText = true;
  startTestDropLongText();
  return true;
}

void startTestPasteText() {
  currentGoal = GoalPasteText;
  emscripten_html5_remove_all_event_listeners();
  const EMSCRIPTEN_RESULT res = emscripten_set_input_callback(
    "#input-b", 0, false, inputCallbackWaitingForPaste);
  ASSERT_RESULT(res,
                EMSCRIPTEN_RESULT_SUCCESS,
                "install callback on existing input element");

  setInputAValue(expectedPasteText);
  printf("Please copy&paste all text from A to B (Ctrl+A, Ctrl+C, Ctrl+V).\n");
}

void* const expectedUserDataPtr = (void*)0x13370001;

bool inputCallbackWaitingForLetterA(const int eventType,
                                    const EmscriptenInputEvent* const inputEvent
                                    __attribute__((nonnull)),
                                    void* const userData) {
  printEvent(inputEvent, __func__);

  assert(currentGoal == GoalInputSingleLetter);

  if (strcmp(inputEvent->inputType, "insertText") != 0) {
    printf("    FAIL    inputType is not 'insertText': %s\n",
           inputEvent->inputType);
    return false;
  }

  if (userData != expectedUserDataPtr) {
    printf("    FAIL    userData not forwarded correctly (%s)\n", __func__);
    return false;
  }

  if (strcmp(inputEvent->data, "a") != 0) {
    printf("    FAIL    this is not an 'a': %s\n", inputEvent->data);
    return false;
  }

  printf("    SUCCESS\n");
  goalPassed_inputSingleLetter = true;
  startTestPasteText();
  return true;
}

void startTestInputSingleLetter() {
  currentGoal = GoalInputSingleLetter;
  const EMSCRIPTEN_RESULT res = emscripten_set_input_callback(
    "#input-a", expectedUserDataPtr, false, inputCallbackWaitingForLetterA);
  ASSERT_RESULT(res,
                EMSCRIPTEN_RESULT_SUCCESS,
                "install callback on existing input element");
  printf("Please insert a single char 'a' into input field A by typing on"
         " the keyboard.\n");
}

bool emptyInputCallback(const int eventType,
                        const EmscriptenInputEvent* const inputEvent
                        __attribute__((nonnull)),
                        void* const userData) {
  return true;
}

// WHEN an input callback is installed on an existing component.
// THEN this succeeds.
// note: for negative test see test_html5_unknown_event_target.c
void testInstallCallback() {
  const EMSCRIPTEN_RESULT res =
    emscripten_set_input_callback("#input-a", 0, false, emptyInputCallback);
  ASSERT_RESULT(res,
                EMSCRIPTEN_RESULT_SUCCESS,
                "install callback on existing input element");
  emscripten_html5_remove_all_event_listeners();
}

int main() {
  createParagraphWithContent(
    "<label for=\"input-a\">A:</label>"
    "<textarea id=\"input-a\" rows=\"5\" cols=\"100\"></textarea>");
  createParagraphWithContent(
    "<label for=\"input-b\">B:</label>"
    "<textarea id=\"input-b\" rows=\"5\" cols=\"100\"></textarea>");
  createParagraphWithContent(
    "<label for=\"checkbox-c\">C:</label>"
    "<input id=\"checkbox-c\" type=\"checkbox\" value=\"checkbox-c-value\">");

  testInstallCallback();
  startTestInputSingleLetter();
  // further tests are invoked by chained callbacks ...

  emscripten_runtime_keepalive_push();
  return 0;
}
