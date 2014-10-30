// Copyright (C) 2011 Milo Yip
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#ifndef RAPIDJSON_RAPIDJSON_H_
#define RAPIDJSON_RAPIDJSON_H_

// Copyright (c) 2011 Milo Yip (miloyip@gmail.com)
// Version 0.1

/*!\file rapidjson.h
    \brief common definitions and configuration

    \see RAPIDJSON_CONFIG
 */

/*! \defgroup RAPIDJSON_CONFIG RapidJSON configuration
    \brief Configuration macros for library features

    Some RapidJSON features are configurable to adapt the library to a wide
    variety of platforms, environments and usage scenarios.  Most of the
    features can be configured in terms of overriden or predefined
    preprocessor macros at compile-time.

    Some additional customization is available in the \ref RAPIDJSON_ERRORS APIs.

    \note These macros should be given on the compiler command-line
          (where applicable)  to avoid inconsistent values when compiling
          different translation units of a single application.
 */

#include <cstdlib>  // malloc(), realloc(), free(), size_t
#include <cstring>  // memset(), memcpy(), memmove(), memcmp()

///////////////////////////////////////////////////////////////////////////////
// RAPIDJSON_NO_INT64DEFINE

/*! \def RAPIDJSON_NO_INT64DEFINE
    \ingroup RAPIDJSON_CONFIG
    \brief Use external 64-bit integer types.

    RapidJSON requires the 64-bit integer types \c int64_t and  \c uint64_t types
    to be available at global scope.

    If users have their own definition, define RAPIDJSON_NO_INT64DEFINE to
    prevent RapidJSON from defining its own types.
*/
#ifndef RAPIDJSON_NO_INT64DEFINE
//!@cond RAPIDJSON_HIDDEN_FROM_DOXYGEN
#ifdef _MSC_VER
#include "msinttypes/stdint.h"
#include "msinttypes/inttypes.h"
#else
// Other compilers should have this.
#include <stdint.h>
#include <inttypes.h>
#endif
//!@endcond
#ifdef RAPIDJSON_DOXYGEN_RUNNING
#define RAPIDJSON_NO_INT64DEFINE
#endif
#endif // RAPIDJSON_NO_INT64TYPEDEF

///////////////////////////////////////////////////////////////////////////////
// RAPIDJSON_FORCEINLINE

#ifndef RAPIDJSON_FORCEINLINE
//!@cond RAPIDJSON_HIDDEN_FROM_DOXYGEN
#ifdef _MSC_VER
#define RAPIDJSON_FORCEINLINE __forceinline
#elif defined(__GNUC__) && __GNUC__ >= 4
#define RAPIDJSON_FORCEINLINE __attribute__((always_inline))
#else
#define RAPIDJSON_FORCEINLINE
#endif
//!@endcond
#endif // RAPIDJSON_FORCEINLINE

///////////////////////////////////////////////////////////////////////////////
// RAPIDJSON_ENDIAN
#define RAPIDJSON_LITTLEENDIAN  0   //!< Little endian machine
#define RAPIDJSON_BIGENDIAN     1   //!< Big endian machine

