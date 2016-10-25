#include <stdio.h>
#include <emscripten.h>
#include <string.h>
#include <GL/glfw.h>

void report_result(int result)
{
  if (result == 0) {
    printf("Test successful!\n");
  } else {
    printf("Test failed!\n");
  }
#ifdef REPORT_RESULT
  REPORT_RESULT();
#endif

  // end test loop.
  emscripten_cancel_main_loop();
}

#define TEST_RESULT(x) if (ret != EMSCRIPTEN_RESULT_SUCCESS) printf("%s returned %s.\n", #x, emscripten_result_to_string(ret));

int currentState = 0;

const int numStates = 6;
const int expectedButton[numStates] =
{
  GLFW_MOUSE_BUTTON_LEFT, GLFW_MOUSE_BUTTON_LEFT,
  GLFW_MOUSE_BUTTON_MIDDLE, GLFW_MOUSE_BUTTON_MIDDLE,
  GLFW_MOUSE_BUTTON_RIGHT, GLFW_MOUSE_BUTTON_RIGHT,
};
const int expectedState[numStates] =
{
  GLFW_PRESS, GLFW_RELEASE,
  GLFW_PRESS, GLFW_RELEASE,
  GLFW_PRESS, GLFW_RELEASE
};

void main_tick()
{
  // Minimal gl loop
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glfwSwapBuffers();

  // Start test code path
  printf("Looping, current state %d.\n", currentState);

  // Test ends.
  if (currentState == numStates) {
    printf("Test ended.\n");
    report_result(0);
  }

  // Events should arrive in the right order
  if (glfwGetMouseButton(expectedButton[currentState]) == expectedState[currentState])
  {
    ++currentState;
  }
  else
  {
    for (int i = GLFW_MOUSE_BUTTON_1; i < GLFW_MOUSE_BUTTON_LAST ; ++i)
    {
      if (i == expectedButton[currentState])
      {
        continue;
      }
      
      if (glfwGetMouseButton(i) == GLFW_PRESS)
      {
          printf("Unexpected button event %d.\n", i);
          report_result(1);
      }
    }
  }
}

void MouseButtonCallback(int button, int action)
{
  printf("MouseButtonCallback called with button %d and action %d.\n", button, action);
  if (button != expectedButton[currentState])
  {
    printf("Unexpected button event %d.\n", button);
    report_result(1);
  }
  if (action != expectedState[currentState])
  {
    printf("Unexpected button action %d.\n", action);
    report_result(1);
  }
}

int main() {

  if (glfwInit() != GL_TRUE)
  {
    printf("glfwInit failed.\n");
    report_result(1);
  }

  if (glfwOpenWindow(800, 600, 0, 0, 0, 0, 0, 0, GLFW_WINDOW) != GL_TRUE)
  {
    printf("glfwOpenWindow failed.\n");
    report_result(1);
  }

  // Set callback to get mouse button changed events.
  glfwSetMouseButtonCallback(&MouseButtonCallback);

  printf("Starting emscripten main loop.\n");
  emscripten_set_main_loop(main_tick, 0, 0);

#ifdef AUTOMATE_SUCCESS
  EM_ASM(
    function sendEvent(type, data) {
      var event = document.createEvent('Event');
      event.initEvent(type, true, true);
      for(var d in data) event[d] = data[d];
      Module['canvas'].dispatchEvent(event);
    }

    // Press buttons in expected test order.
    // http://www.w3schools.com/jsref/event_button.asp
    // 0 : Left mouse button
    // 1 : Wheel button or middle button (if present)
    // 2 : Right mouse button
    //
    // http://www.w3schools.com/jsref/event_buttons.asp
    // 1 : Left mouse button
    // 2 : Right mouse button
    // 4 : Wheel button or middle button

    delay = 50;
    // Left
    sendEvent('mousedown', { screenX: 1, screenY: 1, clientX: 1, clientY: 1, button: 0, buttons: 1 });
    setTimeout(function() {
      sendEvent('mouseup', { screenX: 1, screenY: 1, clientX: 1, clientY: 1, button: 0, buttons: 1 });

      // Middle
      setTimeout(function() {
        sendEvent('mousedown', { screenX: 1, screenY: 1, clientX: 1, clientY: 1, button: 1, buttons: 4 });
        setTimeout(function() {
          sendEvent('mouseup', { screenX: 1, screenY: 1, clientX: 1, clientY: 1, button: 1, buttons: 4 });

          // right
          setTimeout(function() {
            sendEvent('mousedown', { screenX: 1, screenY: 1, clientX: 1, clientY: 1, button: 2, buttons: 2 });
            setTimeout(function() {
              sendEvent('mouseup', { screenX: 1, screenY: 1, clientX: 1, clientY: 1, button: 2, buttons: 2 });
            }, delay);
          }, delay);
        }, delay);
      }, delay);
    }, delay);
  );
#endif
}
