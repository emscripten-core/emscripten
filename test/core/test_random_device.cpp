// Copyright 2014 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <random>
#include <iostream>
#include <exception>

auto main()
  -> int
try
{
  std::random_device rd;
  std::cout << rd() << ", a random was read\n";
}
catch( const std::exception& e )
{
  std::cerr << e.what();
}