//! Endianness of the machine.
/*!
    \def RAPIDJSON_ENDIAN
    \ingroup RAPIDJSON_CONFIG

    GCC 4.6 provided macro for detecting endianness of the target machine. But other
    compilers may not have this. User can define RAPIDJSON_ENDIAN to either
    \ref RAPIDJSON_LITTLEENDIAN or \ref RAPIDJSON_BIGENDIAN.

    Default detection implemented with reference to
    \li https://gcc.gnu.org/onlinedocs/gcc-4.6.0/cpp/Common-Predefined-Macros.html
    \li http://www.boost.org/doc/libs/1_42_0/boost/detail/endian.hpp
*/
#ifndef RAPIDJSON_ENDIAN
// Detect with GCC 4.6's macro
#  ifdef __BYTE_ORDER__
#    if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#      define RAPIDJSON_ENDIAN RAPIDJSON_LITTLEENDIAN
#    elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#      define RAPIDJSON_ENDIAN RAPIDJSON_BIGENDIAN
#    else
#      error Unknown machine endianess detected. User needs to define RAPIDJSON_ENDIAN.
#    endif // __BYTE_ORDER__
// Detect with GLIBC's endian.h
#  elif defined(__GLIBC__)
#    include <endian.h>
#    if (__BYTE_ORDER == __LITTLE_ENDIAN)
#      define RAPIDJSON_ENDIAN RAPIDJSON_LITTLEENDIAN
#    elif (__BYTE_ORDER == __BIG_ENDIAN)
#      define RAPIDJSON_ENDIAN RAPIDJSON_BIGENDIAN
#    else
#      error Unknown machine endianess detected. User needs to define RAPIDJSON_ENDIAN.
#   endif // __GLIBC__
// Detect with _LITTLE_ENDIAN and _BIG_ENDIAN macro
#  elif defined(_LITTLE_ENDIAN) && !defined(_BIG_ENDIAN)
#    define RAPIDJSON_ENDIAN RAPIDJSON_LITTLEENDIAN
#  elif defined(_BIG_ENDIAN) && !defined(_LITTLE_ENDIAN)
#    define RAPIDJSON_ENDIAN RAPIDJSON_BIGENDIAN
// Detect with architecture macros
#  elif defined(__sparc) || defined(__sparc__) || defined(_POWER) || defined(__powerpc__) || defined(__ppc__) || defined(__hpux) || defined(__hppa) || defined(_MIPSEB) || defined(_POWER) || defined(__s390__)
#    define RAPIDJSON_ENDIAN RAPIDJSON_BIGENDIAN
#  elif defined(__i386__) || defined(__alpha__) || defined(__ia64) || defined(__ia64__) || defined(_M_IX86) || defined(_M_IA64) || defined(_M_ALPHA) || defined(__amd64) || defined(__amd64__) || defined(_M_AMD64) || defined(__x86_64) || defined(__x86_64__) || defined(_M_X64) || defined(__bfin__)
#    define RAPIDJSON_ENDIAN RAPIDJSON_LITTLEENDIAN
#  elif defined(RAPIDJSON_DOXYGEN_RUNNING)
#    define RAPIDJSON_ENDIAN
#  else
#    error Unknown machine endianess detected. User needs to define RAPIDJSON_ENDIAN.   
#  endif
#endif // RAPIDJSON_ENDIAN

///////////////////////////////////////////////////////////////////////////////
// RAPIDJSON_64BIT

//! Whether using 64-bit architecture
#ifndef RAPIDJSON_64BIT
#if defined(__LP64__) || defined(_WIN64)
#define RAPIDJSON_64BIT 1
#else
#define RAPIDJSON_64BIT 0
#endif
#endif // RAPIDJSON_64BIT

///////////////////////////////////////////////////////////////////////////////
// RAPIDJSON_ALIGN

//! Data alignment of the machine.
/*! \ingroup RAPIDJSON_CONFIG
    \param x pointer to align

    Some machines require strict data alignment. Currently the default uses 4 bytes
    alignment. User can customize by defining the RAPIDJSON_ALIGN function macro.,
*/
#ifndef RAPIDJSON_ALIGN
#define RAPIDJSON_ALIGN(x) ((x + 3u) & ~3u)
#endif

///////////////////////////////////////////////////////////////////////////////
// RAPIDJSON_UINT64_C2

//! Construct a 64-bit literal by a pair of 32-bit integer.
/*!
    64-bit literal with or without ULL suffix is prone to compiler warnings.
    UINT64_C() is C macro which cause compilation problems.
    Use this macro to define 64-bit constants by a pair of 32-bit integer.
*/
#ifndef RAPIDJSON_UINT64_C2
#define RAPIDJSON_UINT64_C2(high32, low32) ((static_cast<uint64_t>(high32) << 32) | static_cast<uint64_t>(low32))
#endif

///////////////////////////////////////////////////////////////////////////////
// RAPIDJSON_SSE2/RAPIDJSON_SSE42/RAPIDJSON_SIMD

/*! \def RAPIDJSON_SIMD
    \ingroup RAPIDJSON_CONFIG
    \brief Enable SSE2/SSE4.2 optimization.

    RapidJSON supports optimized implementations for some parsing operations
    based on the SSE2 or SSE4.2 SIMD extensions on modern Intel-compatible
    processors.

    To enable these optimizations, two different symbols can be defined;
    \code
    // Enable SSE2 optimization.
    #define RAPIDJSON_SSE2

    // Enable SSE4.2 optimization.
    #define RAPIDJSON_SSE42
    \endcode

    \c RAPIDJSON_SSE42 takes precedence, if both are defined.

    If any of these symbols is defined, RapidJSON defines the macro
    \c RAPIDJSON_SIMD to indicate the availability of the optimized code.
*/
#if defined(RAPIDJSON_SSE2) || defined(RAPIDJSON_SSE42) \
    || defined(RAPIDJSON_DOXYGEN_RUNNING)
#define RAPIDJSON_SIMD
#endif

///////////////////////////////////////////////////////////////////////////////
// RAPIDJSON_NO_SIZETYPEDEFINE

