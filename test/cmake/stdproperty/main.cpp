// Copyright 2016 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <iostream>

struct vector2d{
	int x,y,z;
};

int main(int argc, char * argv[])
{
	vector2d v{1,2,3};
	std::cout << v.x << "," << v.y << "," << v.z << std::endl;
}
