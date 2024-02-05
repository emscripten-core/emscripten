// Copyright 2012 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <string>
#include <iostream>

int main() {
	std::string output("If you see this - the world is all right!");
	
	auto func = [=](std::string text) -> int {
		std::cout << text << std::endl;
		return 5;
	};
	
	return func(output);
}
