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

#ifndef RAPIDJSON_READER_H_
#define RAPIDJSON_READER_H_

/*! \file reader.h */

#include "rapidjson.h"
#include "encodings.h"
#include "internal/meta.h"
#include "internal/pow10.h"
#include "internal/stack.h"

#if defined(RAPIDJSON_SIMD) && defined(_MSC_VER)
#include <intrin.h>
#pragma intrinsic(_BitScanForward)
#endif
#ifdef RAPIDJSON_SSE42
#include <nmmintrin.h>
#elif defined(RAPIDJSON_SSE2)
#include <emmintrin.h>
#endif

#ifdef _MSC_VER
RAPIDJSON_DIAG_PUSH
RAPIDJSON_DIAG_OFF(4127)  // conditional expression is constant
RAPIDJSON_DIAG_OFF(4702)  // unreachable code
#endif

//!@cond RAPIDJSON_HIDDEN_FROM_DOXYGEN
#define RAPIDJSON_NOTHING /* deliberately empty */
#ifndef RAPIDJSON_PARSE_ERROR_EARLY_RETURN
#define RAPIDJSON_PARSE_ERROR_EARLY_RETURN(value) \
    RAPIDJSON_MULTILINEMACRO_BEGIN \
    if (HasParseError()) { return value; } \
    RAPIDJSON_MULTILINEMACRO_END
#endif
#define RAPIDJSON_PARSE_ERROR_EARLY_RETURN_VOID \
    RAPIDJSON_PARSE_ERROR_EARLY_RETURN(RAPIDJSON_NOTHING)
//!@endcond

/*! \def RAPIDJSON_PARSE_ERROR_NORETURN
    \ingroup RAPIDJSON_ERRORS
    \brief Macro to indicate a parse error.
    \param parseErrorCode \ref rapidjson::ParseErrorCode of the error
    \param offset  position of the error in JSON input (\c size_t)

    This macros can be used as a customization point for the internal
    error handling mechanism of RapidJSON.

    A common usage model is to throw an exception instead of requiring the
    caller to explicitly check the \ref rapidjson::GenericReader::Parse's
    return value:

    \code
    #define RAPIDJSON_PARSE_ERROR_NORETURN(parseErrorCode,offset) \
       throw ParseException(parseErrorCode, #parseErrorCode, offset)

    #include <stdexcept>               // std::runtime_error
    #include "rapidjson/error/error.h" // rapidjson::ParseResult

    struct ParseException : std::runtime_error, rapidjson::ParseResult {
      ParseException(rapidjson::ParseErrorCode code, const char* msg, size_t offset)
        : std::runtime_error(msg), ParseResult(code, offset) {}
    };

    #include "rapidjson/reader.h"
    \endcode

    \see RAPIDJSON_PARSE_ERROR, rapidjson::GenericReader::Parse
 */
#ifndef RAPIDJSON_PARSE_ERROR_NORETURN
#define RAPIDJSON_PARSE_ERROR_NORETURN(parseErrorCode, offset) \
    RAPIDJSON_MULTILINEMACRO_BEGIN \
    RAPIDJSON_ASSERT(!HasParseError()); /* Error can only be assigned once */ \
    SetParseError(parseErrorCode, offset); \
    RAPIDJSON_MULTILINEMACRO_END
#endif

/*! \def RAPIDJSON_PARSE_ERROR
    \ingroup RAPIDJSON_ERRORS
    \brief (Internal) macro to indicate and handle a parse error.
    \param parseErrorCode \ref rapidjson::ParseErrorCode of the error
    \param offset  position of the error in JSON input (\c size_t)

    Invokes RAPIDJSON_PARSE_ERROR_NORETURN and stops the parsing.

    \see RAPIDJSON_PARSE_ERROR_NORETURN
    \hideinitializer
 */
#ifndef RAPIDJSON_PARSE_ERROR
#define RAPIDJSON_PARSE_ERROR(parseErrorCode, offset) \
    RAPIDJSON_MULTILINEMACRO_BEGIN \
    RAPIDJSON_PARSE_ERROR_NORETURN(parseErrorCode, offset); \
    RAPIDJSON_PARSE_ERROR_EARLY_RETURN_VOID; \
    RAPIDJSON_MULTILINEMACRO_END
#endif

#include "error/error.h" // ParseErrorCode, ParseResult

namespace rapidjson {

///////////////////////////////////////////////////////////////////////////////
// ParseFlag

//! Combination of parseFlags
/*! \see Reader::Parse, Document::Parse, Document::ParseInsitu, Document::ParseStream
 */
enum ParseFlag {
    kParseDefaultFlags = 0,         //!< Default parse flags. Non-destructive parsing. Text strings are decoded into allocated buffer.
    kParseInsituFlag = 1,           //!< In-situ(destructive) parsing.
    kParseValidateEncodingFlag = 2, //!< Validate encoding of JSON strings.
    kParseIterativeFlag = 4,        //!< Iterative(constant complexity in terms of function call stack size) parsing.
    kParseStopWhenDoneFlag = 8      //!< After parsing a complete JSON root from stream, stop further processing the rest of stream. When this flag is used, parser will not generate kParseErrorDocumentRootNotSingular error.
};

///////////////////////////////////////////////////////////////////////////////
// Handler

/*! \class rapidjson::Handler
    \brief Concept for receiving events from GenericReader upon parsing.
    The functions return true if no error occurs. If they return false, 
    the event publisher should terminate the process.
\code
concept Handler {
    typename Ch;

    bool Null();
    bool Bool(bool b);
    bool Int(int i);
    bool Uint(unsigned i);
    bool Int64(int64_t i);
    bool Uint64(uint64_t i);
    bool Double(double d);
    bool String(const Ch* str, SizeType length, bool copy);
    bool StartObject();
    bool Key(const Ch* str, SizeType length, bool copy);
    bool EndObject(SizeType memberCount);
    bool StartArray();
    bool EndArray(SizeType elementCount);
};
\endcode
*/
///////////////////////////////////////////////////////////////////////////////
// BaseReaderHandler

//! Default implementation of Handler.
/*! This can be used as base class of any reader handler.
    \note implements Handler concept
*/
template<typename Encoding = UTF8<>, typename Derived = void>
struct BaseReaderHandler {
    typedef typename Encoding::Ch Ch;

    typedef typename internal::SelectIf<internal::IsSame<Derived, void>, BaseReaderHandler, Derived>::Type Override;

