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

#ifndef RAPIDJSON_DOCUMENT_H_
#define RAPIDJSON_DOCUMENT_H_

/*! \file document.h */

#include "reader.h"
#include "internal/meta.h"
#include "internal/strfunc.h"
#include <new>      // placement new

#ifdef _MSC_VER
RAPIDJSON_DIAG_PUSH
RAPIDJSON_DIAG_OFF(4127) // conditional expression is constant
#elif defined(__GNUC__)
RAPIDJSON_DIAG_PUSH
RAPIDJSON_DIAG_OFF(effc++)
#endif

///////////////////////////////////////////////////////////////////////////////
// RAPIDJSON_HAS_STDSTRING

#ifndef RAPIDJSON_HAS_STDSTRING
#ifdef RAPIDJSON_DOXYGEN_RUNNING
#define RAPIDJSON_HAS_STDSTRING 1 // force generation of documentation
#else
#define RAPIDJSON_HAS_STDSTRING 0 // no std::string support by default
#endif
/*! \def RAPIDJSON_HAS_STDSTRING
    \ingroup RAPIDJSON_CONFIG
    \brief Enable RapidJSON support for \c std::string

    By defining this preprocessor symbol to \c 1, several convenience functions for using
    \ref rapidjson::GenericValue with \c std::string are enabled, especially
    for construction and comparison.

    \hideinitializer
*/
#include <string>
#endif // RAPIDJSON_HAS_STDSTRING

#ifndef RAPIDJSON_NOMEMBERITERATORCLASS
#include <iterator> // std::iterator, std::random_access_iterator_tag
#endif

#if RAPIDJSON_HAS_CXX11_RVALUE_REFS
#include <utility> // std::move
#endif

namespace rapidjson {

// Forward declaration.
template <typename Encoding, typename Allocator>
class GenericValue;

//! Name-value pair in a JSON object value.
/*!
    This class was internal to GenericValue. It used to be a inner struct.
    But a compiler (IBM XL C/C++ for AIX) have reported to have problem with that so it moved as a namespace scope struct.
    https://code.google.com/p/rapidjson/issues/detail?id=64
*/
template <typename Encoding, typename Allocator> 
struct GenericMember { 
    GenericValue<Encoding, Allocator> name;     //!< name of member (must be a string)
    GenericValue<Encoding, Allocator> value;    //!< value of member.
};

///////////////////////////////////////////////////////////////////////////////
// GenericMemberIterator

#ifndef RAPIDJSON_NOMEMBERITERATORCLASS

//! (Constant) member iterator for a JSON object value
/*!
    \tparam Const Is this a constant iterator?
    \tparam Encoding    Encoding of the value. (Even non-string values need to have the same encoding in a document)
    \tparam Allocator   Allocator type for allocating memory of object, array and string.

    This class implements a Random Access Iterator for GenericMember elements
    of a GenericValue, see ISO/IEC 14882:2003(E) C++ standard, 24.1 [lib.iterator.requirements].

    \note This iterator implementation is mainly intended to avoid implicit
        conversions from iterator values to \c NULL,
        e.g. from GenericValue::FindMember.

    \note Define \c RAPIDJSON_NOMEMBERITERATORCLASS to fall back to a
        pointer-based implementation, if your platform doesn't provide
        the C++ <iterator> header.

    \see GenericMember, GenericValue::MemberIterator, GenericValue::ConstMemberIterator
 */
template <bool Const, typename Encoding, typename Allocator>
class GenericMemberIterator
    : public std::iterator<std::random_access_iterator_tag
        , typename internal::MaybeAddConst<Const,GenericMember<Encoding,Allocator> >::Type> {

    friend class GenericValue<Encoding,Allocator>;
    template <bool, typename, typename> friend class GenericMemberIterator;

    typedef GenericMember<Encoding,Allocator> PlainType;
    typedef typename internal::MaybeAddConst<Const,PlainType>::Type ValueType;
    typedef std::iterator<std::random_access_iterator_tag,ValueType> BaseType;

public:
    //! Iterator type itself
    typedef GenericMemberIterator Iterator;
    //! Constant iterator type
    typedef GenericMemberIterator<true,Encoding,Allocator>  ConstIterator;
    //! Non-constant iterator type
    typedef GenericMemberIterator<false,Encoding,Allocator> NonConstIterator;

    //! Pointer to (const) GenericMember
    typedef typename BaseType::pointer         Pointer;
    //! Reference to (const) GenericMember
    typedef typename BaseType::reference       Reference;
    //! Signed integer type (e.g. \c ptrdiff_t)
    typedef typename BaseType::difference_type DifferenceType;

    //! Default constructor (singular value)
    /*! Creates an iterator pointing to no element.
        \note All operations, except for comparisons, are undefined on such values.
     */
    GenericMemberIterator() : ptr_() {}

    //! Iterator conversions to more const
    /*!
        \param it (Non-const) iterator to copy from

        Allows the creation of an iterator from another GenericMemberIterator
        that is "less const".  Especially, creating a non-constant iterator
        from a constant iterator are disabled:
        \li const -> non-const (not ok)
        \li const -> const (ok)
        \li non-const -> const (ok)
        \li non-const -> non-const (ok)

        \note If the \c Const template parameter is already \c false, this
            constructor effectively defines a regular copy-constructor.
            Otherwise, the copy constructor is implicitly defined.
    */
    GenericMemberIterator(const NonConstIterator & it) : ptr_(it.ptr_) {}

    //! @name stepping
    //@{
    Iterator& operator++(){ ++ptr_; return *this; }
    Iterator& operator--(){ --ptr_; return *this; }
    Iterator  operator++(int){ Iterator old(*this); ++ptr_; return old; }
    Iterator  operator--(int){ Iterator old(*this); --ptr_; return old; }
    //@}

    //! @name increment/decrement
    //@{
    Iterator operator+(DifferenceType n) const { return Iterator(ptr_+n); }
    Iterator operator-(DifferenceType n) const { return Iterator(ptr_-n); }

    Iterator& operator+=(DifferenceType n) { ptr_+=n; return *this; }
    Iterator& operator-=(DifferenceType n) { ptr_-=n; return *this; }
    //@}

    //! @name relations
    //@{
    bool operator==(ConstIterator that) const { return ptr_ == that.ptr_; }
    bool operator!=(ConstIterator that) const { return ptr_ != that.ptr_; }
    bool operator<=(ConstIterator that) const { return ptr_ <= that.ptr_; }
    bool operator>=(ConstIterator that) const { return ptr_ >= that.ptr_; }
    bool operator< (ConstIterator that) const { return ptr_ < that.ptr_; }
    bool operator> (ConstIterator that) const { return ptr_ > that.ptr_; }
    //@}

    //! @name dereference
    //@{
    Reference operator*() const { return *ptr_; }
    Pointer   operator->() const { return ptr_; }
    Reference operator[](DifferenceType n) const { return ptr_[n]; }
    //@}

    //! Distance
    DifferenceType operator-(ConstIterator that) const { return ptr_-that.ptr_; }

private:
    //! Internal constructor from plain pointer
    explicit GenericMemberIterator(Pointer p) : ptr_(p) {}

    Pointer ptr_; //!< raw pointer
};

#else // RAPIDJSON_NOMEMBERITERATORCLASS

// class-based member iterator implementation disabled, use plain pointers

template <bool Const, typename Encoding, typename Allocator>
struct GenericMemberIterator;

//! non-const GenericMemberIterator
template <typename Encoding, typename Allocator>
struct GenericMemberIterator<false,Encoding,Allocator> {
    //! use plain pointer as iterator type
    typedef GenericMember<Encoding,Allocator>* Iterator;
};
//! const GenericMemberIterator
template <typename Encoding, typename Allocator>
struct GenericMemberIterator<true,Encoding,Allocator> {
    //! use plain const pointer as iterator type
    typedef const GenericMember<Encoding,Allocator>* Iterator;
};

#endif // RAPIDJSON_NOMEMBERITERATORCLASS

///////////////////////////////////////////////////////////////////////////////
// GenericStringRef

//! Reference to a constant string (not taking a copy)
/*!
    \tparam CharType character type of the string

    This helper class is used to automatically infer constant string
    references for string literals, especially from \c const \b (!)
    character arrays.

    The main use is for creating JSON string values without copying the
    source string via an \ref Allocator.  This requires that the referenced
    string pointers have a sufficient lifetime, which exceeds the lifetime
    of the associated GenericValue.

    \b Example
    \code
    Value v("foo");   // ok, no need to copy & calculate length
    const char foo[] = "foo";
    v.SetString(foo); // ok

    const char* bar = foo;
    // Value x(bar); // not ok, can't rely on bar's lifetime
    Value x(StringRef(bar)); // lifetime explicitly guaranteed by user
    Value y(StringRef(bar, 3));  // ok, explicitly pass length
    \endcode

    \see StringRef, GenericValue::SetString
*/
template<typename CharType>
struct GenericStringRef {
    typedef CharType Ch; //!< character type of the string

    //! Create string reference from \c const character array
    /*!
        This constructor implicitly creates a constant string reference from
        a \c const character array.  It has better performance than
        \ref StringRef(const CharType*) by inferring the string \ref length
        from the array length, and also supports strings containing null
        characters.

        \tparam N length of the string, automatically inferred

        \param str Constant character array, lifetime assumed to be longer
            than the use of the string in e.g. a GenericValue

        \post \ref s == str

        \note Constant complexity.
        \note There is a hidden, private overload to disallow references to
            non-const character arrays to be created via this constructor.
            By this, e.g. function-scope arrays used to be filled via
            \c snprintf are excluded from consideration.
            In such cases, the referenced string should be \b copied to the
            GenericValue instead.
     */
    template<SizeType N>
    GenericStringRef(const CharType (&str)[N]) RAPIDJSON_NOEXCEPT
        : s(str), length(N-1) {}

