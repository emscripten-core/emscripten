#include <syslog.h>
#include <stddef.h>

int main() {
  // The current implementation of syslog that we use comes
  // from musl and doesn't do anything useful under emscripten.
  // This test exists simply to ensure that it compile and link.
  syslog(LOG_CRIT, "%s", "log1");
  openlog(NULL, LOG_CONS, LOG_USER);
  syslog(LOG_CRIT, "%s", "log2");
}
