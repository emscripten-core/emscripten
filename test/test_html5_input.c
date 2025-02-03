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

// The following input type strings are provided by the browser depending
// on the kind of input used to change an element.
// See https://w3c.github.io/input-events/#interface-InputEvent-Attributes
// * typing on the keyboard:
static const char* const inputType_insertText = "insertText";
// * copy and paste:
static const char* const inputType_insertFromPaste = "insertFromPaste";
// * drag and drop:
static const char* const inputType_insertFromDrop = "insertFromDrop";

enum TestGoal {
  GoalUnspecified,
  GoalInputSingleLetter,
  GoalPasteText,
  GoalDragAndDropVeryLongText,
  GoalCheckboxChange,
};

static enum TestGoal currentGoal = GoalUnspecified;

static bool goalPassed_inputSingleLetter = false;
static bool goalPassed_pasteText = false;
static bool goalPassed_dropVeryLongText = false;
static bool goalPassed_checkboxChange = false;

static int testStatus() {
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

static void printEvent(const EmscriptenInputEvent* const inputEvent,
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

static void checkEqual(const EMSCRIPTEN_RESULT expected,
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

static void testingDone() {
  emscripten_html5_remove_all_event_listeners();

#ifdef REPORT_RESULT
  REPORT_RESULT(testStatus());
#endif
}

static bool inputCallbackWaitingForCheckboxChange(
  const int eventType,
  const EmscriptenInputEvent* const inputEvent __attribute__((nonnull)),
  void* const userData) {
  printEvent(inputEvent, __func__);

  if (currentGoal != GoalCheckboxChange) {
    printf("    FAIL    unexpected call to %s\n", __func__);
    return false;
  }

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

static void startTestChangeCheckbox() {
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

static const char* const longPasteText =
  "This is an example for the longest string that will be received in wasm "
  "from JS.  128 chars incl. 0-term. after the arrow tip->And this remainder "
  "will be truncated.";

static bool inputCallbackWaitingForPasteOfLongText(
  const int eventType,
  const EmscriptenInputEvent* const inputEvent __attribute__((nonnull)),
  void* const userData) {
  printEvent(inputEvent, __func__);

  if (currentGoal != GoalDragAndDropVeryLongText) {
    printf("    FAIL    unexpected call to %s\n", __func__);
    return false;
  }

  if (strcmp(inputEvent->inputType, inputType_insertFromDrop) != 0) {
    printf("    FAIL    inputType is not '%s': %s\n",
           inputType_insertFromDrop,
           inputEvent->inputType);
    return false;
  }

  // note: Truncation of the string is expected on the wasm side.
  const int expectedLen = EM_HTML5_LONG_STRING_LEN_BYTES - 1;
  const int receivedLen = strlen(inputEvent->data);
  if (receivedLen != expectedLen) {
    printf("    FAIL    wrong data length %d, expected %d\n",
           receivedLen,
           expectedLen);
    return false;
  }

  if (strncmp(inputEvent->data,
              longPasteText,
              EM_HTML5_LONG_STRING_LEN_BYTES - 1) != 0) {
    printf("    FAIL    wrong text received: %s\n", inputEvent->data);
    return false;
  }

  if (inputEvent->data[EM_HTML5_LONG_STRING_LEN_BYTES - 1] != '\0') {
    printf("    FAIL    zero termination missing\n");
    return false;
  }

  printf("    SUCCESS\n");
  goalPassed_dropVeryLongText = true;
  startTestChangeCheckbox();
  return true;
}

static void startTestDropVeryLongText() {
  currentGoal = GoalDragAndDropVeryLongText;
  emscripten_html5_remove_all_event_listeners();
  const EMSCRIPTEN_RESULT res = emscripten_set_input_callback(
    "#input-a", 0, false, inputCallbackWaitingForPasteOfLongText);
  ASSERT_RESULT(res,
                EMSCRIPTEN_RESULT_SUCCESS,
                "install callback on existing input element");

  setInputAValue("");
  setInputBValue(longPasteText);
  printf("Please drag&drop all text from B to A (Ctrl+A, drag, drop).\n");
}

static const char* const expectedPasteText = "I like emscripten.";

static bool
inputCallbackWaitingForPaste(const int eventType,
                             const EmscriptenInputEvent* const inputEvent
                             __attribute__((nonnull)),
                             void* const userData) {
  printEvent(inputEvent, __func__);

  if (currentGoal != GoalPasteText) {
    printf("    FAIL    unexpected call to %s\n", __func__);
    return false;
  }

  if (strcmp(inputEvent->inputType, inputType_insertFromPaste) != 0) {
    printf("    FAIL    inputType is not '%s': %s\n",
           inputType_insertFromPaste,
           inputEvent->inputType);
    return false;
  }

  if (strcmp(inputEvent->data, expectedPasteText) != 0) {
    printf("    FAIL    wrong text received: %s\n", inputEvent->data);
    return false;
  }

  printf("    SUCCESS\n");
  goalPassed_pasteText = true;
  startTestDropVeryLongText();
  return true;
}

static void startTestPasteText() {
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

static void* const expectedUserDataPtr = (void*)0x13370001;

static bool
inputCallbackWaitingForLetterA(const int eventType,
                               const EmscriptenInputEvent* const inputEvent
                               __attribute__((nonnull)),
                               void* const userData) {
  printEvent(inputEvent, __func__);

  if (currentGoal != GoalInputSingleLetter) {
    printf("    FAIL    unexpected call to %s\n", __func__);
    return false;
  }

  if (strcmp(inputEvent->inputType, inputType_insertText) != 0) {
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

static void startTestInputSingleLetter() {
  currentGoal = GoalInputSingleLetter;
  const EMSCRIPTEN_RESULT res = emscripten_set_input_callback(
    "#input-a", expectedUserDataPtr, false, inputCallbackWaitingForLetterA);
  ASSERT_RESULT(res,
                EMSCRIPTEN_RESULT_SUCCESS,
                "install callback on existing input element");
  printf("Please insert a single char 'a' into input field A by typing on"
         " the keyboard.\n");
}

static bool emptyInputCallback(const int eventType,
                               const EmscriptenInputEvent* const inputEvent
                               __attribute__((nonnull)),
                               void* const userData) {
  return true;
}

// WHEN an input callback is installed on an existing component.
// THEN this succeeds.
// note: for negative test see test_html5_unknown_event_target.c
static void testInstallCallback() {
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

  return 0;
}
