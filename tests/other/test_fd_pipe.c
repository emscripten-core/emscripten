// Copyright 2018 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>

int pipefd[2];
char buf[5];

int main() {
  const char *filename = "test.dat";

  // Create a pipe
  int status = pipe(pipefd);
  if (status == -1) {
    return 1;
  }
  int fdr = pipefd[0];
  int fdw = pipefd[1];

  // Write to it
  if (write(fdw, "XXXX", 4) != 4) {
    return 2;
  }
  // Close it
  if (close(fdw)) {
    return 3;
  }
  // This write should fail
  if (write(fdw, "YYYY", 4) != -1) {
    return 4;
  }
  // The error number is EBADF
  if (errno != EBADF) {
    return 5;
  }

  // read from the other end of the pipe
  if(read(fdr, buf, 5) != 4){
      return 6;
  }

  // We should have read what we wrote to the other end
  if( strncmp(buf, "XXXX", 5) != 0){
    return 7;
  }

  // Close the read end
  if (close(fdr)) {
    return 8;
  }
 
  // Now reading should return an error
  if (read(fdr, buf, 5) != -1) {
    return 9;
  }

  // The error number is EBADF
  if (errno != EBADF) {
    return 10;
  }

  printf("ok\n");

  return 0;
}
