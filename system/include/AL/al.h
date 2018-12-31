#ifndef OPENAL_AL_H__
#define OPENAL_AL_H__

#ifdef __cplusplus
extern "C" {
#endif

#define AL_BITS 0x2002
#define AL_BUFFER 0x1009
#define AL_BUFFERS_PROCESSED 0x1016
#define AL_BUFFERS_QUEUED 0x1015
#define AL_BYTE_OFFSET 0x1026
#define AL_CHANNELS 0x2003
#define AL_CONE_INNER_ANGLE 0x1001
#define AL_CONE_OUTER_ANGLE 0x1002
#define AL_CONE_OUTER_GAIN 0x1022
#define AL_DIRECTION 0x1005
#define AL_DISTANCE_MODEL 0xD000
#define AL_DOPPLER_FACTOR 0xC000
#define AL_DOPPLER_VELOCITY 0xC001
#define AL_EXPONENT_DISTANCE 0xD005
#define AL_EXPONENT_DISTANCE_CLAMPED 0xD006
#define AL_EXTENSIONS 0xB004
#define AL_FALSE 0
#define AL_FORMAT_MONO16 0x1101
#define AL_FORMAT_MONO8 0x1100
#define AL_FORMAT_STEREO16 0x1103
#define AL_FORMAT_STEREO8 0x1102
#define AL_FREQUENCY 0x2001
#define AL_GAIN 0x100A
#define AL_ILLEGAL_COMMAND AL_INVALID_OPERATION
#define AL_ILLEGAL_ENUM AL_INVALID_ENUM
#define AL_INITIAL 0x1011
#define AL_INVALID (-1)
#define AL_INVALID_ENUM 0xA002
#define AL_INVALID_NAME 0xA001
#define AL_INVALID_OPERATION 0xA004
#define AL_INVALID_VALUE 0xA003
#define AL_INVERSE_DISTANCE 0xD001
#define AL_INVERSE_DISTANCE_CLAMPED 0xD002
#define AL_LINEAR_DISTANCE 0xD003
#define AL_LINEAR_DISTANCE_CLAMPED 0xD004
#define AL_LOOPING 0x1007
#define AL_MAX_DISTANCE 0x1023
#define AL_MAX_GAIN 0x100E
#define AL_MIN_GAIN 0x100D
#define AL_NONE 0
#define AL_NO_ERROR 0
#define AL_ORIENTATION 0x100F
#define AL_OUT_OF_MEMORY 0xA005
#define AL_PAUSED 0x1013
#define AL_PENDING 0x2011
#define AL_PITCH 0x1003
#define AL_PLAYING 0x1012
#define AL_POSITION 0x1004
#define AL_PROCESSED 0x2012
#define AL_REFERENCE_DISTANCE 0x1020
#define AL_RENDERER 0xB003
#define AL_ROLLOFF_FACTOR 0x1021
#define AL_SAMPLE_OFFSET 0x1025
#define AL_SEC_OFFSET 0x1024
#define AL_SIZE 0x2004
#define AL_SOURCE_RELATIVE 0x202
#define AL_SOURCE_STATE 0x1010
#define AL_SOURCE_TYPE 0x1027
#define AL_SPEED_OF_SOUND 0xC003
#define AL_STATIC 0x1028
#define AL_STOPPED 0x1014
#define AL_STREAMING 0x1029
#define AL_TRUE 1
#define AL_UNDETERMINED 0x1030
#define AL_UNUSED 0x2010
#define AL_VELOCITY 0x1006
#define AL_VENDOR 0xB001
#define AL_VERSION 0xB002
#define AL_VERSION_1_0
#define AL_VERSION_1_1
#define OPENAL

typedef char ALboolean;
typedef char ALchar;
typedef double ALdouble;
typedef float ALfloat;
typedef int ALenum;
typedef int ALint;
typedef int ALsizei;
typedef short ALshort;
typedef signed char ALbyte;
typedef unsigned char ALubyte;
typedef unsigned int ALuint;
typedef unsigned short ALushort;
typedef void ALvoid;

extern ALboolean alGetBoolean(ALenum param);
extern ALboolean alIsBuffer(ALuint buffer);
extern ALboolean alIsEnabled(ALenum capability);
extern ALboolean alIsExtensionPresent(const ALchar *extname);
extern ALboolean alIsSource(ALuint source);
extern ALdouble alGetDouble(ALenum param);
extern ALenum alGetEnumValue(const ALchar *ename);
extern ALenum alGetError(void);
extern ALfloat alGetFloat(ALenum param);
extern ALint alGetInteger(ALenum param);
extern const ALchar *alGetString(ALenum param);
extern void *alGetProcAddress(const ALchar *fname);
extern void alBuffer3f(ALuint buffer, ALenum param, ALfloat value1, ALfloat value2, ALfloat value3);
extern void alBuffer3i(ALuint buffer, ALenum param, ALint value1, ALint value2, ALint value3);
extern void alBufferData(ALuint buffer, ALenum format, const ALvoid *data, ALsizei size, ALsizei freq);
extern void alBufferf(ALuint buffer, ALenum param, ALfloat value);
extern void alBufferfv(ALuint buffer, ALenum param, const ALfloat *values);
extern void alBufferi(ALuint buffer, ALenum param, ALint value);
extern void alBufferiv(ALuint buffer, ALenum param, const ALint *values);
extern void alDeleteBuffers(ALsizei n, const ALuint *buffers);
extern void alDeleteSources(ALsizei n, const ALuint *sources);
extern void alDisable(ALenum capability);
extern void alDistanceModel(ALenum distanceModel);
extern void alDopplerFactor(ALfloat value);
extern void alDopplerVelocity(ALfloat value);
extern void alEnable(ALenum capability);
extern void alGenBuffers(ALsizei n, ALuint *buffers);
extern void alGenSources(ALsizei n, ALuint *sources);
extern void alGetBooleanv(ALenum param, ALboolean *values);
extern void alGetBuffer3f(ALuint buffer, ALenum param, ALfloat *value1, ALfloat *value2, ALfloat *value3);
extern void alGetBuffer3i(ALuint buffer, ALenum param, ALint *value1, ALint *value2, ALint *value3);
extern void alGetBufferf(ALuint buffer, ALenum param, ALfloat *value);
extern void alGetBufferfv(ALuint buffer, ALenum param, ALfloat *values);
extern void alGetBufferi(ALuint buffer, ALenum param, ALint *value);
extern void alGetBufferiv(ALuint buffer, ALenum param, ALint *values);
extern void alGetDoublev(ALenum param, ALdouble *values);
extern void alGetFloatv(ALenum param, ALfloat *values);
extern void alGetIntegerv(ALenum param, ALint *values);
extern void alGetListener3f(ALenum param, ALfloat *value1, ALfloat *value2, ALfloat *value3);
extern void alGetListener3i(ALenum param, ALint *value1, ALint *value2, ALint *value3);
extern void alGetListenerf(ALenum param, ALfloat *value);
extern void alGetListenerfv(ALenum param, ALfloat *values);
extern void alGetListeneri(ALenum param, ALint *value);
extern void alGetListeneriv(ALenum param, ALint *values);
extern void alGetSource3f(ALuint source, ALenum param, ALfloat *value1, ALfloat *value2, ALfloat *value3);
extern void alGetSource3i(ALuint source, ALenum param, ALint *value1, ALint *value2, ALint *value3);
extern void alGetSourcef(ALuint source, ALenum param, ALfloat *value);
extern void alGetSourcefv(ALuint source, ALenum param, ALfloat *values);
extern void alGetSourcei(ALuint source, ALenum param, ALint *value);
extern void alGetSourceiv(ALuint source, ALenum param, ALint *values);
extern void alListener3f(ALenum param, ALfloat value1, ALfloat value2, ALfloat value3);
extern void alListener3i(ALenum param, ALint value1, ALint value2, ALint value3);
extern void alListenerf(ALenum param, ALfloat value);
extern void alListenerfv(ALenum param, const ALfloat *values);
extern void alListeneri(ALenum param, ALint value);
extern void alListeneriv(ALenum param, const ALint *values);
extern void alSource3f(ALuint source, ALenum param, ALfloat value1, ALfloat value2, ALfloat value3);
extern void alSource3i(ALuint source, ALenum param, ALint value1, ALint value2, ALint value3);
extern void alSourcePause(ALuint source);
extern void alSourcePausev(ALsizei n, const ALuint *sources);
extern void alSourcePlay(ALuint source);
extern void alSourcePlayv(ALsizei n, const ALuint *sources);
extern void alSourceQueueBuffers(ALuint source, ALsizei nb, const ALuint *buffers);
extern void alSourceRewind(ALuint source);
extern void alSourceRewindv(ALsizei n, const ALuint *sources);
extern void alSourceStop(ALuint source);
extern void alSourceStopv(ALsizei n, const ALuint *sources);
extern void alSourceUnqueueBuffers(ALuint source, ALsizei nb, ALuint *buffers);
extern void alSourcef(ALuint source, ALenum param, ALfloat value);
extern void alSourcefv(ALuint source, ALenum param, const ALfloat *values);
extern void alSourcei(ALuint source, ALenum param, ALint value);
extern void alSourceiv(ALuint source, ALenum param, const ALint *values);
extern void alSpeedOfSound(ALfloat value);

#ifdef __cplusplus
}
#endif

#endif