    bool Default() { return true; }
    bool Null() { return static_cast<Override&>(*this).Default(); }
    bool Bool(bool) { return static_cast<Override&>(*this).Default(); }
    bool Int(int) { return static_cast<Override&>(*this).Default(); }
    bool Uint(unsigned) { return static_cast<Override&>(*this).Default(); }
    bool Int64(int64_t) { return static_cast<Override&>(*this).Default(); }
    bool Uint64(uint64_t) { return static_cast<Override&>(*this).Default(); }
    bool Double(double) { return static_cast<Override&>(*this).Default(); }
    bool String(const Ch*, SizeType, bool) { return static_cast<Override&>(*this).Default(); }
    bool StartObject() { return static_cast<Override&>(*this).Default(); }
    bool Key(const Ch* str, SizeType len, bool copy) { return static_cast<Override&>(*this).String(str, len, copy); }
    bool EndObject(SizeType) { return static_cast<Override&>(*this).Default(); }
    bool StartArray() { return static_cast<Override&>(*this).Default(); }
    bool EndArray(SizeType) { return static_cast<Override&>(*this).Default(); }
};

///////////////////////////////////////////////////////////////////////////////
// StreamLocalCopy

namespace internal {

template<typename Stream, int = StreamTraits<Stream>::copyOptimization>
class StreamLocalCopy;

//! Do copy optimization.
template<typename Stream>
class StreamLocalCopy<Stream, 1> {
public:
    StreamLocalCopy(Stream& original) : s(original), original_(original) {}
    ~StreamLocalCopy() { original_ = s; }

    Stream s;

private:
    StreamLocalCopy& operator=(const StreamLocalCopy&) /* = delete */;

    Stream& original_;
};

//! Keep reference.
template<typename Stream>
class StreamLocalCopy<Stream, 0> {
public:
    StreamLocalCopy(Stream& original) : s(original) {}

    Stream& s;

private:
    StreamLocalCopy& operator=(const StreamLocalCopy&) /* = delete */;
};

} // namespace internal

///////////////////////////////////////////////////////////////////////////////
// SkipWhitespace

//! Skip the JSON white spaces in a stream.
/*! \param is A input stream for skipping white spaces.
    \note This function has SSE2/SSE4.2 specialization.
*/
template<typename InputStream>
void SkipWhitespace(InputStream& is) {
    internal::StreamLocalCopy<InputStream> copy(is);
    InputStream& s(copy.s);

    while (s.Peek() == ' ' || s.Peek() == '\n' || s.Peek() == '\r' || s.Peek() == '\t')
        s.Take();
}

#ifdef RAPIDJSON_SSE42
//! Skip whitespace with SSE 4.2 pcmpistrm instruction, testing 16 8-byte characters at once.
inline const char *SkipWhitespace_SIMD(const char* p) {
	// Fast return for single non-whitespace
	if (*p == ' ' || *p == '\n' || *p == '\r' || *p == '\t')
		++p;
	else
		return p;

	// 16-byte align to the next boundary
	const char* nextAligned = reinterpret_cast<const char*>((reinterpret_cast<size_t>(p) + 15) & ~15);
	while (p != nextAligned)
		if (*p == ' ' || *p == '\n' || *p == '\r' || *p == '\t')
			++p;
		else
			return p;

    // The rest of string using SIMD
	static const char whitespace[16] = " \n\r\t";
	const __m128i w = _mm_loadu_si128((const __m128i *)&whitespace[0]);

    for (;; p += 16) {
        const __m128i s = _mm_load_si128((const __m128i *)p);
        const unsigned r = _mm_cvtsi128_si32(_mm_cmpistrm(w, s, _SIDD_UBYTE_OPS | _SIDD_CMP_EQUAL_ANY | _SIDD_BIT_MASK | _SIDD_NEGATIVE_POLARITY));
        if (r != 0) {   // some of characters is non-whitespace
#ifdef _MSC_VER         // Find the index of first non-whitespace
            unsigned long offset;
            _BitScanForward(&offset, r);
            return p + offset;
#else
            return p + __builtin_ffs(r) - 1;
#endif
        }
    }
}

#elif defined(RAPIDJSON_SSE2)

//! Skip whitespace with SSE2 instructions, testing 16 8-byte characters at once.
inline const char *SkipWhitespace_SIMD(const char* p) {
	// Fast return for single non-whitespace
	if (*p == ' ' || *p == '\n' || *p == '\r' || *p == '\t')
		++p;
	else
		return p;

    // 16-byte align to the next boundary
	const char* nextAligned = reinterpret_cast<const char*>((reinterpret_cast<size_t>(p) + 15) & ~15);
	while (p != nextAligned)
		if (*p == ' ' || *p == '\n' || *p == '\r' || *p == '\t')
			++p;
		else
			return p;

    // The rest of string
	static const char whitespaces[4][17] = {
		"                ",
		"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",
		"\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r",
		"\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t"};

		const __m128i w0 = _mm_loadu_si128((const __m128i *)&whitespaces[0][0]);
		const __m128i w1 = _mm_loadu_si128((const __m128i *)&whitespaces[1][0]);
		const __m128i w2 = _mm_loadu_si128((const __m128i *)&whitespaces[2][0]);
		const __m128i w3 = _mm_loadu_si128((const __m128i *)&whitespaces[3][0]);

    for (;; p += 16) {
        const __m128i s = _mm_load_si128((const __m128i *)p);
        __m128i x = _mm_cmpeq_epi8(s, w0);
        x = _mm_or_si128(x, _mm_cmpeq_epi8(s, w1));
        x = _mm_or_si128(x, _mm_cmpeq_epi8(s, w2));
        x = _mm_or_si128(x, _mm_cmpeq_epi8(s, w3));
        unsigned short r = (unsigned short)~_mm_movemask_epi8(x);
        if (r != 0) {   // some of characters may be non-whitespace
#ifdef _MSC_VER         // Find the index of first non-whitespace
            unsigned long offset;
            _BitScanForward(&offset, r);
            return p + offset;
#else
            return p + __builtin_ffs(r) - 1;
#endif
        }
    }
}

#endif // RAPIDJSON_SSE2

#ifdef RAPIDJSON_SIMD
//! Template function specialization for InsituStringStream
template<> inline void SkipWhitespace(InsituStringStream& is) { 
    is.src_ = const_cast<char*>(SkipWhitespace_SIMD(is.src_));
}

//! Template function specialization for StringStream
template<> inline void SkipWhitespace(StringStream& is) {
    is.src_ = SkipWhitespace_SIMD(is.src_);
}
#endif // RAPIDJSON_SIMD

///////////////////////////////////////////////////////////////////////////////
// GenericReader

//! SAX-style JSON parser. Use \ref Reader for UTF8 encoding and default allocator.
/*! GenericReader parses JSON text from a stream, and send events synchronously to an 
    object implementing Handler concept.

    It needs to allocate a stack for storing a single decoded string during 
    non-destructive parsing.

    For in-situ parsing, the decoded string is directly written to the source 
    text string, no temporary buffer is required.

    A GenericReader object can be reused for parsing multiple JSON text.
    
    \tparam SourceEncoding Encoding of the input stream.
    \tparam TargetEncoding Encoding of the parse output.
    \tparam StackAllocator Allocator type for stack.
*/
template <typename SourceEncoding, typename TargetEncoding, typename StackAllocator = CrtAllocator>
class GenericReader {
public:
    typedef typename SourceEncoding::Ch Ch; //!< SourceEncoding character type

