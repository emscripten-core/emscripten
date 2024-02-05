// Copyright 2018 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <iostream>

struct base_1
{
    std::string v;
};

struct base_2
{
    std::string v;
};

struct derived : base_1, base_2
{
    derived()
    {
        base_1::v = "a";
        base_2::v = "b";
    }
};

int main()
{
    try
    {
        throw derived();
    }
    catch (const base_2&)
    {
        try
        {
            std::rethrow_exception(std::current_exception());
        }
        catch (const base_1& ex)
        {
            std::cout << ex.v << std::endl;
        }
    }

    return 0;
}
