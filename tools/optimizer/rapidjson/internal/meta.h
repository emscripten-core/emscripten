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

#ifndef RAPIDJSON_INTERNAL_META_H_
#define RAPIDJSON_INTERNAL_META_H_

#ifndef RAPIDJSON_RAPIDJSON_H_
#error <rapidjson.h> not yet included. Do not include this file directly.
#endif

#ifdef __GNUC__
RAPIDJSON_DIAG_PUSH
RAPIDJSON_DIAG_OFF(effc++)
#endif
#if defined(_MSC_VER)
RAPIDJSON_DIAG_PUSH
RAPIDJSON_DIAG_OFF(6334)
#endif

#if RAPIDJSON_HAS_CXX11_TYPETRAITS
#include <type_traits>
#endif

//@cond RAPIDJSON_INTERNAL
namespace rapidjson {
namespace internal {

// Helper to wrap/convert arbitrary types to void, useful for arbitrary type matching
template <typename T> struct Void { typedef void Type; };

///////////////////////////////////////////////////////////////////////////////
// BoolType, TrueType, FalseType
//
template <bool Cond> struct BoolType {
    static const bool Value = Cond;
    typedef BoolType Type;
};
typedef BoolType<true> TrueType;
typedef BoolType<false> FalseType;


///////////////////////////////////////////////////////////////////////////////
// SelectIf, BoolExpr, NotExpr, AndExpr, OrExpr
//

template <bool C> struct SelectIfImpl { template <typename T1, typename T2> struct Apply { typedef T1 Type; }; };
template <> struct SelectIfImpl<false> { template <typename T1, typename T2> struct Apply { typedef T2 Type; }; };
template <bool C, typename T1, typename T2> struct SelectIfCond : SelectIfImpl<C>::template Apply<T1,T2> {};
template <typename C, typename T1, typename T2> struct SelectIf : SelectIfCond<C::Value, T1, T2> {};

template <bool Cond1, bool Cond2> struct AndExprCond : FalseType {};
template <> struct AndExprCond<true, true> : TrueType {};
template <bool Cond1, bool Cond2> struct OrExprCond : TrueType {};
template <> struct OrExprCond<false, false> : FalseType {};

template <typename C> struct BoolExpr : SelectIf<C,TrueType,FalseType>::Type {};
template <typename C> struct NotExpr  : SelectIf<C,FalseType,TrueType>::Type {};
template <typename C1, typename C2> struct AndExpr : AndExprCond<C1::Value, C2::Value>::Type {};
template <typename C1, typename C2> struct OrExpr  : OrExprCond<C1::Value, C2::Value>::Type {};


///////////////////////////////////////////////////////////////////////////////
// AddConst, MaybeAddConst, RemoveConst
template <typename T> struct AddConst { typedef const T Type; };
template <bool Constify, typename T> struct MaybeAddConst : SelectIfCond<Constify, const T, T> {};
template <typename T> struct RemoveConst { typedef T Type; };
template <typename T> struct RemoveConst<const T> { typedef T Type; };


///////////////////////////////////////////////////////////////////////////////
// IsSame, IsConst, IsMoreConst, IsPointer
//
template <typename T, typename U> struct IsSame : FalseType {};
template <typename T> struct IsSame<T, T> : TrueType {};

template <typename T> struct IsConst : FalseType {};
template <typename T> struct IsConst<const T> : TrueType {};

template <typename CT, typename T>
struct IsMoreConst
    : AndExpr<IsSame<typename RemoveConst<CT>::Type, typename RemoveConst<T>::Type>,
              BoolType<IsConst<CT>::Value >= IsConst<T>::Value> >::Type {};

template <typename T> struct IsPointer : FalseType {};
template <typename T> struct IsPointer<T*> : TrueType {};

///////////////////////////////////////////////////////////////////////////////
// IsBaseOf
//
#if RAPIDJSON_HAS_CXX11_TYPETRAITS

template <typename B, typename D> struct IsBaseOf
    : BoolType< ::std::is_base_of<B,D>::value> {};

#else // simplified version adopted from Boost

template<typename B, typename D> struct IsBaseOfImpl {
    RAPIDJSON_STATIC_ASSERT(sizeof(B) != 0);
    RAPIDJSON_STATIC_ASSERT(sizeof(D) != 0);

    typedef char (&Yes)[1];
    typedef char (&No) [2];

    template <typename T>
    static Yes Check(const D*, T);
    static No  Check(const B*, int);

    struct Host {
        operator const B*() const;
        operator const D*();
    };

    enum { Value = (sizeof(Check(Host(), 0)) == sizeof(Yes)) };
};

template <typename B, typename D> struct IsBaseOf
    : OrExpr<IsSame<B, D>, BoolExpr<IsBaseOfImpl<B, D> > >::Type {};

#endif // RAPIDJSON_HAS_CXX11_TYPETRAITS


//////////////////////////////////////////////////////////////////////////
// EnableIf / DisableIf
//
template <bool Condition, typename T = void> struct EnableIfCond  { typedef T Type; };
template <typename T> struct EnableIfCond<false, T> { /* empty */ };

template <bool Condition, typename T = void> struct DisableIfCond { typedef T Type; };
template <typename T> struct DisableIfCond<true, T> { /* empty */ };

template <typename Condition, typename T = void>
struct EnableIf : EnableIfCond<Condition::Value, T> {};

template <typename Condition, typename T = void>
struct DisableIf : DisableIfCond<Condition::Value, T> {};

// SFINAE helpers
struct SfinaeTag {};
template <typename T> struct RemoveSfinaeTag;
template <typename T> struct RemoveSfinaeTag<SfinaeTag&(*)(T)> { typedef T Type; };

#define RAPIDJSON_REMOVEFPTR_(type) \
    typename ::rapidjson::internal::RemoveSfinaeTag \
        < ::rapidjson::internal::SfinaeTag&(*) type>::Type

#define RAPIDJSON_ENABLEIF(cond) \
    typename ::rapidjson::internal::EnableIf \
        <RAPIDJSON_REMOVEFPTR_(cond)>::Type * = NULL

#define RAPIDJSON_DISABLEIF(cond) \
    typename ::rapidjson::internal::DisableIf \
        <RAPIDJSON_REMOVEFPTR_(cond)>::Type * = NULL

#define RAPIDJSON_ENABLEIF_RETURN(cond,returntype) \
    typename ::rapidjson::internal::EnableIf \
        <RAPIDJSON_REMOVEFPTR_(cond), \
         RAPIDJSON_REMOVEFPTR_(returntype)>::Type

#define RAPIDJSON_DISABLEIF_RETURN(cond,returntype) \
    typename ::rapidjson::internal::DisableIf \
        <RAPIDJSON_REMOVEFPTR_(cond), \
         RAPIDJSON_REMOVEFPTR_(returntype)>::Type

} // namespace internal
} // namespace rapidjson
//@endcond

#if defined(__GNUC__) || defined(_MSC_VER)
RAPIDJSON_DIAG_POP
#endif

#endif // RAPIDJSON_INTERNAL_META_H_
