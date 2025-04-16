#ifndef BT_TYPE_DEFINITIONS_H
#define BT_TYPE_DEFINITIONS_H

///This file provides some platform/compiler checks for common definitions
#include "LinearMath/btScalar.h"
#include "LinearMath/btMinMax.h"

#ifdef PFX_USE_FREE_VECTORMATH
#include "physics_effects/base_level/base/pfx_vectormath_include.win32.h"
typedef Vectormath::Aos::Vector3    vmVector3;
typedef Vectormath::Aos::Quat       vmQuat;
typedef Vectormath::Aos::Matrix3    vmMatrix3;
typedef Vectormath::Aos::Transform3 vmTransform3;
typedef Vectormath::Aos::Point3     vmPoint3;
#else
#include "vectormath/vmInclude.h"
#endif//PFX_USE_FREE_VECTORMATH





#ifdef _WIN32

typedef union
{
  unsigned int u;
  void *p;
} addr64;

#define USE_WIN32_THREADING 1

		#if defined(__MINGW32__) || defined(__CYGWIN__) || (defined (_MSC_VER) && _MSC_VER < 1300)
		#else
		#endif //__MINGW32__

		typedef unsigned char     uint8_t;
#ifndef __PHYSICS_COMMON_H__
#ifndef PFX_USE_FREE_VECTORMATH
#ifndef __BT_SKIP_UINT64_H
		typedef unsigned long int uint64_t;
#endif //__BT_SKIP_UINT64_H
#endif //PFX_USE_FREE_VECTORMATH
		typedef unsigned int      uint32_t;
#endif //__PHYSICS_COMMON_H__
		typedef unsigned short    uint16_t;

		#include <malloc.h>
		#define memalign(alignment, size) malloc(size);
			
#include <string.h> //memcpy

		

		#include <stdio.h>		
		#define spu_printf printf
		
#else
		#include <stdint.h>
		#include <stdlib.h>
		#include <string.h> //for memcpy

#if defined	(__CELLOS_LV2__)
	// Playstation 3 Cell SDK
#include <spu_printf.h>
		
#else
	// posix system

#define USE_PTHREADS    (1)

#ifdef USE_LIBSPE2
#include <stdio.h>		
#define spu_printf printf	
#define DWORD unsigned int
			typedef union
			{
			  unsigned long long ull;
			  unsigned int ui[2];
			  void *p;
			} addr64;
#endif // USE_LIBSPE2

#endif	//__CELLOS_LV2__
	
#endif

#ifdef __SPU__
#include <stdio.h>		
#define printf spu_printf
#endif

/* Included here because we need uint*_t typedefs */
#include "PpuAddressSpace.h"

#endif //BT_TYPE_DEFINITIONS_H