    //! Explicitly create string reference from \c const character pointer
    /*!
        This constructor can be used to \b explicitly  create a reference to
        a constant string pointer.

        \see StringRef(const CharType*)

        \param str Constant character pointer, lifetime assumed to be longer
            than the use of the string in e.g. a GenericValue

        \post \ref s == str

        \note There is a hidden, private overload to disallow references to
            non-const character arrays to be created via this constructor.
            By this, e.g. function-scope arrays used to be filled via
            \c snprintf are excluded from consideration.
            In such cases, the referenced string should be \b copied to the
            GenericValue instead.
     */
    explicit GenericStringRef(const CharType* str)
        : s(str), length(internal::StrLen(str)){ RAPIDJSON_ASSERT(s != NULL); }

    //! Create constant string reference from pointer and length
    /*! \param str constant string, lifetime assumed to be longer than the use of the string in e.g. a GenericValue
        \param len length of the string, excluding the trailing NULL terminator

        \post \ref s == str && \ref length == len
        \note Constant complexity.
     */
    GenericStringRef(const CharType* str, SizeType len)
        : s(str), length(len) { RAPIDJSON_ASSERT(s != NULL); }

    //! implicit conversion to plain CharType pointer
    operator const Ch *() const { return s; }

    const Ch* const s; //!< plain CharType pointer
    const SizeType length; //!< length of the string (excluding the trailing NULL terminator)

private:
    //! Disallow copy-assignment
    GenericStringRef operator=(const GenericStringRef&);
    //! Disallow construction from non-const array
    template<SizeType N>
    GenericStringRef(CharType (&str)[N]) /* = delete */;
};

//! Mark a character pointer as constant string
/*! Mark a plain character pointer as a "string literal".  This function
    can be used to avoid copying a character string to be referenced as a
    value in a JSON GenericValue object, if the string's lifetime is known
    to be valid long enough.
    \tparam CharType Character type of the string
    \param str Constant string, lifetime assumed to be longer than the use of the string in e.g. a GenericValue
    \return GenericStringRef string reference object
    \relatesalso GenericStringRef

    \see GenericValue::GenericValue(StringRefType), GenericValue::operator=(StringRefType), GenericValue::SetString(StringRefType), GenericValue::PushBack(StringRefType, Allocator&), GenericValue::AddMember
*/
template<typename CharType>
inline GenericStringRef<CharType> StringRef(const CharType* str) {
    return GenericStringRef<CharType>(str, internal::StrLen(str));
}

//! Mark a character pointer as constant string
/*! Mark a plain character pointer as a "string literal".  This function
    can be used to avoid copying a character string to be referenced as a
    value in a JSON GenericValue object, if the string's lifetime is known
    to be valid long enough.

    This version has better performance with supplied length, and also
    supports string containing null characters.

    \tparam CharType character type of the string
    \param str Constant string, lifetime assumed to be longer than the use of the string in e.g. a GenericValue
    \param length The length of source string.
    \return GenericStringRef string reference object
    \relatesalso GenericStringRef
*/
template<typename CharType>
inline GenericStringRef<CharType> StringRef(const CharType* str, size_t length) {
    return GenericStringRef<CharType>(str, SizeType(length));
}

#if RAPIDJSON_HAS_STDSTRING
//! Mark a string object as constant string
/*! Mark a string object (e.g. \c std::string) as a "string literal".
    This function can be used to avoid copying a string to be referenced as a
    value in a JSON GenericValue object, if the string's lifetime is known
    to be valid long enough.

    \tparam CharType character type of the string
    \param str Constant string, lifetime assumed to be longer than the use of the string in e.g. a GenericValue
    \return GenericStringRef string reference object
    \relatesalso GenericStringRef
    \note Requires the definition of the preprocessor symbol \ref RAPIDJSON_HAS_STDSTRING.
*/
template<typename CharType>
inline GenericStringRef<CharType> StringRef(const std::basic_string<CharType>& str) {
    return GenericStringRef<CharType>(str.data(), SizeType(str.size()));
}
#endif

///////////////////////////////////////////////////////////////////////////////
// GenericValue type traits
namespace internal {

template <typename T, typename Encoding = void, typename Allocator = void>
struct IsGenericValueImpl : FalseType {};

// select candidates according to nested encoding and allocator types
template <typename T> struct IsGenericValueImpl<T, typename Void<typename T::EncodingType>::Type, typename Void<typename T::AllocatorType>::Type>
    : IsBaseOf<GenericValue<typename T::EncodingType, typename T::AllocatorType>, T>::Type {};

// helper to match arbitrary GenericValue instantiations, including derived classes
template <typename T> struct IsGenericValue : IsGenericValueImpl<T>::Type {};

} // namespace internal

///////////////////////////////////////////////////////////////////////////////
// GenericValue

//! Represents a JSON value. Use Value for UTF8 encoding and default allocator.
/*!
    A JSON value can be one of 7 types. This class is a variant type supporting
    these types.

    Use the Value if UTF8 and default allocator

    \tparam Encoding    Encoding of the value. (Even non-string values need to have the same encoding in a document)
    \tparam Allocator   Allocator type for allocating memory of object, array and string.
*/
template <typename Encoding, typename Allocator = MemoryPoolAllocator<> > 
class GenericValue {
public:
    //! Name-value pair in an object.
    typedef GenericMember<Encoding, Allocator> Member;
    typedef Encoding EncodingType;                  //!< Encoding type from template parameter.
    typedef Allocator AllocatorType;                //!< Allocator type from template parameter.
    typedef typename Encoding::Ch Ch;               //!< Character type derived from Encoding.
    typedef GenericStringRef<Ch> StringRefType;     //!< Reference to a constant string
    typedef typename GenericMemberIterator<false,Encoding,Allocator>::Iterator MemberIterator;  //!< Member iterator for iterating in object.
    typedef typename GenericMemberIterator<true,Encoding,Allocator>::Iterator ConstMemberIterator;  //!< Constant member iterator for iterating in object.
    typedef GenericValue* ValueIterator;            //!< Value iterator for iterating in array.
    typedef const GenericValue* ConstValueIterator; //!< Constant value iterator for iterating in array.

    //!@name Constructors and destructor.
    //@{

    //! Default constructor creates a null value.
    GenericValue() RAPIDJSON_NOEXCEPT : data_(), flags_(kNullFlag) {}

#if RAPIDJSON_HAS_CXX11_RVALUE_REFS
    //! Move constructor in C++11
    GenericValue(GenericValue&& rhs) RAPIDJSON_NOEXCEPT : data_(rhs.data_), flags_(rhs.flags_) {
        rhs.flags_ = kNullFlag; // give up contents
    }
#endif

private:
    //! Copy constructor is not permitted.
    GenericValue(const GenericValue& rhs);

public:

    //! Constructor with JSON value type.
    /*! This creates a Value of specified type with default content.
        \param type Type of the value.
        \note Default content for number is zero.
    */
    explicit GenericValue(Type type) RAPIDJSON_NOEXCEPT : data_(), flags_() {
        static const unsigned defaultFlags[7] = {
            kNullFlag, kFalseFlag, kTrueFlag, kObjectFlag, kArrayFlag, kConstStringFlag,
            kNumberAnyFlag
        };
        RAPIDJSON_ASSERT(type <= kNumberType);
        flags_ = defaultFlags[type];
    }

    //! Explicit copy constructor (with allocator)
    /*! Creates a copy of a Value by using the given Allocator
        \tparam SourceAllocator allocator of \c rhs
        \param rhs Value to copy from (read-only)
        \param allocator Allocator for allocating copied elements and buffers. Commonly use GenericDocument::GetAllocator().
        \see CopyFrom()
    */
    template< typename SourceAllocator >
    GenericValue(const GenericValue<Encoding, SourceAllocator>& rhs, Allocator & allocator);

    //! Constructor for boolean value.
    /*! \param b Boolean value
        \note This constructor is limited to \em real boolean values and rejects
            implicitly converted types like arbitrary pointers.  Use an explicit cast
            to \c bool, if you want to construct a boolean JSON value in such cases.
     */
#ifndef RAPIDJSON_DOXYGEN_RUNNING // hide SFINAE from Doxygen
    template <typename T>
    explicit GenericValue(T b, RAPIDJSON_ENABLEIF((internal::IsSame<T,bool>))) RAPIDJSON_NOEXCEPT
#else
    explicit GenericValue(bool b) RAPIDJSON_NOEXCEPT
#endif
        : data_(), flags_(b ? kTrueFlag : kFalseFlag) {
            // safe-guard against failing SFINAE
            RAPIDJSON_STATIC_ASSERT((internal::IsSame<bool,T>::Value));
    }

    //! Constructor for int value.
    explicit GenericValue(int i) RAPIDJSON_NOEXCEPT : data_(), flags_(kNumberIntFlag) {
        data_.n.i64 = i;
        if (i >= 0)
            flags_ |= kUintFlag | kUint64Flag;
    }

    //! Constructor for unsigned value.
    explicit GenericValue(unsigned u) RAPIDJSON_NOEXCEPT : data_(), flags_(kNumberUintFlag) {
        data_.n.u64 = u; 
        if (!(u & 0x80000000))
            flags_ |= kIntFlag | kInt64Flag;
    }

    //! Constructor for int64_t value.
    explicit GenericValue(int64_t i64) RAPIDJSON_NOEXCEPT : data_(), flags_(kNumberInt64Flag) {
        data_.n.i64 = i64;
        if (i64 >= 0) {
            flags_ |= kNumberUint64Flag;
            if (!(static_cast<uint64_t>(i64) & RAPIDJSON_UINT64_C2(0xFFFFFFFF, 0x00000000)))
                flags_ |= kUintFlag;
            if (!(static_cast<uint64_t>(i64) & RAPIDJSON_UINT64_C2(0xFFFFFFFF, 0x80000000)))
                flags_ |= kIntFlag;
        }
        else if (i64 >= static_cast<int64_t>(RAPIDJSON_UINT64_C2(0xFFFFFFFF, 0x80000000)))
            flags_ |= kIntFlag;
    }