    //! Constructor.
    /*! \param allocator Optional allocator for allocating stack memory. (Only use for non-destructive parsing)
        \param stackCapacity stack capacity in bytes for storing a single decoded string.  (Only use for non-destructive parsing)
    */
    GenericReader(StackAllocator* stackAllocator = 0, size_t stackCapacity = kDefaultStackCapacity) : stack_(stackAllocator, stackCapacity), parseResult_() {}

    //! Parse JSON text.
    /*! \tparam parseFlags Combination of \ref ParseFlag.
        \tparam InputStream Type of input stream, implementing Stream concept.
        \tparam Handler Type of handler, implementing Handler concept.
        \param is Input stream to be parsed.
        \param handler The handler to receive events.
        \return Whether the parsing is successful.
    */
    template <unsigned parseFlags, typename InputStream, typename Handler>
    ParseResult Parse(InputStream& is, Handler& handler) {
        if (parseFlags & kParseIterativeFlag)
            return IterativeParse<parseFlags>(is, handler);

        parseResult_.Clear();

        ClearStackOnExit scope(*this);

        SkipWhitespace(is);

        if (is.Peek() == '\0') {
            RAPIDJSON_PARSE_ERROR_NORETURN(kParseErrorDocumentEmpty, is.Tell());
            RAPIDJSON_PARSE_ERROR_EARLY_RETURN(parseResult_);
        }
        else {
            ParseValue<parseFlags>(is, handler);
            RAPIDJSON_PARSE_ERROR_EARLY_RETURN(parseResult_);

            if (!(parseFlags & kParseStopWhenDoneFlag)) {
                SkipWhitespace(is);

                if (is.Peek() != '\0') {
                    RAPIDJSON_PARSE_ERROR_NORETURN(kParseErrorDocumentRootNotSingular, is.Tell());
                    RAPIDJSON_PARSE_ERROR_EARLY_RETURN(parseResult_);
                }
            }
        }

        return parseResult_;
    }

    //! Parse JSON text (with \ref kParseDefaultFlags)
    /*! \tparam InputStream Type of input stream, implementing Stream concept
        \tparam Handler Type of handler, implementing Handler concept.
        \param is Input stream to be parsed.
        \param handler The handler to receive events.
        \return Whether the parsing is successful.
    */
    template <typename InputStream, typename Handler>
    ParseResult Parse(InputStream& is, Handler& handler) {
        return Parse<kParseDefaultFlags>(is, handler);
    }

    //! Whether a parse error has occured in the last parsing.
    bool HasParseError() const { return parseResult_.IsError(); }
    
    //! Get the \ref ParseErrorCode of last parsing.
    ParseErrorCode GetParseErrorCode() const { return parseResult_.Code(); }

    //! Get the position of last parsing error in input, 0 otherwise.
    size_t GetErrorOffset() const { return parseResult_.Offset(); }

protected:
    void SetParseError(ParseErrorCode code, size_t offset) { parseResult_.Set(code, offset); }

private:
    // Prohibit copy constructor & assignment operator.
    GenericReader(const GenericReader&);
    GenericReader& operator=(const GenericReader&);

    void ClearStack() { stack_.Clear(); }

    // clear stack on any exit from ParseStream, e.g. due to exception
    struct ClearStackOnExit {
        explicit ClearStackOnExit(GenericReader& r) : r_(r) {}
        ~ClearStackOnExit() { r_.ClearStack(); }
    private:
        GenericReader& r_;
        ClearStackOnExit(const ClearStackOnExit&);
        ClearStackOnExit& operator=(const ClearStackOnExit&);
    };

    // Parse object: { string : value, ... }
    template<unsigned parseFlags, typename InputStream, typename Handler>
    void ParseObject(InputStream& is, Handler& handler) {
        RAPIDJSON_ASSERT(is.Peek() == '{');
        is.Take();  // Skip '{'
        
        if (!handler.StartObject())
            RAPIDJSON_PARSE_ERROR(kParseErrorTermination, is.Tell());

        SkipWhitespace(is);

        if (is.Peek() == '}') {
            is.Take();
            if (!handler.EndObject(0))  // empty object
                RAPIDJSON_PARSE_ERROR(kParseErrorTermination, is.Tell());
            return;
        }

        for (SizeType memberCount = 0;;) {
            if (is.Peek() != '"')
                RAPIDJSON_PARSE_ERROR(kParseErrorObjectMissName, is.Tell());

            ParseString<parseFlags>(is, handler, true);
            RAPIDJSON_PARSE_ERROR_EARLY_RETURN_VOID;

            SkipWhitespace(is);

            if (is.Take() != ':')
                RAPIDJSON_PARSE_ERROR(kParseErrorObjectMissColon, is.Tell());

            SkipWhitespace(is);

            ParseValue<parseFlags>(is, handler);
            RAPIDJSON_PARSE_ERROR_EARLY_RETURN_VOID;

            SkipWhitespace(is);

            ++memberCount;

            switch (is.Take()) {
                case ',': SkipWhitespace(is); break;
                case '}': 
                    if (!handler.EndObject(memberCount))
                        RAPIDJSON_PARSE_ERROR(kParseErrorTermination, is.Tell());
                    else
                        return;
                default:  RAPIDJSON_PARSE_ERROR(kParseErrorObjectMissCommaOrCurlyBracket, is.Tell());
            }
        }
    }

    // Parse array: [ value, ... ]
    template<unsigned parseFlags, typename InputStream, typename Handler>
    void ParseArray(InputStream& is, Handler& handler) {
        RAPIDJSON_ASSERT(is.Peek() == '[');
        is.Take();  // Skip '['
        
        if (!handler.StartArray())
            RAPIDJSON_PARSE_ERROR(kParseErrorTermination, is.Tell());
        
        SkipWhitespace(is);

        if (is.Peek() == ']') {
            is.Take();
            if (!handler.EndArray(0)) // empty array
                RAPIDJSON_PARSE_ERROR(kParseErrorTermination, is.Tell());
            return;
        }

        for (SizeType elementCount = 0;;) {
            ParseValue<parseFlags>(is, handler);
            RAPIDJSON_PARSE_ERROR_EARLY_RETURN_VOID;

            ++elementCount;
            SkipWhitespace(is);

            switch (is.Take()) {
                case ',': SkipWhitespace(is); break;
                case ']': 
                    if (!handler.EndArray(elementCount))
                        RAPIDJSON_PARSE_ERROR(kParseErrorTermination, is.Tell());
                    else
                        return;
                default:  RAPIDJSON_PARSE_ERROR(kParseErrorArrayMissCommaOrSquareBracket, is.Tell());
            }
        }
    }

