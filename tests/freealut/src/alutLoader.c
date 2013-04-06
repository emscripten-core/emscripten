#include "alutInternal.h"
#include <ctype.h>

/****************************************************************************/

typedef enum
{
  LittleEndian,
  BigEndian,
  UnknwonEndian                 /* has anybody still a PDP11? :-) */
} Endianess;

/* test from Harbison & Steele, "C - A Reference Manual", section 6.1.2 */
static Endianess endianess(void)
{
  union
  {
    long l;
    char c[sizeof(long)];
  } u;

  u.l = 1;
  return (u.c[0] == 1) ? LittleEndian : ((u.c[sizeof(long) - 1] == 1) ? BigEndian : UnknwonEndian);
}

/****************************************************************************/

static int safeToLower(int c)
{
  return isupper(c) ? tolower(c) : c;
}

static int hasSuffixIgnoringCase(const char *string, const char *suffix)
{
  const char *stringPointer = string;
  const char *suffixPointer = suffix;

  if (suffix[0] == '\0')
  {
    return 1;
  }

  while (*stringPointer != '\0')
  {
    stringPointer++;
  }

  while (*suffixPointer != '\0')
  {
    suffixPointer++;
  }

  if (stringPointer - string < suffixPointer - suffix)
  {
    return 0;
  }

  while (safeToLower(*--suffixPointer) == safeToLower(*--stringPointer))
  {
    if (suffixPointer == suffix)
    {
      return 1;
    }
  }

  return 0;
}

static BufferData *loadWavFile(InputStream * stream)
{
  ALboolean found_header = AL_FALSE;
  UInt32LittleEndian chunkLength;
  Int32BigEndian magic;
  UInt16LittleEndian audioFormat;
  UInt16LittleEndian numChannels;
  UInt32LittleEndian sampleFrequency;
  UInt32LittleEndian byteRate;
  UInt16LittleEndian blockAlign;
  UInt16LittleEndian bitsPerSample;
  Codec *codec = _alutCodecLinear;

  if (!_alutInputStreamReadUInt32LE(stream, &chunkLength) || !_alutInputStreamReadInt32BE(stream, &magic))
  {
    return NULL;
  }

  if (magic != 0x57415645)      /* "WAVE" */
  {
    _alutSetError(ALUT_ERROR_UNSUPPORTED_FILE_SUBTYPE);
    return NULL;
  }

  while (1)
  {
    if (!_alutInputStreamReadInt32BE(stream, &magic) || !_alutInputStreamReadUInt32LE(stream, &chunkLength))
    {
      return NULL;
    }

    if (magic == 0x666d7420)    /* "fmt " */
    {
      found_header = AL_TRUE;

      if (chunkLength < 16)
      {
        _alutSetError(ALUT_ERROR_CORRUPT_OR_TRUNCATED_DATA);
        return NULL;
      }

      if (!_alutInputStreamReadUInt16LE(stream, &audioFormat) ||
          !_alutInputStreamReadUInt16LE(stream, &numChannels) ||
          !_alutInputStreamReadUInt32LE(stream, &sampleFrequency) ||
          !_alutInputStreamReadUInt32LE(stream, &byteRate) ||
          !_alutInputStreamReadUInt16LE(stream, &blockAlign) || !_alutInputStreamReadUInt16LE(stream, &bitsPerSample))
      {
        return NULL;
      }

      if (!_alutInputStreamSkip(stream, chunkLength - 16))
      {
        return NULL;
      }

      switch (audioFormat)
      {
      case 1:                  /* PCM */
        codec = (bitsPerSample == 8 || endianess() == LittleEndian) ? _alutCodecLinear : _alutCodecPCM16;
        break;
      case 7:                  /* uLaw */
        bitsPerSample *= 2;     /* ToDo: ??? */
        codec = _alutCodecULaw;
        break;
      default:
        _alutSetError(ALUT_ERROR_UNSUPPORTED_FILE_SUBTYPE);
        return NULL;
      }
    }
    else if (magic == 0x64617461) /* "data" */
    {
      ALvoid *data;

      if (!found_header)
      {
        /* ToDo: A bit wrong to check here, fmt chunk could come later... */
        _alutSetError(ALUT_ERROR_CORRUPT_OR_TRUNCATED_DATA);
        return NULL;
      }
      data = _alutInputStreamRead(stream, chunkLength);
      if (data == NULL)
      {
        return NULL;
      }
      return codec(data, chunkLength, numChannels, bitsPerSample, (ALfloat) sampleFrequency);
    }
    else
    {
      if (!_alutInputStreamSkip(stream, chunkLength))
      {
        return NULL;
      }
    }

    if ((chunkLength & 1) && !_alutInputStreamEOF(stream) && !_alutInputStreamSkip(stream, 1))
    {
      return NULL;
    }
  }
}