    //! Constructor for uint64_t value.
    explicit GenericValue(uint64_t u64) RAPIDJSON_NOEXCEPT : data_(), flags_(kNumberUint64Flag) {
        data_.n.u64 = u64;
        if (!(u64 & RAPIDJSON_UINT64_C2(0x80000000, 0x00000000)))
            flags_ |= kInt64Flag;
        if (!(u64 & RAPIDJSON_UINT64_C2(0xFFFFFFFF, 0x00000000)))
            flags_ |= kUintFlag;
        if (!(u64 & RAPIDJSON_UINT64_C2(0xFFFFFFFF, 0x80000000)))
            flags_ |= kIntFlag;
    }

    //! Constructor for double value.
    explicit GenericValue(double d) RAPIDJSON_NOEXCEPT : data_(), flags_(kNumberDoubleFlag) { data_.n.d = d; }

    //! Constructor for constant string (i.e. do not make a copy of string)
    GenericValue(const Ch* s, SizeType length) RAPIDJSON_NOEXCEPT : data_(), flags_() { SetStringRaw(StringRef(s, length)); }

    //! Constructor for constant string (i.e. do not make a copy of string)
    explicit GenericValue(StringRefType s) RAPIDJSON_NOEXCEPT : data_(), flags_() { SetStringRaw(s); }

    //! Constructor for copy-string (i.e. do make a copy of string)
    GenericValue(const Ch* s, SizeType length, Allocator& allocator) : data_(), flags_() { SetStringRaw(StringRef(s, length), allocator); }

    //! Constructor for copy-string (i.e. do make a copy of string)
    GenericValue(const Ch*s, Allocator& allocator) : data_(), flags_() { SetStringRaw(StringRef(s), allocator); }

#if RAPIDJSON_HAS_STDSTRING
    //! Constructor for copy-string from a string object (i.e. do make a copy of string)
    /*! \note Requires the definition of the preprocessor symbol \ref RAPIDJSON_HAS_STDSTRING.
     */
    GenericValue(const std::basic_string<Ch>& s, Allocator& allocator) : data_(), flags_() { SetStringRaw(StringRef(s), allocator); }
#endif

    //! Destructor.
    /*! Need to destruct elements of array, members of object, or copy-string.
    */
    ~GenericValue() {
        if (Allocator::kNeedFree) { // Shortcut by Allocator's trait
            switch(flags_) {
            case kArrayFlag:
                for (GenericValue* v = data_.a.elements; v != data_.a.elements + data_.a.size; ++v)
                    v->~GenericValue();
                Allocator::Free(data_.a.elements);
                break;

            case kObjectFlag:
                for (MemberIterator m = MemberBegin(); m != MemberEnd(); ++m)
                    m->~Member();
                Allocator::Free(data_.o.members);
                break;

            case kCopyStringFlag:
                Allocator::Free(const_cast<Ch*>(data_.s.str));
                break;

            default:
                break;  // Do nothing for other types.
            }
        }
    }

    //@}

    //!@name Assignment operators
    //@{

    //! Assignment with move semantics.
    /*! \param rhs Source of the assignment. It will become a null value after assignment.
    */
    GenericValue& operator=(GenericValue& rhs) RAPIDJSON_NOEXCEPT {
        RAPIDJSON_ASSERT(this != &rhs);
        this->~GenericValue();
        RawAssign(rhs);
        return *this;
    }

#if RAPIDJSON_HAS_CXX11_RVALUE_REFS
    //! Move assignment in C++11
    GenericValue& operator=(GenericValue&& rhs) RAPIDJSON_NOEXCEPT {
        return *this = rhs.Move();
    }
#endif

    //! Assignment of constant string reference (no copy)
    /*! \param str Constant string reference to be assigned
        \note This overload is needed to avoid clashes with the generic primitive type assignment overload below.
        \see GenericStringRef, operator=(T)
    */
    GenericValue& operator=(StringRefType str) RAPIDJSON_NOEXCEPT {
        GenericValue s(str);
        return *this = s;
    }

    //! Assignment with primitive types.
    /*! \tparam T Either \ref Type, \c int, \c unsigned, \c int64_t, \c uint64_t
        \param value The value to be assigned.

        \note The source type \c T explicitly disallows all pointer types,
            especially (\c const) \ref Ch*.  This helps avoiding implicitly
            referencing character strings with insufficient lifetime, use
            \ref SetString(const Ch*, Allocator&) (for copying) or
            \ref StringRef() (to explicitly mark the pointer as constant) instead.
            All other pointer types would implicitly convert to \c bool,
            use \ref SetBool() instead.
    */
    template <typename T>
    RAPIDJSON_DISABLEIF_RETURN((internal::IsPointer<T>), (GenericValue&))
    operator=(T value) {
        GenericValue v(value);
        return *this = v;
    }

    //! Deep-copy assignment from Value
    /*! Assigns a \b copy of the Value to the current Value object
        \tparam SourceAllocator Allocator type of \c rhs
        \param rhs Value to copy from (read-only)
        \param allocator Allocator to use for copying
     */
    template <typename SourceAllocator>
    GenericValue& CopyFrom(const GenericValue<Encoding, SourceAllocator>& rhs, Allocator& allocator) {
        RAPIDJSON_ASSERT((void*)this != (void const*)&rhs);
        this->~GenericValue();
        new (this) GenericValue(rhs, allocator);
        return *this;
    }

    //! Exchange the contents of this value with those of other.
    /*!
        \param other Another value.
        \note Constant complexity.
    */
    GenericValue& Swap(GenericValue& other) RAPIDJSON_NOEXCEPT {
        GenericValue temp;
        temp.RawAssign(*this);
        RawAssign(other);
        other.RawAssign(temp);
        return *this;
    }

    //! Prepare Value for move semantics
    /*! \return *this */
    GenericValue& Move() RAPIDJSON_NOEXCEPT { return *this; }
    //@}

    //!@name Equal-to and not-equal-to operators
    //@{
    //! Equal-to operator
    /*!
        \note If an object contains duplicated named member, comparing equality with any object is always \c false.
        \note Linear time complexity (number of all values in the subtree and total lengths of all strings).
    */
    template <typename SourceAllocator>
    bool operator==(const GenericValue<Encoding, SourceAllocator>& rhs) const {
        typedef GenericValue<Encoding, SourceAllocator> RhsType;
        if (GetType() != rhs.GetType())
            return false;

        switch (GetType()) {
        case kObjectType: // Warning: O(n^2) inner-loop
            if (data_.o.size != rhs.data_.o.size)
                return false;           
            for (ConstMemberIterator lhsMemberItr = MemberBegin(); lhsMemberItr != MemberEnd(); ++lhsMemberItr) {
                typename RhsType::ConstMemberIterator rhsMemberItr = rhs.FindMember(lhsMemberItr->name);
                if (rhsMemberItr == rhs.MemberEnd() || lhsMemberItr->value != rhsMemberItr->value)
                    return false;
            }
            return true;
            
        case kArrayType:
            if (data_.a.size != rhs.data_.a.size)
                return false;
            for (SizeType i = 0; i < data_.a.size; i++)
                if ((*this)[i] != rhs[i])
                    return false;
            return true;

        case kStringType:
            return StringEqual(rhs);

        case kNumberType:
            if (IsDouble() || rhs.IsDouble())
                return GetDouble() == rhs.GetDouble(); // May convert one operand from integer to double.
            else
                return data_.n.u64 == rhs.data_.n.u64;

        default: // kTrueType, kFalseType, kNullType
            return true;
        }
    }

    //! Equal-to operator with const C-string pointer
    bool operator==(const Ch* rhs) const { return *this == GenericValue(StringRef(rhs)); }

#if RAPIDJSON_HAS_STDSTRING
    //! Equal-to operator with string object
    /*! \note Requires the definition of the preprocessor symbol \ref RAPIDJSON_HAS_STDSTRING.
     */
    bool operator==(const std::basic_string<Ch>& rhs) const { return *this == GenericValue(StringRef(rhs)); }
#endif

    //! Equal-to operator with primitive types
    /*! \tparam T Either \ref Type, \c int, \c unsigned, \c int64_t, \c uint64_t, \c double, \c true, \c false
    */
    template <typename T> RAPIDJSON_DISABLEIF_RETURN((internal::OrExpr<internal::IsPointer<T>,internal::IsGenericValue<T> >), (bool)) operator==(const T& rhs) const { return *this == GenericValue(rhs); }

    //! Not-equal-to operator
    /*! \return !(*this == rhs)
     */
    template <typename SourceAllocator>
    bool operator!=(const GenericValue<Encoding, SourceAllocator>& rhs) const { return !(*this == rhs); }

    //! Not-equal-to operator with const C-string pointer
    bool operator!=(const Ch* rhs) const { return !(*this == rhs); }

    //! Not-equal-to operator with arbitrary types
    /*! \return !(*this == rhs)
     */
    template <typename T> RAPIDJSON_DISABLEIF_RETURN((internal::IsGenericValue<T>), (bool)) operator!=(const T& rhs) const { return !(*this == rhs); }

    //! Equal-to operator with arbitrary types (symmetric version)
    /*! \return (rhs == lhs)
     */
    template <typename T> friend RAPIDJSON_DISABLEIF_RETURN((internal::IsGenericValue<T>), (bool)) operator==(const T& lhs, const GenericValue& rhs) { return rhs == lhs; }

    //! Not-Equal-to operator with arbitrary types (symmetric version)
    /*! \return !(rhs == lhs)
     */
    template <typename T> friend RAPIDJSON_DISABLEIF_RETURN((internal::IsGenericValue<T>), (bool)) operator!=(const T& lhs, const GenericValue& rhs) { return !(rhs == lhs); }
    //@}

