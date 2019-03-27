#include <stdio.h>

#include "sqlite3.h"

int main(int argc, char *argv[]) {
  int rc;
  sqlite3 *newDb = 0;
  rc = sqlite3_open(":memory:", &newDb);
  if (rc == 0) {
      printf("ALL OK\n");
  } else {
      printf("ERROR: rc = %d\n", rc);
  }
  return rc;
}
