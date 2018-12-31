/*
  SDL_mixer:  An audio mixer library based on the SDL library
  Copyright (C) 1997-2012 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

/* $Id$ */

#ifndef _SDL_MIXER_H
#define _SDL_MIXER_H

#include "SDL_types.h"
#include "SDL_rwops.h"
#include "SDL_audio.h"
#include "SDL_endian.h"
#include "SDL_version.h"
#include "begin_code.h"

/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
extern "C" {
#endif

/* Printable format: "%d.%d.%d", MAJOR, MINOR, PATCHLEVEL
*/
#define SDL_MIXER_MAJOR_VERSION	1
#define SDL_MIXER_MINOR_VERSION	2
#define SDL_MIXER_PATCHLEVEL    12

/* This macro can be used to fill a version structure with the compile-time
 * version of the SDL_mixer library.
 */
#define SDL_MIXER_VERSION(X)						\
{									\
	(X)->major = SDL_MIXER_MAJOR_VERSION;				\
	(X)->minor = SDL_MIXER_MINOR_VERSION;				\
	(X)->patch = SDL_MIXER_PATCHLEVEL;				\
}

/* Backwards compatibility */
#define MIX_MAJOR_VERSION	SDL_MIXER_MAJOR_VERSION
#define MIX_MINOR_VERSION	SDL_MIXER_MINOR_VERSION
#define MIX_PATCHLEVEL		SDL_MIXER_PATCHLEVEL
#define MIX_VERSION(X)		SDL_MIXER_VERSION(X)

/* This function gets the version of the dynamically linked SDL_mixer library.
   it should NOT be used to fill a version structure, instead you should
   use the SDL_MIXER_VERSION() macro.
 */
extern DECLSPEC const SDL_version * SDLCALL Mix_Linked_Version(void);

typedef enum
{
    MIX_INIT_FLAC        = 0x00000001,
    MIX_INIT_MOD         = 0x00000002,
    MIX_INIT_MP3         = 0x00000004,
    MIX_INIT_OGG         = 0x00000008,
    MIX_INIT_FLUIDSYNTH  = 0x00000010
} MIX_InitFlags;

/* Loads dynamic libraries and prepares them for use.  Flags should be
   one or more flags from MIX_InitFlags OR'd together.
   It returns the flags successfully initialized, or 0 on failure.
 */
extern DECLSPEC int SDLCALL Mix_Init(int flags);

/* Unloads libraries loaded with Mix_Init */
extern DECLSPEC void SDLCALL Mix_Quit(void);


/* The default mixer has 8 simultaneous mixing channels */
#ifndef MIX_CHANNELS
#define MIX_CHANNELS	8
#endif

/* Good default values for a PC soundcard */
#define MIX_DEFAULT_FREQUENCY	22050
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
#define MIX_DEFAULT_FORMAT	AUDIO_S16LSB
#else
#define MIX_DEFAULT_FORMAT	AUDIO_S16MSB
#endif
#define MIX_DEFAULT_CHANNELS	2
#define MIX_MAX_VOLUME		128	/* Volume of a chunk */

/* The internal format for an audio chunk */
typedef struct Mix_Chunk {
	int allocated;
	Uint8 *abuf;
	Uint32 alen;
	Uint8 volume;		/* Per-sample volume, 0-128 */
} Mix_Chunk;

/* The different fading types supported */
typedef enum {
	MIX_NO_FADING,
	MIX_FADING_OUT,
	MIX_FADING_IN
} Mix_Fading;

typedef enum {
	MUS_NONE,
	MUS_CMD,
	MUS_WAV,
	MUS_MOD,
	MUS_MID,
	MUS_OGG,
	MUS_MP3,
	MUS_MP3_MAD,
	MUS_FLAC,
	MUS_MODPLUG
} Mix_MusicType;

/* The internal format for a music chunk interpreted via mikmod */
typedef struct _Mix_Music Mix_Music;

/* Open the mixer with a certain audio format */
extern DECLSPEC int SDLCALL Mix_OpenAudio(int frequency, Uint16 format, int channels,
							int chunksize);

/* Dynamically change the number of channels managed by the mixer.
   If decreasing the number of channels, the upper channels are
   stopped.
   This function returns the new number of allocated channels.
 */
extern DECLSPEC int SDLCALL Mix_AllocateChannels(int numchans);

/* Find out what the actual audio device parameters are.
   This function returns 1 if the audio has been opened, 0 otherwise.
 */
extern DECLSPEC int SDLCALL Mix_QuerySpec(int *frequency,Uint16 *format,int *channels);

/* Load a wave file or a music (.mod .s3m .it .xm) file */
extern DECLSPEC Mix_Chunk * SDLCALL Mix_LoadWAV_RW(SDL_RWops *src, int freesrc);
extern DECLSPEC Mix_Chunk * SDLCALL Mix_LoadWAV(const char *file);
extern DECLSPEC Mix_Music * SDLCALL Mix_LoadMUS(const char *file);

/* Load a music file from an SDL_RWop object (Ogg and MikMod specific currently)
   Matt Campbell (matt@campbellhome.dhs.org) April 2000 */
extern DECLSPEC Mix_Music * SDLCALL Mix_LoadMUS_RW(SDL_RWops *rw);

/* Load a music file from an SDL_RWop object assuming a specific format */
extern DECLSPEC Mix_Music * SDLCALL Mix_LoadMUSType_RW(SDL_RWops *rw, Mix_MusicType type, int freesrc);

/* Load a wave file of the mixer format from a memory buffer */
extern DECLSPEC Mix_Chunk * SDLCALL Mix_QuickLoad_WAV(Uint8 *mem);

/* Load raw audio data of the mixer format from a memory buffer */
extern DECLSPEC Mix_Chunk * SDLCALL Mix_QuickLoad_RAW(Uint8 *mem, Uint32 len);

/* Free an audio chunk previously loaded */
extern DECLSPEC void SDLCALL Mix_FreeChunk(Mix_Chunk *chunk);
extern DECLSPEC void SDLCALL Mix_FreeMusic(Mix_Music *music);

/* Get a list of chunk/music decoders that this build of SDL_mixer provides.
   This list can change between builds AND runs of the program, if external
   libraries that add functionality become available.
   You must successfully call Mix_OpenAudio() before calling these functions.
   This API is only available in SDL_mixer 1.2.9 and later.

   // usage...
   int i;
   const int total = Mix_GetNumChunkDecoders();
   for (i = 0; i < total; i++)
       printf("Supported chunk decoder: [%s]\n", Mix_GetChunkDecoder(i));

   Appearing in this list doesn't promise your specific audio file will
   decode...but it's handy to know if you have, say, a functioning Timidity
   install.

   These return values are static, read-only data; do not modify or free it.
   The pointers remain valid until you call Mix_CloseAudio().
*/
extern DECLSPEC int SDLCALL Mix_GetNumChunkDecoders(void);
extern DECLSPEC const char * SDLCALL Mix_GetChunkDecoder(int index);
extern DECLSPEC int SDLCALL Mix_GetNumMusicDecoders(void);
extern DECLSPEC const char * SDLCALL Mix_GetMusicDecoder(int index);

/* Find out the music format of a mixer music, or the currently playing
   music, if 'music' is NULL.
*/
extern DECLSPEC Mix_MusicType SDLCALL Mix_GetMusicType(const Mix_Music *music);

/* Set a function that is called after all mixing is performed.
   This can be used to provide real-time visual display of the audio stream
   or add a custom mixer filter for the stream data.
*/
extern DECLSPEC void SDLCALL Mix_SetPostMix(void (*mix_func)
                             (void *udata, Uint8 *stream, int len), void *arg);

/* Add your own music player or additional mixer function.
   If 'mix_func' is NULL, the default music player is re-enabled.
 */
extern DECLSPEC void SDLCALL Mix_HookMusic(void (*mix_func)
                          (void *udata, Uint8 *stream, int len), void *arg);

/* Add your own callback when the music has finished playing.
   This callback is only called if the music finishes naturally.
 */
extern DECLSPEC void SDLCALL Mix_HookMusicFinished(void (*music_finished)(void));

/* Get a pointer to the user data for the current music hook */
extern DECLSPEC void * SDLCALL Mix_GetMusicHookData(void);

/*
 * Add your own callback when a channel has finished playing. NULL
 *  to disable callback. The callback may be called from the mixer's audio 
 *  callback or it could be called as a result of Mix_HaltChannel(), etc.
 *  do not call SDL_LockAudio() from this callback; you will either be 
 *  inside the audio callback, or SDL_mixer will explicitly lock the audio
 *  before calling your callback.
 */
extern DECLSPEC void SDLCALL Mix_ChannelFinished(void (*channel_finished)(int channel));


/* Special Effects API by ryan c. gordon. (icculus@icculus.org) */

#define MIX_CHANNEL_POST  -2

/* This is the format of a special effect callback:
 *
 *   myeffect(int chan, void *stream, int len, void *udata);
 *
 * (chan) is the channel number that your effect is affecting. (stream) is
 *  the buffer of data to work upon. (len) is the size of (stream), and
 *  (udata) is a user-defined bit of data, which you pass as the last arg of
 *  Mix_RegisterEffect(), and is passed back unmolested to your callback.
 *  Your effect changes the contents of (stream) based on whatever parameters
 *  are significant, or just leaves it be, if you prefer. You can do whatever
 *  you like to the buffer, though, and it will continue in its changed state
 *  down the mixing pipeline, through any other effect functions, then finally
 *  to be mixed with the rest of the channels and music for the final output
 *  stream.
 *
 * DO NOT EVER call SDL_LockAudio() from your callback function!
 */
typedef void (*Mix_EffectFunc_t)(int chan, void *stream, int len, void *udata);

/*
 * This is a callback that signifies that a channel has finished all its
 *  loops and has completed playback. This gets called if the buffer
 *  plays out normally, or if you call Mix_HaltChannel(), implicitly stop
 *  a channel via Mix_AllocateChannels(), or unregister a callback while
 *  it's still playing.
 *
 * DO NOT EVER call SDL_LockAudio() from your callback function!
 */
typedef void (*Mix_EffectDone_t)(int chan, void *udata);


/* Register a special effect function. At mixing time, the channel data is
 *  copied into a buffer and passed through each registered effect function.
 *  After it passes through all the functions, it is mixed into the final
 *  output stream. The copy to buffer is performed once, then each effect
 *  function performs on the output of the previous effect. Understand that
 *  this extra copy to a buffer is not performed if there are no effects
 *  registered for a given chunk, which saves CPU cycles, and any given
 *  effect will be extra cycles, too, so it is crucial that your code run
 *  fast. Also note that the data that your function is given is in the
 *  format of the sound device, and not the format you gave to Mix_OpenAudio(),
 *  although they may in reality be the same. This is an unfortunate but
 *  necessary speed concern. Use Mix_QuerySpec() to determine if you can
 *  handle the data before you register your effect, and take appropriate
 *  actions.
 * You may also specify a callback (Mix_EffectDone_t) that is called when
 *  the channel finishes playing. This gives you a more fine-grained control
 *  than Mix_ChannelFinished(), in case you need to free effect-specific
 *  resources, etc. If you don't need this, you can specify NULL.
 * You may set the callbacks before or after calling Mix_PlayChannel().
 * Things like Mix_SetPanning() are just internal special effect functions,
 *  so if you are using that, you've already incurred the overhead of a copy
 *  to a separate buffer, and that these effects will be in the queue with
 *  any functions you've registered. The list of registered effects for a
 *  channel is reset when a chunk finishes playing, so you need to explicitly
 *  set them with each call to Mix_PlayChannel*().
 * You may also register a special effect function that is to be run after
 *  final mixing occurs. The rules for these callbacks are identical to those
 *  in Mix_RegisterEffect, but they are run after all the channels and the
 *  music have been mixed into a single stream, whereas channel-specific
 *  effects run on a given channel before any other mixing occurs. These
 *  global effect callbacks are call "posteffects". Posteffects only have
 *  their Mix_EffectDone_t function called when they are unregistered (since
 *  the main output stream is never "done" in the same sense as a channel).
 *  You must unregister them manually when you've had enough. Your callback
 *  will be told that the channel being mixed is (MIX_CHANNEL_POST) if the
 *  processing is considered a posteffect.
 *
 * After all these effects have finished processing, the callback registered
 *  through Mix_SetPostMix() runs, and then the stream goes to the audio
 *  device. 
 *
 * DO NOT EVER call SDL_LockAudio() from your callback function!
 *
 * returns zero if error (no such channel), nonzero if added.
 *  Error messages can be retrieved from Mix_GetError().
 */
extern DECLSPEC int SDLCALL Mix_RegisterEffect(int chan, Mix_EffectFunc_t f,
					Mix_EffectDone_t d, void *arg);


/* You may not need to call this explicitly, unless you need to stop an
 *  effect from processing in the middle of a chunk's playback.
 * Posteffects are never implicitly unregistered as they are for channels,
 *  but they may be explicitly unregistered through this function by
 *  specifying MIX_CHANNEL_POST for a channel.
 * returns zero if error (no such channel or effect), nonzero if removed.
 *  Error messages can be retrieved from Mix_GetError().
 */
extern DECLSPEC int SDLCALL Mix_UnregisterEffect(int channel, Mix_EffectFunc_t f);


/* You may not need to call this explicitly, unless you need to stop all
 *  effects from processing in the middle of a chunk's playback. Note that
 *  this will also shut off some internal effect processing, since
 *  Mix_SetPanning() and others may use this API under the hood. This is
 *  called internally when a channel completes playback.
 * Posteffects are never implicitly unregistered as they are for channels,
 *  but they may be explicitly unregistered through this function by
 *  specifying MIX_CHANNEL_POST for a channel.
 * returns zero if error (no such channel), nonzero if all effects removed.
 *  Error messages can be retrieved from Mix_GetError().
 */
extern DECLSPEC int SDLCALL Mix_UnregisterAllEffects(int channel);


#define MIX_EFFECTSMAXSPEED  "MIX_EFFECTSMAXSPEED"

/*
 * These are the internally-defined mixing effects. They use the same API that
 *  effects defined in the application use, but are provided here as a
 *  convenience. Some effects can reduce their quality or use more memory in
 *  the name of speed; to enable this, make sure the environment variable
 *  MIX_EFFECTSMAXSPEED (see above) is defined before you call
 *  Mix_OpenAudio().
 */


/* Set the panning of a channel. The left and right channels are specified
 *  as integers between 0 and 255, quietest to loudest, respectively.
 *
 * Technically, this is just individual volume control for a sample with
 *  two (stereo) channels, so it can be used for more than just panning.
 *  If you want real panning, call it like this:
 *
 *   Mix_SetPanning(channel, left, 255 - left);
 *
 * ...which isn't so hard.
 *
 * Setting (channel) to MIX_CHANNEL_POST registers this as a posteffect, and
 *  the panning will be done to the final mixed stream before passing it on
 *  to the audio device.
 *
 * This uses the Mix_RegisterEffect() API internally, and returns without
 *  registering the effect function if the audio device is not configured
 *  for stereo output. Setting both (left) and (right) to 255 causes this
 *  effect to be unregistered, since that is the data's normal state.
 *
 * returns zero if error (no such channel or Mix_RegisterEffect() fails),
 *  nonzero if panning effect enabled. Note that an audio device in mono
 *  mode is a no-op, but this call will return successful in that case.
 *  Error messages can be retrieved from Mix_GetError().
 */
extern DECLSPEC int SDLCALL Mix_SetPanning(int channel, Uint8 left, Uint8 right);


/* Set the position of a channel. (angle) is an integer from 0 to 360, that
 *  specifies the location of the sound in relation to the listener. (angle)
 *  will be reduced as neccesary (540 becomes 180 degrees, -100 becomes 260).
 *  Angle 0 is due north, and rotates clockwise as the value increases.
 *  For efficiency, the precision of this effect may be limited (angles 1
 *  through 7 might all produce the same effect, 8 through 15 are equal, etc).
 *  (distance) is an integer between 0 and 255 that specifies the space
 *  between the sound and the listener. The larger the number, the further
 *  away the sound is. Using 255 does not guarantee that the channel will be
 *  culled from the mixing process or be completely silent. For efficiency,
 *  the precision of this effect may be limited (distance 0 through 5 might
 *  all produce the same effect, 6 through 10 are equal, etc). Setting (angle)
 *  and (distance) to 0 unregisters this effect, since the data would be
 *  unchanged.
 *
 * If you need more precise positional audio, consider using OpenAL for
 *  spatialized effects instead of SDL_mixer. This is only meant to be a
 *  basic effect for simple "3D" games.
 *
 * If the audio device is configured for mono output, then you won't get
 *  any effectiveness from the angle; however, distance attenuation on the
 *  channel will still occur. While this effect will function with stereo
 *  voices, it makes more sense to use voices with only one channel of sound,
 *  so when they are mixed through this effect, the positioning will sound
 *  correct. You can convert them to mono through SDL before giving them to
 *  the mixer in the first place if you like.
 *
 * Setting (channel) to MIX_CHANNEL_POST registers this as a posteffect, and
 *  the positioning will be done to the final mixed stream before passing it
 *  on to the audio device.
 *
 * This is a convenience wrapper over Mix_SetDistance() and Mix_SetPanning().
 *
 * returns zero if error (no such channel or Mix_RegisterEffect() fails),
 *  nonzero if position effect is enabled.
 *  Error messages can be retrieved from Mix_GetError().
 */
extern DECLSPEC int SDLCALL Mix_SetPosition(int channel, Sint16 angle, Uint8 distance);


/* Set the "distance" of a channel. (distance) is an integer from 0 to 255
 *  that specifies the location of the sound in relation to the listener.
 *  Distance 0 is overlapping the listener, and 255 is as far away as possible
 *  A distance of 255 does not guarantee silence; in such a case, you might
 *  want to try changing the chunk's volume, or just cull the sample from the
 *  mixing process with Mix_HaltChannel().
 * For efficiency, the precision of this effect may be limited (distances 1
 *  through 7 might all produce the same effect, 8 through 15 are equal, etc).
 *  (distance) is an integer between 0 and 255 that specifies the space
 *  between the sound and the listener. The larger the number, the further
 *  away the sound is.
 * Setting (distance) to 0 unregisters this effect, since the data would be
 *  unchanged.
 * If you need more precise positional audio, consider using OpenAL for
 *  spatialized effects instead of SDL_mixer. This is only meant to be a
 *  basic effect for simple "3D" games.
 *
 * Setting (channel) to MIX_CHANNEL_POST registers this as a posteffect, and
 *  the distance attenuation will be done to the final mixed stream before
 *  passing it on to the audio device.
 *
 * This uses the Mix_RegisterEffect() API internally.
 *
 * returns zero if error (no such channel or Mix_RegisterEffect() fails),
 *  nonzero if position effect is enabled.
 *  Error messages can be retrieved from Mix_GetError().
 */
extern DECLSPEC int SDLCALL Mix_SetDistance(int channel, Uint8 distance);


/*
 * !!! FIXME : Haven't implemented, since the effect goes past the
 *              end of the sound buffer. Will have to think about this.
 *               --ryan.
 */
#if 0
/* Causes an echo effect to be mixed into a sound. (echo) is the amount
 *  of echo to mix. 0 is no echo, 255 is infinite (and probably not
 *  what you want).
 *
 * Setting (channel) to MIX_CHANNEL_POST registers this as a posteffect, and
 *  the reverbing will be done to the final mixed stream before passing it on
 *  to the audio device.
 *
 * This uses the Mix_RegisterEffect() API internally. If you specify an echo
 *  of zero, the effect is unregistered, as the data is already in that state.
 *
 * returns zero if error (no such channel or Mix_RegisterEffect() fails),
 *  nonzero if reversing effect is enabled.
 *  Error messages can be retrieved from Mix_GetError().
 */
extern no_parse_DECLSPEC int SDLCALL Mix_SetReverb(int channel, Uint8 echo);
#endif

/* Causes a channel to reverse its stereo. This is handy if the user has his
 *  speakers hooked up backwards, or you would like to have a minor bit of
 *  psychedelia in your sound code.  :)  Calling this function with (flip)
 *  set to non-zero reverses the chunks's usual channels. If (flip) is zero,
 *  the effect is unregistered.
 *
 * This uses the Mix_RegisterEffect() API internally, and thus is probably
 *  more CPU intensive than having the user just plug in his speakers
 *  correctly. Mix_SetReverseStereo() returns without registering the effect
 *  function if the audio device is not configured for stereo output.
 *
 * If you specify MIX_CHANNEL_POST for (channel), then this the effect is used
 *  on the final mixed stream before sending it on to the audio device (a
 *  posteffect).
 *
 * returns zero if error (no such channel or Mix_RegisterEffect() fails),
 *  nonzero if reversing effect is enabled. Note that an audio device in mono
 *  mode is a no-op, but this call will return successful in that case.
 *  Error messages can be retrieved from Mix_GetError().
 */
extern DECLSPEC int SDLCALL Mix_SetReverseStereo(int channel, int flip);

/* end of effects API. --ryan. */


/* Reserve the first channels (0 -> n-1) for the application, i.e. don't allocate
   them dynamically to the next sample if requested with a -1 value below.
   Returns the number of reserved channels.
 */
extern DECLSPEC int SDLCALL Mix_ReserveChannels(int num);

/* Channel grouping functions */

/* Attach a tag to a channel. A tag can be assigned to several mixer
   channels, to form groups of channels.
   If 'tag' is -1, the tag is removed (actually -1 is the tag used to
   represent the group of all the channels).
   Returns true if everything was OK.
 */
extern DECLSPEC int SDLCALL Mix_GroupChannel(int which, int tag);
/* Assign several consecutive channels to a group */
extern DECLSPEC int SDLCALL Mix_GroupChannels(int from, int to, int tag);
/* Finds the first available channel in a group of channels,
   returning -1 if none are available.
 */
extern DECLSPEC int SDLCALL Mix_GroupAvailable(int tag);
/* Returns the number of channels in a group. This is also a subtle
   way to get the total number of channels when 'tag' is -1
 */
extern DECLSPEC int SDLCALL Mix_GroupCount(int tag);
/* Finds the "oldest" sample playing in a group of channels */
extern DECLSPEC int SDLCALL Mix_GroupOldest(int tag);
/* Finds the "most recent" (i.e. last) sample playing in a group of channels */
extern DECLSPEC int SDLCALL Mix_GroupNewer(int tag);

/* Play an audio chunk on a specific channel.
   If the specified channel is -1, play on the first free channel.
   If 'loops' is greater than zero, loop the sound that many times.
   If 'loops' is -1, loop inifinitely (~65000 times).
   Returns which channel was used to play the sound.
*/
#define Mix_PlayChannel(channel,chunk,loops) Mix_PlayChannelTimed(channel,chunk,loops,-1)
/* The same as above, but the sound is played at most 'ticks' milliseconds */
extern DECLSPEC int SDLCALL Mix_PlayChannelTimed(int channel, Mix_Chunk *chunk, int loops, int ticks);
extern DECLSPEC int SDLCALL Mix_PlayMusic(Mix_Music *music, int loops);

/* Fade in music or a channel over "ms" milliseconds, same semantics as the "Play" functions */
extern DECLSPEC int SDLCALL Mix_FadeInMusic(Mix_Music *music, int loops, int ms);
extern DECLSPEC int SDLCALL Mix_FadeInMusicPos(Mix_Music *music, int loops, int ms, double position);
#define Mix_FadeInChannel(channel,chunk,loops,ms) Mix_FadeInChannelTimed(channel,chunk,loops,ms,-1)
extern DECLSPEC int SDLCALL Mix_FadeInChannelTimed(int channel, Mix_Chunk *chunk, int loops, int ms, int ticks);

/* Set the volume in the range of 0-128 of a specific channel or chunk.
   If the specified channel is -1, set volume for all channels.
   Returns the original volume.
   If the specified volume is -1, just return the current volume.
*/
extern DECLSPEC int SDLCALL Mix_Volume(int channel, int volume);
extern DECLSPEC int SDLCALL Mix_VolumeChunk(Mix_Chunk *chunk, int volume);
extern DECLSPEC int SDLCALL Mix_VolumeMusic(int volume);

/* Halt playing of a particular channel */
extern DECLSPEC int SDLCALL Mix_HaltChannel(int channel);
extern DECLSPEC int SDLCALL Mix_HaltGroup(int tag);
extern DECLSPEC int SDLCALL Mix_HaltMusic(void);

/* Change the expiration delay for a particular channel.
   The sample will stop playing after the 'ticks' milliseconds have elapsed,
   or remove the expiration if 'ticks' is -1
*/
extern DECLSPEC int SDLCALL Mix_ExpireChannel(int channel, int ticks);

/* Halt a channel, fading it out progressively till it's silent
   The ms parameter indicates the number of milliseconds the fading
   will take.
 */
extern DECLSPEC int SDLCALL Mix_FadeOutChannel(int which, int ms);
extern DECLSPEC int SDLCALL Mix_FadeOutGroup(int tag, int ms);
extern DECLSPEC int SDLCALL Mix_FadeOutMusic(int ms);

/* Query the fading status of a channel */
extern DECLSPEC Mix_Fading SDLCALL Mix_FadingMusic(void);
extern DECLSPEC Mix_Fading SDLCALL Mix_FadingChannel(int which);

/* Pause/Resume a particular channel */
extern DECLSPEC void SDLCALL Mix_Pause(int channel);
extern DECLSPEC void SDLCALL Mix_Resume(int channel);
extern DECLSPEC int SDLCALL Mix_Paused(int channel);

/* Pause/Resume the music stream */
extern DECLSPEC void SDLCALL Mix_PauseMusic(void);
extern DECLSPEC void SDLCALL Mix_ResumeMusic(void);
extern DECLSPEC void SDLCALL Mix_RewindMusic(void);
extern DECLSPEC int SDLCALL Mix_PausedMusic(void);

/* Set the current position in the music stream.
   This returns 0 if successful, or -1 if it failed or isn't implemented.
   This function is only implemented for MOD music formats (set pattern
   order number) and for OGG, FLAC, MP3_MAD, and MODPLUG music (set 
   position in seconds), at the moment.
*/
extern DECLSPEC int SDLCALL Mix_SetMusicPosition(double position);

/* Check the status of a specific channel.
   If the specified channel is -1, check all channels.
*/
extern DECLSPEC int SDLCALL Mix_Playing(int channel);
extern DECLSPEC int SDLCALL Mix_PlayingMusic(void);

/* Stop music and set external music playback command */
extern DECLSPEC int SDLCALL Mix_SetMusicCMD(const char *command);

/* Synchro value is set by MikMod from modules while playing */
extern DECLSPEC int SDLCALL Mix_SetSynchroValue(int value);
extern DECLSPEC int SDLCALL Mix_GetSynchroValue(void);

/* Set/Get/Iterate SoundFonts paths to use by supported MIDI backends */
extern DECLSPEC int SDLCALL Mix_SetSoundFonts(const char *paths);
extern DECLSPEC const char* SDLCALL Mix_GetSoundFonts(void);
extern DECLSPEC int SDLCALL Mix_EachSoundFont(int (*function)(const char*, void*), void *data);

/* Get the Mix_Chunk currently associated with a mixer channel
    Returns NULL if it's an invalid channel, or there's no chunk associated.
*/
extern DECLSPEC Mix_Chunk * SDLCALL Mix_GetChunk(int channel);

/* Close the mixer, halting all playing audio */
extern DECLSPEC void SDLCALL Mix_CloseAudio(void);

/* We'll use SDL for reporting errors */
#define Mix_SetError	SDL_SetError
#define Mix_GetError	SDL_GetError

/* Ends C function definitions when using C++ */
#ifdef __cplusplus
}
#endif
#include "close_code.h"

#endif /* _SDL_MIXER_H */
