#include <emscripten/emscripten.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int error_count = 0;
int pending_count = 0;

enum Status {
  PENDING,
  SUCCESS,
  FAIL
};

struct Request {
  const char* target;
  Status status;
  int expected_http_status;
  const char* expected_http_status_text;
  const char* expected_response_text;
};

void onload(unsigned handle, void* arg, void* data, unsigned length) {
  Request* request = (Request*) arg;
  const char* text = (const char*) data;
  if (text != nullptr && request->expected_response_text == nullptr) {
    printf("Error: Request [%s] returned [%s] but no response was expected.\n",
      request->target, text);
    error_count++;
    request->status = FAIL;
  } else if (strcmp(text, request->expected_response_text) != 0) {
    printf("Error: Request [%s] expected response text [%s] but got [%s]\n",
      request->target, request->expected_response_text, text);
    error_count++;
    request->status = FAIL;
  } else {
    printf("Pass: Request [%s] returned expected text\n", request->target);
    request->status = SUCCESS;
  }
  pending_count--;
  delete request;
}

void onerror(unsigned handle, void* arg, int status, const char* status_text) {
  Request* request = (Request*) arg;
  if (status_text == nullptr && request->expected_http_status_text != nullptr) {
    printf("Error: Request [%s] expected response text [%s] but none was given\n",
      request->target, request->expected_http_status_text);
    error_count++;
    request->status = FAIL;
  } else if (status_text != nullptr && request->expected_http_status_text == nullptr) {
    printf("Error: Request [%s] gave status text [%s] but no response was expted\n",
      request->target, status_text);
    error_count++;
    request->status = FAIL;
  } else if (strcmp(status_text, request->expected_http_status_text) != 0) {
    printf("Error: Request [%s] expected status text [%s] but got [%s]\n",
      request->target, request->expected_http_status_text, status_text);
    error_count++;
    request->status = FAIL;
  } else {
    printf("Pass: Request [%s] returned expected status code and message\n",
      request->target);
    request->status = SUCCESS;
  }
  pending_count--;
  delete request;
}

void onprogress(unsigned handle, void* arg, int loaded, int total) {
  Request* request = (Request*) arg;
  printf("[%s] %i/%i\n", request->target, loaded, total);
}

void create_request(const char* target, const char* method, int expected_http_status, const char* expected_http_status_text, const char* expected_response_text) {
  Request* request = new Request();
  request->target = target;
  request->status = PENDING;
  request->expected_http_status = expected_http_status;
  request->expected_http_status_text = expected_http_status_text;
  request->expected_response_text = expected_response_text;
  pending_count++;
  emscripten_async_wget2_data(target, method, "", (void*) request, true,
    onload, onerror, onprogress);
};

void wait_http(void* arg) {
  Request* request = (Request*) arg;

};

void wait() {
  if (pending_count == 0) {
    printf("There were %d errors\n", error_count);
    emscripten_cancel_main_loop();
    REPORT_RESULT(error_count);
  } else {
    printf("Not ready (%i requests pending)\n", pending_count);
  }
}

int main() {
  /* Sends Test Requests.
   * Call signature is
   * `create_request(url, method, expected status code, expected message, expected content);`
   * Note that if the request is a success, the status code and status message
   * will not be checked, and if the request is a failure, the contents of the
   * request will not be checked. I do not believe it is possible to retrieve
   * and pass this information to the script currently.
   */
  create_request("//httpstat.us/404/cors", "GET", 404, "Not Found", nullptr);
  create_request("//httpstat.us/503/cors", "GET", 503, "Service Unavailable", nullptr);
  create_request("//httpstat.us/200/cors", "GET", 0, nullptr, "200 OK");
  create_request("//httpstat.us/0", "GET", 0, nullptr, nullptr); // <- Should cause CORS error
  create_request("hello.txt", "GET", 0, nullptr, "Hello Emscripten!");
  emscripten_set_main_loop(wait, 0, 0);
}