    template<unsigned parseFlags, typename InputStream, typename Handler>
    void ParseNull(InputStream& is, Handler& handler) {
        RAPIDJSON_ASSERT(is.Peek() == 'n');
        is.Take();

        if (is.Take() == 'u' && is.Take() == 'l' && is.Take() == 'l') {
            if (!handler.Null())
                RAPIDJSON_PARSE_ERROR(kParseErrorTermination, is.Tell());
        }
        else
            RAPIDJSON_PARSE_ERROR(kParseErrorValueInvalid, is.Tell() - 1);
    }

    template<unsigned parseFlags, typename InputStream, typename Handler>
    void ParseTrue(InputStream& is, Handler& handler) {
        RAPIDJSON_ASSERT(is.Peek() == 't');
        is.Take();

        if (is.Take() == 'r' && is.Take() == 'u' && is.Take() == 'e') {
            if (!handler.Bool(true))
                RAPIDJSON_PARSE_ERROR(kParseErrorTermination, is.Tell());
        }
        else
            RAPIDJSON_PARSE_ERROR(kParseErrorValueInvalid, is.Tell() - 1);
    }

    template<unsigned parseFlags, typename InputStream, typename Handler>
    void ParseFalse(InputStream& is, Handler& handler) {
        RAPIDJSON_ASSERT(is.Peek() == 'f');
        is.Take();

        if (is.Take() == 'a' && is.Take() == 'l' && is.Take() == 's' && is.Take() == 'e') {
            if (!handler.Bool(false))
                RAPIDJSON_PARSE_ERROR(kParseErrorTermination, is.Tell());
        }
        else
            RAPIDJSON_PARSE_ERROR(kParseErrorValueInvalid, is.Tell() - 1);
    }

    // Helper function to parse four hexidecimal digits in \uXXXX in ParseString().
    template<typename InputStream>
    unsigned ParseHex4(InputStream& is) {
        unsigned codepoint = 0;
        for (int i = 0; i < 4; i++) {
            Ch c = is.Take();
            codepoint <<= 4;
            codepoint += static_cast<unsigned>(c);
            if (c >= '0' && c <= '9')
                codepoint -= '0';
            else if (c >= 'A' && c <= 'F')
                codepoint -= 'A' - 10;
            else if (c >= 'a' && c <= 'f')
                codepoint -= 'a' - 10;
            else {
                RAPIDJSON_PARSE_ERROR_NORETURN(kParseErrorStringUnicodeEscapeInvalidHex, is.Tell() - 1);
                RAPIDJSON_PARSE_ERROR_EARLY_RETURN(0);
            }
        }
        return codepoint;
    }

    class StackStream {
    public:
        typedef typename TargetEncoding::Ch Ch;

        StackStream(internal::Stack<StackAllocator>& stack) : stack_(stack), length_(0) {}
        RAPIDJSON_FORCEINLINE void Put(Ch c) {
            *stack_.template Push<Ch>() = c;
            ++length_;
        }
        internal::Stack<StackAllocator>& stack_;
        SizeType length_;

    private:
        StackStream(const StackStream&);
        StackStream& operator=(const StackStream&);
    };

    // Parse string and generate String event. Different code paths for kParseInsituFlag.
    template<unsigned parseFlags, typename InputStream, typename Handler>
    void ParseString(InputStream& is, Handler& handler, bool isKey = false) {
        internal::StreamLocalCopy<InputStream> copy(is);
        InputStream& s(copy.s);

        bool success = false;
        if (parseFlags & kParseInsituFlag) {
            typename InputStream::Ch *head = s.PutBegin();
            ParseStringToStream<parseFlags, SourceEncoding, SourceEncoding>(s, s);
            RAPIDJSON_PARSE_ERROR_EARLY_RETURN_VOID;
            size_t length = s.PutEnd(head) - 1;
            RAPIDJSON_ASSERT(length <= 0xFFFFFFFF);
            const typename TargetEncoding::Ch* const str = (typename TargetEncoding::Ch*)head;
            success = (isKey ? handler.Key(str, SizeType(length), false) : handler.String(str, SizeType(length), false));
        }
        else {
            StackStream stackStream(stack_);
            ParseStringToStream<parseFlags, SourceEncoding, TargetEncoding>(s, stackStream);
            RAPIDJSON_PARSE_ERROR_EARLY_RETURN_VOID;
            const typename TargetEncoding::Ch* const str = stack_.template Pop<typename TargetEncoding::Ch>(stackStream.length_);
            success = (isKey ? handler.Key(str, stackStream.length_ - 1, true) : handler.String(str, stackStream.length_ - 1, true));
        }
        if (!success)
            RAPIDJSON_PARSE_ERROR(kParseErrorTermination, s.Tell());
    }

