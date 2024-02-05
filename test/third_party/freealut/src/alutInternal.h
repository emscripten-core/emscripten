/*
 * This file should be #included as the first header in all *.c files.
 */

#if !defined(ALUT_INTERNAL_H)
#define ALUT_INTERNAL_H

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>

#if HAVE_STDINT_H
#include <stdint.h>
#elif _MSC_VER < 1300
typedef char int8_t;
typedef unsigned char uint8_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef int int32_t;
typedef unsigned int uint32_t;
#elif HAVE_BASETSD_H
#include <basetsd.h>
typedef INT8 int8_t;
typedef UINT8 uint8_t;
typedef INT16 int16_t;
typedef UINT16 uint16_t;
typedef INT32 int32_t;
typedef UINT32 uint32_t;
#else
#error Do not know sized types on this platform
#endif

typedef int16_t Int16BigEndian;
typedef uint16_t UInt16LittleEndian;
typedef int32_t Int32BigEndian;
typedef uint32_t UInt32LittleEndian;

#if HAVE___ATTRIBUTE__
#define UNUSED(x) x __attribute__((unused))
#else
#define UNUSED(x) x
#endif

#include <AL/alut.h>

#define AU_HEADER_SIZE 24

/* see: http://en.wikipedia.org/wiki/Au_file_format, G.72x are missing */
enum AUEncoding
{
  AU_ULAW_8 = 1,                /* 8-bit ISDN u-law */
  AU_PCM_8 = 2,                 /* 8-bit linear PCM (signed) */
  AU_PCM_16 = 3,                /* 16-bit linear PCM (signed, big-endian) */
  AU_PCM_24 = 4,                /* 24-bit linear PCM */
  AU_PCM_32 = 5,                /* 32-bit linear PCM */
  AU_FLOAT_32 = 6,              /* 32-bit IEEE floating point */
  AU_FLOAT_64 = 7,              /* 64-bit IEEE floating point */
  AU_ALAW_8 = 27                /* 8-bit ISDN a-law */
};

/* in alutCodec.c */
typedef ALvoid *Codec(ALvoid * data, size_t length, ALint numChannels, ALint bitsPerSample, ALfloat sampleFrequency);
extern Codec _alutCodecLinear;
extern Codec _alutCodecPCM8s;
extern Codec _alutCodecPCM16;
extern Codec _alutCodecULaw;
extern Codec _alutCodecALaw;

/* in alutError.c */
extern void _alutSetError(ALenum err);

/* in alutInit.c */
extern ALboolean _alutSanityCheck(void);

/* in alutInputStream.c */
typedef struct InputStream_struct InputStream;
extern InputStream *_alutInputStreamConstructFromFile(const char *fileName);
extern InputStream *_alutInputStreamConstructFromMemory(const ALvoid * data, size_t length);
extern const char *_alutInputStreamGetFileName(const InputStream * stream);
extern size_t _alutInputStreamGetRemainingLength(const InputStream * stream);
extern ALboolean _alutInputStreamDestroy(InputStream * stream);
extern ALboolean _alutInputStreamEOF(InputStream * stream);
extern ALvoid *_alutInputStreamRead(InputStream * stream, size_t length);
extern ALboolean _alutInputStreamSkip(InputStream * stream, size_t numBytesToSkip);
extern ALboolean _alutInputStreamReadUInt16LE(InputStream * stream, UInt16LittleEndian * value);
extern ALboolean _alutInputStreamReadInt32BE(InputStream * stream, Int32BigEndian * value);
extern ALboolean _alutInputStreamReadUInt32LE(InputStream * stream, UInt32LittleEndian * value);

/* in alutLoader.c */
extern ALuint _alutCreateBufferFromInputStream(InputStream * stream);
extern void *_alutLoadMemoryFromInputStream(InputStream * stream, ALenum * format, ALsizei * size, ALfloat * frequency);

/* in alutOutputStream.c */
typedef struct OutputStream_struct OutputStream;
extern OutputStream *_alutOutputStreamConstruct(size_t maximumLength);
extern ALboolean _alutOutputStreamDestroy(OutputStream * stream);
extern void *_alutOutputStreamGetData(OutputStream * stream);
extern size_t _alutOutputStreamGetLength(OutputStream * stream);
extern ALboolean _alutOutputStreamWriteInt16BE(OutputStream * stream, Int16BigEndian value);
extern ALboolean _alutOutputStreamWriteInt32BE(OutputStream * stream, Int32BigEndian value);

/* in alutUtil.c */
extern ALvoid *_alutMalloc(size_t size);
extern ALboolean _alutFormatConstruct(ALint numChannels, ALint bitsPerSample, ALenum * format);
extern ALboolean _alutFormatGetNumChannels(ALenum format, ALint * numChannels);
extern ALboolean _alutFormatGetBitsPerSample(ALenum format, ALint * bitsPerSample);

/* in alutWaveform.c */
typedef struct BufferData_struct BufferData;
extern BufferData *_alutBufferDataConstruct(ALvoid * data, size_t length, ALint numChannels, ALint bitsPerSample, ALfloat sampleFrequency);
extern ALboolean _alutBufferDataDestroy(BufferData * bufferData);
extern void _alutBufferDataDetachData(BufferData * bufferData);
extern ALvoid *_alutBufferDataGetData(const BufferData * bufferData);
extern size_t _alutBufferDataGetLength(const BufferData * bufferData);
extern ALfloat _alutBufferDataGetSampleFrequency(const BufferData * bufferData);
extern ALboolean _alutGetFormat(const BufferData * bufferData, ALenum * format);
extern ALuint _alutPassBufferData(BufferData * bufferData);

#endif /* not ALUT_INTERNAL_H */