    //!@name Type
    //@{

    Type GetType()  const { return static_cast<Type>(flags_ & kTypeMask); }
    bool IsNull()   const { return flags_ == kNullFlag; }
    bool IsFalse()  const { return flags_ == kFalseFlag; }
    bool IsTrue()   const { return flags_ == kTrueFlag; }
    bool IsBool()   const { return (flags_ & kBoolFlag) != 0; }
    bool IsObject() const { return flags_ == kObjectFlag; }
    bool IsArray()  const { return flags_ == kArrayFlag; }
    bool IsNumber() const { return (flags_ & kNumberFlag) != 0; }
    bool IsInt()    const { return (flags_ & kIntFlag) != 0; }
    bool IsUint()   const { return (flags_ & kUintFlag) != 0; }
    bool IsInt64()  const { return (flags_ & kInt64Flag) != 0; }
    bool IsUint64() const { return (flags_ & kUint64Flag) != 0; }
    bool IsDouble() const { return (flags_ & kDoubleFlag) != 0; }
    bool IsString() const { return (flags_ & kStringFlag) != 0; }

    //@}

    //!@name Null
    //@{

    GenericValue& SetNull() { this->~GenericValue(); new (this) GenericValue(); return *this; }

    //@}

    //!@name Bool
    //@{

    bool GetBool() const { RAPIDJSON_ASSERT(IsBool()); return flags_ == kTrueFlag; }
    //!< Set boolean value
    /*! \post IsBool() == true */
    GenericValue& SetBool(bool b) { this->~GenericValue(); new (this) GenericValue(b); return *this; }

    //@}

    //!@name Object
    //@{

    //! Set this value as an empty object.
    /*! \post IsObject() == true */
    GenericValue& SetObject() { this->~GenericValue(); new (this) GenericValue(kObjectType); return *this; }

    //! Get the number of members in the object.
    SizeType MemberCount() const { RAPIDJSON_ASSERT(IsObject()); return data_.o.size; }

    //! Check whether the object is empty.
    bool ObjectEmpty() const { RAPIDJSON_ASSERT(IsObject()); return data_.o.size == 0; }

    //! Get a value from an object associated with the name.
    /*! \pre IsObject() == true
        \tparam T Either \c Ch or \c const \c Ch (template used for disambiguation with \ref operator[](SizeType))
        \note In version 0.1x, if the member is not found, this function returns a null value. This makes issue 7.
        Since 0.2, if the name is not correct, it will assert.
        If user is unsure whether a member exists, user should use HasMember() first.
        A better approach is to use FindMember().
        \note Linear time complexity.
    */
    template <typename T>
    RAPIDJSON_DISABLEIF_RETURN((internal::NotExpr<internal::IsSame<typename internal::RemoveConst<T>::Type, Ch> >),(GenericValue&)) operator[](T* name) {
        GenericValue n(StringRef(name));
        return (*this)[n];
    }
    template <typename T>
    RAPIDJSON_DISABLEIF_RETURN((internal::NotExpr<internal::IsSame<typename internal::RemoveConst<T>::Type, Ch> >),(const GenericValue&)) operator[](T* name) const { return const_cast<GenericValue&>(*this)[name]; }

    //! Get a value from an object associated with the name.
    /*! \pre IsObject() == true
        \tparam SourceAllocator Allocator of the \c name value

        \note Compared to \ref operator[](T*), this version is faster because it does not need a StrLen().
        And it can also handle strings with embedded null characters.

        \note Linear time complexity.
    */
    template <typename SourceAllocator>
    GenericValue& operator[](const GenericValue<Encoding, SourceAllocator>& name) {
        MemberIterator member = FindMember(name);
        if (member != MemberEnd())
            return member->value;
        else {
            RAPIDJSON_ASSERT(false);    // see above note
            static GenericValue NullValue;
            return NullValue;
        }
    }
    template <typename SourceAllocator>
    const GenericValue& operator[](const GenericValue<Encoding, SourceAllocator>& name) const { return const_cast<GenericValue&>(*this)[name]; }

    //! Const member iterator
    /*! \pre IsObject() == true */
    ConstMemberIterator MemberBegin() const { RAPIDJSON_ASSERT(IsObject()); return ConstMemberIterator(data_.o.members); }
    //! Const \em past-the-end member iterator
    /*! \pre IsObject() == true */
    ConstMemberIterator MemberEnd() const   { RAPIDJSON_ASSERT(IsObject()); return ConstMemberIterator(data_.o.members + data_.o.size); }
    //! Member iterator
    /*! \pre IsObject() == true */
    MemberIterator MemberBegin()            { RAPIDJSON_ASSERT(IsObject()); return MemberIterator(data_.o.members); }
    //! \em Past-the-end member iterator
    /*! \pre IsObject() == true */
    MemberIterator MemberEnd()              { RAPIDJSON_ASSERT(IsObject()); return MemberIterator(data_.o.members + data_.o.size); }

    //! Check whether a member exists in the object.
    /*!
        \param name Member name to be searched.
        \pre IsObject() == true
        \return Whether a member with that name exists.
        \note It is better to use FindMember() directly if you need the obtain the value as well.
        \note Linear time complexity.
    */
    bool HasMember(const Ch* name) const { return FindMember(name) != MemberEnd(); }

    //! Check whether a member exists in the object with GenericValue name.
    /*!
        This version is faster because it does not need a StrLen(). It can also handle string with null character.
        \param name Member name to be searched.
        \pre IsObject() == true
        \return Whether a member with that name exists.
        \note It is better to use FindMember() directly if you need the obtain the value as well.
        \note Linear time complexity.
    */
    template <typename SourceAllocator>
    bool HasMember(const GenericValue<Encoding, SourceAllocator>& name) const { return FindMember(name) != MemberEnd(); }

    //! Find member by name.
    /*!
        \param name Member name to be searched.
        \pre IsObject() == true
        \return Iterator to member, if it exists.
            Otherwise returns \ref MemberEnd().

        \note Earlier versions of Rapidjson returned a \c NULL pointer, in case
            the requested member doesn't exist. For consistency with e.g.
            \c std::map, this has been changed to MemberEnd() now.
        \note Linear time complexity.
    */
    MemberIterator FindMember(const Ch* name) {
        GenericValue n(StringRef(name));
        return FindMember(n);
    }

    ConstMemberIterator FindMember(const Ch* name) const { return const_cast<GenericValue&>(*this).FindMember(name); }

    //! Find member by name.
    /*!
        This version is faster because it does not need a StrLen(). It can also handle string with null character.
        \param name Member name to be searched.
        \pre IsObject() == true
        \return Iterator to member, if it exists.
            Otherwise returns \ref MemberEnd().

        \note Earlier versions of Rapidjson returned a \c NULL pointer, in case
            the requested member doesn't exist. For consistency with e.g.
            \c std::map, this has been changed to MemberEnd() now.
        \note Linear time complexity.
    */
    template <typename SourceAllocator>
    MemberIterator FindMember(const GenericValue<Encoding, SourceAllocator>& name) {
        RAPIDJSON_ASSERT(IsObject());
        RAPIDJSON_ASSERT(name.IsString());
        MemberIterator member = MemberBegin();
        for ( ; member != MemberEnd(); ++member)
            if (name.StringEqual(member->name))
                break;
        return member;
    }
    template <typename SourceAllocator> ConstMemberIterator FindMember(const GenericValue<Encoding, SourceAllocator>& name) const { return const_cast<GenericValue&>(*this).FindMember(name); }

    //! Add a member (name-value pair) to the object.
    /*! \param name A string value as name of member.
        \param value Value of any type.
        \param allocator    Allocator for reallocating memory. It must be the same one as used before. Commonly use GenericDocument::GetAllocator().
        \return The value itself for fluent API.
        \note The ownership of \c name and \c value will be transferred to this object on success.
        \pre  IsObject() && name.IsString()
        \post name.IsNull() && value.IsNull()
        \note Amortized Constant time complexity.
    */
    GenericValue& AddMember(GenericValue& name, GenericValue& value, Allocator& allocator) {
        RAPIDJSON_ASSERT(IsObject());
        RAPIDJSON_ASSERT(name.IsString());

        Object& o = data_.o;
        if (o.size >= o.capacity) {
            if (o.capacity == 0) {
                o.capacity = kDefaultObjectCapacity;
                o.members = reinterpret_cast<Member*>(allocator.Malloc(o.capacity * sizeof(Member)));
            }
            else {
                SizeType oldCapacity = o.capacity;
                o.capacity += (oldCapacity + 1) / 2; // grow by factor 1.5
                o.members = reinterpret_cast<Member*>(allocator.Realloc(o.members, oldCapacity * sizeof(Member), o.capacity * sizeof(Member)));
            }
        }
        o.members[o.size].name.RawAssign(name);
        o.members[o.size].value.RawAssign(value);
        o.size++;
        return *this;
    }

#if RAPIDJSON_HAS_CXX11_RVALUE_REFS
    GenericValue& AddMember(GenericValue&& name, GenericValue&& value, Allocator& allocator) {
        return AddMember(name, value, allocator);
    }
    GenericValue& AddMember(GenericValue&& name, GenericValue& value, Allocator& allocator) {
        return AddMember(name, value, allocator);
    }
    GenericValue& AddMember(GenericValue& name, GenericValue&& value, Allocator& allocator) {
        return AddMember(name, value, allocator);
    }
    GenericValue& AddMember(StringRefType name, GenericValue&& value, Allocator& allocator) {
        GenericValue n(name);
        return AddMember(n, value, allocator);
    }
#endif // RAPIDJSON_HAS_CXX11_RVALUE_REFS


