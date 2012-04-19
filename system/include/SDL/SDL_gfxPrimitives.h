/* 

SDL_gfxPrimitives.h: graphics primitives for SDL

Copyright (C) 2001-2011  Andreas Schiffler

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

   1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would be
   appreciated but is not required.

   2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.

   3. This notice may not be removed or altered from any source
   distribution.

Andreas Schiffler -- aschiffler at ferzkopp dot net

*/

#ifndef _SDL_gfxPrimitives_h
#define _SDL_gfxPrimitives_h

#include <math.h>
#ifndef M_PI
#define M_PI	3.1415926535897932384626433832795
#endif

#include "SDL.h"

/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
extern "C" {
#endif

	/* ----- Versioning */

#define SDL_GFXPRIMITIVES_MAJOR	2
#define SDL_GFXPRIMITIVES_MINOR	0
#define SDL_GFXPRIMITIVES_MICRO	23


	/* ---- Function Prototypes */

#ifdef _MSC_VER
#  if defined(DLL_EXPORT) && !defined(LIBSDL_GFX_DLL_IMPORT)
#    define SDL_GFXPRIMITIVES_SCOPE __declspec(dllexport)
#  else
#    ifdef LIBSDL_GFX_DLL_IMPORT
#      define SDL_GFXPRIMITIVES_SCOPE __declspec(dllimport)
#    endif
#  endif
#endif
#ifndef SDL_GFXPRIMITIVES_SCOPE
#  define SDL_GFXPRIMITIVES_SCOPE extern
#endif

	/* Note: all ___Color routines expect the color to be in format 0xRRGGBBAA */

	/* Pixel */

	SDL_GFXPRIMITIVES_SCOPE int pixelColor(SDL_Surface * dst, Sint16 x, Sint16 y, Uint32 color);
	SDL_GFXPRIMITIVES_SCOPE int pixelRGBA(SDL_Surface * dst, Sint16 x, Sint16 y, Uint8 r, Uint8 g, Uint8 b, Uint8 a);

	/* Horizontal line */

	SDL_GFXPRIMITIVES_SCOPE int hlineColor(SDL_Surface * dst, Sint16 x1, Sint16 x2, Sint16 y, Uint32 color);
	SDL_GFXPRIMITIVES_SCOPE int hlineRGBA(SDL_Surface * dst, Sint16 x1, Sint16 x2, Sint16 y, Uint8 r, Uint8 g, Uint8 b, Uint8 a);

	/* Vertical line */

	SDL_GFXPRIMITIVES_SCOPE int vlineColor(SDL_Surface * dst, Sint16 x, Sint16 y1, Sint16 y2, Uint32 color);
	SDL_GFXPRIMITIVES_SCOPE int vlineRGBA(SDL_Surface * dst, Sint16 x, Sint16 y1, Sint16 y2, Uint8 r, Uint8 g, Uint8 b, Uint8 a);

	/* Rectangle */

	SDL_GFXPRIMITIVES_SCOPE int rectangleColor(SDL_Surface * dst, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Uint32 color);
	SDL_GFXPRIMITIVES_SCOPE int rectangleRGBA(SDL_Surface * dst, Sint16 x1, Sint16 y1,
		Sint16 x2, Sint16 y2, Uint8 r, Uint8 g, Uint8 b, Uint8 a);

	/* Rounded-Corner Rectangle */

	SDL_GFXPRIMITIVES_SCOPE int roundedRectangleColor(SDL_Surface * dst, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Sint16 rad, Uint32 color);
	SDL_GFXPRIMITIVES_SCOPE int roundedRectangleRGBA(SDL_Surface * dst, Sint16 x1, Sint16 y1,
		Sint16 x2, Sint16 y2, Sint16 rad, Uint8 r, Uint8 g, Uint8 b, Uint8 a);

	/* Filled rectangle (Box) */

	SDL_GFXPRIMITIVES_SCOPE int boxColor(SDL_Surface * dst, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Uint32 color);
	SDL_GFXPRIMITIVES_SCOPE int boxRGBA(SDL_Surface * dst, Sint16 x1, Sint16 y1, Sint16 x2,
		Sint16 y2, Uint8 r, Uint8 g, Uint8 b, Uint8 a);

	/* Rounded-Corner Filled rectangle (Box) */

	SDL_GFXPRIMITIVES_SCOPE int roundedBoxColor(SDL_Surface * dst, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Sint16 rad, Uint32 color);
	SDL_GFXPRIMITIVES_SCOPE int roundedBoxRGBA(SDL_Surface * dst, Sint16 x1, Sint16 y1, Sint16 x2,
		Sint16 y2, Sint16 rad, Uint8 r, Uint8 g, Uint8 b, Uint8 a);

	/* Line */

	SDL_GFXPRIMITIVES_SCOPE int lineColor(SDL_Surface * dst, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Uint32 color);
	SDL_GFXPRIMITIVES_SCOPE int lineRGBA(SDL_Surface * dst, Sint16 x1, Sint16 y1,
		Sint16 x2, Sint16 y2, Uint8 r, Uint8 g, Uint8 b, Uint8 a);

	/* AA Line */

	SDL_GFXPRIMITIVES_SCOPE int aalineColor(SDL_Surface * dst, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Uint32 color);
	SDL_GFXPRIMITIVES_SCOPE int aalineRGBA(SDL_Surface * dst, Sint16 x1, Sint16 y1,
		Sint16 x2, Sint16 y2, Uint8 r, Uint8 g, Uint8 b, Uint8 a);

	/* Thick Line */
	SDL_GFXPRIMITIVES_SCOPE int thickLineColor(SDL_Surface * dst, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, 
		Uint8 width, Uint32 color);
	SDL_GFXPRIMITIVES_SCOPE int thickLineRGBA(SDL_Surface * dst, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, 
		Uint8 width, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
	
	/* Circle */

	SDL_GFXPRIMITIVES_SCOPE int circleColor(SDL_Surface * dst, Sint16 x, Sint16 y, Sint16 rad, Uint32 color);
	SDL_GFXPRIMITIVES_SCOPE int circleRGBA(SDL_Surface * dst, Sint16 x, Sint16 y, Sint16 rad, Uint8 r, Uint8 g, Uint8 b, Uint8 a);

	/* Arc */

	SDL_GFXPRIMITIVES_SCOPE int arcColor(SDL_Surface * dst, Sint16 x, Sint16 y, Sint16 rad, Sint16 start, Sint16 end, Uint32 color);
	SDL_GFXPRIMITIVES_SCOPE int arcRGBA(SDL_Surface * dst, Sint16 x, Sint16 y, Sint16 rad, Sint16 start, Sint16 end, 
		Uint8 r, Uint8 g, Uint8 b, Uint8 a);

	/* AA Circle */

	SDL_GFXPRIMITIVES_SCOPE int aacircleColor(SDL_Surface * dst, Sint16 x, Sint16 y, Sint16 rad, Uint32 color);
	SDL_GFXPRIMITIVES_SCOPE int aacircleRGBA(SDL_Surface * dst, Sint16 x, Sint16 y,
		Sint16 rad, Uint8 r, Uint8 g, Uint8 b, Uint8 a);

	/* Filled Circle */

	SDL_GFXPRIMITIVES_SCOPE int filledCircleColor(SDL_Surface * dst, Sint16 x, Sint16 y, Sint16 r, Uint32 color);
	SDL_GFXPRIMITIVES_SCOPE int filledCircleRGBA(SDL_Surface * dst, Sint16 x, Sint16 y,
		Sint16 rad, Uint8 r, Uint8 g, Uint8 b, Uint8 a);

	/* Ellipse */

	SDL_GFXPRIMITIVES_SCOPE int ellipseColor(SDL_Surface * dst, Sint16 x, Sint16 y, Sint16 rx, Sint16 ry, Uint32 color);
	SDL_GFXPRIMITIVES_SCOPE int ellipseRGBA(SDL_Surface * dst, Sint16 x, Sint16 y,
		Sint16 rx, Sint16 ry, Uint8 r, Uint8 g, Uint8 b, Uint8 a);

	/* AA Ellipse */

	SDL_GFXPRIMITIVES_SCOPE int aaellipseColor(SDL_Surface * dst, Sint16 x, Sint16 y, Sint16 rx, Sint16 ry, Uint32 color);
	SDL_GFXPRIMITIVES_SCOPE int aaellipseRGBA(SDL_Surface * dst, Sint16 x, Sint16 y,
		Sint16 rx, Sint16 ry, Uint8 r, Uint8 g, Uint8 b, Uint8 a);

	/* Filled Ellipse */

	SDL_GFXPRIMITIVES_SCOPE int filledEllipseColor(SDL_Surface * dst, Sint16 x, Sint16 y, Sint16 rx, Sint16 ry, Uint32 color);
	SDL_GFXPRIMITIVES_SCOPE int filledEllipseRGBA(SDL_Surface * dst, Sint16 x, Sint16 y,
		Sint16 rx, Sint16 ry, Uint8 r, Uint8 g, Uint8 b, Uint8 a);

	/* Pie */

	SDL_GFXPRIMITIVES_SCOPE int pieColor(SDL_Surface * dst, Sint16 x, Sint16 y, Sint16 rad,
		Sint16 start, Sint16 end, Uint32 color);
	SDL_GFXPRIMITIVES_SCOPE int pieRGBA(SDL_Surface * dst, Sint16 x, Sint16 y, Sint16 rad,
		Sint16 start, Sint16 end, Uint8 r, Uint8 g, Uint8 b, Uint8 a);

	/* Filled Pie */

	SDL_GFXPRIMITIVES_SCOPE int filledPieColor(SDL_Surface * dst, Sint16 x, Sint16 y, Sint16 rad,
		Sint16 start, Sint16 end, Uint32 color);
	SDL_GFXPRIMITIVES_SCOPE int filledPieRGBA(SDL_Surface * dst, Sint16 x, Sint16 y, Sint16 rad,
		Sint16 start, Sint16 end, Uint8 r, Uint8 g, Uint8 b, Uint8 a);

	/* Trigon */

	SDL_GFXPRIMITIVES_SCOPE int trigonColor(SDL_Surface * dst, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Sint16 x3, Sint16 y3, Uint32 color);
	SDL_GFXPRIMITIVES_SCOPE int trigonRGBA(SDL_Surface * dst, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Sint16 x3, Sint16 y3,
		Uint8 r, Uint8 g, Uint8 b, Uint8 a);

	/* AA-Trigon */

	SDL_GFXPRIMITIVES_SCOPE int aatrigonColor(SDL_Surface * dst, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Sint16 x3, Sint16 y3, Uint32 color);
	SDL_GFXPRIMITIVES_SCOPE int aatrigonRGBA(SDL_Surface * dst,  Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Sint16 x3, Sint16 y3,
		Uint8 r, Uint8 g, Uint8 b, Uint8 a);

	/* Filled Trigon */

	SDL_GFXPRIMITIVES_SCOPE int filledTrigonColor(SDL_Surface * dst, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Sint16 x3, Sint16 y3, Uint32 color);
	SDL_GFXPRIMITIVES_SCOPE int filledTrigonRGBA(SDL_Surface * dst, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Sint16 x3, Sint16 y3,
		Uint8 r, Uint8 g, Uint8 b, Uint8 a);

	/* Polygon */

	SDL_GFXPRIMITIVES_SCOPE int polygonColor(SDL_Surface * dst, const Sint16 * vx, const Sint16 * vy, int n, Uint32 color);
	SDL_GFXPRIMITIVES_SCOPE int polygonRGBA(SDL_Surface * dst, const Sint16 * vx, const Sint16 * vy,
		int n, Uint8 r, Uint8 g, Uint8 b, Uint8 a);

	/* AA-Polygon */

	SDL_GFXPRIMITIVES_SCOPE int aapolygonColor(SDL_Surface * dst, const Sint16 * vx, const Sint16 * vy, int n, Uint32 color);
	SDL_GFXPRIMITIVES_SCOPE int aapolygonRGBA(SDL_Surface * dst, const Sint16 * vx, const Sint16 * vy,
		int n, Uint8 r, Uint8 g, Uint8 b, Uint8 a);

	/* Filled Polygon */

	SDL_GFXPRIMITIVES_SCOPE int filledPolygonColor(SDL_Surface * dst, const Sint16 * vx, const Sint16 * vy, int n, Uint32 color);
	SDL_GFXPRIMITIVES_SCOPE int filledPolygonRGBA(SDL_Surface * dst, const Sint16 * vx,
		const Sint16 * vy, int n, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
	SDL_GFXPRIMITIVES_SCOPE int texturedPolygon(SDL_Surface * dst, const Sint16 * vx, const Sint16 * vy, int n, SDL_Surface * texture,int texture_dx,int texture_dy);

	/* (Note: These MT versions are required for multi-threaded operation.) */

	SDL_GFXPRIMITIVES_SCOPE int filledPolygonColorMT(SDL_Surface * dst, const Sint16 * vx, const Sint16 * vy, int n, Uint32 color, int **polyInts, int *polyAllocated);
	SDL_GFXPRIMITIVES_SCOPE int filledPolygonRGBAMT(SDL_Surface * dst, const Sint16 * vx,
		const Sint16 * vy, int n, Uint8 r, Uint8 g, Uint8 b, Uint8 a,
		int **polyInts, int *polyAllocated);
	SDL_GFXPRIMITIVES_SCOPE int texturedPolygonMT(SDL_Surface * dst, const Sint16 * vx, const Sint16 * vy, int n, SDL_Surface * texture,int texture_dx,int texture_dy, int **polyInts, int *polyAllocated);

	/* Bezier */

	SDL_GFXPRIMITIVES_SCOPE int bezierColor(SDL_Surface * dst, const Sint16 * vx, const Sint16 * vy, int n, int s, Uint32 color);
	SDL_GFXPRIMITIVES_SCOPE int bezierRGBA(SDL_Surface * dst, const Sint16 * vx, const Sint16 * vy,
		int n, int s, Uint8 r, Uint8 g, Uint8 b, Uint8 a);

	/* Characters/Strings */

	SDL_GFXPRIMITIVES_SCOPE void gfxPrimitivesSetFont(const void *fontdata, Uint32 cw, Uint32 ch);
	SDL_GFXPRIMITIVES_SCOPE void gfxPrimitivesSetFontRotation(Uint32 rotation);
	SDL_GFXPRIMITIVES_SCOPE int characterColor(SDL_Surface * dst, Sint16 x, Sint16 y, char c, Uint32 color);
	SDL_GFXPRIMITIVES_SCOPE int characterRGBA(SDL_Surface * dst, Sint16 x, Sint16 y, char c, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
	SDL_GFXPRIMITIVES_SCOPE int stringColor(SDL_Surface * dst, Sint16 x, Sint16 y, const char *s, Uint32 color);
	SDL_GFXPRIMITIVES_SCOPE int stringRGBA(SDL_Surface * dst, Sint16 x, Sint16 y, const char *s, Uint8 r, Uint8 g, Uint8 b, Uint8 a);

	/* Ends C function definitions when using C++ */
#ifdef __cplusplus
}
#endif

#endif				/* _SDL_gfxPrimitives_h */
