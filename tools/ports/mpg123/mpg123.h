/*
	libmpg123: MPEG Audio Decoder library (version 1.26.2)

	copyright 1995-2015 by the mpg123 project
	free software under the terms of the LGPL 2.1
	see COPYING and AUTHORS files in distribution or http://mpg123.org
*/

#ifndef MPG123_LIB_H
#define MPG123_LIB_H

#include <fmt123.h>

/** \file mpg123.h The header file for the libmpg123 MPEG Audio decoder */

/** A macro to check at compile time which set of API functions to expect.
 * This should be incremented at least each time a new symbol is added
 * to the header.
 */
#define MPG123_API_VERSION 45

#ifndef MPG123_EXPORT
/** Defines needed for MS Visual Studio(tm) DLL builds.
 * Every public function must be prefixed with MPG123_EXPORT. When building 
 * the DLL ensure to define BUILD_MPG123_DLL. This makes the function accessible
 * for clients and includes it in the import library which is created together
 * with the DLL. When consuming the DLL ensure to define LINK_MPG123_DLL which
 * imports the functions from the DLL. 
 */
#ifdef BUILD_MPG123_DLL
/* The dll exports. */
#define MPG123_EXPORT __declspec(dllexport)
#else
#ifdef LINK_MPG123_DLL
/* The exe imports. */
#define MPG123_EXPORT __declspec(dllimport)
#else
/* Nothing on normal/UNIX builds */
#define MPG123_EXPORT
#endif
#endif
#endif

/* This is for Visual Studio, so this header works as distributed in the binary downloads */
#if defined(_MSC_VER) && !defined(MPG123_DEF_SSIZE_T)
#define MPG123_DEF_SSIZE_T
#include <stddef.h>
typedef ptrdiff_t ssize_t;
#endif

#ifndef MPG123_NO_CONFIGURE /* Enable use of this file without configure. */
#include <stdlib.h>
#include <sys/types.h>

/* Simplified large file handling.
	I used to have a check here that prevents building for a library with conflicting large file setup
	(application that uses 32 bit offsets with library that uses 64 bits).
	While that was perfectly fine in an environment where there is one incarnation of the library,
	it hurt GNU/Linux and Solaris systems with multilib where the distribution fails to provide the
	correct header matching the 32 bit library (where large files need explicit support) or
	the 64 bit library (where there is no distinction).

	New approach: When the app defines _FILE_OFFSET_BITS, it wants non-default large file support,
	and thus functions with added suffix (mpg123_open_64).
	Any mismatch will be caught at link time because of the _FILE_OFFSET_BITS setting used when
	building libmpg123. Plus, there's dual mode large file support in mpg123 since 1.12 now.
	Link failure is not the expected outcome of any half-sane usage anymore.

	More complication: What about client code defining _LARGEFILE64_SOURCE? It might want direct access to the _64 functions, along with the ones without suffix. Well, that's possible now via defining MPG123_NO_LARGENAME and MPG123_LARGESUFFIX, respectively, for disabling or enforcing the suffix names.
*/

/*
	Now, the renaming of large file aware functions.
	By default, it appends underscore _FILE_OFFSET_BITS (so, mpg123_seek_64 for mpg123_seek), if _FILE_OFFSET_BITS is defined. You can force a different suffix via MPG123_LARGESUFFIX (that must include the underscore), or you can just disable the whole mess by defining MPG123_NO_LARGENAME.
*/
#if (!defined MPG123_NO_LARGENAME) && ((defined _FILE_OFFSET_BITS) || (defined MPG123_LARGESUFFIX))

/* Need some trickery to concatenate the value(s) of the given macro(s). */
#define MPG123_MACROCAT_REALLY(a, b) a ## b
#define MPG123_MACROCAT(a, b) MPG123_MACROCAT_REALLY(a, b)
#ifndef MPG123_LARGESUFFIX
#define MPG123_LARGESUFFIX MPG123_MACROCAT(_, _FILE_OFFSET_BITS)
#endif
#define MPG123_LARGENAME(func) MPG123_MACROCAT(func, MPG123_LARGESUFFIX)

#define mpg123_open_fixed   MPG123_LARGENAME(mpg123_open_fixed)
#define mpg123_open         MPG123_LARGENAME(mpg123_open)
#define mpg123_open_fd      MPG123_LARGENAME(mpg123_open_fd)
#define mpg123_open_handle  MPG123_LARGENAME(mpg123_open_handle)
#define mpg123_framebyframe_decode MPG123_LARGENAME(mpg123_framebyframe_decode)
#define mpg123_decode_frame MPG123_LARGENAME(mpg123_decode_frame)
#define mpg123_tell         MPG123_LARGENAME(mpg123_tell)
#define mpg123_tellframe    MPG123_LARGENAME(mpg123_tellframe)
#define mpg123_tell_stream  MPG123_LARGENAME(mpg123_tell_stream)
#define mpg123_seek         MPG123_LARGENAME(mpg123_seek)
#define mpg123_feedseek     MPG123_LARGENAME(mpg123_feedseek)
#define mpg123_seek_frame   MPG123_LARGENAME(mpg123_seek_frame)
#define mpg123_timeframe    MPG123_LARGENAME(mpg123_timeframe)
#define mpg123_index        MPG123_LARGENAME(mpg123_index)
#define mpg123_set_index    MPG123_LARGENAME(mpg123_set_index)
#define mpg123_position     MPG123_LARGENAME(mpg123_position)
#define mpg123_length       MPG123_LARGENAME(mpg123_length)
#define mpg123_framelength  MPG123_LARGENAME(mpg123_framelength)
#define mpg123_set_filesize MPG123_LARGENAME(mpg123_set_filesize)
#define mpg123_replace_reader MPG123_LARGENAME(mpg123_replace_reader)
#define mpg123_replace_reader_handle MPG123_LARGENAME(mpg123_replace_reader_handle)
#define mpg123_framepos MPG123_LARGENAME(mpg123_framepos)

#endif /* largefile hackery */

#endif /* MPG123_NO_CONFIGURE */