    // Parse string to an output is
    // This function handles the prefix/suffix double quotes, escaping, and optional encoding validation.
    template<unsigned parseFlags, typename SEncoding, typename TEncoding, typename InputStream, typename OutputStream>
    RAPIDJSON_FORCEINLINE void ParseStringToStream(InputStream& is, OutputStream& os) {
//!@cond RAPIDJSON_HIDDEN_FROM_DOXYGEN
#define Z16 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
        static const char escape[256] = {
            Z16, Z16, 0, 0,'\"', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,'/', 
            Z16, Z16, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,'\\', 0, 0, 0, 
            0, 0,'\b', 0, 0, 0,'\f', 0, 0, 0, 0, 0, 0, 0,'\n', 0, 
            0, 0,'\r', 0,'\t', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
            Z16, Z16, Z16, Z16, Z16, Z16, Z16, Z16
        };
#undef Z16
//!@endcond

        RAPIDJSON_ASSERT(is.Peek() == '\"');
        is.Take();  // Skip '\"'

        for (;;) {
            Ch c = is.Peek();
            if (c == '\\') {    // Escape
                is.Take();
                Ch e = is.Take();
                if ((sizeof(Ch) == 1 || unsigned(e) < 256) && escape[(unsigned char)e]) {
                    os.Put(escape[(unsigned char)e]);
                }
                else if (e == 'u') {    // Unicode
                    unsigned codepoint = ParseHex4(is);
                    if (codepoint >= 0xD800 && codepoint <= 0xDBFF) {
                        // Handle UTF-16 surrogate pair
                        if (is.Take() != '\\' || is.Take() != 'u')
                            RAPIDJSON_PARSE_ERROR(kParseErrorStringUnicodeSurrogateInvalid, is.Tell() - 2);
                        unsigned codepoint2 = ParseHex4(is);
                        if (codepoint2 < 0xDC00 || codepoint2 > 0xDFFF)
                            RAPIDJSON_PARSE_ERROR(kParseErrorStringUnicodeSurrogateInvalid, is.Tell() - 2);
                        codepoint = (((codepoint - 0xD800) << 10) | (codepoint2 - 0xDC00)) + 0x10000;
                    }
                    TEncoding::Encode(os, codepoint);
                }
                else
                    RAPIDJSON_PARSE_ERROR(kParseErrorStringEscapeInvalid, is.Tell() - 1);
            }
            else if (c == '"') {    // Closing double quote
                is.Take();
                os.Put('\0');   // null-terminate the string
                return;
            }
            else if (c == '\0')
                RAPIDJSON_PARSE_ERROR(kParseErrorStringMissQuotationMark, is.Tell() - 1);
            else if ((unsigned)c < 0x20) // RFC 4627: unescaped = %x20-21 / %x23-5B / %x5D-10FFFF
                RAPIDJSON_PARSE_ERROR(kParseErrorStringEscapeInvalid, is.Tell() - 1);
            else {
                if (parseFlags & kParseValidateEncodingFlag ? 
                    !Transcoder<SEncoding, TEncoding>::Validate(is, os) : 
                    !Transcoder<SEncoding, TEncoding>::Transcode(is, os))
                    RAPIDJSON_PARSE_ERROR(kParseErrorStringInvalidEncoding, is.Tell());
            }
        }
    }

    inline double StrtodFastPath(double significand, int exp) {
        // Fast path only works on limited range of values.
        // But for simplicity and performance, currently only implement this.
        // see http://www.exploringbinary.com/fast-path-decimal-to-floating-point-conversion/
        if (exp < -308)
            return 0.0;
        else if (exp >= 0)
            return significand * internal::Pow10(exp);
        else
            return significand / internal::Pow10(-exp);
    }

    template<unsigned parseFlags, typename InputStream, typename Handler>
    void ParseNumber(InputStream& is, Handler& handler) {
        internal::StreamLocalCopy<InputStream> copy(is);
        InputStream& s(copy.s);

        // Parse minus
        bool minus = false;
        if (s.Peek() == '-') {
            minus = true;
            s.Take();
        }

        // Parse int: zero / ( digit1-9 *DIGIT )
        unsigned i = 0;
        uint64_t i64 = 0;
        bool use64bit = false;
        if (s.Peek() == '0') {
            i = 0;
            s.Take();
        }
        else if (s.Peek() >= '1' && s.Peek() <= '9') {
            i = static_cast<unsigned>(s.Take() - '0');

            if (minus)
                while (s.Peek() >= '0' && s.Peek() <= '9') {
                    if (i >= 214748364) { // 2^31 = 2147483648
                        if (i != 214748364 || s.Peek() > '8') {
                            i64 = i;
                            use64bit = true;
                            break;
                        }
                    }
                    i = i * 10 + static_cast<unsigned>(s.Take() - '0');
                }
            else
                while (s.Peek() >= '0' && s.Peek() <= '9') {
                    if (i >= 429496729) { // 2^32 - 1 = 4294967295
                        if (i != 429496729 || s.Peek() > '5') {
                            i64 = i;
                            use64bit = true;
                            break;
                        }
                    }
                    i = i * 10 + static_cast<unsigned>(s.Take() - '0');
                }
        }
        else
            RAPIDJSON_PARSE_ERROR(kParseErrorValueInvalid, s.Tell());

        // Parse 64bit int
        double d = 0.0;
        bool useDouble = false;
        if (use64bit) {
            if (minus) 
                while (s.Peek() >= '0' && s.Peek() <= '9') {                    
                     if (i64 >= RAPIDJSON_UINT64_C2(0x0CCCCCCC, 0xCCCCCCCC)) // 2^63 = 9223372036854775808
                        if (i64 != RAPIDJSON_UINT64_C2(0x0CCCCCCC, 0xCCCCCCCC) || s.Peek() > '8') {
                            d = (double)i64;
                            useDouble = true;
                            break;
                        }
                    i64 = i64 * 10 + static_cast<unsigned>(s.Take() - '0');
                }
            else
                while (s.Peek() >= '0' && s.Peek() <= '9') {                    
                    if (i64 >= RAPIDJSON_UINT64_C2(0x19999999, 0x99999999)) // 2^64 - 1 = 18446744073709551615
                        if (i64 != RAPIDJSON_UINT64_C2(0x19999999, 0x99999999) || s.Peek() > '5') {
                            d = (double)i64;
                            useDouble = true;
                            break;
                        }
                    i64 = i64 * 10 + static_cast<unsigned>(s.Take() - '0');
                }
        }

        // Force double for big integer
        if (useDouble) {
            while (s.Peek() >= '0' && s.Peek() <= '9') {
                if (d >= 1.7976931348623157e307) // DBL_MAX / 10.0
                    RAPIDJSON_PARSE_ERROR(kParseErrorNumberTooBig, s.Tell());
                d = d * 10 + (s.Take() - '0');
            }
        }

        // Parse frac = decimal-point 1*DIGIT
        int expFrac = 0;
        if (s.Peek() == '.') {
            s.Take();

#if RAPIDJSON_64BIT
            // Use i64 to store significand in 64-bit architecture
            if (!useDouble) {
                if (!use64bit)
                    i64 = i;
        
                while (s.Peek() >= '0' && s.Peek() <= '9') {
                    if (i64 >= RAPIDJSON_UINT64_C2(0x19999999, 0x99999999))
                        break;
                    else {
                        i64 = i64 * 10 + static_cast<unsigned>(s.Take() - '0');
                        --expFrac;
                    }
                }

                d = (double)i64;
            }
#else
            // Use double to store significand in 32-bit architecture
            if (!useDouble)
                d = use64bit ? (double)i64 : (double)i;
#endif
            useDouble = true;

            while (s.Peek() >= '0' && s.Peek() <= '9') {
                d = d * 10 + (s.Take() - '0');
                --expFrac;
            }

            if (expFrac == 0)
                RAPIDJSON_PARSE_ERROR(kParseErrorNumberMissFraction, s.Tell());
        }

        // Parse exp = e [ minus / plus ] 1*DIGIT
        int exp = 0;
        if (s.Peek() == 'e' || s.Peek() == 'E') {
            if (!useDouble) {
                d = use64bit ? (double)i64 : (double)i;
                useDouble = true;
            }
            s.Take();

            bool expMinus = false;
            if (s.Peek() == '+')
                s.Take();
            else if (s.Peek() == '-') {
                s.Take();
                expMinus = true;
            }

            if (s.Peek() >= '0' && s.Peek() <= '9') {
                exp = s.Take() - '0';
                while (s.Peek() >= '0' && s.Peek() <= '9') {
                    exp = exp * 10 + (s.Take() - '0');
                    if (exp > 308 && !expMinus) // exp > 308 should be rare, so it should be checked first.
                        RAPIDJSON_PARSE_ERROR(kParseErrorNumberTooBig, s.Tell());
                }
            }
            else
                RAPIDJSON_PARSE_ERROR(kParseErrorNumberMissExponent, s.Tell());

            if (expMinus)
                exp = -exp;
        }

        // Finish parsing, call event according to the type of number.
        bool cont = true;
        if (useDouble) {
            int expSum = exp + expFrac;
            if (expSum < -308) {
                // Prevent expSum < -308, making Pow10(expSum) = 0
                d = StrtodFastPath(d, exp);
                d = StrtodFastPath(d, expFrac);
            }
            else
                d = StrtodFastPath(d, expSum);

            cont = handler.Double(minus ? -d : d);
        }
        else {
            if (use64bit) {
                if (minus)
                    cont = handler.Int64(-(int64_t)i64);
                else
                    cont = handler.Uint64(i64);
            }
            else {
                if (minus)
                    cont = handler.Int(-(int)i);
                else
                    cont = handler.Uint(i);
            }
        }
        if (!cont)
            RAPIDJSON_PARSE_ERROR(kParseErrorTermination, s.Tell());
    }