    //! Add a member (name-value pair) to the object.
    /*! \param name A constant string reference as name of member.
        \param value Value of any type.
        \param allocator    Allocator for reallocating memory. It must be the same one as used before. Commonly use GenericDocument::GetAllocator().
        \return The value itself for fluent API.
        \note The ownership of \c value will be transferred to this object on success.
        \pre  IsObject()
        \post value.IsNull()
        \note Amortized Constant time complexity.
    */
    GenericValue& AddMember(StringRefType name, GenericValue& value, Allocator& allocator) {
        GenericValue n(name);
        return AddMember(n, value, allocator);
    }

    //! Add a constant string value as member (name-value pair) to the object.
    /*! \param name A constant string reference as name of member.
        \param value constant string reference as value of member.
        \param allocator    Allocator for reallocating memory. It must be the same one as used before. Commonly use GenericDocument::GetAllocator().
        \return The value itself for fluent API.
        \pre  IsObject()
        \note This overload is needed to avoid clashes with the generic primitive type AddMember(StringRefType,T,Allocator&) overload below.
        \note Amortized Constant time complexity.
    */
    GenericValue& AddMember(StringRefType name, StringRefType value, Allocator& allocator) {
        GenericValue v(value);
        return AddMember(name, v, allocator);
    }

    //! Add any primitive value as member (name-value pair) to the object.
    /*! \tparam T Either \ref Type, \c int, \c unsigned, \c int64_t, \c uint64_t
        \param name A constant string reference as name of member.
        \param value Value of primitive type \c T as value of member
        \param allocator Allocator for reallocating memory. Commonly use GenericDocument::GetAllocator().
        \return The value itself for fluent API.
        \pre  IsObject()

        \note The source type \c T explicitly disallows all pointer types,
            especially (\c const) \ref Ch*.  This helps avoiding implicitly
            referencing character strings with insufficient lifetime, use
            \ref AddMember(StringRefType, GenericValue&, Allocator&) or \ref
            AddMember(StringRefType, StringRefType, Allocator&).
            All other pointer types would implicitly convert to \c bool,
            use an explicit cast instead, if needed.
        \note Amortized Constant time complexity.
    */
    template <typename T>
    RAPIDJSON_DISABLEIF_RETURN((internal::OrExpr<internal::IsPointer<T>, internal::IsGenericValue<T> >), (GenericValue&))
    AddMember(StringRefType name, T value, Allocator& allocator) {
        GenericValue n(name);
        GenericValue v(value);
        return AddMember(n, v, allocator);
    }

    //! Remove all members in the object.
    /*! This function do not deallocate memory in the object, i.e. the capacity is unchanged.
        \note Linear time complexity.
    */
    void RemoveAllMembers() {
        RAPIDJSON_ASSERT(IsObject()); 
        for (MemberIterator m = MemberBegin(); m != MemberEnd(); ++m)
            m->~Member();
        data_.o.size = 0;
    }

    //! Remove a member in object by its name.
    /*! \param name Name of member to be removed.
        \return Whether the member existed.
        \note This function may reorder the object members. Use \ref
            EraseMember(ConstMemberIterator) if you need to preserve the
            relative order of the remaining members.
        \note Linear time complexity.
    */
    bool RemoveMember(const Ch* name) {
        GenericValue n(StringRef(name));
        return RemoveMember(n);
    }

    template <typename SourceAllocator>
    bool RemoveMember(const GenericValue<Encoding, SourceAllocator>& name) {
        MemberIterator m = FindMember(name);
        if (m != MemberEnd()) {
            RemoveMember(m);
            return true;
        }
        else
            return false;
    }

    //! Remove a member in object by iterator.
    /*! \param m member iterator (obtained by FindMember() or MemberBegin()).
        \return the new iterator after removal.
        \note This function may reorder the object members. Use \ref
            EraseMember(ConstMemberIterator) if you need to preserve the
            relative order of the remaining members.
        \note Constant time complexity.
    */
    MemberIterator RemoveMember(MemberIterator m) {
        RAPIDJSON_ASSERT(IsObject());
        RAPIDJSON_ASSERT(data_.o.size > 0);
        RAPIDJSON_ASSERT(data_.o.members != 0);
        RAPIDJSON_ASSERT(m >= MemberBegin() && m < MemberEnd());

        MemberIterator last(data_.o.members + (data_.o.size - 1));
        if (data_.o.size > 1 && m != last) {
            // Move the last one to this place
            *m = *last;
        }
        else {
            // Only one left, just destroy
            m->~Member();
        }
        --data_.o.size;
        return m;
    }

    //! Remove a member from an object by iterator.
    /*! \param pos iterator to the member to remove
        \pre IsObject() == true && \ref MemberBegin() <= \c pos < \ref MemberEnd()
        \return Iterator following the removed element.
            If the iterator \c pos refers to the last element, the \ref MemberEnd() iterator is returned.
        \note This function preserves the relative order of the remaining object
            members. If you do not need this, use the more efficient \ref RemoveMember(MemberIterator).
        \note Linear time complexity.
    */
    MemberIterator EraseMember(ConstMemberIterator pos) {
        return EraseMember(pos, pos +1);
    }

    //! Remove members in the range [first, last) from an object.
    /*! \param first iterator to the first member to remove
        \param last  iterator following the last member to remove
        \pre IsObject() == true && \ref MemberBegin() <= \c first <= \c last <= \ref MemberEnd()
        \return Iterator following the last removed element.
        \note This function preserves the relative order of the remaining object
            members.
        \note Linear time complexity.
    */
    MemberIterator EraseMember(ConstMemberIterator first, ConstMemberIterator last) {
        RAPIDJSON_ASSERT(IsObject());
        RAPIDJSON_ASSERT(data_.o.size > 0);
        RAPIDJSON_ASSERT(data_.o.members != 0);
        RAPIDJSON_ASSERT(first >= MemberBegin());
        RAPIDJSON_ASSERT(first <= last);
        RAPIDJSON_ASSERT(last <= MemberEnd());

        MemberIterator pos = MemberBegin() + (first - MemberBegin());
        for (MemberIterator itr = pos; itr != last; ++itr)
            itr->~Member();
        std::memmove(&*pos, &*last, (MemberEnd() - last) * sizeof(Member));
        data_.o.size -= (last - first);
        return pos;
    }

    //@}

    //!@name Array
    //@{

    //! Set this value as an empty array.
    /*! \post IsArray == true */
    GenericValue& SetArray() {  this->~GenericValue(); new (this) GenericValue(kArrayType); return *this; }

    //! Get the number of elements in array.
    SizeType Size() const { RAPIDJSON_ASSERT(IsArray()); return data_.a.size; }

    //! Get the capacity of array.
    SizeType Capacity() const { RAPIDJSON_ASSERT(IsArray()); return data_.a.capacity; }

    //! Check whether the array is empty.
    bool Empty() const { RAPIDJSON_ASSERT(IsArray()); return data_.a.size == 0; }

    //! Remove all elements in the array.
    /*! This function do not deallocate memory in the array, i.e. the capacity is unchanged.
        \note Linear time complexity.
    */
    void Clear() {
        RAPIDJSON_ASSERT(IsArray()); 
        for (SizeType i = 0; i < data_.a.size; ++i)
            data_.a.elements[i].~GenericValue();
        data_.a.size = 0;
    }

    //! Get an element from array by index.
    /*! \pre IsArray() == true
        \param index Zero-based index of element.
        \see operator[](T*)
    */
    GenericValue& operator[](SizeType index) {
        RAPIDJSON_ASSERT(IsArray());
        RAPIDJSON_ASSERT(index < data_.a.size);
        return data_.a.elements[index];
    }
    const GenericValue& operator[](SizeType index) const { return const_cast<GenericValue&>(*this)[index]; }

    //! Element iterator
    /*! \pre IsArray() == true */
    ValueIterator Begin() { RAPIDJSON_ASSERT(IsArray()); return data_.a.elements; }
    //! \em Past-the-end element iterator
    /*! \pre IsArray() == true */
    ValueIterator End() { RAPIDJSON_ASSERT(IsArray()); return data_.a.elements + data_.a.size; }
    //! Constant element iterator
    /*! \pre IsArray() == true */
    ConstValueIterator Begin() const { return const_cast<GenericValue&>(*this).Begin(); }
    //! Constant \em past-the-end element iterator
    /*! \pre IsArray() == true */
    ConstValueIterator End() const { return const_cast<GenericValue&>(*this).End(); }

    //! Request the array to have enough capacity to store elements.
    /*! \param newCapacity  The capacity that the array at least need to have.
        \param allocator    Allocator for reallocating memory. It must be the same one as used before. Commonly use GenericDocument::GetAllocator().
        \return The value itself for fluent API.
        \note Linear time complexity.
    */
    GenericValue& Reserve(SizeType newCapacity, Allocator &allocator) {
        RAPIDJSON_ASSERT(IsArray());
        if (newCapacity > data_.a.capacity) {
            data_.a.elements = (GenericValue*)allocator.Realloc(data_.a.elements, data_.a.capacity * sizeof(GenericValue), newCapacity * sizeof(GenericValue));
            data_.a.capacity = newCapacity;
        }
        return *this;
    }

    //! Append a GenericValue at the end of the array.
    /*! \param value        Value to be appended.
        \param allocator    Allocator for reallocating memory. It must be the same one as used before. Commonly use GenericDocument::GetAllocator().
        \pre IsArray() == true
        \post value.IsNull() == true
        \return The value itself for fluent API.
        \note The ownership of \c value will be transferred to this array on success.
        \note If the number of elements to be appended is known, calls Reserve() once first may be more efficient.
        \note Amortized constant time complexity.
    */
    GenericValue& PushBack(GenericValue& value, Allocator& allocator) {
        RAPIDJSON_ASSERT(IsArray());
        if (data_.a.size >= data_.a.capacity)
            Reserve(data_.a.capacity == 0 ? kDefaultArrayCapacity : (data_.a.capacity + (data_.a.capacity + 1) / 2), allocator);
        data_.a.elements[data_.a.size++].RawAssign(value);
        return *this;
    }

#if RAPIDJSON_HAS_CXX11_RVALUE_REFS
    GenericValue& PushBack(GenericValue&& value, Allocator& allocator) {
        return PushBack(value, allocator);
    }
#endif // RAPIDJSON_HAS_CXX11_RVALUE_REFS

