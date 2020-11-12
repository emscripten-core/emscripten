#include <stdio.h>
#include <stdlib.h>
#include <AL/alut.h>

/*
 * This is a minimal test for error handling.
 */

int main(int argc, char **argv)
{
  ALuint buffer;

  alutInit(&argc, argv);
  buffer = alutCreateBufferFromFile("no_such_file_in_existance.wav");
  alutExit();

  if (buffer != AL_NONE)
  {
    fprintf(stderr, "expected an I/O error\n");
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