    // Parse any JSON value
    template<unsigned parseFlags, typename InputStream, typename Handler>
    void ParseValue(InputStream& is, Handler& handler) {
        switch (is.Peek()) {
            case 'n': ParseNull  <parseFlags>(is, handler); break;
            case 't': ParseTrue  <parseFlags>(is, handler); break;
            case 'f': ParseFalse <parseFlags>(is, handler); break;
            case '"': ParseString<parseFlags>(is, handler); break;
            case '{': ParseObject<parseFlags>(is, handler); break;
            case '[': ParseArray <parseFlags>(is, handler); break;
            default : ParseNumber<parseFlags>(is, handler);
        }
    }

    // Iterative Parsing

    // States
    enum IterativeParsingState {
        IterativeParsingStartState = 0,
        IterativeParsingFinishState,
        IterativeParsingErrorState,

        // Object states
        IterativeParsingObjectInitialState,
        IterativeParsingMemberKeyState,
        IterativeParsingKeyValueDelimiterState,
        IterativeParsingMemberValueState,
        IterativeParsingMemberDelimiterState,
        IterativeParsingObjectFinishState,

        // Array states
        IterativeParsingArrayInitialState,
        IterativeParsingElementState,
        IterativeParsingElementDelimiterState,
        IterativeParsingArrayFinishState,

        // Single value state
        IterativeParsingValueState,

        cIterativeParsingStateCount
    };

    // Tokens
    enum Token {
        LeftBracketToken = 0,
        RightBracketToken,

        LeftCurlyBracketToken,
        RightCurlyBracketToken,

        CommaToken,
        ColonToken,

        StringToken,
        FalseToken,
        TrueToken,
        NullToken,
        NumberToken,

        kTokenCount
    };

    RAPIDJSON_FORCEINLINE Token Tokenize(Ch c) {

//!@cond RAPIDJSON_HIDDEN_FROM_DOXYGEN
#define N NumberToken
#define N16 N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N
        // Maps from ASCII to Token
        static const unsigned char tokenMap[256] = {
            N16, // 00~0F
            N16, // 10~1F
            N, N, StringToken, N, N, N, N, N, N, N, N, N, CommaToken, N, N, N, // 20~2F
            N, N, N, N, N, N, N, N, N, N, ColonToken, N, N, N, N, N, // 30~3F
            N16, // 40~4F
            N, N, N, N, N, N, N, N, N, N, N, LeftBracketToken, N, RightBracketToken, N, N, // 50~5F
            N, N, N, N, N, N, FalseToken, N, N, N, N, N, N, N, NullToken, N, // 60~6F
            N, N, N, N, TrueToken, N, N, N, N, N, N, LeftCurlyBracketToken, N, RightCurlyBracketToken, N, N, // 70~7F
            N16, N16, N16, N16, N16, N16, N16, N16 // 80~FF
        };
#undef N
#undef N16
//!@endcond
        
        if (sizeof(Ch) == 1 || unsigned(c) < 256)
            return (Token)tokenMap[(unsigned char)c];
        else
            return NumberToken;
    }