    //! Append a constant string reference at the end of the array.
    /*! \param value        Constant string reference to be appended.
        \param allocator    Allocator for reallocating memory. It must be the same one used previously. Commonly use GenericDocument::GetAllocator().
        \pre IsArray() == true
        \return The value itself for fluent API.
        \note If the number of elements to be appended is known, calls Reserve() once first may be more efficient.
        \note Amortized constant time complexity.
        \see GenericStringRef
    */
    GenericValue& PushBack(StringRefType value, Allocator& allocator) {
        return (*this).template PushBack<StringRefType>(value, allocator);
    }

    //! Append a primitive value at the end of the array.
    /*! \tparam T Either \ref Type, \c int, \c unsigned, \c int64_t, \c uint64_t
        \param value Value of primitive type T to be appended.
        \param allocator    Allocator for reallocating memory. It must be the same one as used before. Commonly use GenericDocument::GetAllocator().
        \pre IsArray() == true
        \return The value itself for fluent API.
        \note If the number of elements to be appended is known, calls Reserve() once first may be more efficient.

        \note The source type \c T explicitly disallows all pointer types,
            especially (\c const) \ref Ch*.  This helps avoiding implicitly
            referencing character strings with insufficient lifetime, use
            \ref PushBack(GenericValue&, Allocator&) or \ref
            PushBack(StringRefType, Allocator&).
            All other pointer types would implicitly convert to \c bool,
            use an explicit cast instead, if needed.
        \note Amortized constant time complexity.
    */
    template <typename T>
    RAPIDJSON_DISABLEIF_RETURN((internal::OrExpr<internal::IsPointer<T>, internal::IsGenericValue<T> >), (GenericValue&))
    PushBack(T value, Allocator& allocator) {
        GenericValue v(value);
        return PushBack(v, allocator);
    }

    //! Remove the last element in the array.
    /*!
        \note Constant time complexity.
    */
    GenericValue& PopBack() {
        RAPIDJSON_ASSERT(IsArray());
        RAPIDJSON_ASSERT(!Empty());
        data_.a.elements[--data_.a.size].~GenericValue();
        return *this;
    }

    //! Remove an element of array by iterator.
    /*!
        \param pos iterator to the element to remove
        \pre IsArray() == true && \ref Begin() <= \c pos < \ref End()
        \return Iterator following the removed element. If the iterator pos refers to the last element, the End() iterator is returned.
        \note Linear time complexity.
    */
    ValueIterator Erase(ConstValueIterator pos) {
        return Erase(pos, pos + 1);
    }

    //! Remove elements in the range [first, last) of the array.
    /*!
        \param first iterator to the first element to remove
        \param last  iterator following the last element to remove
        \pre IsArray() == true && \ref Begin() <= \c first <= \c last <= \ref End()
        \return Iterator following the last removed element.
        \note Linear time complexity.
    */
    ValueIterator Erase(ConstValueIterator first, ConstValueIterator last) {
        RAPIDJSON_ASSERT(IsArray());
        RAPIDJSON_ASSERT(data_.a.size > 0);
        RAPIDJSON_ASSERT(data_.a.elements != 0);
        RAPIDJSON_ASSERT(first >= Begin());
        RAPIDJSON_ASSERT(first <= last);
        RAPIDJSON_ASSERT(last <= End());
        ValueIterator pos = Begin() + (first - Begin());
        for (ValueIterator itr = pos; itr != last; ++itr)
            itr->~GenericValue();       
        std::memmove(pos, last, (End() - last) * sizeof(GenericValue));
        data_.a.size -= (last - first);
        return pos;
    }

    //@}

    //!@name Number
    //@{

    int GetInt() const          { RAPIDJSON_ASSERT(flags_ & kIntFlag);   return data_.n.i.i;   }
    unsigned GetUint() const    { RAPIDJSON_ASSERT(flags_ & kUintFlag);  return data_.n.u.u;   }
    int64_t GetInt64() const    { RAPIDJSON_ASSERT(flags_ & kInt64Flag); return data_.n.i64; }
    uint64_t GetUint64() const  { RAPIDJSON_ASSERT(flags_ & kUint64Flag); return data_.n.u64; }

    double GetDouble() const {
        RAPIDJSON_ASSERT(IsNumber());
        if ((flags_ & kDoubleFlag) != 0)                return data_.n.d;   // exact type, no conversion.
        if ((flags_ & kIntFlag) != 0)                   return data_.n.i.i; // int -> double
        if ((flags_ & kUintFlag) != 0)                  return data_.n.u.u; // unsigned -> double
        if ((flags_ & kInt64Flag) != 0)                 return (double)data_.n.i64; // int64_t -> double (may lose precision)
        RAPIDJSON_ASSERT((flags_ & kUint64Flag) != 0);  return (double)data_.n.u64; // uint64_t -> double (may lose precision)
    }

    GenericValue& SetInt(int i)             { this->~GenericValue(); new (this) GenericValue(i);    return *this; }
    GenericValue& SetUint(unsigned u)       { this->~GenericValue(); new (this) GenericValue(u);    return *this; }
    GenericValue& SetInt64(int64_t i64)     { this->~GenericValue(); new (this) GenericValue(i64);  return *this; }
    GenericValue& SetUint64(uint64_t u64)   { this->~GenericValue(); new (this) GenericValue(u64);  return *this; }
    GenericValue& SetDouble(double d)       { this->~GenericValue(); new (this) GenericValue(d);    return *this; }

    //@}

    //!@name String
    //@{

    const Ch* GetString() const { RAPIDJSON_ASSERT(IsString()); return ((flags_ & kInlineStrFlag) ? data_.ss.str : data_.s.str); }

    //! Get the length of string.
    /*! Since rapidjson permits "\\u0000" in the json string, strlen(v.GetString()) may not equal to v.GetStringLength().
    */
    SizeType GetStringLength() const { RAPIDJSON_ASSERT(IsString()); return ((flags_ & kInlineStrFlag) ? (data_.ss.GetLength()) : data_.s.length); }

    //! Set this value as a string without copying source string.
    /*! This version has better performance with supplied length, and also support string containing null character.
        \param s source string pointer. 
        \param length The length of source string, excluding the trailing null terminator.
        \return The value itself for fluent API.
        \post IsString() == true && GetString() == s && GetStringLength() == length
        \see SetString(StringRefType)
    */
    GenericValue& SetString(const Ch* s, SizeType length) { return SetString(StringRef(s, length)); }

    //! Set this value as a string without copying source string.
    /*! \param s source string reference
        \return The value itself for fluent API.
        \post IsString() == true && GetString() == s && GetStringLength() == s.length
    */
    GenericValue& SetString(StringRefType s) { this->~GenericValue(); SetStringRaw(s); return *this; }

    //! Set this value as a string by copying from source string.
    /*! This version has better performance with supplied length, and also support string containing null character.
        \param s source string. 
        \param length The length of source string, excluding the trailing null terminator.
        \param allocator Allocator for allocating copied buffer. Commonly use GenericDocument::GetAllocator().
        \return The value itself for fluent API.
        \post IsString() == true && GetString() != s && strcmp(GetString(),s) == 0 && GetStringLength() == length
    */
    GenericValue& SetString(const Ch* s, SizeType length, Allocator& allocator) { this->~GenericValue(); SetStringRaw(StringRef(s, length), allocator); return *this; }

    //! Set this value as a string by copying from source string.
    /*! \param s source string. 
        \param allocator Allocator for allocating copied buffer. Commonly use GenericDocument::GetAllocator().
        \return The value itself for fluent API.
        \post IsString() == true && GetString() != s && strcmp(GetString(),s) == 0 && GetStringLength() == length
    */
    GenericValue& SetString(const Ch* s, Allocator& allocator) { return SetString(s, internal::StrLen(s), allocator); }

#if RAPIDJSON_HAS_STDSTRING
    //! Set this value as a string by copying from source string.
    /*! \param s source string.
        \param allocator Allocator for allocating copied buffer. Commonly use GenericDocument::GetAllocator().
        \return The value itself for fluent API.
        \post IsString() == true && GetString() != s.data() && strcmp(GetString(),s.data() == 0 && GetStringLength() == s.size()
        \note Requires the definition of the preprocessor symbol \ref RAPIDJSON_HAS_STDSTRING.
    */
    GenericValue& SetString(const std::basic_string<Ch>& s, Allocator& allocator) { return SetString(s.data(), s.size(), allocator); }
#endif

    //@}

    //! Generate events of this value to a Handler.
    /*! This function adopts the GoF visitor pattern.
        Typical usage is to output this JSON value as JSON text via Writer, which is a Handler.
        It can also be used to deep clone this value via GenericDocument, which is also a Handler.
        \tparam Handler type of handler.
        \param handler An object implementing concept Handler.
    */
    template <typename Handler>
    bool Accept(Handler& handler) const {
        switch(GetType()) {
        case kNullType:     return handler.Null();
        case kFalseType:    return handler.Bool(false);
        case kTrueType:     return handler.Bool(true);

        case kObjectType:
            if (!handler.StartObject())
                return false;
            for (ConstMemberIterator m = MemberBegin(); m != MemberEnd(); ++m) {
                if (!handler.Key(m->name.GetString(), m->name.GetStringLength(), (m->name.flags_ & kCopyFlag) != 0))
                    return false;
                if (!m->value.Accept(handler))
                    return false;
            }
            return handler.EndObject(data_.o.size);

        case kArrayType:
            if (!handler.StartArray())
                return false;
            for (GenericValue* v = data_.a.elements; v != data_.a.elements + data_.a.size; ++v)
                if (!v->Accept(handler))
                    return false;
            return handler.EndArray(data_.a.size);
    
        case kStringType:
            return handler.String(GetString(), GetStringLength(), (flags_ & kCopyFlag) != 0);
    
        case kNumberType:
            if (IsInt())            return handler.Int(data_.n.i.i);
            else if (IsUint())      return handler.Uint(data_.n.u.u);
            else if (IsInt64())     return handler.Int64(data_.n.i64);
            else if (IsUint64())    return handler.Uint64(data_.n.u64);
            else                    return handler.Double(data_.n.d);
    
        default:
            RAPIDJSON_ASSERT(false);
        }
        return false;
    }

private:
    template <typename, typename> friend class GenericValue;
    template <typename, typename, typename> friend class GenericDocument;