static BufferData *loadAUFile(InputStream * stream)
{
  Int32BigEndian dataOffset;    /* byte offset to data part, minimum 24 */
  Int32BigEndian len;           /* number of bytes in the data part, -1 = not known */
  Int32BigEndian encoding;      /* encoding of the data part, see AUEncoding */
  Int32BigEndian sampleFrequency; /* number of samples per second */
  Int32BigEndian numChannels;   /* number of interleaved channels */
  size_t length;
  Codec *codec;
  char *data;
  ALint bitsPerSample;

  if (!_alutInputStreamReadInt32BE(stream, &dataOffset) ||
      !_alutInputStreamReadInt32BE(stream, &len) ||
      !_alutInputStreamReadInt32BE(stream, &encoding) ||
      !_alutInputStreamReadInt32BE(stream, &sampleFrequency) || !_alutInputStreamReadInt32BE(stream, &numChannels))
  {
    return AL_FALSE;
  }

  length = (len == -1) ? (_alutInputStreamGetRemainingLength(stream) - AU_HEADER_SIZE - dataOffset) : (size_t) len;

  if (!(dataOffset >= AU_HEADER_SIZE && length > 0 && sampleFrequency >= 1 && numChannels >= 1))
  {
    _alutSetError(ALUT_ERROR_CORRUPT_OR_TRUNCATED_DATA);
    return AL_FALSE;
  }

  if (!_alutInputStreamSkip(stream, dataOffset - AU_HEADER_SIZE))
  {
    return AL_FALSE;
  }

  switch (encoding)
  {
  case AU_ULAW_8:
    bitsPerSample = 16;
    codec = _alutCodecULaw;
    break;
  case AU_PCM_8:
    bitsPerSample = 8;
    codec = _alutCodecPCM8s;
    break;
  case AU_PCM_16:
    bitsPerSample = 16;
    codec = (endianess() == BigEndian) ? _alutCodecLinear : _alutCodecPCM16;
    break;
  case AU_ALAW_8:
    bitsPerSample = 16;
    codec = _alutCodecALaw;
    break;
  default:
    _alutSetError(ALUT_ERROR_UNSUPPORTED_FILE_SUBTYPE);
    return AL_FALSE;
  }

  data = _alutInputStreamRead(stream, length);
  if (data == NULL)
  {
    return NULL;
  }
  return codec(data, length, numChannels, bitsPerSample, (ALfloat) sampleFrequency);
}

static BufferData *loadRawFile(InputStream * stream)
{
  size_t length = _alutInputStreamGetRemainingLength(stream);
  ALvoid *data = _alutInputStreamRead(stream, length);

  if (data == NULL)
  {
    return NULL;
  }
  /* Guesses */
  return _alutCodecLinear(data, length, 1, 8, 8000);
}

static BufferData *loadFile(InputStream * stream)
{
  const char *fileName;
  Int32BigEndian magic;

  /* Raw files have no magic number - so use the fileName extension */

  fileName = _alutInputStreamGetFileName(stream);
  if (fileName != NULL && hasSuffixIgnoringCase(fileName, ".raw"))
  {
    return loadRawFile(stream);
  }

  /* For other file formats, read the quasi-standard four byte magic number */
  if (!_alutInputStreamReadInt32BE(stream, &magic))
  {
    return AL_FALSE;
  }

  /* Magic number 'RIFF' == Microsoft '.wav' format */
  if (magic == 0x52494646)
  {
    return loadWavFile(stream);
  }

  /* Magic number '.snd' == Sun & Next's '.au' format */
  if (magic == 0x2E736E64)
  {
    return loadAUFile(stream);
  }

  _alutSetError(ALUT_ERROR_UNSUPPORTED_FILE_TYPE);
  return AL_FALSE;
}

ALuint _alutCreateBufferFromInputStream(InputStream * stream)
{
  BufferData *bufferData;
  ALuint buffer;

  if (stream == NULL)
  {
    return AL_NONE;
  }

  bufferData = loadFile(stream);
  _alutInputStreamDestroy(stream);
  if (bufferData == NULL)
  {
    return AL_NONE;
  }

  buffer = _alutPassBufferData(bufferData);
  _alutBufferDataDestroy(bufferData);

  return buffer;
}

ALuint alutCreateBufferFromFile(const char *fileName)
{
  InputStream *stream;

  if (!_alutSanityCheck())
  {
    return AL_NONE;
  }
  stream = _alutInputStreamConstructFromFile(fileName);
  return _alutCreateBufferFromInputStream(stream);
}

