/*
 * Copyright 2013 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdlib.h>
#include <AL/alut.h>

/*
  This is the 'Hello World' program from the ALUT
  reference manual.

  Link using '-lalut -lopenal -lpthread'.
*/

int main(int argc, char **argv)
{
  ALuint helloBuffer, helloSource;

  alutInit(&argc, argv);
  helloBuffer = alutCreateBufferHelloWorld();
  alGenSources(1, &helloSource);
  alSourcei(helloSource, AL_BUFFER, helloBuffer);
  alSourcePlay(helloSource);
  alutSleep(1);
  alutExit();
  return EXIT_SUCCESS;
}