    enum {
        kBoolFlag = 0x100,
        kNumberFlag = 0x200,
        kIntFlag = 0x400,
        kUintFlag = 0x800,
        kInt64Flag = 0x1000,
        kUint64Flag = 0x2000,
        kDoubleFlag = 0x4000,
        kStringFlag = 0x100000,
        kCopyFlag = 0x200000,
        kInlineStrFlag = 0x400000,

        // Initial flags of different types.
        kNullFlag = kNullType,
        kTrueFlag = kTrueType | kBoolFlag,
        kFalseFlag = kFalseType | kBoolFlag,
        kNumberIntFlag = kNumberType | kNumberFlag | kIntFlag | kInt64Flag,
        kNumberUintFlag = kNumberType | kNumberFlag | kUintFlag | kUint64Flag | kInt64Flag,
        kNumberInt64Flag = kNumberType | kNumberFlag | kInt64Flag,
        kNumberUint64Flag = kNumberType | kNumberFlag | kUint64Flag,
        kNumberDoubleFlag = kNumberType | kNumberFlag | kDoubleFlag,
        kNumberAnyFlag = kNumberType | kNumberFlag | kIntFlag | kInt64Flag | kUintFlag | kUint64Flag | kDoubleFlag,
        kConstStringFlag = kStringType | kStringFlag,
        kCopyStringFlag = kStringType | kStringFlag | kCopyFlag,
        kShortStringFlag = kStringType | kStringFlag | kCopyFlag | kInlineStrFlag,
        kObjectFlag = kObjectType,
        kArrayFlag = kArrayType,

        kTypeMask = 0xFF    // bitwise-and with mask of 0xFF can be optimized by compiler
    };

    static const SizeType kDefaultArrayCapacity = 16;
    static const SizeType kDefaultObjectCapacity = 16;

    struct String {
        const Ch* str;
        SizeType length;
        unsigned hashcode;  //!< reserved
    };  // 12 bytes in 32-bit mode, 16 bytes in 64-bit mode

    // implementation detail: ShortString can represent zero-terminated strings up to MaxSize chars
    // (excluding the terminating zero) and store a value to determine the length of the contained
    // string in the last character str[LenPos] by storing "MaxSize - length" there. If the string
    // to store has the maximal length of MaxSize then str[LenPos] will be 0 and therefore act as
    // the string terminator as well. For getting the string length back from that value just use
    // "MaxSize - str[LenPos]".
    // This allows to store 11-chars strings in 32-bit mode and 15-chars strings in 64-bit mode
    // inline (for `UTF8`-encoded strings).
    struct ShortString {
        enum { MaxChars = sizeof(String) / sizeof(Ch), MaxSize = MaxChars - 1, LenPos = MaxSize };
        Ch str[MaxChars];

        inline static bool Usable(SizeType len) { return            (MaxSize >= len); }
        inline void     SetLength(SizeType len) { str[LenPos] = (Ch)(MaxSize -  len); }
        inline SizeType GetLength() const       { return  (SizeType)(MaxSize -  str[LenPos]); }
    };  // at most as many bytes as "String" above => 12 bytes in 32-bit mode, 16 bytes in 64-bit mode

    // By using proper binary layout, retrieval of different integer types do not need conversions.
    union Number {
#if RAPIDJSON_ENDIAN == RAPIDJSON_LITTLEENDIAN
        struct I {
            int i;
            char padding[4];
        }i;
        struct U {
            unsigned u;
            char padding2[4];
        }u;
#else
        struct I {
            char padding[4];
            int i;
        }i;
        struct U {
            char padding2[4];
            unsigned u;
        }u;
#endif
        int64_t i64;
        uint64_t u64;
        double d;
    };  // 8 bytes

    struct Object {
        Member* members;
        SizeType size;
        SizeType capacity;
    };  // 12 bytes in 32-bit mode, 16 bytes in 64-bit mode

    struct Array {
        GenericValue* elements;
        SizeType size;
        SizeType capacity;
    };  // 12 bytes in 32-bit mode, 16 bytes in 64-bit mode

    union Data {
        String s;
        ShortString ss;
        Number n;
        Object o;
        Array a;
    };  // 12 bytes in 32-bit mode, 16 bytes in 64-bit mode

    // Initialize this value as array with initial data, without calling destructor.
    void SetArrayRaw(GenericValue* values, SizeType count, Allocator& allocator) {
        flags_ = kArrayFlag;
        data_.a.elements = (GenericValue*)allocator.Malloc(count * sizeof(GenericValue));
        std::memcpy(data_.a.elements, values, count * sizeof(GenericValue));
        data_.a.size = data_.a.capacity = count;
    }

    //! Initialize this value as object with initial data, without calling destructor.
    void SetObjectRaw(Member* members, SizeType count, Allocator& allocator) {
        flags_ = kObjectFlag;
        data_.o.members = (Member*)allocator.Malloc(count * sizeof(Member));
        std::memcpy(data_.o.members, members, count * sizeof(Member));
        data_.o.size = data_.o.capacity = count;
    }

    //! Initialize this value as constant string, without calling destructor.
    void SetStringRaw(StringRefType s) RAPIDJSON_NOEXCEPT {
        flags_ = kConstStringFlag;
        data_.s.str = s;
        data_.s.length = s.length;
    }

    //! Initialize this value as copy string with initial data, without calling destructor.
    void SetStringRaw(StringRefType s, Allocator& allocator) {
        Ch* str = NULL;
        if(ShortString::Usable(s.length)) {
            flags_ = kShortStringFlag;
            data_.ss.SetLength(s.length);
            str = data_.ss.str;
        } else {
            flags_ = kCopyStringFlag;
            data_.s.length = s.length;
            str = (Ch *)allocator.Malloc((s.length + 1) * sizeof(Ch));
            data_.s.str = str;
        }
        std::memcpy(str, s, s.length * sizeof(Ch));
        str[s.length] = '\0';
    }

    //! Assignment without calling destructor
    void RawAssign(GenericValue& rhs) RAPIDJSON_NOEXCEPT {
        data_ = rhs.data_;
        flags_ = rhs.flags_;
        rhs.flags_ = kNullFlag;
    }

    template <typename SourceAllocator>
    bool StringEqual(const GenericValue<Encoding, SourceAllocator>& rhs) const {
        RAPIDJSON_ASSERT(IsString());
        RAPIDJSON_ASSERT(rhs.IsString());

        const SizeType len1 = GetStringLength();
        const SizeType len2 = rhs.GetStringLength();
        if(len1 != len2) { return false; }

        const Ch* const str1 = GetString();
        const Ch* const str2 = rhs.GetString();
        if(str1 == str2) { return true; } // fast path for constant string

        return (std::memcmp(str1, str2, sizeof(Ch) * len1) == 0);
    }

    Data data_;
    unsigned flags_;
};

//! GenericValue with UTF8 encoding
typedef GenericValue<UTF8<> > Value;

///////////////////////////////////////////////////////////////////////////////
// GenericDocument 

//! A document for parsing JSON text as DOM.
/*!
    \note implements Handler concept
    \tparam Encoding Encoding for both parsing and string storage.
    \tparam Allocator Allocator for allocating memory for the DOM
    \tparam StackAllocator Allocator for allocating memory for stack during parsing.
    \warning Although GenericDocument inherits from GenericValue, the API does \b not provide any virtual functions, especially no virtual destructor.  To avoid memory leaks, do not \c delete a GenericDocument object via a pointer to a GenericValue.
*/
template <typename Encoding, typename Allocator = MemoryPoolAllocator<>, typename StackAllocator = CrtAllocator>
class GenericDocument : public GenericValue<Encoding, Allocator> {
public:
    typedef typename Encoding::Ch Ch;                       //!< Character type derived from Encoding.
    typedef GenericValue<Encoding, Allocator> ValueType;    //!< Value type of the document.
    typedef Allocator AllocatorType;                        //!< Allocator type from template parameter.

    //! Constructor
    /*! \param allocator        Optional allocator for allocating memory.
        \param stackCapacity    Optional initial capacity of stack in bytes.
        \param stackAllocator   Optional allocator for allocating memory for stack.
    */
    GenericDocument(Allocator* allocator = 0, size_t stackCapacity = kDefaultStackCapacity, StackAllocator* stackAllocator = 0) : 
        allocator_(allocator), ownAllocator_(0), stack_(stackAllocator, stackCapacity), parseResult_()
    {
        if (!allocator_)
            ownAllocator_ = allocator_ = new Allocator();
    }

#if RAPIDJSON_HAS_CXX11_RVALUE_REFS
    //! Move constructor in C++11
    GenericDocument(GenericDocument&& rhs) RAPIDJSON_NOEXCEPT
        : ValueType(std::move(rhs)),
          allocator_(rhs.allocator_),
          ownAllocator_(rhs.ownAllocator_),
          stack_(std::move(rhs.stack_)),
          parseResult_(rhs.parseResult_)
    {
        rhs.allocator_ = 0;
        rhs.ownAllocator_ = 0;
        rhs.parseResult_ = ParseResult();
    }
#endif

