#ifndef OPENAL_ALEXT_H__
#define OPENAL_ALEXT_H__

#include "alc.h"
#include "al.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef ALC_SOFT_pause_device
#define ALC_SOFT_pause_device
#endif // ALC_SOFT_pause_device

#ifndef ALC_SOFT_HRTF
#define ALC_SOFT_HRTF
#define ALC_HRTF_SOFT                           0x1992
#define ALC_DONT_CARE_SOFT                      0x0002
#define ALC_HRTF_STATUS_SOFT                    0x1993
#define ALC_HRTF_DISABLED_SOFT                  0x0000
#define ALC_HRTF_ENABLED_SOFT                   0x0001
#define ALC_HRTF_DENIED_SOFT                    0x0002
#define ALC_HRTF_REQUIRED_SOFT                  0x0003
#define ALC_HRTF_HEADPHONES_DETECTED_SOFT       0x0004
#define ALC_HRTF_UNSUPPORTED_FORMAT_SOFT        0x0005
#define ALC_NUM_HRTF_SPECIFIERS_SOFT            0x1994
#define ALC_HRTF_SPECIFIER_SOFT                 0x1995
#define ALC_HRTF_ID_SOFT                        0x1996
#endif // ALC_SOFT_HRTF

#ifndef AL_EXT_float32
#define AL_EXT_float32
#define AL_FORMAT_MONO_FLOAT32                  0x10010
#define AL_FORMAT_STEREO_FLOAT32                0x10011
#endif // AL_EXT_float32

#ifndef AL_SOFT_loop_points
#define AL_SOFT_loop_points
#define AL_LOOP_POINTS_SOFT                     0x2015
#endif // AL_SOFT_loop_points

#ifndef AL_SOFT_source_length
#define AL_SOFT_source_length
#endif // AL_SOFT_source_length

#ifndef AL_EXT_source_distance_model
#define AL_EXT_source_distance_model
#define AL_SOURCE_DISTANCE_MODEL                0x200
#endif // AL_EXT_source_distance_model

#ifndef AL_SOFT_source_spatialize
#define AL_SOFT_source_spatialize
#define AL_SOURCE_SPATIALIZE_SOFT               0x1214
#define AL_AUTO_SOFT                            0x0002
#endif // AL_SOFT_source_spatialize

#ifdef __cplusplus
}
#endif

#endif // OPENAL_ALEXT_H__