    RAPIDJSON_FORCEINLINE IterativeParsingState Predict(IterativeParsingState state, Token token) {
        // current state x one lookahead token -> new state
        static const char G[cIterativeParsingStateCount][kTokenCount] = {
            // Start
            {
                IterativeParsingArrayInitialState,  // Left bracket
                IterativeParsingErrorState,         // Right bracket
                IterativeParsingObjectInitialState, // Left curly bracket
                IterativeParsingErrorState,         // Right curly bracket
                IterativeParsingErrorState,         // Comma
                IterativeParsingErrorState,         // Colon
                IterativeParsingValueState,         // String
                IterativeParsingValueState,         // False
                IterativeParsingValueState,         // True
                IterativeParsingValueState,         // Null
                IterativeParsingValueState          // Number
            },
            // Finish(sink state)
            {
                IterativeParsingErrorState, IterativeParsingErrorState, IterativeParsingErrorState, IterativeParsingErrorState, IterativeParsingErrorState,
                IterativeParsingErrorState, IterativeParsingErrorState, IterativeParsingErrorState, IterativeParsingErrorState, IterativeParsingErrorState,
                IterativeParsingErrorState
            },
            // Error(sink state)
            {
                IterativeParsingErrorState, IterativeParsingErrorState, IterativeParsingErrorState, IterativeParsingErrorState, IterativeParsingErrorState,
                IterativeParsingErrorState, IterativeParsingErrorState, IterativeParsingErrorState, IterativeParsingErrorState, IterativeParsingErrorState,
                IterativeParsingErrorState
            },
            // ObjectInitial
            {
                IterativeParsingErrorState,         // Left bracket
                IterativeParsingErrorState,         // Right bracket
                IterativeParsingErrorState,         // Left curly bracket
                IterativeParsingObjectFinishState,  // Right curly bracket
                IterativeParsingErrorState,         // Comma
                IterativeParsingErrorState,         // Colon
                IterativeParsingMemberKeyState,     // String
                IterativeParsingErrorState,         // False
                IterativeParsingErrorState,         // True
                IterativeParsingErrorState,         // Null
                IterativeParsingErrorState          // Number
            },
            // MemberKey
            {
                IterativeParsingErrorState,             // Left bracket
                IterativeParsingErrorState,             // Right bracket
                IterativeParsingErrorState,             // Left curly bracket
                IterativeParsingErrorState,             // Right curly bracket
                IterativeParsingErrorState,             // Comma
                IterativeParsingKeyValueDelimiterState, // Colon
                IterativeParsingErrorState,             // String
                IterativeParsingErrorState,             // False
                IterativeParsingErrorState,             // True
                IterativeParsingErrorState,             // Null
                IterativeParsingErrorState              // Number
            },
            // KeyValueDelimiter
            {
                IterativeParsingArrayInitialState,      // Left bracket(push MemberValue state)
                IterativeParsingErrorState,             // Right bracket
                IterativeParsingObjectInitialState,     // Left curly bracket(push MemberValue state)
                IterativeParsingErrorState,             // Right curly bracket
                IterativeParsingErrorState,             // Comma
                IterativeParsingErrorState,             // Colon
                IterativeParsingMemberValueState,       // String
                IterativeParsingMemberValueState,       // False
                IterativeParsingMemberValueState,       // True
                IterativeParsingMemberValueState,       // Null
                IterativeParsingMemberValueState        // Number
            },
            // MemberValue
            {
                IterativeParsingErrorState,             // Left bracket
                IterativeParsingErrorState,             // Right bracket
                IterativeParsingErrorState,             // Left curly bracket
                IterativeParsingObjectFinishState,      // Right curly bracket
                IterativeParsingMemberDelimiterState,   // Comma
                IterativeParsingErrorState,             // Colon
                IterativeParsingErrorState,             // String
                IterativeParsingErrorState,             // False
                IterativeParsingErrorState,             // True
                IterativeParsingErrorState,             // Null
                IterativeParsingErrorState              // Number
            },
            // MemberDelimiter
            {
                IterativeParsingErrorState,         // Left bracket
                IterativeParsingErrorState,         // Right bracket
                IterativeParsingErrorState,         // Left curly bracket
                IterativeParsingErrorState,         // Right curly bracket
                IterativeParsingErrorState,         // Comma
                IterativeParsingErrorState,         // Colon
                IterativeParsingMemberKeyState,     // String
                IterativeParsingErrorState,         // False
                IterativeParsingErrorState,         // True
                IterativeParsingErrorState,         // Null
                IterativeParsingErrorState          // Number
            },
            // ObjectFinish(sink state)
            {
                IterativeParsingErrorState, IterativeParsingErrorState, IterativeParsingErrorState, IterativeParsingErrorState, IterativeParsingErrorState,
                IterativeParsingErrorState, IterativeParsingErrorState, IterativeParsingErrorState, IterativeParsingErrorState, IterativeParsingErrorState,
                IterativeParsingErrorState
            },
            // ArrayInitial
            {
                IterativeParsingArrayInitialState,      // Left bracket(push Element state)
                IterativeParsingArrayFinishState,       // Right bracket
                IterativeParsingObjectInitialState,     // Left curly bracket(push Element state)
                IterativeParsingErrorState,             // Right curly bracket
                IterativeParsingErrorState,             // Comma
                IterativeParsingErrorState,             // Colon
                IterativeParsingElementState,           // String
                IterativeParsingElementState,           // False
                IterativeParsingElementState,           // True
                IterativeParsingElementState,           // Null
                IterativeParsingElementState            // Number
            },
            // Element
            {
                IterativeParsingErrorState,             // Left bracket
                IterativeParsingArrayFinishState,       // Right bracket
                IterativeParsingErrorState,             // Left curly bracket
                IterativeParsingErrorState,             // Right curly bracket
                IterativeParsingElementDelimiterState,  // Comma
                IterativeParsingErrorState,             // Colon
                IterativeParsingErrorState,             // String
                IterativeParsingErrorState,             // False
                IterativeParsingErrorState,             // True
                IterativeParsingErrorState,             // Null
                IterativeParsingErrorState              // Number
            },
            // ElementDelimiter
            {
                IterativeParsingArrayInitialState,      // Left bracket(push Element state)
                IterativeParsingErrorState,             // Right bracket
                IterativeParsingObjectInitialState,     // Left curly bracket(push Element state)
                IterativeParsingErrorState,             // Right curly bracket
                IterativeParsingErrorState,             // Comma
                IterativeParsingErrorState,             // Colon
                IterativeParsingElementState,           // String
                IterativeParsingElementState,           // False
                IterativeParsingElementState,           // True
                IterativeParsingElementState,           // Null
                IterativeParsingElementState            // Number
            },
            // ArrayFinish(sink state)
            {
                IterativeParsingErrorState, IterativeParsingErrorState, IterativeParsingErrorState, IterativeParsingErrorState, IterativeParsingErrorState,
                IterativeParsingErrorState, IterativeParsingErrorState, IterativeParsingErrorState, IterativeParsingErrorState, IterativeParsingErrorState,
                IterativeParsingErrorState
            },
            // Single Value (sink state)
            {
                IterativeParsingErrorState, IterativeParsingErrorState, IterativeParsingErrorState, IterativeParsingErrorState, IterativeParsingErrorState,
                IterativeParsingErrorState, IterativeParsingErrorState, IterativeParsingErrorState, IterativeParsingErrorState, IterativeParsingErrorState,
                IterativeParsingErrorState
            }
        }; // End of G

        return (IterativeParsingState)G[state][token];
    }

