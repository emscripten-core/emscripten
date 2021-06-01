#include "alutInternal.h"

struct BufferData_struct
{
  ALvoid *data;
  size_t length;
  ALint numChannels;
  ALint bitsPerSample;
  ALfloat sampleFrequency;
};

BufferData *_alutBufferDataConstruct(ALvoid * data, size_t length, ALint numChannels, ALint bitsPerSample, ALfloat sampleFrequency)
{
  BufferData *bufferData = (BufferData *) _alutMalloc(sizeof(BufferData));

  if (bufferData == NULL)
  {
    return NULL;
  }

  bufferData->data = data;
  bufferData->length = length;
  bufferData->numChannels = numChannels;
  bufferData->bitsPerSample = bitsPerSample;
  bufferData->sampleFrequency = sampleFrequency;

  return bufferData;
}

ALboolean _alutBufferDataDestroy(BufferData * bufferData)
{
  if (bufferData->data != NULL)
  {
    free(bufferData->data);
  }
  free(bufferData);
  return AL_TRUE;
}

ALvoid *_alutBufferDataGetData(const BufferData * bufferData)
{
  return bufferData->data;
}

void _alutBufferDataDetachData(BufferData * bufferData)
{
  bufferData->data = NULL;
}

size_t _alutBufferDataGetLength(const BufferData * bufferData)
{
  return bufferData->length;
}

static ALint getNumChannels(const BufferData * bufferData)
{
  return bufferData->numChannels;
}

static ALint getBitsPerSample(const BufferData * bufferData)
{
  return bufferData->bitsPerSample;
}

ALfloat _alutBufferDataGetSampleFrequency(const BufferData * bufferData)
{
  return bufferData->sampleFrequency;
}

/****************************************************************************
 * The utility functions below do not know the internal BufferData
 * representation.
 ****************************************************************************/

ALboolean _alutGetFormat(const BufferData * bufferData, ALenum * format)
{
  if (!_alutFormatConstruct(getNumChannels(bufferData), getBitsPerSample(bufferData), format))
  {
    _alutSetError(ALUT_ERROR_UNSUPPORTED_FILE_SUBTYPE);
    return AL_FALSE;
  }
  return AL_TRUE;
}

static ALuint generateBuffer(void)
{
  ALuint buffer;

  alGenBuffers(1, &buffer);
  if (alGetError() != AL_NO_ERROR)
  {
    _alutSetError(ALUT_ERROR_GEN_BUFFERS);
    return AL_NONE;
  }
  return buffer;
}

static ALboolean passBufferData(BufferData * bufferData, ALuint bid)
{
  ALenum format;
  size_t size;
  ALfloat frequency;

  if (!_alutGetFormat(bufferData, &format))
  {
    return AL_FALSE;
  }
  /* GCC is a bit picky about casting function calls, so we do it in two
   * steps... */
  size = _alutBufferDataGetLength(bufferData);
  frequency = _alutBufferDataGetSampleFrequency(bufferData);
  alBufferData(bid, format, _alutBufferDataGetData(bufferData), (ALsizei) size, (ALsizei) frequency);
  if (alGetError() != AL_NO_ERROR)
  {
    _alutSetError(ALUT_ERROR_BUFFER_DATA);
    return AL_FALSE;
  }
  return AL_TRUE;
}

ALuint _alutPassBufferData(BufferData * bufferData)
{
  ALuint buffer = generateBuffer();

  if (buffer == AL_NONE)
  {
    return AL_NONE;
  }

  if (!passBufferData(bufferData, buffer))
  {
    return AL_NONE;
  }

  return buffer;
}