    ~GenericDocument() {
        Destroy();
    }

#if RAPIDJSON_HAS_CXX11_RVALUE_REFS
    //! Move assignment in C++11
    GenericDocument& operator=(GenericDocument&& rhs) RAPIDJSON_NOEXCEPT
    {
        // The cast to ValueType is necessary here, because otherwise it would
        // attempt to call GenericValue's templated assignment operator.
        ValueType::operator=(std::forward<ValueType>(rhs));

        // Calling the destructor here would prematurely call stack_'s destructor
        Destroy();

        allocator_ = rhs.allocator_;
        ownAllocator_ = rhs.ownAllocator_;
        stack_ = std::move(rhs.stack_);
        parseResult_ = rhs.parseResult_;

        rhs.allocator_ = 0;
        rhs.ownAllocator_ = 0;
        rhs.parseResult_ = ParseResult();

        return *this;
    }
#endif

    //!@name Parse from stream
    //!@{

    //! Parse JSON text from an input stream (with Encoding conversion)
    /*! \tparam parseFlags Combination of \ref ParseFlag.
        \tparam SourceEncoding Encoding of input stream
        \tparam InputStream Type of input stream, implementing Stream concept
        \param is Input stream to be parsed.
        \return The document itself for fluent API.
    */
    template <unsigned parseFlags, typename SourceEncoding, typename InputStream>
    GenericDocument& ParseStream(InputStream& is) {
        ValueType::SetNull(); // Remove existing root if exist
        GenericReader<SourceEncoding, Encoding, Allocator> reader(&GetAllocator());
        ClearStackOnExit scope(*this);
        parseResult_ = reader.template Parse<parseFlags>(is, *this);
        if (parseResult_) {
            RAPIDJSON_ASSERT(stack_.GetSize() == sizeof(ValueType)); // Got one and only one root object
            this->RawAssign(*stack_.template Pop<ValueType>(1));    // Add this-> to prevent issue 13.
        }
        return *this;
    }

    //! Parse JSON text from an input stream
    /*! \tparam parseFlags Combination of \ref ParseFlag.
        \tparam InputStream Type of input stream, implementing Stream concept
        \param is Input stream to be parsed.
        \return The document itself for fluent API.
    */
    template <unsigned parseFlags, typename InputStream>
    GenericDocument& ParseStream(InputStream& is) {
        return ParseStream<parseFlags,Encoding,InputStream>(is);
    }

    //! Parse JSON text from an input stream (with \ref kParseDefaultFlags)
    /*! \tparam InputStream Type of input stream, implementing Stream concept
        \param is Input stream to be parsed.
        \return The document itself for fluent API.
    */
    template <typename InputStream>
    GenericDocument& ParseStream(InputStream& is) {
        return ParseStream<kParseDefaultFlags, Encoding, InputStream>(is);
    }
    //!@}

    //!@name Parse in-place from mutable string
    //!@{

    //! Parse JSON text from a mutable string (with Encoding conversion)
    /*! \tparam parseFlags Combination of \ref ParseFlag.
        \tparam SourceEncoding Transcoding from input Encoding
        \param str Mutable zero-terminated string to be parsed.
        \return The document itself for fluent API.
    */
    template <unsigned parseFlags, typename SourceEncoding>
    GenericDocument& ParseInsitu(Ch* str) {
        GenericInsituStringStream<Encoding> s(str);
        return ParseStream<parseFlags | kParseInsituFlag, SourceEncoding>(s);
    }

    //! Parse JSON text from a mutable string
    /*! \tparam parseFlags Combination of \ref ParseFlag.
        \param str Mutable zero-terminated string to be parsed.
        \return The document itself for fluent API.
    */
    template <unsigned parseFlags>
    GenericDocument& ParseInsitu(Ch* str) {
        return ParseInsitu<parseFlags, Encoding>(str);
    }

    //! Parse JSON text from a mutable string (with \ref kParseDefaultFlags)
    /*! \param str Mutable zero-terminated string to be parsed.
        \return The document itself for fluent API.
    */
    GenericDocument& ParseInsitu(Ch* str) {
        return ParseInsitu<kParseDefaultFlags, Encoding>(str);
    }
    //!@}

    //!@name Parse from read-only string
    //!@{

    //! Parse JSON text from a read-only string (with Encoding conversion)
    /*! \tparam parseFlags Combination of \ref ParseFlag (must not contain \ref kParseInsituFlag).
        \tparam SourceEncoding Transcoding from input Encoding
        \param str Read-only zero-terminated string to be parsed.
    */
    template <unsigned parseFlags, typename SourceEncoding>
    GenericDocument& Parse(const Ch* str) {
        RAPIDJSON_ASSERT(!(parseFlags & kParseInsituFlag));
        GenericStringStream<SourceEncoding> s(str);
        return ParseStream<parseFlags, SourceEncoding>(s);
    }

    //! Parse JSON text from a read-only string
    /*! \tparam parseFlags Combination of \ref ParseFlag (must not contain \ref kParseInsituFlag).
        \param str Read-only zero-terminated string to be parsed.
    */
    template <unsigned parseFlags>
    GenericDocument& Parse(const Ch* str) {
        return Parse<parseFlags, Encoding>(str);
    }

    //! Parse JSON text from a read-only string (with \ref kParseDefaultFlags)
    /*! \param str Read-only zero-terminated string to be parsed.
    */
    GenericDocument& Parse(const Ch* str) {
        return Parse<kParseDefaultFlags>(str);
    }
    //!@}

    //!@name Handling parse errors
    //!@{

    //! Whether a parse error has occured in the last parsing.
    bool HasParseError() const { return parseResult_.IsError(); }

    //! Get the \ref ParseErrorCode of last parsing.
    ParseErrorCode GetParseError() const { return parseResult_.Code(); }

    //! Get the position of last parsing error in input, 0 otherwise.
    size_t GetErrorOffset() const { return parseResult_.Offset(); }

    //!@}

    //! Get the allocator of this document.
    Allocator& GetAllocator() { return *allocator_; }

    //! Get the capacity of stack in bytes.
    size_t GetStackCapacity() const { return stack_.GetCapacity(); }

private:
    // clear stack on any exit from ParseStream, e.g. due to exception
    struct ClearStackOnExit {
        explicit ClearStackOnExit(GenericDocument& d) : d_(d) {}
        ~ClearStackOnExit() { d_.ClearStack(); }
    private:
        ClearStackOnExit(const ClearStackOnExit&);
        ClearStackOnExit& operator=(const ClearStackOnExit&);
        GenericDocument& d_;
    };

    // callers of the following private Handler functions
    template <typename,typename,typename> friend class GenericReader; // for parsing
    template <typename, typename> friend class GenericValue; // for deep copying

    // Implementation of Handler
    bool Null() { new (stack_.template Push<ValueType>()) ValueType(); return true; }
    bool Bool(bool b) { new (stack_.template Push<ValueType>()) ValueType(b); return true; }
    bool Int(int i) { new (stack_.template Push<ValueType>()) ValueType(i); return true; }
    bool Uint(unsigned i) { new (stack_.template Push<ValueType>()) ValueType(i); return true; }
    bool Int64(int64_t i) { new (stack_.template Push<ValueType>()) ValueType(i); return true; }
    bool Uint64(uint64_t i) { new (stack_.template Push<ValueType>()) ValueType(i); return true; }
    bool Double(double d) { new (stack_.template Push<ValueType>()) ValueType(d); return true; }

    bool String(const Ch* str, SizeType length, bool copy) { 
        if (copy) 
            new (stack_.template Push<ValueType>()) ValueType(str, length, GetAllocator());
        else
            new (stack_.template Push<ValueType>()) ValueType(str, length);
        return true;
    }

    bool StartObject() { new (stack_.template Push<ValueType>()) ValueType(kObjectType); return true; }
    
    bool Key(const Ch* str, SizeType length, bool copy) { return String(str, length, copy); }

    bool EndObject(SizeType memberCount) {
        typename ValueType::Member* members = stack_.template Pop<typename ValueType::Member>(memberCount);
        stack_.template Top<ValueType>()->SetObjectRaw(members, (SizeType)memberCount, GetAllocator());
        return true;
    }

    bool StartArray() { new (stack_.template Push<ValueType>()) ValueType(kArrayType); return true; }
    
    bool EndArray(SizeType elementCount) {
        ValueType* elements = stack_.template Pop<ValueType>(elementCount);
        stack_.template Top<ValueType>()->SetArrayRaw(elements, elementCount, GetAllocator());
        return true;
    }

private:
    //! Prohibit assignment
    GenericDocument& operator=(const GenericDocument&);

    void ClearStack() {
        if (Allocator::kNeedFree)
            while (stack_.GetSize() > 0)    // Here assumes all elements in stack array are GenericValue (Member is actually 2 GenericValue objects)
                (stack_.template Pop<ValueType>(1))->~ValueType();
        else
            stack_.Clear();
        stack_.ShrinkToFit();
    }

    void Destroy() {
        delete ownAllocator_;
    }

    static const size_t kDefaultStackCapacity = 1024;
    Allocator* allocator_;
    Allocator* ownAllocator_;
    internal::Stack<StackAllocator> stack_;
    ParseResult parseResult_;
};

//! GenericDocument with UTF8 encoding
typedef GenericDocument<UTF8<> > Document;

// defined here due to the dependency on GenericDocument
template <typename Encoding, typename Allocator>
template <typename SourceAllocator>
inline
GenericValue<Encoding,Allocator>::GenericValue(const GenericValue<Encoding,SourceAllocator>& rhs, Allocator& allocator)
{
    GenericDocument<Encoding,Allocator> d(&allocator);
    rhs.Accept(d);
    RawAssign(*d.stack_.template Pop<GenericValue>(1));
}

} // namespace rapidjson

#if defined(_MSC_VER) || defined(__GNUC__)
RAPIDJSON_DIAG_POP
#endif

#endif // RAPIDJSON_DOCUMENT_H_
