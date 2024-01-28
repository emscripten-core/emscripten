// Copyright 2017 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef WEBGPU_ENUM_CLASS_BITMASKS_H_
#define WEBGPU_ENUM_CLASS_BITMASKS_H_

#include <type_traits>

// The operators in wgpu:: namespace need be introduced into other namespaces with
// using-declarations for C++ Argument Dependent Lookup to work.
#define WGPU_IMPORT_BITMASK_OPERATORS \
    using wgpu::operator|;            \
    using wgpu::operator&;            \
    using wgpu::operator^;            \
    using wgpu::operator~;            \
    using wgpu::operator&=;           \
    using wgpu::operator|=;           \
    using wgpu::operator^=;           \
    using wgpu::HasZeroOrOneBits;

namespace wgpu {

template <typename T>
struct IsWGPUBitmask {
    static constexpr bool enable = false;
};

template <typename T, typename Enable = void>
struct LowerBitmask {
    static constexpr bool enable = false;
};

template <typename T>
struct LowerBitmask<T, typename std::enable_if<IsWGPUBitmask<T>::enable>::type> {
    static constexpr bool enable = true;
    using type = T;
    constexpr static T Lower(T t) { return t; }
};

template <typename T>
struct BoolConvertible {
    using Integral = typename std::underlying_type<T>::type;

    // NOLINTNEXTLINE(runtime/explicit)
    explicit constexpr BoolConvertible(Integral value) : value(value) {}
    constexpr operator bool() const { return value != 0; }
    constexpr operator T() const { return static_cast<T>(value); }

    Integral value;
};

template <typename T>
struct LowerBitmask<BoolConvertible<T>> {
    static constexpr bool enable = true;
    using type = T;
    static constexpr type Lower(BoolConvertible<T> t) { return t; }
};

template <
    typename T1,
    typename T2,
    typename = typename std::enable_if<LowerBitmask<T1>::enable && LowerBitmask<T2>::enable>::type>
constexpr BoolConvertible<typename LowerBitmask<T1>::type> operator|(T1 left, T2 right) {
    using T = typename LowerBitmask<T1>::type;
    using Integral = typename std::underlying_type<T>::type;
    return BoolConvertible<T>(static_cast<Integral>(LowerBitmask<T1>::Lower(left)) |
                              static_cast<Integral>(LowerBitmask<T2>::Lower(right)));
}

template <
    typename T1,
    typename T2,
    typename = typename std::enable_if<LowerBitmask<T1>::enable && LowerBitmask<T2>::enable>::type>
constexpr BoolConvertible<typename LowerBitmask<T1>::type> operator&(T1 left, T2 right) {
    using T = typename LowerBitmask<T1>::type;
    using Integral = typename std::underlying_type<T>::type;
    return BoolConvertible<T>(static_cast<Integral>(LowerBitmask<T1>::Lower(left)) &
                              static_cast<Integral>(LowerBitmask<T2>::Lower(right)));
}

template <
    typename T1,
    typename T2,
    typename = typename std::enable_if<LowerBitmask<T1>::enable && LowerBitmask<T2>::enable>::type>
constexpr BoolConvertible<typename LowerBitmask<T1>::type> operator^(T1 left, T2 right) {
    using T = typename LowerBitmask<T1>::type;
    using Integral = typename std::underlying_type<T>::type;
    return BoolConvertible<T>(static_cast<Integral>(LowerBitmask<T1>::Lower(left)) ^
                              static_cast<Integral>(LowerBitmask<T2>::Lower(right)));
}

template <typename T1>
constexpr BoolConvertible<typename LowerBitmask<T1>::type> operator~(T1 t) {
    using T = typename LowerBitmask<T1>::type;
    using Integral = typename std::underlying_type<T>::type;
    return BoolConvertible<T>(~static_cast<Integral>(LowerBitmask<T1>::Lower(t)));
}

template <
    typename T,
    typename T2,
    typename = typename std::enable_if<IsWGPUBitmask<T>::enable && LowerBitmask<T2>::enable>::type>
constexpr T& operator&=(T& l, T2 right) {
    T r = LowerBitmask<T2>::Lower(right);
    l = l & r;
    return l;
}

template <
    typename T,
    typename T2,
    typename = typename std::enable_if<IsWGPUBitmask<T>::enable && LowerBitmask<T2>::enable>::type>
constexpr T& operator|=(T& l, T2 right) {
    T r = LowerBitmask<T2>::Lower(right);
    l = l | r;
    return l;
}

template <
    typename T,
    typename T2,
    typename = typename std::enable_if<IsWGPUBitmask<T>::enable && LowerBitmask<T2>::enable>::type>
constexpr T& operator^=(T& l, T2 right) {
    T r = LowerBitmask<T2>::Lower(right);
    l = l ^ r;
    return l;
}

template <typename T>
constexpr bool HasZeroOrOneBits(T value) {
    using Integral = typename std::underlying_type<T>::type;
    return (static_cast<Integral>(value) & (static_cast<Integral>(value) - 1)) == 0;
}

}  // namespace wgpu

#endif  // WEBGPU_ENUM_CLASS_BITMASKS_H_
