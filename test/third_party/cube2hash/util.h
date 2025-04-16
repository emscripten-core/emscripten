#ifndef _UTIL_H
#define _UTIL_H

#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>

#define BIGENDIAN 0

#ifndef bool
#define bool unsigned short int
#define true 1
#define false 0
#endif

typedef unsigned long long int chunk;
typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;

#endif
