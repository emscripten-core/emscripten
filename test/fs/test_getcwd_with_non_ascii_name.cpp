// Copyright 2017 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <cassert>
#include <fstream>
#include <sys/stat.h>
#include <unistd.h>

std::string makeLongStr(const std::string& str, int strCount)
{
    std::string result;

    for (auto i = 0; i < strCount; ++i)
    {
        result += str;
    }

    return result;
}

void doPositiveTest(const std::string& dirName)
{
    printf("positive test case: %s\n", dirName.c_str());

    const auto fullDirName = "/" + dirName;
    auto ret = mkdir(fullDirName.c_str(), 0777);
    assert(ret == 0);
    ret = chdir(fullDirName.c_str());
    assert(ret == 0);

    char buf[4096];
    auto ret2 = getcwd(buf, sizeof(buf));
    assert(ret2 != 0);
    printf("ret2 = %s\n", ret2);
}

void doNegativeTest(const std::string& dirName)
{
    printf("negative test case: %s\n", dirName.c_str());

    auto ret = mkdir(dirName.c_str(), 0777);
    assert(ret == 0);
    ret = chdir(dirName.c_str());
    assert(ret == 0);

    unsigned char buf[4096];
    const unsigned char Filler = 0xFE;
    memset(buf, Filler, sizeof(buf));

    const auto allowedBufferSize = dirName.length() / 2;
    auto ret2 = getcwd(reinterpret_cast<char*>(buf), allowedBufferSize);
    assert(ret2 == 0);
    assert(errno == ERANGE);

    for (auto i = allowedBufferSize; i < sizeof(buf); ++i)
    {
        assert(buf[i] == Filler);
    }
}

int main()
{
    // Short non-ascii name
    doPositiveTest(u8"абвгд");
    
    // Long non-ascii name
    const auto longStr = "abcde" + makeLongStr(u8"абвгд", 25);
    assert(longStr.length() == 255);
    doPositiveTest(longStr);

    // The negative test passes a half-sized buffer to getcwd and makes sure that it fails.
    doNegativeTest(makeLongStr(u8"abcde", 10));
}
