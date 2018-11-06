// Copyright 2014 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <stdexcept>
#include <iostream>

namespace
{
  // An exception that can be derived virtually or not based on macro
  // definition.
  class test_exception
    : virtual public std::runtime_error
  {
  public:
    test_exception()
      : std::runtime_error("test_exception")
    {}
  };

  // Test class that logs its construction and destruction.
  class TestClass
  {
  public:
    TestClass()
    { std::cout << "TestClass::Construction"  << std::endl; }

    ~TestClass()
    { std::cout << "TestClass::Destruction" << std::endl; }
  };
}

int main()
{
  try {
    TestClass testClass;
    throw test_exception();
  }
  catch (const std::exception& ex) {
    std::cout << "Caught exception: " << ex.what() << std::endl;
  }

  // Something goes very wrong between handling the exception and resuming
  // normal execution when the exception is virtually derived.
  return 0;
}