#ifndef RAPIDJSON_NO_SIZETYPEDEFINE
/*! \def RAPIDJSON_NO_SIZETYPEDEFINE
    \ingroup RAPIDJSON_CONFIG
    \brief User-provided \c SizeType definition.

    In order to avoid using 32-bit size types for indexing strings and arrays,
    define this preprocessor symbol and provide the type rapidjson::SizeType
    before including RapidJSON:
    \code
    #define RAPIDJSON_NO_SIZETYPEDEFINE
    namespace rapidjson { typedef ::std::size_t SizeType; }
    #include "rapidjson/..."
    \endcode

    \see rapidjson::SizeType
*/
#ifdef RAPIDJSON_DOXYGEN_RUNNING
#define RAPIDJSON_NO_SIZETYPEDEFINE
#endif
namespace rapidjson {
//! Size type (for string lengths, array sizes, etc.)
/*! RapidJSON uses 32-bit array/string indices even on 64-bit platforms,
    instead of using \c size_t. Users may override the SizeType by defining
    \ref RAPIDJSON_NO_SIZETYPEDEFINE.
*/
typedef unsigned SizeType;
} // namespace rapidjson
#endif

// always import std::size_t to rapidjson namespace
namespace rapidjson {
using std::size_t;
} // namespace rapidjson

///////////////////////////////////////////////////////////////////////////////
// RAPIDJSON_ASSERT

//! Assertion.
/*! \ingroup RAPIDJSON_CONFIG
    By default, rapidjson uses C \c assert() for internal assertions.
    User can override it by defining RAPIDJSON_ASSERT(x) macro.

    \note Parsing errors are handled and can be customized by the
          \ref RAPIDJSON_ERRORS APIs.
*/
#ifndef RAPIDJSON_ASSERT
#include <cassert>
#define RAPIDJSON_ASSERT(x) assert(x)
#endif // RAPIDJSON_ASSERT

///////////////////////////////////////////////////////////////////////////////
// RAPIDJSON_STATIC_ASSERT

// Adopt from boost
#ifndef RAPIDJSON_STATIC_ASSERT
//!@cond RAPIDJSON_HIDDEN_FROM_DOXYGEN
namespace rapidjson {

template <bool x> struct STATIC_ASSERTION_FAILURE;
template <> struct STATIC_ASSERTION_FAILURE<true> { enum { value = 1 }; };
template<int x> struct StaticAssertTest {};
} // namespace rapidjson

#define RAPIDJSON_JOIN(X, Y) RAPIDJSON_DO_JOIN(X, Y)
#define RAPIDJSON_DO_JOIN(X, Y) RAPIDJSON_DO_JOIN2(X, Y)
#define RAPIDJSON_DO_JOIN2(X, Y) X##Y

#if defined(__GNUC__)
#define RAPIDJSON_STATIC_ASSERT_UNUSED_ATTRIBUTE __attribute__((unused))
#else
#define RAPIDJSON_STATIC_ASSERT_UNUSED_ATTRIBUTE 
#endif
//!@endcond

/*! \def RAPIDJSON_STATIC_ASSERT
    \brief (Internal) macro to check for conditions at compile-time
    \param x compile-time condition
    \hideinitializer
 */
#define RAPIDJSON_STATIC_ASSERT(x) typedef ::rapidjson::StaticAssertTest<\
    sizeof(::rapidjson::STATIC_ASSERTION_FAILURE<bool(x) >)>\
    RAPIDJSON_JOIN(StaticAssertTypedef, __LINE__) RAPIDJSON_STATIC_ASSERT_UNUSED_ATTRIBUTE
#endif

///////////////////////////////////////////////////////////////////////////////
// Helpers

//!@cond RAPIDJSON_HIDDEN_FROM_DOXYGEN

#define RAPIDJSON_MULTILINEMACRO_BEGIN do {  
#define RAPIDJSON_MULTILINEMACRO_END \
} while((void)0, 0)

// adopted from Boost
#define RAPIDJSON_VERSION_CODE(x,y,z) \
  (((x)*100000) + ((y)*100) + (z))

// token stringification
#define RAPIDJSON_STRINGIFY(x) RAPIDJSON_DO_STRINGIFY(x)
#define RAPIDJSON_DO_STRINGIFY(x) #x

///////////////////////////////////////////////////////////////////////////////
// RAPIDJSON_DIAG_PUSH/POP, RAPIDJSON_DIAG_OFF

#if defined(__GNUC__)
#define RAPIDJSON_GNUC \
    RAPIDJSON_VERSION_CODE(__GNUC__,__GNUC_MINOR__,__GNUC_PATCHLEVEL__)
