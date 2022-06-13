#include "alutInternal.h"
#include <string.h>

struct OutputStream_struct
{
  char *data;
  char *current;
  size_t maximumLength;
};

/****************************************************************************
 * The functions below know the internal OutputStream representation.
 ****************************************************************************/

OutputStream *_alutOutputStreamConstruct(size_t maximumLength)
{
  OutputStream *stream = (OutputStream *) _alutMalloc(sizeof(OutputStream));

  if (stream == NULL)
  {
    return NULL;
  }
  stream->data = _alutMalloc(maximumLength);
  if (stream->data == NULL)
  {
    free(stream);
    return NULL;
  }
  stream->current = stream->data;
  stream->maximumLength = maximumLength;
  return stream;
}

ALboolean _alutOutputStreamDestroy(OutputStream * stream)
{
  free(stream->data);
  free(stream);
  return AL_TRUE;
}

void *_alutOutputStreamGetData(OutputStream * stream)
{
  return stream->data;
}

size_t _alutOutputStreamGetLength(OutputStream * stream)
{
  return stream->current - stream->data;
}

static ALboolean streamWrite(OutputStream * stream, const void *ptr, size_t numBytesToWrite)
{
  size_t remainingLength = stream->maximumLength - _alutOutputStreamGetLength(stream);

  if (remainingLength < numBytesToWrite)
  {
    /* this should never happen within our library */
    _alutSetError(ALUT_ERROR_IO_ERROR);
    return AL_FALSE;
  }
  memcpy(stream->current, ptr, numBytesToWrite);
  stream->current += numBytesToWrite;
  return AL_TRUE;
}

/****************************************************************************
 * The utility functions below do not know the internal OutputStream
 * representation.
 ****************************************************************************/

ALboolean _alutOutputStreamWriteInt16BE(OutputStream * stream, Int16BigEndian value)
{
  unsigned char buf[2];

  buf[0] = (unsigned char)(value >> 8);
  buf[1] = (unsigned char)value;
  return streamWrite(stream, buf, 2);
}

ALboolean _alutOutputStreamWriteInt32BE(OutputStream * stream, Int32BigEndian value)
{
  unsigned char buf[4];

  buf[0] = (unsigned char)(value >> 24);
  buf[1] = (unsigned char)(value >> 16);
  buf[2] = (unsigned char)(value >> 8);
  buf[3] = (unsigned char)value;
  return streamWrite(stream, buf, 4);
}