    // Make an advance in the token stream and state based on the candidate destination state which was returned by Transit().
    // May return a new state on state pop.
    template <unsigned parseFlags, typename InputStream, typename Handler>
    RAPIDJSON_FORCEINLINE IterativeParsingState Transit(IterativeParsingState src, Token token, IterativeParsingState dst, InputStream& is, Handler& handler) {
        switch (dst) {
        case IterativeParsingStartState:
            RAPIDJSON_ASSERT(false);
            return IterativeParsingErrorState;

        case IterativeParsingFinishState:
            return dst;

        case IterativeParsingErrorState:
            return dst;

        case IterativeParsingObjectInitialState:
        case IterativeParsingArrayInitialState:
        {
            // Push the state(Element or MemeberValue) if we are nested in another array or value of member.
            // In this way we can get the correct state on ObjectFinish or ArrayFinish by frame pop.
            IterativeParsingState n = src;
            if (src == IterativeParsingArrayInitialState || src == IterativeParsingElementDelimiterState)
                n = IterativeParsingElementState;
            else if (src == IterativeParsingKeyValueDelimiterState)
                n = IterativeParsingMemberValueState;
            // Push current state.
            *stack_.template Push<SizeType>(1) = n;
            // Initialize and push the member/element count.
            *stack_.template Push<SizeType>(1) = 0;
            // Call handler
            bool hr = (dst == IterativeParsingObjectInitialState) ? handler.StartObject() : handler.StartArray();
            // On handler short circuits the parsing.
            if (!hr) {
                RAPIDJSON_PARSE_ERROR_NORETURN(kParseErrorTermination, is.Tell());
                return IterativeParsingErrorState;
            }
            else {
                is.Take();
                return dst;
            }
        }

        case IterativeParsingMemberKeyState:
            ParseString<parseFlags>(is, handler, true);
            if (HasParseError())
                return IterativeParsingErrorState;
            else
                return dst;

        case IterativeParsingKeyValueDelimiterState:
            if (token == ColonToken) {
                is.Take();
                return dst;
            }
            else
                return IterativeParsingErrorState;

        case IterativeParsingMemberValueState:
            // Must be non-compound value. Or it would be ObjectInitial or ArrayInitial state.
            ParseValue<parseFlags>(is, handler);
            if (HasParseError()) {
                return IterativeParsingErrorState;
            }
            return dst;

        case IterativeParsingElementState:
            // Must be non-compound value. Or it would be ObjectInitial or ArrayInitial state.
            ParseValue<parseFlags>(is, handler);
            if (HasParseError()) {
                return IterativeParsingErrorState;
            }
            return dst;

        case IterativeParsingMemberDelimiterState:
        case IterativeParsingElementDelimiterState:
            is.Take();
            // Update member/element count.
            *stack_.template Top<SizeType>() = *stack_.template Top<SizeType>() + 1;
            return dst;

        case IterativeParsingObjectFinishState:
        {
            // Get member count.
            SizeType c = *stack_.template Pop<SizeType>(1);
            // If the object is not empty, count the last member.
            if (src == IterativeParsingMemberValueState)
                ++c;
            // Restore the state.
            IterativeParsingState n = static_cast<IterativeParsingState>(*stack_.template Pop<SizeType>(1));
            // Transit to Finish state if this is the topmost scope.
            if (n == IterativeParsingStartState)
                n = IterativeParsingFinishState;
            // Call handler
            bool hr = handler.EndObject(c);
            // On handler short circuits the parsing.
            if (!hr) {
                RAPIDJSON_PARSE_ERROR_NORETURN(kParseErrorTermination, is.Tell());
                return IterativeParsingErrorState;
            }
            else {
                is.Take();
                return n;
            }
        }

        case IterativeParsingArrayFinishState:
        {
            // Get element count.
            SizeType c = *stack_.template Pop<SizeType>(1);
            // If the array is not empty, count the last element.
            if (src == IterativeParsingElementState)
                ++c;
            // Restore the state.
            IterativeParsingState n = static_cast<IterativeParsingState>(*stack_.template Pop<SizeType>(1));
            // Transit to Finish state if this is the topmost scope.
            if (n == IterativeParsingStartState)
                n = IterativeParsingFinishState;
            // Call handler
            bool hr = handler.EndArray(c);
            // On handler short circuits the parsing.
            if (!hr) {
                RAPIDJSON_PARSE_ERROR_NORETURN(kParseErrorTermination, is.Tell());
                return IterativeParsingErrorState;
            }
            else {
                is.Take();
                return n;
            }
        }

        case IterativeParsingValueState:
            // Must be non-compound value. Or it would be ObjectInitial or ArrayInitial state.
            ParseValue<parseFlags>(is, handler);
            if (HasParseError()) {
                return IterativeParsingErrorState;
            }
            return IterativeParsingFinishState;

        default:
            RAPIDJSON_ASSERT(false);
            return IterativeParsingErrorState;
        }
    }

    template <typename InputStream>
    void HandleError(IterativeParsingState src, InputStream& is) {
        if (HasParseError()) {
            // Error flag has been set.
            return;
        }
        
        switch (src) {
        case IterativeParsingStartState:            RAPIDJSON_PARSE_ERROR(kParseErrorDocumentEmpty, is.Tell());
        case IterativeParsingFinishState:           RAPIDJSON_PARSE_ERROR(kParseErrorDocumentRootNotSingular, is.Tell());
        case IterativeParsingObjectInitialState:
        case IterativeParsingMemberDelimiterState:  RAPIDJSON_PARSE_ERROR(kParseErrorObjectMissName, is.Tell());
        case IterativeParsingMemberKeyState:        RAPIDJSON_PARSE_ERROR(kParseErrorObjectMissColon, is.Tell());
        case IterativeParsingMemberValueState:      RAPIDJSON_PARSE_ERROR(kParseErrorObjectMissCommaOrCurlyBracket, is.Tell());
        case IterativeParsingElementState:          RAPIDJSON_PARSE_ERROR(kParseErrorArrayMissCommaOrSquareBracket, is.Tell());
        default:                                    RAPIDJSON_PARSE_ERROR(kParseErrorUnspecificSyntaxError, is.Tell());
        }       
    }

    template <unsigned parseFlags, typename InputStream, typename Handler>
    ParseResult IterativeParse(InputStream& is, Handler& handler) {
        parseResult_.Clear();
        ClearStackOnExit scope(*this);
        IterativeParsingState state = IterativeParsingStartState;

        SkipWhitespace(is);
        while (is.Peek() != '\0') {
            Token t = Tokenize(is.Peek());
            IterativeParsingState n = Predict(state, t);
            IterativeParsingState d = Transit<parseFlags>(state, t, n, is, handler);

            if (d == IterativeParsingErrorState) {
                HandleError(state, is);
                break;
            }

            state = d;

            // Do not further consume streams if a root JSON has been parsed.
            if ((parseFlags & kParseStopWhenDoneFlag) && state == IterativeParsingFinishState)
                break;

            SkipWhitespace(is);
        }

        // Handle the end of file.
        if (state != IterativeParsingFinishState)
            HandleError(state, is);

        return parseResult_;
    }

    static const size_t kDefaultStackCapacity = 256;    //!< Default stack capacity in bytes for storing a single decoded string.
    internal::Stack<StackAllocator> stack_;  //!< A stack for storing decoded string temporarily during non-destructive parsing.
    ParseResult parseResult_;
}; // class GenericReader

//! Reader with UTF8 encoding and default allocator.
typedef GenericReader<UTF8<>, UTF8<> > Reader;

} // namespace rapidjson

#ifdef _MSC_VER
RAPIDJSON_DIAG_POP
#endif

#endif // RAPIDJSON_READER_H_