#endif

#if defined(__clang__) || (defined(RAPIDJSON_GNUC) && RAPIDJSON_GNUC >= RAPIDJSON_VERSION_CODE(4,2,0))

#define RAPIDJSON_PRAGMA(x) _Pragma(RAPIDJSON_STRINGIFY(x))
#define RAPIDJSON_DIAG_PRAGMA(x) RAPIDJSON_PRAGMA(GCC diagnostic x)
#define RAPIDJSON_DIAG_OFF(x) \
    RAPIDJSON_DIAG_PRAGMA(ignored RAPIDJSON_STRINGIFY(RAPIDJSON_JOIN(-W,x)))

// push/pop support in Clang and GCC>=4.6
#if defined(__clang__) || (defined(RAPIDJSON_GNUC) && RAPIDJSON_GNUC >= RAPIDJSON_VERSION_CODE(4,6,0))
#define RAPIDJSON_DIAG_PUSH RAPIDJSON_DIAG_PRAGMA(push)
#define RAPIDJSON_DIAG_POP  RAPIDJSON_DIAG_PRAGMA(pop)
#else // GCC >= 4.2, < 4.6
#define RAPIDJSON_DIAG_PUSH /* ignored */
#define RAPIDJSON_DIAG_POP /* ignored */
#endif

#elif defined(_MSC_VER)

// pragma (MSVC specific)
#define RAPIDJSON_PRAGMA(x) __pragma(x)
#define RAPIDJSON_DIAG_PRAGMA(x) RAPIDJSON_PRAGMA(warning(x))

#define RAPIDJSON_DIAG_OFF(x) RAPIDJSON_DIAG_PRAGMA(disable: x)
#define RAPIDJSON_DIAG_PUSH RAPIDJSON_DIAG_PRAGMA(push)
#define RAPIDJSON_DIAG_POP  RAPIDJSON_DIAG_PRAGMA(pop)

#else

#define RAPIDJSON_DIAG_OFF(x) /* ignored */
#define RAPIDJSON_DIAG_PUSH   /* ignored */
#define RAPIDJSON_DIAG_POP    /* ignored */

#endif // RAPIDJSON_DIAG_*

///////////////////////////////////////////////////////////////////////////////
// C++11 features

#ifndef RAPIDJSON_HAS_CXX11_RVALUE_REFS
#if defined(__clang__)
#define RAPIDJSON_HAS_CXX11_RVALUE_REFS __has_feature(cxx_rvalue_references)
#elif (defined(RAPIDJSON_GNUC) && (RAPIDJSON_GNUC >= RAPIDJSON_VERSION_CODE(4,3,0)) && defined(__GXX_EXPERIMENTAL_CXX0X__)) || \
      (defined(_MSC_VER) && _MSC_VER >= 1600)

#define RAPIDJSON_HAS_CXX11_RVALUE_REFS 1
#else
#define RAPIDJSON_HAS_CXX11_RVALUE_REFS 0
#endif
#endif // RAPIDJSON_HAS_CXX11_RVALUE_REFS

#ifndef RAPIDJSON_HAS_CXX11_NOEXCEPT
#if defined(__clang__)
#define RAPIDJSON_HAS_CXX11_NOEXCEPT __has_feature(cxx_noexcept)
#elif (defined(RAPIDJSON_GNUC) && (RAPIDJSON_GNUC >= RAPIDJSON_VERSION_CODE(4,6,0)) && defined(__GXX_EXPERIMENTAL_CXX0X__))
//    (defined(_MSC_VER) && _MSC_VER >= ????) // not yet supported
#define RAPIDJSON_HAS_CXX11_NOEXCEPT 1
#else
#define RAPIDJSON_HAS_CXX11_NOEXCEPT 0
#endif
#endif
#if RAPIDJSON_HAS_CXX11_NOEXCEPT
#define RAPIDJSON_NOEXCEPT noexcept
#else
#define RAPIDJSON_NOEXCEPT /* noexcept */
#endif // RAPIDJSON_HAS_CXX11_NOEXCEPT

// no automatic detection, yet
#ifndef RAPIDJSON_HAS_CXX11_TYPETRAITS
#define RAPIDJSON_HAS_CXX11_TYPETRAITS 0
#endif

//!@endcond

///////////////////////////////////////////////////////////////////////////////
// Allocators and Encodings

#include "allocators.h"
#include "encodings.h"