ALuint alutCreateBufferFromFileImage(const ALvoid * data, ALsizei length)
{
  InputStream *stream;

  if (!_alutSanityCheck())
  {
    return AL_NONE;
  }
  stream = _alutInputStreamConstructFromMemory(data, length);
  return _alutCreateBufferFromInputStream(stream);
}

void *_alutLoadMemoryFromInputStream(InputStream * stream, ALenum * format, ALsizei * size, ALfloat * frequency)
{
  BufferData *bufferData;
  ALenum fmt;
  void *data;

  if (stream == NULL)
  {
    return NULL;
  }

  bufferData = loadFile(stream);
  if (bufferData == NULL)
  {
    _alutInputStreamDestroy(stream);
    return NULL;
  }
  _alutInputStreamDestroy(stream);

  if (!_alutGetFormat(bufferData, &fmt))
  {
    _alutBufferDataDestroy(bufferData);
    return NULL;
  }

  if (size != NULL)
  {
    *size = (ALsizei) _alutBufferDataGetLength(bufferData);
  }

  if (format != NULL)
  {
    *format = fmt;
  }

  if (frequency != NULL)
  {
    *frequency = _alutBufferDataGetSampleFrequency(bufferData);
  }

  data = _alutBufferDataGetData(bufferData);
  _alutBufferDataDetachData(bufferData);
  _alutBufferDataDestroy(bufferData);
  return data;
}

ALvoid *alutLoadMemoryFromFile(const char *fileName, ALenum * format, ALsizei * size, ALfloat * frequency)
{
  InputStream *stream;

  if (!_alutSanityCheck())
  {
    return NULL;
  }
  stream = _alutInputStreamConstructFromFile(fileName);
  return _alutLoadMemoryFromInputStream(stream, format, size, frequency);
}

ALvoid *alutLoadMemoryFromFileImage(const ALvoid * data, ALsizei length, ALenum * format, ALsizei * size, ALfloat * frequency)
{
  InputStream *stream;

  if (!_alutSanityCheck())
  {
    return NULL;
  }
  stream = _alutInputStreamConstructFromMemory(data, length);
  return _alutLoadMemoryFromInputStream(stream, format, size, frequency);
}

/*
  Yukky backwards compatibility crap.
*/

void alutLoadWAVFile(ALbyte * fileName, ALenum * format, void **data, ALsizei * size, ALsizei * frequency
#if !defined(__APPLE__)
                     , ALboolean * loop
#endif
  )
{
  InputStream *stream;
  ALfloat freq;

  /* Don't do an _alutSanityCheck () because it's not required in ALUT 0.x.x */

  stream = _alutInputStreamConstructFromFile(fileName);
  *data = _alutLoadMemoryFromInputStream(stream, format, size, &freq);
  if (*data == NULL)
  {
    return;
  }

  if (frequency)
  {
    *frequency = (ALsizei) freq;
  }

#if !defined(__APPLE__)
  if (loop)
  {
    *loop = AL_FALSE;
  }
#endif
}

void alutLoadWAVMemory(ALbyte * buffer, ALenum * format, void **data, ALsizei * size, ALsizei * frequency
#if !defined(__APPLE__)
                       , ALboolean * loop
#endif
  )
{
  InputStream *stream;
  ALfloat freq;

  /* Don't do an _alutSanityCheck () because it's not required in ALUT 0.x.x */

  /* ToDo: Can we do something less insane than passing 0x7FFFFFFF? */
  stream = _alutInputStreamConstructFromMemory(buffer, 0x7FFFFFFF);
  _alutLoadMemoryFromInputStream(stream, format, size, &freq);
  if (*data == NULL)
  {
    return;
  }

  if (frequency)
  {
    *frequency = (ALsizei) freq;
  }

#if !defined(__APPLE__)
  if (loop)
  {
    *loop = AL_FALSE;
  }
#endif
}

void alutUnloadWAV(ALenum UNUSED(format), ALvoid * data, ALsizei UNUSED(size), ALsizei UNUSED(frequency))
{
  /* Don't do an _alutSanityCheck () because it's not required in ALUT 0.x.x */

  free(data);
}

const char *alutGetMIMETypes(ALenum loader)
{
  if (!_alutSanityCheck())
  {
    return NULL;
  }

  /* We do not distinguish the loaders yet... */
  switch (loader)
  {
  case ALUT_LOADER_BUFFER:
    return "audio/basic,audio/x-raw,audio/x-wav";

  case ALUT_LOADER_MEMORY:
    return "audio/basic,audio/x-raw,audio/x-wav";

  default:
    _alutSetError(ALUT_ERROR_INVALID_ENUM);
    return NULL;
  }
}
