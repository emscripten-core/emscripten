/*
Bullet Continuous Collision Detection and Physics Library
Copyright (c) 2003-2010 Erwin Coumans  http://bulletphysics.org

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose, 
including commercial applications, and to alter it and redistribute it freely, 
subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/


#ifndef BT_PPU_ADDRESS_SPACE_H
#define BT_PPU_ADDRESS_SPACE_H


#ifdef _WIN32
//stop those casting warnings until we have a better solution for ppu_address_t / void* / uint64 conversions
#pragma warning (disable: 4311)
#pragma warning (disable: 4312)
#endif //_WIN32


#if defined(_WIN64)
	typedef unsigned __int64 ppu_address_t;
#elif defined(__LP64__) || defined(__x86_64__)
	typedef uint64_t ppu_address_t;
#else
	typedef uint32_t ppu_address_t;
#endif //defined(_WIN64)

#endif //BT_PPU_ADDRESS_SPACE_H

