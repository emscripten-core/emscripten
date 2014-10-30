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

#ifndef RAPIDJSON_FILEWRITESTREAM_H_
#define RAPIDJSON_FILEWRITESTREAM_H_

#include "rapidjson.h"
#include <cstdio>

namespace rapidjson {

//! Wrapper of C file stream for input using fread().
/*!
    \note implements Stream concept
*/
class FileWriteStream {
public:
    typedef char Ch;    //!< Character type. Only support char.

    FileWriteStream(FILE* fp, char* buffer, size_t bufferSize) : fp_(fp), buffer_(buffer), bufferEnd_(buffer + bufferSize), current_(buffer_) { 
        RAPIDJSON_ASSERT(fp_ != 0);
    }

    void Put(char c) { 
        if (current_ >= bufferEnd_)
            Flush();

        *current_++ = c;
    }

    void PutN(char c, size_t n) {
        size_t avail = static_cast<size_t>(bufferEnd_ - current_);
        while (n > avail) {
            std::memset(current_, c, avail);
            current_ += avail;
            Flush();
            n -= avail;
            avail = static_cast<size_t>(bufferEnd_ - current_);
        }

        if (n > 0) {
            std::memset(current_, c, n);
            current_ += n;
        }
    }

    void Flush() {
        if (current_ != buffer_) {
            fwrite(buffer_, 1, static_cast<size_t>(current_ - buffer_), fp_);
            current_ = buffer_;
        }
    }

    // Not implemented
    char Peek() const { RAPIDJSON_ASSERT(false); return 0; }
    char Take() { RAPIDJSON_ASSERT(false); return 0; }
    size_t Tell() const { RAPIDJSON_ASSERT(false); return 0; }
    char* PutBegin() { RAPIDJSON_ASSERT(false); return 0; }
    size_t PutEnd(char*) { RAPIDJSON_ASSERT(false); return 0; }

private:
    // Prohibit copy constructor & assignment operator.
    FileWriteStream(const FileWriteStream&);
    FileWriteStream& operator=(const FileWriteStream&);

    FILE* fp_;
    char *buffer_;
    char *bufferEnd_;
    char *current_;
};

//! Implement specialized version of PutN() with memset() for better performance.
template<>
inline void PutN(FileWriteStream& stream, char c, size_t n) {
    stream.PutN(c, n);
}

} // namespace rapidjson

#endif // RAPIDJSON_FILESTREAM_H_
