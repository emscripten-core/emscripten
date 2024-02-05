#include <stdlib.h>
#include <stdio.h>
#include <AL/alut.h>

/*
 * This program loads and plays a file the deprecated ALUT 0.x.x way.
 */

static void playFile(const char *fileName)
{
  ALenum format;
  void *data;
  ALsizei size;
  ALsizei frequency;

#if !defined(__APPLE__)
  ALboolean loop;
#endif
  ALuint buffer;
  ALuint source;
  ALenum error;
  ALint status;

  /* Create an AL buffer from the given sound file. */
  alutLoadWAVFile((ALbyte *) "file1.wav", &format, &data, &size, &frequency
#if !defined(__APPLE__)
                  , &loop
#endif
    );
  alGenBuffers(1, &buffer);
  alBufferData(buffer, format, data, size, frequency);
  free(data);

  /* Generate a single source, attach the buffer to it and start playing. */
  alGenSources(1, &source);
  alSourcei(source, AL_BUFFER, buffer);
  alSourcePlay(source);

  /* Normally nothing should go wrong above, but one never knows... */
  error = alGetError();
  if (error != ALUT_ERROR_NO_ERROR)
  {
    fprintf(stderr, "%s\n", alGetString(error));
    alutExit();
    exit(EXIT_FAILURE);
  }

  /* Check every 0.1 seconds if the sound is still playing. */
  do
  {
    alutSleep(0.1f);
    alGetSourcei(source, AL_SOURCE_STATE, &status);
  }
  while (status == AL_PLAYING);
}

int main(int argc, char **argv)
{
  /* Initialise ALUT and eat any ALUT-specific commandline flags. */
  if (!alutInit(&argc, argv))
  {
    ALenum error = alutGetError();

    fprintf(stderr, "%s\n", alutGetErrorString(error));
    exit(EXIT_FAILURE);
  }

  /* If everything is OK, play the sound files and exit when finished. */
  playFile("file1.wav");

  if (!alutExit())
  {
    ALenum error = alutGetError();

    fprintf(stderr, "%s\n", alutGetErrorString(error));
    exit(EXIT_FAILURE);
  }
  return EXIT_SUCCESS;
}
