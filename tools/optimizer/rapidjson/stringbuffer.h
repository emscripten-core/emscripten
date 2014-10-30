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

#ifndef RAPIDJSON_STRINGBUFFER_H_
#define RAPIDJSON_STRINGBUFFER_H_

#include "rapidjson.h"
#include "internal/stack.h"

namespace rapidjson {

//! Represents an in-memory output stream.
/*!
    \tparam Encoding Encoding of the stream.
    \tparam Allocator type for allocating memory buffer.
    \note implements Stream concept
*/
template <typename Encoding, typename Allocator = CrtAllocator>
struct GenericStringBuffer {
    typedef typename Encoding::Ch Ch;

    GenericStringBuffer(Allocator* allocator = 0, size_t capacity = kDefaultCapacity) : stack_(allocator, capacity) {}

    void Put(Ch c) { *stack_.template Push<Ch>() = c; }
    void Flush() {}

    void Clear() { stack_.Clear(); }
    void ShrinkToFit() {
        // Push and pop a null terminator. This is safe.
        *stack_.template Push<Ch>() = '\0';
        stack_.ShrinkToFit();
        stack_.template Pop<Ch>(1);
    }
    Ch* Push(size_t count) { return stack_.template Push<Ch>(count); }
    void Pop(size_t count) { stack_.template Pop<Ch>(count); }

    const Ch* GetString() const {
        // Push and pop a null terminator. This is safe.
        *stack_.template Push<Ch>() = '\0';
        stack_.template Pop<Ch>(1);

        return stack_.template Bottom<Ch>();
    }

    size_t GetSize() const { return stack_.GetSize(); }

    static const size_t kDefaultCapacity = 256;
    mutable internal::Stack<Allocator> stack_;
};

//! String buffer with UTF8 encoding
typedef GenericStringBuffer<UTF8<> > StringBuffer;

//! Implement specialized version of PutN() with memset() for better performance.
template<>
inline void PutN(GenericStringBuffer<UTF8<> >& stream, char c, size_t n) {
    std::memset(stream.stack_.Push<char>(n), c, n * sizeof(c));
}

} // namespace rapidjson

#endif // RAPIDJSON_STRINGBUFFER_H_
