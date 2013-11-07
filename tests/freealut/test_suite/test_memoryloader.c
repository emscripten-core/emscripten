#include <stdio.h>
#include <stdlib.h>
#include <AL/alut.h>

/*
 * This program loads and plays a variety of files from memory,
 * basically a modified version of test_suite/test_fileloader.c.
 */

static ALbyte fileBuffer[100000];

static void playFile(const char *fileName)
{
  FILE *fileDescriptor;
  size_t fileLength;
  ALuint buffer;
  ALuint source;
  ALenum error;
  ALint status;

  /* Load the sound file into memory. */
  fileDescriptor = fopen(fileName, "rb");
  if (fileDescriptor == NULL)
  {
    fprintf(stderr, "Error opening file %s\n", fileName);
    alutExit();
    exit(EXIT_FAILURE);
  }

  fileLength = fread(fileBuffer, 1, sizeof(fileBuffer), fileDescriptor);
  if (ferror(fileDescriptor))
  {
    fprintf(stderr, "Error reading file %s\n", fileName);
    alutExit();
    exit(EXIT_FAILURE);
  }

  fclose(fileDescriptor);

  /* Create an AL buffer from the given sound file. */
  buffer = alutCreateBufferFromFileImage(fileBuffer, fileLength);
  if (buffer == AL_NONE)
  {
    error = alutGetError();
    fprintf(stderr, "Error creating buffer from file image: '%s'\n", alutGetErrorString(error));
    alutExit();
    exit(EXIT_FAILURE);
  }

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
  playFile("file2.au");
  /* Note that we can not play raw sound files from memory because the
   * format can't be guessed without a file name. */

  if (!alutExit())
  {
    ALenum error = alutGetError();

    fprintf(stderr, "%s\n", alutGetErrorString(error));
    exit(EXIT_FAILURE);
  }
  return EXIT_SUCCESS;
}
