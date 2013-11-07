#ifndef OPENAL_ALC_H__
#define OPENAL_ALC_H__

#ifdef __cplusplus
extern "C" {
#endif

#define ALCAPI ALC_API
#define ALCAPIENTRY ALC_APIENTRY
#define ALC_ALL_ATTRIBUTES 0x1003
#define ALC_ALL_DEVICES_SPECIFIER 0x1013
#define ALC_ATTRIBUTES_SIZE 0x1002
#define ALC_CAPTURE_DEFAULT_DEVICE_SPECIFIER 0x311
#define ALC_CAPTURE_DEVICE_SPECIFIER 0x310
#define ALC_CAPTURE_SAMPLES 0x312
#define ALC_DEFAULT_ALL_DEVICES_SPECIFIER 0x1012
#define ALC_DEFAULT_DEVICE_SPECIFIER 0x1004
#define ALC_DEVICE_SPECIFIER 0x1005
#define ALC_ENUMERATE_ALL_EXT 1
#define ALC_EXTENSIONS 0x1006
#define ALC_EXT_CAPTURE 1
#define ALC_FALSE 0
#define ALC_FREQUENCY 0x1007
#define ALC_INVALID 0
#define ALC_INVALID_CONTEXT 0xA002
#define ALC_INVALID_DEVICE 0xA001
#define ALC_INVALID_ENUM 0xA003
#define ALC_INVALID_VALUE 0xA004
#define ALC_MAJOR_VERSION 0x1000
#define ALC_MINOR_VERSION 0x1001
#define ALC_MONO_SOURCES 0x1010
#define ALC_NO_ERROR 0
#define ALC_OUT_OF_MEMORY 0xA005
#define ALC_REFRESH 0x1008
#define ALC_STEREO_SOURCES 0x1011
#define ALC_SYNC 0x1009
#define ALC_TRUE 1
#define ALC_VERSION_0_1 1
#define AL_ALC_H

struct ALCcontext_struct;
struct ALCdevice_struct;
typedef char ALCboolean;
typedef char ALCchar;
typedef double ALCdouble;
typedef float ALCfloat;
typedef int ALCenum;
typedef int ALCint;
typedef int ALCsizei;
typedef short ALCshort;
typedef signed char ALCbyte;
typedef struct ALCcontext_struct ALCcontext;
typedef struct ALCdevice_struct ALCdevice;
typedef unsigned char ALCubyte;
typedef unsigned int ALCuint;
typedef unsigned short ALCushort;
typedef void ALCvoid;

extern ALCboolean alcCaptureCloseDevice(ALCdevice *device);
extern ALCboolean alcCloseDevice(ALCdevice *device);
extern ALCboolean alcIsExtensionPresent(ALCdevice *device, const ALCchar *extname);
extern ALCboolean alcMakeContextCurrent(ALCcontext *context);
extern ALCcontext *alcCreateContext(ALCdevice *device, const ALCint *attrlist);
extern ALCcontext *alcGetCurrentContext();
extern ALCdevice *alcCaptureOpenDevice(const ALCchar *devicename, ALCuint frequency, ALCenum format, ALCsizei buffersize);
extern ALCdevice *alcGetContextsDevice(ALCcontext *context);
extern ALCdevice *alcOpenDevice(const ALCchar *devicename);
extern ALCenum alcGetEnumValue(ALCdevice *device, const ALCchar *enumname);
extern ALCenum alcGetError(ALCdevice *device);
extern const ALCchar *alcGetString(ALCdevice *device, ALCenum param);
extern void *alcGetProcAddress(ALCdevice *device, const ALCchar *funcname);
extern void alcCaptureSamples(ALCdevice *device, ALCvoid *buffer, ALCsizei samples);
extern void alcCaptureStart(ALCdevice *device);
extern void alcCaptureStop(ALCdevice *device);
extern void alcDestroyContext(ALCcontext *context);
extern void alcGetIntegerv(ALCdevice *device, ALCenum param, ALCsizei size, ALCint *values);
extern void alcProcessContext(ALCcontext *context);
extern void alcSuspendContext(ALCcontext *context);

#ifdef __cplusplus
}
#endif

#endif
