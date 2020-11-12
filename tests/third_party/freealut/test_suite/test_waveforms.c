#include <stdlib.h>
#include <AL/alut.h>

/*
 * This program plays a 440Hz tone using a variety of waveforms.
 */

static void playTone(ALenum waveshape)
{
  ALuint buffer, source;

  buffer = alutCreateBufferWaveform(waveshape, 440, 0, 1);
  alGenSources(1, &source);
  alSourcei(source, AL_BUFFER, buffer);
  alSourcePlay(source);
  alutSleep(1);
}

int main(int argc, char **argv)
{
  alutInit(&argc, argv);

  playTone(ALUT_WAVEFORM_SINE);
  playTone(ALUT_WAVEFORM_SQUARE);
  playTone(ALUT_WAVEFORM_SAWTOOTH);
  playTone(ALUT_WAVEFORM_WHITENOISE);
  playTone(ALUT_WAVEFORM_IMPULSE);

  alutExit();
  return EXIT_SUCCESS;
}