#ifdef __cplusplus
extern "C" {
#endif

/** \defgroup mpg123_init mpg123 library and handle setup
 *
 * Functions to initialise and shutdown the mpg123 library and handles.
 * The parameters of handles have workable defaults, you only have to tune them when you want to tune something;-)
 * Tip: Use a RVA setting...
 *
 * @{
 */

/** Opaque structure for the libmpg123 decoder handle. */
struct mpg123_handle_struct;

/** Opaque structure for the libmpg123 decoder handle.
 *  Most functions take a pointer to a mpg123_handle as first argument and operate on its data in an object-oriented manner.
 */
typedef struct mpg123_handle_struct mpg123_handle;

/** Function to initialise the mpg123 library. 
 * This should be called once in a non-parallel context. It is not explicitly
 * thread-safe, but repeated/concurrent calls still _should_ be safe as static
 * tables are filled with the same values anyway.
 *
 *	\return MPG123_OK if successful, otherwise an error number.
 */
MPG123_EXPORT int mpg123_init(void);

/** Superfluous Function to close down the mpg123 library.
 * This was created with the thought that there sometime will be cleanup code
 * to be run after library use. This never materialized. You can forget about
 * this function and it is only here for old programs that do call it.
 */
MPG123_EXPORT void mpg123_exit(void);

/** Create a handle with optional choice of decoder (named by a string, see mpg123_decoders() or mpg123_supported_decoders()).
 *  and optional retrieval of an error code to feed to mpg123_plain_strerror().
 *  Optional means: Any of or both the parameters may be NULL.
 *
 *  \param decoder optional choice of decoder variant (NULL for default)
 *  \param error optional address to store error codes
 *  \return Non-NULL pointer to fresh handle when successful.
 */
MPG123_EXPORT mpg123_handle *mpg123_new(const char* decoder, int *error);

/** Delete handle, mh is either a valid mpg123 handle or NULL.
 *  \param mh handle
 */
MPG123_EXPORT void mpg123_delete(mpg123_handle *mh);

/** Free plain memory allocated within libmpg123.
 *  This is for library users that are not sure to use the same underlying
 *  memory allocator as libmpg123. It is just a wrapper over free() in
 *  the underlying C library.
 */
MPG123_EXPORT void mpg123_free(void *ptr);

/** Enumeration of the parameters types that it is possible to set/get. */
enum mpg123_parms
{
	MPG123_VERBOSE = 0,        /**< set verbosity value for enabling messages to stderr, >= 0 makes sense (integer) */
	MPG123_FLAGS,          /**< set all flags, p.ex val = MPG123_GAPLESS|MPG123_MONO_MIX (integer) */
	MPG123_ADD_FLAGS,      /**< add some flags (integer) */
	MPG123_FORCE_RATE,     /**< when value > 0, force output rate to that value (integer) */
	MPG123_DOWN_SAMPLE,    /**< 0=native rate, 1=half rate, 2=quarter rate (integer) */
	MPG123_RVA,            /**< one of the RVA choices above (integer) */
	MPG123_DOWNSPEED,      /**< play a frame N times (integer) */
	MPG123_UPSPEED,        /**< play every Nth frame (integer) */
	MPG123_START_FRAME,    /**< start with this frame (skip frames before that, integer) */ 
	MPG123_DECODE_FRAMES,  /**< decode only this number of frames (integer) */
	MPG123_ICY_INTERVAL,   /**< Stream contains ICY metadata with this interval (integer).
	                            Make sure to set this _before_ opening a stream.*/
	MPG123_OUTSCALE,       /**< the scale for output samples (amplitude - integer or float according to mpg123 output format, normally integer) */
	MPG123_TIMEOUT,        /**< timeout for reading from a stream (not supported on win32, integer) */
	MPG123_REMOVE_FLAGS,   /**< remove some flags (inverse of MPG123_ADD_FLAGS, integer) */
	MPG123_RESYNC_LIMIT,   /**< Try resync on frame parsing for that many bytes or until end of stream (<0 ... integer). This can enlarge the limit for skipping junk on beginning, too (but not reduce it).  */
	MPG123_INDEX_SIZE      /**< Set the frame index size (if supported). Values <0 mean that the index is allowed to grow dynamically in these steps (in positive direction, of course) -- Use this when you really want a full index with every individual frame. */
	,MPG123_PREFRAMES /**< Decode/ignore that many frames in advance for layer 3. This is needed to fill bit reservoir after seeking, for example (but also at least one frame in advance is needed to have all "normal" data for layer 3). Give a positive integer value, please.*/
	,MPG123_FEEDPOOL  /**< For feeder mode, keep that many buffers in a pool to avoid frequent malloc/free. The pool is allocated on mpg123_open_feed(). If you change this parameter afterwards, you can trigger growth and shrinkage during decoding. The default value could change any time. If you care about this, then set it. (integer) */
	,MPG123_FEEDBUFFER /**< Minimal size of one internal feeder buffer, again, the default value is subject to change. (integer) */
	,MPG123_FREEFORMAT_SIZE /**< Tell the parser a free-format frame size to
	 * avoid read-ahead to get it. A value of -1 (default) means that the parser
	 * will determine it. The parameter value is applied during decoder setup
	 * for a freshly opened stream only.
	 */
};

/** Flag bits for MPG123_FLAGS, use the usual binary or to combine. */
enum mpg123_param_flags
{
	 MPG123_FORCE_MONO   = 0x7  /**<     0111 Force some mono mode: This is a test bitmask for seeing if any mono forcing is active. */
	,MPG123_MONO_LEFT    = 0x1  /**<     0001 Force playback of left channel only.  */
	,MPG123_MONO_RIGHT   = 0x2  /**<     0010 Force playback of right channel only. */
	,MPG123_MONO_MIX     = 0x4  /**<     0100 Force playback of mixed mono.         */
	,MPG123_FORCE_STEREO = 0x8  /**<     1000 Force stereo output.                  */
	,MPG123_FORCE_8BIT   = 0x10 /**< 00010000 Force 8bit formats.                   */
	,MPG123_QUIET        = 0x20 /**< 00100000 Suppress any printouts (overrules verbose).                    */
	,MPG123_GAPLESS      = 0x40 /**< 01000000 Enable gapless decoding (default on if libmpg123 has support). */
	,MPG123_NO_RESYNC    = 0x80 /**< 10000000 Disable resync stream after error.                             */
	,MPG123_SEEKBUFFER   = 0x100 /**< 000100000000 Enable small buffer on non-seekable streams to allow some peek-ahead (for better MPEG sync). */
	,MPG123_FUZZY        = 0x200 /**< 001000000000 Enable fuzzy seeks (guessing byte offsets or using approximate seek points from Xing TOC) */
	,MPG123_FORCE_FLOAT  = 0x400 /**< 010000000000 Force floating point output (32 or 64 bits depends on mpg123 internal precision). */
	,MPG123_PLAIN_ID3TEXT = 0x800 /**< 100000000000 Do not translate ID3 text data to UTF-8. ID3 strings will contain the raw text data, with the first byte containing the ID3 encoding code. */
	,MPG123_IGNORE_STREAMLENGTH = 0x1000 /**< 1000000000000 Ignore any stream length information contained in the stream, which can be contained in a 'TLEN' frame of an ID3v2 tag or a Xing tag */
	,MPG123_SKIP_ID3V2 = 0x2000 /**< 10 0000 0000 0000 Do not parse ID3v2 tags, just skip them. */
	,MPG123_IGNORE_INFOFRAME = 0x4000 /**< 100 0000 0000 0000 Do not parse the LAME/Xing info frame, treat it as normal MPEG data. */
	,MPG123_AUTO_RESAMPLE = 0x8000 /**< 1000 0000 0000 0000 Allow automatic internal resampling of any kind (default on if supported). Especially when going lowlevel with replacing output buffer, you might want to unset this flag. Setting MPG123_DOWNSAMPLE or MPG123_FORCE_RATE will override this. */
	,MPG123_PICTURE = 0x10000 /**< 17th bit: Enable storage of pictures from tags (ID3v2 APIC). */
	,MPG123_NO_PEEK_END    = 0x20000 /**< 18th bit: Do not seek to the end of
	 *  the stream in order to probe
	 *  the stream length and search for the id3v1 field. This also means
	 *  the file size is unknown unless set using mpg123_set_filesize() and
	 *  the stream is assumed as non-seekable unless overridden.
	 */
	,MPG123_FORCE_SEEKABLE = 0x40000 /**< 19th bit: Force the stream to be seekable. */
	,MPG123_STORE_RAW_ID3  = 0x80000 /**< store raw ID3 data (even if skipping) */
	,MPG123_FORCE_ENDIAN   = 0x100000 /**< Enforce endianess of output samples.
	 *  This is not reflected in the format codes. If this flag is set along with
	 *  MPG123_BIG_ENDIAN, MPG123_ENC_SIGNED16 means s16be, without
	 *  MPG123_BIG_ENDIAN, it means s16le. Normal operation without
	 *  MPG123_FORCE_ENDIAN produces output in native byte order.
	 */
	,MPG123_BIG_ENDIAN     = 0x200000 /**< Choose big endian instead of little. */
	,MPG123_NO_READAHEAD   = 0x400000 /**< Disable read-ahead in parser. If
	 * you know you provide full frames to the feeder API, this enables
	 * decoder output from the first one on, instead of having to wait for
	 * the next frame to confirm that the stream is healthy. It also disables
	 * free format support unless you provide a frame size using
	 * MPG123_FREEFORMAT_SIZE.
	 */
	,MPG123_FLOAT_FALLBACK = 0x800000 /**< Consider floating point output encoding only after
	 * trying other (possibly downsampled) rates and encodings first. This is to
	 * support efficient playback where floating point output is only configured for
	 * an external resampler, bypassing that resampler when the desired rate can
	 * be produced directly. This is enabled by default to be closer to older versions
	 * of libmpg123 which did not enable float automatically at all. If disabled,
	 * float is considered after the 16 bit default and higher-bit integer encodings
	 * for any rate. */
	,MPG123_NO_FRANKENSTEIN = 0x1000000 /**< Disable support for Frankenstein streams
	 * (different MPEG streams stiched together). Do not accept serious change of MPEG
	 * header inside a single stream. With this flag, the audio output format cannot
	 * change during decoding unless you open a new stream. This also stops decoding
	 * after an announced end of stream (Info header contained a number of frames
	 * and this number has been reached). This makes your MP3 files behave more like
	 * ordinary media files with defined structure, rather than stream dumps with
	 * some sugar. */
};

/** choices for MPG123_RVA */
enum mpg123_param_rva
{
	 MPG123_RVA_OFF   = 0 /**< RVA disabled (default).   */
	,MPG123_RVA_MIX   = 1 /**< Use mix/track/radio gain. */
	,MPG123_RVA_ALBUM = 2 /**< Use album/audiophile gain */
	,MPG123_RVA_MAX   = MPG123_RVA_ALBUM /**< The maximum RVA code, may increase in future. */
};

/** Set a specific parameter, for a specific mpg123_handle, using a parameter 
 *  type key chosen from the mpg123_parms enumeration, to the specified value.
 *  \param mh handle
 *  \param type parameter choice
 *  \param value integer value
 *  \param fvalue floating point value
 *  \return MPG123_OK on success
 */
MPG123_EXPORT int mpg123_param( mpg123_handle *mh
,	enum mpg123_parms type, long value, double fvalue );

/** Get a specific parameter, for a specific mpg123_handle. 
 *  See the mpg123_parms enumeration for a list of available parameters.
 *  \param mh handle
 *  \param type parameter choice
 *  \param value integer value return address
 *  \param fvalue floating point value return address
 *  \return MPG123_OK on success
 */
MPG123_EXPORT int mpg123_getparam( mpg123_handle *mh
,	enum mpg123_parms type, long *value, double *fvalue );

/** Feature set available for query with mpg123_feature. */
enum mpg123_feature_set
{
	 MPG123_FEATURE_ABI_UTF8OPEN = 0     /**< mpg123 expects path names to be given in UTF-8 encoding instead of plain native. */
	,MPG123_FEATURE_OUTPUT_8BIT          /**< 8bit output   */
	,MPG123_FEATURE_OUTPUT_16BIT         /**< 16bit output  */
	,MPG123_FEATURE_OUTPUT_32BIT         /**< 32bit output  */
	,MPG123_FEATURE_INDEX                /**< support for building a frame index for accurate seeking */
	,MPG123_FEATURE_PARSE_ID3V2          /**< id3v2 parsing */
	,MPG123_FEATURE_DECODE_LAYER1        /**< mpeg layer-1 decoder enabled */
	,MPG123_FEATURE_DECODE_LAYER2        /**< mpeg layer-2 decoder enabled */
	,MPG123_FEATURE_DECODE_LAYER3        /**< mpeg layer-3 decoder enabled */
	,MPG123_FEATURE_DECODE_ACCURATE      /**< accurate decoder rounding    */
	,MPG123_FEATURE_DECODE_DOWNSAMPLE    /**< downsample (sample omit)     */
	,MPG123_FEATURE_DECODE_NTOM          /**< flexible rate decoding       */
	,MPG123_FEATURE_PARSE_ICY            /**< ICY support                  */
	,MPG123_FEATURE_TIMEOUT_READ         /**< Reader with timeout (network). */
	,MPG123_FEATURE_EQUALIZER            /**< tunable equalizer */
	,MPG123_FEATURE_MOREINFO             /**< more info extraction (for frame analyzer) */
	,MPG123_FEATURE_OUTPUT_FLOAT32      /**< 32 bit float output */
	,MPG123_FEATURE_OUTPUT_FLOAT64      /**< 64 bit float output (usually never) */
};

/** Query libmpg123 features.
 *  \param key feature selection
 *  \return 1 for success, 0 for unimplemented functions
 */
MPG123_EXPORT int mpg123_feature(const enum mpg123_feature_set key);

/** Query libmpg123 features with better ABI compatibility
 *
 *  This is the same as mpg123_feature(), but this time not using
 *  the enum as argument. Compilers don't have to agree on the size of
 *  enums and hence they are not safe in public API.
 *
 *  \param key feature selection
 *  \return 1 for success, 0 for unimplemented functions
 */
MPG123_EXPORT int mpg123_feature2(int key);

/* @} */


/** \defgroup mpg123_error mpg123 error handling
 *
 * Functions to get text version of the error numbers and an enumeration
 * of the error codes returned by libmpg123.
 *
 * Most functions operating on a mpg123_handle simply return MPG123_OK (0)
 * on success and MPG123_ERR (-1) on failure, setting the internal error
 * variable of the handle to the specific error code. If there was not a valid
 * (non-NULL) handle provided to a function operating on one, MPG123_BAD_HANDLE
 * may be returned if this can not be confused with a valid positive return
 * value.
 * Meaning: A function expected to return positive integers on success will
 * always indicate error or a special condition by returning a negative one.
 *
 * Decoding/seek functions may also return message codes MPG123_DONE,
 * MPG123_NEW_FORMAT and MPG123_NEED_MORE (all negative, see below on how to
 * react). Note that calls to those can be nested, so generally watch out
 * for these codes after initial handle setup.
 * Especially any function that needs information about the current stream
 * to work will try to at least parse the beginning if that did not happen
 * yet.
 *
 * On a function that is supposed to return MPG123_OK on success and
 * MPG123_ERR on failure, make sure you check for != MPG123_OK, not
 * == MPG123_ERR, as the error code could get more specific in future,
 * or there is just a special message from a decoding routine as indicated
 * above.
 *
 * @{
 */

/** Enumeration of the message and error codes and returned by libmpg123 functions. */
enum mpg123_errors
{
	MPG123_DONE=-12,	/**< Message: Track ended. Stop decoding. */
	MPG123_NEW_FORMAT=-11,	/**< Message: Output format will be different on next call. Note that some libmpg123 versions between 1.4.3 and 1.8.0 insist on you calling mpg123_getformat() after getting this message code. Newer verisons behave like advertised: You have the chance to call mpg123_getformat(), but you can also just continue decoding and get your data. */
	MPG123_NEED_MORE=-10,	/**< Message: For feed reader: "Feed me more!" (call mpg123_feed() or mpg123_decode() with some new input data). */
	MPG123_ERR=-1,			/**< Generic Error */
	MPG123_OK=0, 			/**< Success */
	MPG123_BAD_OUTFORMAT, 	/**< Unable to set up output format! */
	MPG123_BAD_CHANNEL,		/**< Invalid channel number specified. */
	MPG123_BAD_RATE,		/**< Invalid sample rate specified.  */
	MPG123_ERR_16TO8TABLE,	/**< Unable to allocate memory for 16 to 8 converter table! */
	MPG123_BAD_PARAM,		/**< Bad parameter id! */
	MPG123_BAD_BUFFER,		/**< Bad buffer given -- invalid pointer or too small size. */
	MPG123_OUT_OF_MEM,		/**< Out of memory -- some malloc() failed. */
	MPG123_NOT_INITIALIZED,	/**< You didn't initialize the library! */
	MPG123_BAD_DECODER,		/**< Invalid decoder choice. */
	MPG123_BAD_HANDLE,		/**< Invalid mpg123 handle. */
	MPG123_NO_BUFFERS,		/**< Unable to initialize frame buffers (out of memory?). */
	MPG123_BAD_RVA,			/**< Invalid RVA mode. */
	MPG123_NO_GAPLESS,		/**< This build doesn't support gapless decoding. */
	MPG123_NO_SPACE,		/**< Not enough buffer space. */
	MPG123_BAD_TYPES,		/**< Incompatible numeric data types. */
	MPG123_BAD_BAND,		/**< Bad equalizer band. */
	MPG123_ERR_NULL,		/**< Null pointer given where valid storage address needed. */
	MPG123_ERR_READER,		/**< Error reading the stream. */
	MPG123_NO_SEEK_FROM_END,/**< Cannot seek from end (end is not known). */
	MPG123_BAD_WHENCE,		/**< Invalid 'whence' for seek function.*/
	MPG123_NO_TIMEOUT,		/**< Build does not support stream timeouts. */
	MPG123_BAD_FILE,		/**< File access error. */
	MPG123_NO_SEEK,			/**< Seek not supported by stream. */
	MPG123_NO_READER,		/**< No stream opened. */
	MPG123_BAD_PARS,		/**< Bad parameter handle. */
	MPG123_BAD_INDEX_PAR,	/**< Bad parameters to mpg123_index() and mpg123_set_index() */
	MPG123_OUT_OF_SYNC,	/**< Lost track in bytestream and did not try to resync. */
	MPG123_RESYNC_FAIL,	/**< Resync failed to find valid MPEG data. */
	MPG123_NO_8BIT,	/**< No 8bit encoding possible. */
	MPG123_BAD_ALIGN,	/**< Stack aligmnent error */
	MPG123_NULL_BUFFER,	/**< NULL input buffer with non-zero size... */
	MPG123_NO_RELSEEK,	/**< Relative seek not possible (screwed up file offset) */
	MPG123_NULL_POINTER, /**< You gave a null pointer somewhere where you shouldn't have. */
	MPG123_BAD_KEY,	/**< Bad key value given. */
	MPG123_NO_INDEX,	/**< No frame index in this build. */
	MPG123_INDEX_FAIL,	/**< Something with frame index went wrong. */
	MPG123_BAD_DECODER_SETUP,	/**< Something prevents a proper decoder setup */
	MPG123_MISSING_FEATURE  /**< This feature has not been built into libmpg123. */
	,MPG123_BAD_VALUE /**< A bad value has been given, somewhere. */
	,MPG123_LSEEK_FAILED /**< Low-level seek failed. */
	,MPG123_BAD_CUSTOM_IO /**< Custom I/O not prepared. */
	,MPG123_LFS_OVERFLOW /**< Offset value overflow during translation of large file API calls -- your client program cannot handle that large file. */
	,MPG123_INT_OVERFLOW /**< Some integer overflow. */
};

/** Look up error strings given integer code.
 *  \param errcode integer error code
 *  \return string describing what that error error code means
 */
MPG123_EXPORT const char* mpg123_plain_strerror(int errcode);

/** Give string describing what error has occured in the context of handle mh.
 *  When a function operating on an mpg123 handle returns MPG123_ERR, you should check for the actual reason via
 *  char *errmsg = mpg123_strerror(mh)
 *  This function will catch mh == NULL and return the message for MPG123_BAD_HANDLE.
 *  \param mh handle
 *  \return error message
 */
MPG123_EXPORT const char* mpg123_strerror(mpg123_handle *mh);

/** Return the plain errcode intead of a string.
 *  \param mh handle
 *  \return error code recorded in handle or MPG123_BAD_HANDLE
 */
MPG123_EXPORT int mpg123_errcode(mpg123_handle *mh);

/*@}*/


/** \defgroup mpg123_decoder mpg123 decoder selection
 *
 * Functions to list and select the available decoders.
 * Perhaps the most prominent feature of mpg123: You have several (optimized) decoders to choose from (on x86 and PPC (MacOS) systems, that is).
 *
 * @{
 */

/** Get available decoder list.
 *  \return NULL-terminated array of generally available decoder names (plain 8bit ASCII)
 */
MPG123_EXPORT const char **mpg123_decoders(void);

/** Get supported decoder list.
 *  \return NULL-terminated array of the decoders supported by the CPU (plain 8bit ASCII)
 */
MPG123_EXPORT const char **mpg123_supported_decoders(void);

/** Set the active decoder.
 *  \param mh handle
 *  \param decoder_name name of decoder
 *  \return MPG123_OK on success
 */
MPG123_EXPORT int mpg123_decoder(mpg123_handle *mh, const char* decoder_name);

/** Get the currently active decoder name.
 *  The active decoder engine can vary depening on output constraints,
 *  mostly non-resampling, integer output is accelerated via 3DNow & Co. but for
 *  other modes a fallback engine kicks in.
 *  Note that this can return a decoder that is only active in the hidden and not
 *  available as decoder choice from the outside.
 *  \param mh handle
 *  \return The decoder name or NULL on error.
 */
MPG123_EXPORT const char* mpg123_current_decoder(mpg123_handle *mh);

/*@}*/


/** \defgroup mpg123_output mpg123 output audio format 
 *
 * Functions to get and select the format of the decoded audio.
 *
 * Before you dive in, please be warned that you might get confused by this.
 * This seems to happen a lot, therefore I am trying to explain in advance.
 * If you do feel confused and just want to decode your normal MPEG audio files that
 * don't alter properties in the middle, just use mpg123_open_fixed() with a fixed encoding
 * and channel count and forget about a matrix of audio formats. If you want to get funky,
 * read ahead ...
 *
 * The mpg123 library decides what output format to use when encountering the first frame in a stream, or actually any frame that is still valid but differs from the frames before in the prompted output format. At such a deciding point, an internal table of allowed encodings, sampling rates and channel setups is consulted. According to this table, an output format is chosen and the decoding engine set up accordingly (including optimized routines for different output formats). This might seem unusual but it just follows from the non-existence of "MPEG audio files" with defined overall properties. There are streams, streams are concatenations of (semi) independent frames. We store streams on disk and call them "MPEG audio files", but that does not change their nature as the decoder is concerned (the LAME/Xing header for gapless decoding makes things interesting again).
 *
 * To get to the point: What you do with mpg123_format() and friends is to fill the internal table of allowed formats before it is used. That includes removing support for some formats or adding your forced sample rate (see MPG123_FORCE_RATE) that will be used with the crude internal resampler. Also keep in mind that the sample encoding is just a question of choice -- the MPEG frames do only indicate their native sampling rate and channel count. If you want to decode to integer or float samples, 8 or 16 bit ... that is your decision. In a "clean" world, libmpg123 would always decode to 32 bit float and let you handle any sample conversion. But there are optimized routines that work faster by directly decoding to the desired encoding / accuracy. We prefer efficiency over conceptual tidyness.
 *
 * People often start out thinking that mpg123_format() should change the actual decoding format on the fly. That is wrong. It only has effect on the next natural change of output format, when libmpg123 will consult its format table again. To make life easier, you might want to call mpg123_format_none() before any thing else and then just allow one desired encoding and a limited set of sample rates / channel choices that you actually intend to deal with. You can force libmpg123 to decode everything to 44100 KHz, stereo, 16 bit integer ... it will duplicate mono channels and even do resampling if needed (unless that feature is disabled in the build, same with some encodings). But I have to stress that the resampling of libmpg123 is very crude and doesn't even contain any kind of "proper" interpolation.
 *
 * In any case, watch out for MPG123_NEW_FORMAT as return message from decoding routines and call mpg123_getformat() to get the currently active output format.
 *
 * @{
 */

/** They can be combined into one number (3) to indicate mono and stereo... */
enum mpg123_channelcount
{
	 MPG123_MONO   = 1 /**< mono */
	,MPG123_STEREO = 2 /**< stereo */
};

/** An array of supported standard sample rates
 *  These are possible native sample rates of MPEG audio files.
 *  You can still force mpg123 to resample to a different one, but by
 *  default you will only get audio in one of these samplings.
 *  This list is in ascending order.
 *  \param list Store a pointer to the sample rates array there.
 *  \param number Store the number of sample rates there. */
MPG123_EXPORT void mpg123_rates(const long **list, size_t *number);

/** An array of supported audio encodings.
 *  An audio encoding is one of the fully qualified members of mpg123_enc_enum (MPG123_ENC_SIGNED_16, not MPG123_SIGNED).
 *  \param list Store a pointer to the encodings array there.
 *  \param number Store the number of encodings there. */
MPG123_EXPORT void mpg123_encodings(const int **list, size_t *number);

/** Return the size (in bytes) of one mono sample of the named encoding.
 * \param encoding The encoding value to analyze.
 * \return positive size of encoding in bytes, 0 on invalid encoding. */
MPG123_EXPORT int mpg123_encsize(int encoding);

/** Configure a mpg123 handle to accept no output format at all, 
 *  use before specifying supported formats with mpg123_format
 *  \param mh handle
 *  \return MPG123_OK on success
 */
MPG123_EXPORT int mpg123_format_none(mpg123_handle *mh);

/** Configure mpg123 handle to accept all formats 
 *  (also any custom rate you may set) -- this is default.
 *  \param mh handle
 *  \return MPG123_OK on success
 */
MPG123_EXPORT int mpg123_format_all(mpg123_handle *mh);

/** Set the audio format support of a mpg123_handle in detail:
 *  \param mh handle
 *  \param rate The sample rate value (in Hertz).
 *  \param channels A combination of MPG123_STEREO and MPG123_MONO.
 *  \param encodings A combination of accepted encodings for rate and channels, p.ex MPG123_ENC_SIGNED16 | MPG123_ENC_ULAW_8 (or 0 for no support). Please note that some encodings may not be supported in the library build and thus will be ignored here.
 *  \return MPG123_OK on success, MPG123_ERR if there was an error. */
MPG123_EXPORT int mpg123_format( mpg123_handle *mh
,	long rate, int channels, int encodings );

/** Set the audio format support of a mpg123_handle in detail:
 *  \param mh handle
 *  \param rate The sample rate value (in Hertz). Special value 0 means
 *     all rates (the reason for this variant of mpg123_format()).
 *  \param channels A combination of MPG123_STEREO and MPG123_MONO.
 *  \param encodings A combination of accepted encodings for rate and channels,
 *     p.ex MPG123_ENC_SIGNED16 | MPG123_ENC_ULAW_8 (or 0 for no support).
 *     Please note that some encodings may not be supported in the library build
 *     and thus will be ignored here.
 *  \return MPG123_OK on success, MPG123_ERR if there was an error. */
MPG123_EXPORT int mpg123_format2( mpg123_handle *mh
,	long rate, int channels, int encodings );

/** Check to see if a specific format at a specific rate is supported 
 *  by mpg123_handle.
 *  \param mh handle
 *  \param rate sampling rate
 *  \param encoding encoding
 *  \return 0 for no support (that includes invalid parameters), MPG123_STEREO, 
 *          MPG123_MONO or MPG123_STEREO|MPG123_MONO. */
MPG123_EXPORT int mpg123_format_support( mpg123_handle *mh
,	long rate, int encoding );

/** Get the current output format written to the addresses given.
 *  If the stream is freshly loaded, this will try to parse enough
 *  of it to give you the format to come. This clears the flag that
 *  would otherwise make the first decoding call return
 *  MPG123_NEW_FORMAT.
 *  \param mh handle
 *  \param rate sampling rate return address
 *  \param channels channel count return address
 *  \param encoding encoding return address
 *  \return MPG123_OK on success
 */
MPG123_EXPORT int mpg123_getformat( mpg123_handle *mh
,	long *rate, int *channels, int *encoding );

/** Get the current output format written to the addresses given.
 *  This differs from plain mpg123_getformat() in that you can choose
 *  _not_ to clear the flag that would trigger the next decoding call
 *  to return MPG123_NEW_FORMAT in case of a new format arriving.
 *  \param mh handle
 *  \param rate sampling rate return address
 *  \param channels channel count return address
 *  \param encoding encoding return address
 *  \param clear_flag if true, clear internal format flag
 *  \return MPG123_OK on success
 */
MPG123_EXPORT int mpg123_getformat2( mpg123_handle *mh
,	long *rate, int *channels, int *encoding, int clear_flag );

/*@}*/


/** \defgroup mpg123_input mpg123 file input and decoding
 *
 * Functions for input bitstream and decoding operations.
 * Decoding/seek functions may also return message codes MPG123_DONE, MPG123_NEW_FORMAT and MPG123_NEED_MORE (please read up on these on how to react!).
 * @{
 */

/** Open a simple MPEG file with fixed properties.
 *
 *  This function shall simplify the common use case of a plain MPEG
 *  file on disk that you want to decode, with one fixed sample
 *  rate and channel count, and usually a length defined by a Lame/Info/Xing
 *  tag. It will:
 *
 *  - set the MPG123_NO_FRANKENSTEIN flag
 *  - set up format support according to given parameters,
 *  - open the file,
 *  - query audio format,
 *  - fix the audio format support table to ensure the format stays the same,
 *  - call mpg123_scan() if there is no header frame to tell the track length.
 *
 *  From that on, you can call mpg123_getformat() for querying the sample
 *  rate (and channel count in case you allowed both) and mpg123_length()
 *  to get a pretty safe number for the duration.
 *  Only the sample rate is left open as that indeed is a fixed property of
 *  MPEG files. You could set MPG123_FORCE_RATE beforehand, but that may trigger
 *  low-quality resampling in the decoder, only do so if in dire need.
 *  The library will convert mono files to stereo for you, and vice versa.
 *  If any constraint cannot be satisified (most likely because of a non-default
 *  build of libmpg123), you get MPG123_ERR returned and can query the detailed
 *  cause from the handle. Only on MPG123_OK there will an open file that you
 *  then close using mpg123_close(), or implicitly on mpg123_delete() or the next
 *  call to open another file.
 *
 *  So, for your usual CD rip collection, you could use
 *
 *    mpg123_open_fixed(mh, path, MPG123_STEREO, MPG123_ENC_SIGNED_16)
 *
 *  and be happy calling mpg123_getformat() to verify 44100 Hz rate, then just
 *  playing away with mpg123_read(). The occasional mono file, or MP2 file,
 *  will also be decoded without you really noticing. Just the speed could be
 *  wrong if you do not care about sample rate at all.
 *  \param mh handle
 *  \param path filesystem path
 *  \param channels allowed channel count, either 1 (MPG123_MONO) or
 *    2 (MPG123_STEREO), or bitwise or of them, but then you're halfway back to
 *    calling mpg123_format() again;-)
 *  \param encoding a definite encoding from enum mpg123_enc_enum
 *    or a bitmask like for mpg123_format(), defeating the purpose somewhat
 */
MPG123_EXPORT int mpg123_open_fixed(mpg123_handle *mh, const char *path
,	int channels, int encoding);

/** Open and prepare to decode the specified file by filesystem path.
 *  This does not open HTTP urls; libmpg123 contains no networking code.
 *  If you want to decode internet streams, use mpg123_open_fd() or mpg123_open_feed().
 *  \param mh handle
 *  \param path filesystem path
 *  \return MPG123_OK on success
 */
MPG123_EXPORT int mpg123_open(mpg123_handle *mh, const char *path);

/** Use an already opened file descriptor as the bitstream input
 *  mpg123_close() will _not_ close the file descriptor.
 *  \param mh handle
 *  \param fd file descriptor
 *  \return MPG123_OK on success
 */
MPG123_EXPORT int mpg123_open_fd(mpg123_handle *mh, int fd);

/** Use an opaque handle as bitstream input. This works only with the
 *  replaced I/O from mpg123_replace_reader_handle()!
 *  mpg123_close() will call the cleanup callback for your handle (if you gave one).
 *  \param mh handle
 *  \param iohandle your handle
 *  \return MPG123_OK on success
 */
MPG123_EXPORT int mpg123_open_handle(mpg123_handle *mh, void *iohandle);

/** Open a new bitstream and prepare for direct feeding
 *  This works together with mpg123_decode(); you are responsible for reading and feeding the input bitstream.
 *  Also, you are expected to handle ICY metadata extraction yourself. This
 *  input method does not handle MPG123_ICY_INTERVAL. It does parse ID3 frames, though.
 *  \param mh handle
 *  \return MPG123_OK on success
 */
MPG123_EXPORT int mpg123_open_feed(mpg123_handle *mh);

/** Closes the source, if libmpg123 opened it.
 *  \param mh handle
 *  \return MPG123_OK on success
 */
MPG123_EXPORT int mpg123_close(mpg123_handle *mh);

/** Read from stream and decode up to outmemsize bytes.
 *
 *  Note: The type of outmemory changed to a void pointer in mpg123 1.26.0
 *  (API version 45).
 *
 *  \param mh handle
 *  \param outmemory address of output buffer to write to
 *  \param outmemsize maximum number of bytes to write
 *  \param done address to store the number of actually decoded bytes to
 *  \return MPG123_OK or error/message code
 */
MPG123_EXPORT int mpg123_read(mpg123_handle *mh
,	void *outmemory, size_t outmemsize, size_t *done );

/** Feed data for a stream that has been opened with mpg123_open_feed().
 *  It's give and take: You provide the bytestream, mpg123 gives you the decoded samples.
 *  \param mh handle
 *  \param in input buffer
 *  \param size number of input bytes
 *  \return MPG123_OK or error/message code.
 */
MPG123_EXPORT int mpg123_feed( mpg123_handle *mh
,	const unsigned char *in, size_t size );

/** Decode MPEG Audio from inmemory to outmemory. 
 *  This is very close to a drop-in replacement for old mpglib.
 *  When you give zero-sized output buffer the input will be parsed until 
 *  decoded data is available. This enables you to get MPG123_NEW_FORMAT (and query it) 
 *  without taking decoded data.
 *  Think of this function being the union of mpg123_read() and mpg123_feed() (which it actually is, sort of;-).
 *  You can actually always decide if you want those specialized functions in separate steps or one call this one here.
 *
 *  Note: The type of outmemory changed to a void pointer in mpg123 1.26.0
 *  (API version 45).
 *
 *  \param mh handle
 *  \param inmemory input buffer
 *  \param inmemsize number of input bytes
 *  \param outmemory output buffer
 *  \param outmemsize maximum number of output bytes
 *  \param done address to store the number of actually decoded bytes to
 *  \return error/message code (watch out especially for MPG123_NEED_MORE)
 */
MPG123_EXPORT int mpg123_decode( mpg123_handle *mh
,	const unsigned char *inmemory, size_t inmemsize
,	void *outmemory, size_t outmemsize, size_t *done );

/** Decode next MPEG frame to internal buffer
 *  or read a frame and return after setting a new format.
 *  \param mh handle
 *  \param num current frame offset gets stored there
 *  \param audio This pointer is set to the internal buffer to read the decoded audio from.
 *  \param bytes number of output bytes ready in the buffer
 *  \return MPG123_OK or error/message code
 */
MPG123_EXPORT int mpg123_decode_frame( mpg123_handle *mh
,	off_t *num, unsigned char **audio, size_t *bytes );

/** Decode current MPEG frame to internal buffer.
 * Warning: This is experimental API that might change in future releases!
 * Please watch mpg123 development closely when using it.
 *  \param mh handle
 *  \param num last frame offset gets stored there
 *  \param audio this pointer is set to the internal buffer to read the decoded audio from.
 *  \param bytes number of output bytes ready in the buffer
 *  \return MPG123_OK or error/message code
 */
MPG123_EXPORT int mpg123_framebyframe_decode( mpg123_handle *mh
,	off_t *num, unsigned char **audio, size_t *bytes );

/** Find, read and parse the next mp3 frame
 * Warning: This is experimental API that might change in future releases!
 * Please watch mpg123 development closely when using it.
 *  \param mh handle
 *  \return MPG123_OK or error/message code
 */
MPG123_EXPORT int mpg123_framebyframe_next(mpg123_handle *mh);

/** Get access to the raw input data for the last parsed frame.
 * This gives you a direct look (and write access) to the frame body data.
 * Together with the raw header, you can reconstruct the whole raw MPEG stream without junk and meta data, or play games by actually modifying the frame body data before decoding this frame (mpg123_framebyframe_decode()).
 * A more sane use would be to use this for CRC checking (see mpg123_info() and MPG123_CRC), the first two bytes of the body make up the CRC16 checksum, if present.
 * You can provide NULL for a parameter pointer when you are not interested in the value.
 *
 * \param mh handle
 * \param header the 4-byte MPEG header
 * \param bodydata pointer to the frame body stored in the handle (without the header)
 * \param bodybytes size of frame body in bytes (without the header)
 * \return MPG123_OK if there was a yet un-decoded frame to get the
 *    data from, MPG123_BAD_HANDLE or MPG123_ERR otherwise (without further
 *    explanation, the error state of the mpg123_handle is not modified by
 *    this function).
 */
MPG123_EXPORT int mpg123_framedata( mpg123_handle *mh
,	unsigned long *header, unsigned char **bodydata, size_t *bodybytes );

/** Get the input position (byte offset in stream) of the last parsed frame.
 *  This can be used for external seek index building, for example.
 *  It just returns the internally stored offset, regardless of validity --
 *  you ensure that a valid frame has been parsed before!
 * \param mh handle
 * \return byte offset in stream
 */
MPG123_EXPORT off_t mpg123_framepos(mpg123_handle *mh);

/*@}*/


/** \defgroup mpg123_seek mpg123 position and seeking
 *
 * Functions querying and manipulating position in the decoded audio bitstream.
 * The position is measured in decoded audio samples, or MPEG frame offset for the specific functions.
 * If gapless code is in effect, the positions are adjusted to compensate the skipped padding/delay - meaning, you should not care about that at all and just use the position defined for the samples you get out of the decoder;-)
 * The general usage is modelled after stdlib's ftell() and fseek().
 * Especially, the whence parameter for the seek functions has the same meaning as the one for fseek() and needs the same constants from stdlib.h: 
 * - SEEK_SET: set position to (or near to) specified offset
 * - SEEK_CUR: change position by offset from now
 * - SEEK_END: set position to offset from end
 *
 * Note that sample-accurate seek only works when gapless support has been enabled at compile time; seek is frame-accurate otherwise.
 * Also, really sample-accurate seeking (meaning that you get the identical sample value after seeking compared to plain decoding up to the position) is only guaranteed when you do not mess with the position code by using MPG123_UPSPEED, MPG123_DOWNSPEED or MPG123_START_FRAME. The first two mainly should cause trouble with NtoM resampling, but in any case with these options in effect, you have to keep in mind that the sample offset is not the same as counting the samples you get from decoding since mpg123 counts the skipped samples, too (or the samples played twice only once)!
 * Short: When you care about the sample position, don't mess with those parameters;-)
 * Also, seeking is not guaranteed to work for all streams (underlying stream may not support it).
 * And yet another caveat: If the stream is concatenated out of differing pieces (Frankenstein stream), seeking may suffer, too.
 *
 * @{
 */

/** Returns the current position in samples.
 *  On the next successful read, you'd get that sample.
 *  \param mh handle
 *  \return sample offset or MPG123_ERR (null handle)
 */
MPG123_EXPORT off_t mpg123_tell(mpg123_handle *mh);

/** Returns the frame number that the next read will give you data from.
 *  \param mh handle
 *  \return frame offset or MPG123_ERR (null handle)
 */
MPG123_EXPORT off_t mpg123_tellframe(mpg123_handle *mh);

/** Returns the current byte offset in the input stream.
 *  \param mh handle
 *  \return byte offset or MPG123_ERR (null handle)
 */
MPG123_EXPORT off_t mpg123_tell_stream(mpg123_handle *mh);

/** Seek to a desired sample offset.
 *  Usage is modelled afer the standard lseek().
 * \param mh handle
 * \param sampleoff offset in PCM samples
 * \param whence one of SEEK_SET, SEEK_CUR or SEEK_END
 * \return The resulting offset >= 0 or error/message code
 */
MPG123_EXPORT off_t mpg123_seek( mpg123_handle *mh
,	off_t sampleoff, int whence );

/** Seek to a desired sample offset in data feeding mode. 
 *  This just prepares things to be right only if you ensure that the next chunk of input data will be from input_offset byte position.
 *  \param mh handle
 *  \param sampleoff offset in PCM samples
 *  \param whence one of SEEK_SET, SEEK_CUR or SEEK_END
 *  \param input_offset The position it expects to be at the 
 *                      next time data is fed to mpg123_decode().
 *  \return The resulting offset >= 0 or error/message code */
MPG123_EXPORT off_t mpg123_feedseek( mpg123_handle *mh
,	off_t sampleoff, int whence, off_t *input_offset );

/** Seek to a desired MPEG frame offset.
 *  Usage is modelled afer the standard lseek().
 * \param mh handle
 * \param frameoff offset in MPEG frames
 * \param whence one of SEEK_SET, SEEK_CUR or SEEK_END
 * \return The resulting offset >= 0 or error/message code */
MPG123_EXPORT off_t mpg123_seek_frame( mpg123_handle *mh
,	off_t frameoff, int whence );

/** Return a MPEG frame offset corresponding to an offset in seconds.
 *  This assumes that the samples per frame do not change in the file/stream, which is a good assumption for any sane file/stream only.
 *  \return frame offset >= 0 or error/message code */
MPG123_EXPORT off_t mpg123_timeframe(mpg123_handle *mh, double sec);

/** Give access to the frame index table that is managed for seeking.
 *  You are asked not to modify the values... Use mpg123_set_index to set the
 *  seek index
 *  \param mh handle
 *  \param offsets pointer to the index array
 *  \param step one index byte offset advances this many MPEG frames
 *  \param fill number of recorded index offsets; size of the array
 *  \return MPG123_OK on success
 */
MPG123_EXPORT int mpg123_index( mpg123_handle *mh
,	off_t **offsets, off_t *step, size_t *fill );

/** Set the frame index table
 *  Setting offsets to NULL and fill > 0 will allocate fill entries. Setting offsets
 *  to NULL and fill to 0 will clear the index and free the allocated memory used by the index.
 *  \param mh handle
 *  \param offsets pointer to the index array
 *  \param step    one index byte offset advances this many MPEG frames
 *  \param fill    number of recorded index offsets; size of the array
 *  \return MPG123_OK on success
 */
MPG123_EXPORT int mpg123_set_index( mpg123_handle *mh
,	off_t *offsets, off_t step, size_t fill );

/** An old crutch to keep old mpg123 binaries happy.
 *  WARNING: This function is there only to avoid runtime linking errors with
 *  standalone mpg123 before version 1.23.0 (if you strangely update the
 *  library but not the end-user program) and actually is broken
 *  for various cases (p.ex. 24 bit output). Do never use. It might eventually
 *  be purged from the library.
 */
MPG123_EXPORT int mpg123_position( mpg123_handle *mh, off_t frame_offset, off_t buffered_bytes, off_t *current_frame, off_t *frames_left, double *current_seconds, double *seconds_left);

/*@}*/


/** \defgroup mpg123_voleq mpg123 volume and equalizer
 *
 * @{
 */

/** another channel enumeration, for left/right choice */
enum mpg123_channels
{
	 MPG123_LEFT=0x1	/**< The Left Channel. */
	,MPG123_RIGHT=0x2	/**< The Right Channel. */
	,MPG123_LR=0x3	/**< Both left and right channel; same as MPG123_LEFT|MPG123_RIGHT */
};

/** Set the 32 Band Audio Equalizer settings.
 *  \param mh handle
 *  \param channel Can be MPG123_LEFT, MPG123_RIGHT or MPG123_LEFT|MPG123_RIGHT for both.
 *  \param band The equaliser band to change (from 0 to 31)
 *  \param val The (linear) adjustment factor.
 *  \return MPG123_OK on success
 */
MPG123_EXPORT int mpg123_eq( mpg123_handle *mh
,	enum mpg123_channels channel, int band, double val );

/** Get the 32 Band Audio Equalizer settings.
 *  \param mh handle
 *  \param channel Can be MPG123_LEFT, MPG123_RIGHT or MPG123_LEFT|MPG123_RIGHT for (arithmetic mean of) both.
 *  \param band The equaliser band to change (from 0 to 31)
 *  \return The (linear) adjustment factor (zero for pad parameters) */
MPG123_EXPORT double mpg123_geteq(mpg123_handle *mh
                                 , enum mpg123_channels channel, int band);

/** Reset the 32 Band Audio Equalizer settings to flat
 *  \param mh handle
 *  \return MPG123_OK on success
 */
MPG123_EXPORT int mpg123_reset_eq(mpg123_handle *mh);

/** Set the absolute output volume including the RVA setting, 
 *  vol<0 just applies (a possibly changed) RVA setting.
 *  \param mh handle
 *  \param vol volume value (linear factor)
 *  \return MPG123_OK on success
 */
MPG123_EXPORT int mpg123_volume(mpg123_handle *mh, double vol);

/** Adjust output volume including the RVA setting by chosen amount
 *  \param mh handle
 *  \param change volume value (linear factor increment)
 *  \return MPG123_OK on success
 */
MPG123_EXPORT int mpg123_volume_change(mpg123_handle *mh, double change);

/** Return current volume setting, the actual value due to RVA, and the RVA 
 *  adjustment itself. It's all as double float value to abstract the sample 
 *  format. The volume values are linear factors / amplitudes (not percent) 
 *  and the RVA value is in decibels.
 *  \param mh handle
 *  \param base return address for base volume (linear factor)
 *  \param really return address for actual volume (linear factor)
 *  \param rva_db return address for RVA value (decibels)
 *  \return MPG123_OK on success
 */
MPG123_EXPORT int mpg123_getvolume(mpg123_handle *mh, double *base, double *really, double *rva_db);

/* TODO: Set some preamp in addition / to replace internal RVA handling? */

/*@}*/


/** \defgroup mpg123_status mpg123 status and information
 *
 * @{
 */

/** Enumeration of the mode types of Variable Bitrate */
enum mpg123_vbr {
	MPG123_CBR=0,	/**< Constant Bitrate Mode (default) */
	MPG123_VBR,		/**< Variable Bitrate Mode */
	MPG123_ABR		/**< Average Bitrate Mode */
};

/** Enumeration of the MPEG Versions */
enum mpg123_version {
	MPG123_1_0=0,	/**< MPEG Version 1.0 */
	MPG123_2_0,		/**< MPEG Version 2.0 */
	MPG123_2_5		/**< MPEG Version 2.5 */
};


/** Enumeration of the MPEG Audio mode.
 *  Only the mono mode has 1 channel, the others have 2 channels. */
enum mpg123_mode {
	MPG123_M_STEREO=0,	/**< Standard Stereo. */
	MPG123_M_JOINT,		/**< Joint Stereo. */
	MPG123_M_DUAL,		/**< Dual Channel. */
	MPG123_M_MONO		/**< Single Channel. */
};


/** Enumeration of the MPEG Audio flag bits */
enum mpg123_flags {
	MPG123_CRC=0x1,			/**< The bitstream is error protected using 16-bit CRC. */
	MPG123_COPYRIGHT=0x2,	/**< The bitstream is copyrighted. */
	MPG123_PRIVATE=0x4,		/**< The private bit has been set. */
	MPG123_ORIGINAL=0x8	/**< The bitstream is an original, not a copy. */
};

/** Data structure for storing information about a frame of MPEG Audio */
struct mpg123_frameinfo
{
	enum mpg123_version version;	/**< The MPEG version (1.0/2.0/2.5). */
	int layer;						/**< The MPEG Audio Layer (MP1/MP2/MP3). */
	long rate; 						/**< The sampling rate in Hz. */
	enum mpg123_mode mode;			/**< The audio mode (Mono, Stereo, Joint-stero, Dual Channel). */
	int mode_ext;					/**< The mode extension bit flag. */
	int framesize;					/**< The size of the frame (in bytes, including header). */
	enum mpg123_flags flags;		/**< MPEG Audio flag bits. Just now I realize that it should be declared as int, not enum. It's a bitwise combination of the enum values. */
	int emphasis;					/**< The emphasis type. */
	int bitrate;					/**< Bitrate of the frame (kbps). */
	int abr_rate;					/**< The target average bitrate. */
	enum mpg123_vbr vbr;			/**< The VBR mode. */
};

/** Data structure for even more detailed information out of the decoder,
  * for MPEG layer III only.
  * This was added to support the frame analyzer by the Lame project and
  * just follows what was used there before. You know what the fields mean
  * if you want use this structure. */
struct mpg123_moreinfo
{
	double xr[2][2][576];
	double sfb[2][2][22];  /* [2][2][SBMAX_l] */
	double sfb_s[2][2][3*13]; /* [2][2][3*SBMAX_s] */
	int qss[2][2];
	int big_values[2][2];
	int sub_gain[2][2][3];
	int scalefac_scale[2][2];
	int preflag[2][2];
	int blocktype[2][2];
	int mixed[2][2];
	int mainbits[2][2];
	int sfbits[2][2];
	int scfsi[2];
	int maindata;
	int padding;
};

/** Get frame information about the MPEG audio bitstream and store it in a mpg123_frameinfo structure.
 *  \param mh handle
 *  \param mi address of existing frameinfo structure to write to
 *  \return MPG123_OK on success
 */
MPG123_EXPORT int mpg123_info(mpg123_handle *mh, struct mpg123_frameinfo *mi);

/** Trigger collection of additional decoder information while decoding.
 *  \param mh handle
 *  \param mi pointer to data storage (NULL to disable collection)
 *  \return MPG123_OK if the collection was enabled/disabled as desired, MPG123_ERR
 *    otherwise (e.g. if the feature is disabled)
 */
MPG123_EXPORT int mpg123_set_moreinfo( mpg123_handle *mh
,	struct mpg123_moreinfo *mi );

/** Get the safe output buffer size for all cases
 *  (when you want to replace the internal buffer)
 *  \return safe buffer size
 */
MPG123_EXPORT size_t mpg123_safe_buffer(void);

/** Make a full parsing scan of each frame in the file. ID3 tags are found. An
 *  accurate length value is stored. Seek index will be filled. A seek back to
 *  current position is performed. At all, this function refuses work when
 *  stream is not seekable.
 *  \param mh handle
 *  \return MPG123_OK on success
 */
MPG123_EXPORT int mpg123_scan(mpg123_handle *mh);

/** Return, if possible, the full (expected) length of current track in
 *  MPEG frames.
 * \param mh handle
 * \return length >= 0 or MPG123_ERR if there is no length guess possible.
 */
MPG123_EXPORT off_t mpg123_framelength(mpg123_handle *mh);

/** Return, if possible, the full (expected) length of current
 *  track in samples (PCM frames).
 *
 *  This relies either on an Info frame at the beginning or a previous
 *  call to mpg123_scan() to get the real number of MPEG frames in a
 *  file. It will guess based on file size if neither Info frame nor
 *  scan data are present. In any case, there is no guarantee that the
 *  decoder will not give you more data, for example in case the open
 *  file gets appended to during decoding.
 * \param mh handle
 * \return length >= 0 or MPG123_ERR if there is no length guess possible.
 */
MPG123_EXPORT off_t mpg123_length(mpg123_handle *mh);

/** Override the value for file size in bytes.
 *  Useful for getting sensible track length values in feed mode or for HTTP streams.
 *  \param mh handle
 *  \param size file size in bytes
 *  \return MPG123_OK on success
 */
MPG123_EXPORT int mpg123_set_filesize(mpg123_handle *mh, off_t size);

/** Get MPEG frame duration in seconds.
 *  \param mh handle
 *  \return frame duration in seconds, <0 on error
 */
MPG123_EXPORT double mpg123_tpf(mpg123_handle *mh);

/** Get MPEG frame duration in samples.
 *  \param mh handle
 *  \return samples per frame for the most recently parsed frame; <0 on errors
 */
MPG123_EXPORT int mpg123_spf(mpg123_handle *mh);

/** Get and reset the clip count.
 *  \param mh handle
 *  \return count of clipped samples
 */
MPG123_EXPORT long mpg123_clip(mpg123_handle *mh);


/** The key values for state information from mpg123_getstate(). */
enum mpg123_state
{
	 MPG123_ACCURATE = 1 /**< Query if positons are currently accurate (integer value, 0 if false, 1 if true). */
	,MPG123_BUFFERFILL   /**< Get fill of internal (feed) input buffer as integer byte count returned as long and as double. An error is returned on integer overflow while converting to (signed) long, but the returned floating point value shold still be fine. */
	,MPG123_FRANKENSTEIN /**< Stream consists of carelessly stitched together files. Seeking may yield unexpected results (also with MPG123_ACCURATE, it may be confused). */
	,MPG123_FRESH_DECODER /**< Decoder structure has been updated, possibly indicating changed stream (integer value, 0 if false, 1 if true). Flag is cleared after retrieval. */
	,MPG123_ENC_DELAY /** Encoder delay read from Info tag (layer III, -1 if unknown). */
	,MPG123_ENC_PADDING /** Encoder padding read from Info tag (layer III, -1 if unknown). */
	,MPG123_DEC_DELAY /** Decoder delay (for layer III only, -1 otherwise). */
};

/** Get various current decoder/stream state information.
 *  \param mh handle
 *  \param key the key to identify the information to give.
 *  \param val the address to return (long) integer values to
 *  \param fval the address to return floating point values to
 *  \return MPG123_OK on success
 */
MPG123_EXPORT int mpg123_getstate( mpg123_handle *mh
,	enum mpg123_state key, long *val, double *fval );

/*@}*/


/** \defgroup mpg123_metadata mpg123 metadata handling
 *
 * Functions to retrieve the metadata from MPEG Audio files and streams.
 * Also includes string handling functions.
 *
 * @{
 */

/** Data structure for storing strings in a safer way than a standard C-String.
 *  Can also hold a number of null-terminated strings. */
typedef struct 
{
	char* p;     /**< pointer to the string data */
	size_t size; /**< raw number of bytes allocated */
	size_t fill; /**< number of used bytes (including closing zero byte) */
} mpg123_string;

/** Allocate and intialize a new string.
 *  \param val optional initial string value (can be NULL)
 */
MPG123_EXPORT mpg123_string* mpg123_new_string(const char* val);

/** Free memory of contents and the string structure itself.
 *  \param sb string handle
 */
MPG123_EXPORT void mpg123_delete_string(mpg123_string* sb);

/** Initialize an existing mpg123_string structure to {NULL, 0, 0}.
 *  If you hand in a NULL pointer here, your program should crash. The other
 *  string functions are more forgiving, but this one here is too basic.
 *  \param sb string handle (address of existing structure on your side)
 */
MPG123_EXPORT void mpg123_init_string(mpg123_string* sb);

/** Free-up memory of the contents of an mpg123_string (not the struct itself).
 *  This also calls mpg123_init_string() and hence is safe to be called
 *  repeatedly.
 *  \param sb string handle
 */
MPG123_EXPORT void mpg123_free_string(mpg123_string* sb);

/** Change the size of a mpg123_string
 *  \param sb string handle
 *  \param news new size in bytes
 *  \return 0 on error, 1 on success
 */
MPG123_EXPORT int mpg123_resize_string(mpg123_string* sb, size_t news);

/** Increase size of a mpg123_string if necessary (it may stay larger).
 *  Note that the functions for adding and setting in current libmpg123
 *  use this instead of mpg123_resize_string().
 *  That way, you can preallocate memory and safely work afterwards with
 *  pieces.
 *  \param sb string handle
 *  \param news new minimum size
 *  \return 0 on error, 1 on success
 */
MPG123_EXPORT int mpg123_grow_string(mpg123_string* sb, size_t news);

/** Copy the contents of one mpg123_string string to another.
 *  Yes the order of arguments is reversed compated to memcpy().
 *  \param from string handle
 *  \param to string handle
 *  \return 0 on error, 1 on success
 */
MPG123_EXPORT int mpg123_copy_string(mpg123_string* from, mpg123_string* to);

/** Move the contents of one mpg123_string string to another.
 *  This frees any memory associated with the target and moves over the
 *  pointers from the source, leaving the source without content after
 *  that. The only possible error is that you hand in NULL pointers.
 *  If you handed in a valid source, its contents will be gone, even if
 *  there was no target to move to. If you hand in a valid target, its
 *  original contents will also always be gone, to be replaced with the
 *  source's contents if there was some.
 *  \param from source string handle
 *  \param to   target string handle
 *  \return 0 on error, 1 on success
 */
MPG123_EXPORT int mpg123_move_string(mpg123_string* from, mpg123_string* to);

/** Append a C-String to an mpg123_string
 *  \param sb string handle
 *  \param stuff to append
 *  \return 0 on error, 1 on success
 */
MPG123_EXPORT int mpg123_add_string(mpg123_string* sb, const char* stuff);

/** Append a C-substring to an mpg123 string
 *  \param sb string handle
 *  \param stuff content to copy
 *  \param from offset to copy from
 *  \param count number of characters to copy (a null-byte is always appended)
 *  \return 0 on error, 1 on success
 */
MPG123_EXPORT int mpg123_add_substring( mpg123_string *sb
,	const char *stuff, size_t from, size_t count );

/** Set the content of a mpg123_string to a C-string
 *  \param sb string handle
 *  \param stuff content to copy
 *  \return 0 on error, 1 on success
 */
MPG123_EXPORT int mpg123_set_string(mpg123_string* sb, const char* stuff);

/** Set the content of a mpg123_string to a C-substring
 *  \param sb string handle
 *  \param stuff the future content
 *  \param from offset to copy from
 *  \param count number of characters to copy (a null-byte is always appended)
 *  \return 0 on error, 1 on success
 */
MPG123_EXPORT int mpg123_set_substring( mpg123_string *sb
,	const char *stuff, size_t from, size_t count );

/** Count characters in a mpg123 string (non-null bytes or Unicode points).
 *  This function is of limited use, as it does just count code points
 *  encoded in an UTF-8 string, only loosely related to the count of visible
 *  characters. Get your full Unicode handling support elsewhere.
 *  \param sb string handle
 *  \param utf8 a flag to tell if the string is in utf8 encoding
 *  \return character count
*/
MPG123_EXPORT size_t mpg123_strlen(mpg123_string *sb, int utf8);

/** Remove trailing \\r and \\n, if present.
 *  \param sb string handle
 *  \return 0 on error, 1 on success
 */
MPG123_EXPORT int mpg123_chomp_string(mpg123_string *sb);

/** Determine if two strings contain the same data.
 *  This only returns 1 if both given handles are non-NULL and
 *  if they are filled with the same bytes.
 *  \param a first string handle
 *  \param b second string handle
 *  \return 0 for different strings, 1 for identical
 */
MPG123_EXPORT int mpg123_same_string(mpg123_string *a, mpg123_string *b);

/** The mpg123 text encodings. This contains encodings we encounter in ID3 tags or ICY meta info. */
enum mpg123_text_encoding
{
	 mpg123_text_unknown  = 0 /**< Unkown encoding... mpg123_id3_encoding can return that on invalid codes. */
	,mpg123_text_utf8     = 1 /**< UTF-8 */
	,mpg123_text_latin1   = 2 /**< ISO-8859-1. Note that sometimes latin1 in ID3 is abused for totally different encodings. */
	,mpg123_text_icy      = 3 /**< ICY metadata encoding, usually CP-1252 but we take it as UTF-8 if it qualifies as such. */
	,mpg123_text_cp1252   = 4 /**< Really CP-1252 without any guessing. */
	,mpg123_text_utf16    = 5 /**< Some UTF-16 encoding. The last of a set of leading BOMs (byte order mark) rules.
	                           *   When there is no BOM, big endian ordering is used. Note that UCS-2 qualifies as UTF-8 when
	                           *   you don't mess with the reserved code points. If you want to decode little endian data
	                           *   without BOM you need to prepend 0xff 0xfe yourself. */
	,mpg123_text_utf16bom = 6 /**< Just an alias for UTF-16, ID3v2 has this as distinct code. */
	,mpg123_text_utf16be  = 7 /**< Another alias for UTF16 from ID3v2. Note, that, because of the mess that is reality,
	                           *   BOMs are used if encountered. There really is not much distinction between the UTF16 types for mpg123
	                           *   One exception: Since this is seen in ID3v2 tags, leading null bytes are skipped for all other UTF16
	                           *   types (we expect a BOM before real data there), not so for utf16be!*/
	,mpg123_text_max      = 7 /**< Placeholder for the maximum encoding value. */
};

/** The encoding byte values from ID3v2. */
enum mpg123_id3_enc
{
	 mpg123_id3_latin1   = 0 /**< Note: This sometimes can mean anything in practice... */
	,mpg123_id3_utf16bom = 1 /**< UTF16, UCS-2 ... it's all the same for practical purposes. */
	,mpg123_id3_utf16be  = 2 /**< Big-endian UTF-16, BOM see note for mpg123_text_utf16be. */
	,mpg123_id3_utf8     = 3 /**< Our lovely overly ASCII-compatible 8 byte encoding for the world. */
	,mpg123_id3_enc_max  = 3 /**< Placeholder to check valid range of encoding byte. */
};

/** Convert ID3 encoding byte to mpg123 encoding index.
 *  \param id3_enc_byte the ID3 encoding code
 *  \return the mpg123 encoding index
 */

MPG123_EXPORT enum mpg123_text_encoding mpg123_enc_from_id3(unsigned char id3_enc_byte);

/** Store text data in string, after converting to UTF-8 from indicated encoding
 *  A prominent error can be that you provided an unknown encoding value, or this build of libmpg123 lacks support for certain encodings (ID3 or ICY stuff missing).
 *  Also, you might want to take a bit of care with preparing the data; for example, strip leading zeroes (I have seen that).
 *  \param sb  target string
 *  \param enc mpg123 text encoding value
 *  \param source source buffer with plain unsigned bytes (you might need to cast from signed char)
 *  \param source_size number of bytes in the source buffer
 *  \return 0 on error, 1 on success (on error, mpg123_free_string is called on sb)
 */
MPG123_EXPORT int mpg123_store_utf8(mpg123_string *sb, enum mpg123_text_encoding enc, const unsigned char *source, size_t source_size);

/** Sub data structure for ID3v2, for storing various text fields (including comments).
 *  This is for ID3v2 COMM, TXXX and all the other text fields.
 *  Only COMM, TXXX and USLT may have a description, only COMM and USLT
 *  have a language.
 *  You should consult the ID3v2 specification for the use of the various text fields
 * ("frames" in ID3v2 documentation, I use "fields" here to separate from MPEG frames). */
typedef struct
{
	char lang[3]; /**< Three-letter language code (not terminated). */
	char id[4];   /**< The ID3v2 text field id, like TALB, TPE2, ... (4 characters, no string termination). */
	mpg123_string description; /**< Empty for the generic comment... */
	mpg123_string text;        /**< ... */
} mpg123_text;

/** The picture type values from ID3v2. */
enum mpg123_id3_pic_type
{
	 mpg123_id3_pic_other          =  0 /**< see ID3v2 docs */
	,mpg123_id3_pic_icon           =  1 /**< see ID3v2 docs */
	,mpg123_id3_pic_other_icon     =  2 /**< see ID3v2 docs */
	,mpg123_id3_pic_front_cover    =  3 /**< see ID3v2 docs */
	,mpg123_id3_pic_back_cover     =  4 /**< see ID3v2 docs */
	,mpg123_id3_pic_leaflet        =  5 /**< see ID3v2 docs */
	,mpg123_id3_pic_media          =  6 /**< see ID3v2 docs */
	,mpg123_id3_pic_lead           =  7 /**< see ID3v2 docs */
	,mpg123_id3_pic_artist         =  8 /**< see ID3v2 docs */
	,mpg123_id3_pic_conductor      =  9 /**< see ID3v2 docs */
	,mpg123_id3_pic_orchestra      = 10 /**< see ID3v2 docs */
	,mpg123_id3_pic_composer       = 11 /**< see ID3v2 docs */
	,mpg123_id3_pic_lyricist       = 12 /**< see ID3v2 docs */
	,mpg123_id3_pic_location       = 13 /**< see ID3v2 docs */
	,mpg123_id3_pic_recording      = 14 /**< see ID3v2 docs */
	,mpg123_id3_pic_performance    = 15 /**< see ID3v2 docs */
	,mpg123_id3_pic_video          = 16 /**< see ID3v2 docs */
	,mpg123_id3_pic_fish           = 17 /**< see ID3v2 docs */
	,mpg123_id3_pic_illustration   = 18 /**< see ID3v2 docs */
	,mpg123_id3_pic_artist_logo    = 19 /**< see ID3v2 docs */
	,mpg123_id3_pic_publisher_logo = 20 /**< see ID3v2 docs */
};

/** Sub data structure for ID3v2, for storing picture data including comment.
 *  This is for the ID3v2 APIC field. You should consult the ID3v2 specification
 *  for the use of the APIC field ("frames" in ID3v2 documentation, I use "fields"
 *  here to separate from MPEG frames). */
typedef struct
{
	char type;                 /**< mpg123_id3_pic_type value */
	mpg123_string description; /**< description string */
	mpg123_string mime_type;   /**< MIME type */
	size_t size;               /**< size in bytes */
	unsigned char* data;       /**< pointer to the image data */
} mpg123_picture;

/** Data structure for storing IDV3v2 tags.
 *  This structure is not a direct binary mapping with the file contents.
 *  The ID3v2 text frames are allowed to contain multiple strings.
 *  So check for null bytes until you reach the mpg123_string fill.
 *  All text is encoded in UTF-8. */
typedef struct
{
	unsigned char version; /**< 3 or 4 for ID3v2.3 or ID3v2.4. */
	mpg123_string *title;   /**< Title string (pointer into text_list). */
	mpg123_string *artist;  /**< Artist string (pointer into text_list). */
	mpg123_string *album;   /**< Album string (pointer into text_list). */
	mpg123_string *year;    /**< The year as a string (pointer into text_list). */
	mpg123_string *genre;   /**< Genre String (pointer into text_list). The genre string(s) may very well need postprocessing, esp. for ID3v2.3. */
	mpg123_string *comment; /**< Pointer to last encountered comment text with empty description. */
	/* Encountered ID3v2 fields are appended to these lists.
	   There can be multiple occurences, the pointers above always point to the last encountered data. */
	mpg123_text    *comment_list; /**< Array of comments. */
	size_t          comments;     /**< Number of comments. */
	mpg123_text    *text;         /**< Array of ID3v2 text fields (including USLT) */
	size_t          texts;        /**< Numer of text fields. */
	mpg123_text    *extra;        /**< The array of extra (TXXX) fields. */
	size_t          extras;       /**< Number of extra text (TXXX) fields. */
	mpg123_picture  *picture;     /**< Array of ID3v2 pictures fields (APIC).
		Only populated if MPG123_PICTURE flag is set! */
	size_t           pictures;    /**< Number of picture (APIC) fields. */
} mpg123_id3v2;

/** Data structure for ID3v1 tags (the last 128 bytes of a file).
 *  Don't take anything for granted (like string termination)!
 *  Also note the change ID3v1.1 did: comment[28] = 0; comment[29] = track_number
 *  It is your task to support ID3v1 only or ID3v1.1 ...*/
typedef struct
{
	char tag[3];         /**< Always the string "TAG", the classic intro. */
	char title[30];      /**< Title string.  */
	char artist[30];     /**< Artist string. */
	char album[30];      /**< Album string. */
	char year[4];        /**< Year string. */
	char comment[30];    /**< Comment string. */
	unsigned char genre; /**< Genre index. */
} mpg123_id3v1;

#define MPG123_ID3     0x3 /**< 0011 There is some ID3 info. Also matches 0010 or NEW_ID3. */
#define MPG123_NEW_ID3 0x1 /**< 0001 There is ID3 info that changed since last call to mpg123_id3. */
#define MPG123_ICY     0xc /**< 1100 There is some ICY info. Also matches 0100 or NEW_ICY.*/
#define MPG123_NEW_ICY 0x4 /**< 0100 There is ICY info that changed since last call to mpg123_icy. */

/** Query if there is (new) meta info, be it ID3 or ICY (or something new in future).
 *  \param mh handle
 *  \return combination of flags, 0 on error (same as "nothing new")
 */
MPG123_EXPORT int mpg123_meta_check(mpg123_handle *mh);

/** Clean up meta data storage (ID3v2 and ICY), freeing memory.
 *  \param mh handle
 */
MPG123_EXPORT void mpg123_meta_free(mpg123_handle *mh);

/** Point v1 and v2 to existing data structures wich may change on any next read/decode function call.
 *  v1 and/or v2 can be set to NULL when there is no corresponding data.
 *  \return MPG123_OK on success
 */
MPG123_EXPORT int mpg123_id3( mpg123_handle *mh
,	mpg123_id3v1 **v1, mpg123_id3v2 **v2 );

/** Return pointers to and size of stored raw ID3 data if storage has
 *  been configured with MPG123_RAW_ID3 and stream parsing passed the
 *  metadata already. Null value with zero size is a possibility!
 *  The storage can change at any next API call.
 *  \param v1 address to store pointer to v1 tag
 *  \param v1_size size of v1 data in bytes
 *  \param v2 address to store pointer to v2 tag
 *  \param v2_size size of v2 data in bytes
 *  \return MPG123_OK or MPG123_ERR. Only on MPG123_OK the output
 *          values are set.
 */
MPG123_EXPORT int mpg123_id3_raw( mpg123_handle *mh
,	unsigned char **v1, size_t *v1_size
,	unsigned char **v2, size_t *v2_size );

/** Point icy_meta to existing data structure wich may change on any next read/decode function call.
 *  \param mh handle
 *  \param icy_meta return address for ICY meta string (set to NULL if nothing there)
 *  \return MPG123_OK on success
 */
MPG123_EXPORT int mpg123_icy(mpg123_handle *mh, char **icy_meta);

/** Decode from windows-1252 (the encoding ICY metainfo used) to UTF-8.
 *  Note that this is very similar to mpg123_store_utf8(&sb, mpg123_text_icy, icy_text, strlen(icy_text+1)) .
 *  \param icy_text The input data in ICY encoding
 *  \return pointer to newly allocated buffer with UTF-8 data (You free() it!) */
MPG123_EXPORT char* mpg123_icy2utf8(const char* icy_text);


/* @} */


/** \defgroup mpg123_advpar mpg123 advanced parameter API
 *
 *  Direct access to a parameter set without full handle around it.
 *	Possible uses:
 *    - Influence behaviour of library _during_ initialization of handle (MPG123_VERBOSE).
 *    - Use one set of parameters for multiple handles.
 *
 *	The functions for handling mpg123_pars (mpg123_par() and mpg123_fmt() 
 *  family) directly return a fully qualified mpg123 error code, the ones 
 *  operating on full handles normally MPG123_OK or MPG123_ERR, storing the 
 *  specific error code itseld inside the handle. 
 *
 * @{
 */

/** Opaque structure for the libmpg123 decoder parameters. */
struct mpg123_pars_struct;

/** Opaque structure for the libmpg123 decoder parameters. */
typedef struct mpg123_pars_struct   mpg123_pars;

/** Create a handle with preset parameters.
 *  \param mp parameter handle
 *  \param decoder decoder choice
 *  \param error error code return address
 *  \return mpg123 handle
 */
MPG123_EXPORT mpg123_handle *mpg123_parnew( mpg123_pars *mp
,	const char* decoder, int *error );

/** Allocate memory for and return a pointer to a new mpg123_pars
 *  \param error error code return address
 *  \return new parameter handle
 */
MPG123_EXPORT mpg123_pars *mpg123_new_pars(int *error);

/** Delete and free up memory used by a mpg123_pars data structure
 *  \param mp parameter handle
 */
MPG123_EXPORT void mpg123_delete_pars(mpg123_pars* mp);

/** Configure mpg123 parameters to accept no output format at all, 
 *  use before specifying supported formats with mpg123_format
 *  \param mp parameter handle
 *  \return MPG123_OK on success
 */
MPG123_EXPORT int mpg123_fmt_none(mpg123_pars *mp);

/** Configure mpg123 parameters to accept all formats 
 *  (also any custom rate you may set) -- this is default. 
 *  \param mp parameter handle
 *  \return MPG123_OK on success
 */
MPG123_EXPORT int mpg123_fmt_all(mpg123_pars *mp);

/** Set the audio format support of a mpg123_pars in detail:
 * \param mp parameter handle
 * \param rate The sample rate value (in Hertz).
 * \param channels A combination of MPG123_STEREO and MPG123_MONO.
 * \param encodings A combination of accepted encodings for rate and channels,
 *                  p.ex MPG123_ENC_SIGNED16|MPG123_ENC_ULAW_8 (or 0 for no
 *                  support).
 * \return MPG123_OK on success
*/
MPG123_EXPORT int mpg123_fmt(mpg123_pars *mp
,	long rate, int channels, int encodings);

/** Set the audio format support of a mpg123_pars in detail:
 * \param mp parameter handle
 * \param rate The sample rate value (in Hertz). Special value 0 means
 *             all rates (reason for this variant of mpg123_fmt).
 * \param channels A combination of MPG123_STEREO and MPG123_MONO.
 * \param encodings A combination of accepted encodings for rate and channels,
 *                  p.ex MPG123_ENC_SIGNED16|MPG123_ENC_ULAW_8 (or 0 for no
 *                  support).
 * \return MPG123_OK on success
*/
MPG123_EXPORT int mpg123_fmt2(mpg123_pars *mp
,	long rate, int channels, int encodings);

/** Check to see if a specific format at a specific rate is supported
 *  by mpg123_pars.
 *  \param mp parameter handle
 *  \param rate sampling rate
 *  \param encoding encoding
 *  \return 0 for no support (that includes invalid parameters), MPG123_STEREO, 
 *          MPG123_MONO or MPG123_STEREO|MPG123_MONO. */
MPG123_EXPORT int mpg123_fmt_support(mpg123_pars *mp, long rate, int encoding);

/** Set a specific parameter, for a specific mpg123_pars, using a parameter 
 *  type key chosen from the mpg123_parms enumeration, to the specified value.
 *  \param mp parameter handle
 *  \param type parameter choice
 *  \param value integer value
 *  \param fvalue floating point value
 *  \return MPG123_OK on success
 */
MPG123_EXPORT int mpg123_par( mpg123_pars *mp
,	enum mpg123_parms type, long value, double fvalue );

/** Get a specific parameter, for a specific mpg123_pars. 
 *  See the mpg123_parms enumeration for a list of available parameters.
 *  \param mp parameter handle
 *  \param type parameter choice
 *  \param value integer value return address
 *  \param fvalue floating point value return address
 *  \return MPG123_OK on success
 */
MPG123_EXPORT int mpg123_getpar( mpg123_pars *mp
,	enum mpg123_parms type, long *value, double *fvalue);

/* @} */


/** \defgroup mpg123_lowio mpg123 low level I/O
  * You may want to do tricky stuff with I/O that does not work with mpg123's default file access or you want to make it decode into your own pocket...
  *
  * @{ */

/** Replace default internal buffer with user-supplied buffer.
  * Instead of working on it's own private buffer, mpg123 will directly use the one you provide for storing decoded audio.
  * Note that the required buffer size could be bigger than expected from output
  * encoding if libmpg123 has to convert from primary decoder output (p.ex. 32 bit
  * storage for 24 bit output).
  *
  *  Note: The type of data changed to a void pointer in mpg123 1.26.0
  *  (API version 45).
  *
  * \param mh handle
  * \param data pointer to user buffer
  * \param size of buffer in bytes
  * \return MPG123_OK on success
  */
MPG123_EXPORT int mpg123_replace_buffer(mpg123_handle *mh
,	void *data, size_t size);

/** The max size of one frame's decoded output with current settings.
 *  Use that to determine an appropriate minimum buffer size for decoding one frame.
 *  \param mh handle
 *  \return maximum decoded data size in bytes
 */
MPG123_EXPORT size_t mpg123_outblock(mpg123_handle *mh);

/** Replace low-level stream access functions; read and lseek as known in POSIX.
 *  You can use this to make any fancy file opening/closing yourself, 
 *  using mpg123_open_fd() to set the file descriptor for your read/lseek
 *  (doesn't need to be a "real" file descriptor...).
 *  Setting a function to NULL means that the default internal read is 
 *  used (active from next mpg123_open call on).
 *  Note: As it would be troublesome to mess with this while having a file open,
 *  this implies mpg123_close().
 * \param mh handle
 * \param r_read callback for reading (behaviour like POSIX read)
 * \param r_lseek callback for seeking (like POSIX lseek)
 * \return MPG123_OK on success
 */
MPG123_EXPORT int mpg123_replace_reader( mpg123_handle *mh
,	ssize_t (*r_read) (int, void *, size_t)
,	off_t (*r_lseek)(int, off_t, int)
);

/** Replace I/O functions with your own ones operating on some kind of
 *  handle instead of integer descriptors.
 *  The handle is a void pointer, so you can pass any data you want...
 *  mpg123_open_handle() is the call you make to use the I/O defined here.
 *  There is no fallback to internal read/seek here.
 *  Note: As it would be troublesome to mess with this while having a file open,
 *  this mpg123_close() is implied here.
 *  \param mh handle
 *  \param r_read callback for reading (behaviour like POSIX read)
 *  \param r_lseek callback for seeking (like POSIX lseek)
 *  \param cleanup A callback to clean up an I/O handle on mpg123_close,
 *         can be NULL for none (you take care of cleaning your handles).
 * \return MPG123_OK on success
 */
MPG123_EXPORT int mpg123_replace_reader_handle( mpg123_handle *mh
,	ssize_t (*r_read) (void *, void *, size_t)
,	off_t (*r_lseek)(void *, off_t, int)
,	void (*cleanup)(void*) );

/* @} */

#ifdef __cplusplus
}
#endif

#endif
