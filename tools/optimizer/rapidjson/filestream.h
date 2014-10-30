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

#ifndef RAPIDJSON_FILESTREAM_H_
#define RAPIDJSON_FILESTREAM_H_

#include "rapidjson.h"
#include <cstdio>

namespace rapidjson {

//! (Deprecated) Wrapper of C file stream for input or output.
/*!
    This simple wrapper does not check the validity of the stream.
    \note implements Stream concept
    \note deprecated: This was only for basic testing in version 0.1, it is found that the performance is very low by using fgetc(). Use FileReadStream instead.
*/
class FileStream {
public:
    typedef char Ch;    //!< Character type. Only support char.

    FileStream(FILE* fp) : fp_(fp), current_('\0'), count_(0) { Read(); }
    char Peek() const { return current_; }
    char Take() { char c = current_; Read(); return c; }
    size_t Tell() const { return count_; }
    void Put(char c) { fputc(c, fp_); }
    void Flush() { fflush(fp_); }

    // Not implemented
    char* PutBegin() { return 0; }
    size_t PutEnd(char*) { return 0; }

private:
    // Prohibit copy constructor & assignment operator.
    FileStream(const FileStream&);
    FileStream& operator=(const FileStream&);

    void Read() {
        RAPIDJSON_ASSERT(fp_ != 0);
        int c = fgetc(fp_);
        if (c != EOF) {
            current_ = (char)c;
            count_++;
        }
        else if (current_ != '\0')
            current_ = '\0';
    }

    FILE* fp_;
    char current_;
    size_t count_;
};

} // namespace rapidjson

#endif // RAPIDJSON_FILESTREAM_H_