//! main RapidJSON namespace
namespace rapidjson {

///////////////////////////////////////////////////////////////////////////////
//  Stream

/*! \class rapidjson::Stream
    \brief Concept for reading and writing characters.

    For read-only stream, no need to implement PutBegin(), Put(), Flush() and PutEnd().

    For write-only stream, only need to implement Put() and Flush().

\code
concept Stream {
    typename Ch;    //!< Character type of the stream.

    //! Read the current character from stream without moving the read cursor.
    Ch Peek() const;

    //! Read the current character from stream and moving the read cursor to next character.
    Ch Take();

    //! Get the current read cursor.
    //! \return Number of characters read from start.
    size_t Tell();

    //! Begin writing operation at the current read pointer.
    //! \return The begin writer pointer.
    Ch* PutBegin();

    //! Write a character.
    void Put(Ch c);

    //! Flush the buffer.
    void Flush();

    //! End the writing operation.
    //! \param begin The begin write pointer returned by PutBegin().
    //! \return Number of characters written.
    size_t PutEnd(Ch* begin);
}
\endcode
*/

//! Provides additional information for stream.
/*!
    By using traits pattern, this type provides a default configuration for stream.
    For custom stream, this type can be specialized for other configuration.
    See TEST(Reader, CustomStringStream) in readertest.cpp for example.
*/
template<typename Stream>
struct StreamTraits {
    //! Whether to make local copy of stream for optimization during parsing.
    /*!
        By default, for safety, streams do not use local copy optimization.
        Stream that can be copied fast should specialize this, like StreamTraits<StringStream>.
    */
    enum { copyOptimization = 0 };
};

//! Put N copies of a character to a stream.
template<typename Stream, typename Ch>
inline void PutN(Stream& stream, Ch c, size_t n) {
    for (size_t i = 0; i < n; i++)
        stream.Put(c);
}

///////////////////////////////////////////////////////////////////////////////
// StringStream

//! Read-only string stream.
/*! \note implements Stream concept
*/
template <typename Encoding>
struct GenericStringStream {
    typedef typename Encoding::Ch Ch;

    GenericStringStream(const Ch *src) : src_(src), head_(src) {}

    Ch Peek() const { return *src_; }
    Ch Take() { return *src_++; }
    size_t Tell() const { return static_cast<size_t>(src_ - head_); }

    Ch* PutBegin() { RAPIDJSON_ASSERT(false); return 0; }
    void Put(Ch) { RAPIDJSON_ASSERT(false); }
    void Flush() { RAPIDJSON_ASSERT(false); }
    size_t PutEnd(Ch*) { RAPIDJSON_ASSERT(false); return 0; }

    const Ch* src_;     //!< Current read position.
    const Ch* head_;    //!< Original head of the string.
};

template <typename Encoding>
struct StreamTraits<GenericStringStream<Encoding> > {
    enum { copyOptimization = 1 };
};

//! String stream with UTF8 encoding.
typedef GenericStringStream<UTF8<> > StringStream;

///////////////////////////////////////////////////////////////////////////////
// InsituStringStream

//! A read-write string stream.
/*! This string stream is particularly designed for in-situ parsing.
    \note implements Stream concept
*/
template <typename Encoding>
struct GenericInsituStringStream {
    typedef typename Encoding::Ch Ch;

    GenericInsituStringStream(Ch *src) : src_(src), dst_(0), head_(src) {}

    // Read
    Ch Peek() { return *src_; }
    Ch Take() { return *src_++; }
    size_t Tell() { return static_cast<size_t>(src_ - head_); }

    // Write
    void Put(Ch c) { RAPIDJSON_ASSERT(dst_ != 0); *dst_++ = c; }

    Ch* PutBegin() { return dst_ = src_; }
    size_t PutEnd(Ch* begin) { return static_cast<size_t>(dst_ - begin); }
    void Flush() {}

    Ch* Push(size_t count) { Ch* begin = dst_; dst_ += count; return begin; }
    void Pop(size_t count) { dst_ -= count; }

    Ch* src_;
    Ch* dst_;
    Ch* head_;
};

template <typename Encoding>
struct StreamTraits<GenericInsituStringStream<Encoding> > {
    enum { copyOptimization = 1 };
};

//! Insitu string stream with UTF8 encoding.
typedef GenericInsituStringStream<UTF8<> > InsituStringStream;

///////////////////////////////////////////////////////////////////////////////
// Type

//! Type of JSON value
enum Type {
    kNullType = 0,      //!< null
    kFalseType = 1,     //!< false
    kTrueType = 2,      //!< true
    kObjectType = 3,    //!< object
    kArrayType = 4,     //!< array 
    kStringType = 5,    //!< string
    kNumberType = 6     //!< number
};

} // namespace rapidjson

#endif // RAPIDJSON_RAPIDJSON_H_
