/***************************************************************************

getarg.h - Support routines for the giflib utilities

SPDX-License-Identifier: MIT

**************************************************************************/

#ifndef _GETARG_H
#define _GETARG_H

#include "gif_lib.h"
#include <stdbool.h>

#define VERSION_COOKIE " Version %d.%d, "

/***************************************************************************
 Error numbers as returned by GAGetArg routine:
***************************************************************************/ 
#define CMD_ERR_NotAnOpt  1    /* None Option found. */
#define CMD_ERR_NoSuchOpt 2    /* Undefined Option Found. */
#define CMD_ERR_WildEmpty 3    /* Empty input for !*? seq. */
#define CMD_ERR_NumRead   4    /* Failed on reading number. */
#define CMD_ERR_AllSatis  5    /* Fail to satisfy (must-'!') option. */

bool GAGetArgs(int argc, char **argv, char *CtrlStr, ...);
void GAPrintErrMsg(int Error);
void GAPrintHowTo(char *CtrlStr);

/******************************************************************************
 From qprintf.c
******************************************************************************/
extern bool GifNoisyPrint;
extern void GifQprintf(char *Format, ...);
extern void PrintGifError(int ErrorCode);

/******************************************************************************
 Color table quantization
******************************************************************************/
int GifQuantizeBuffer(unsigned int Width, unsigned int Height,
                   int *ColorMapSize, GifByteType * RedInput,
                   GifByteType * GreenInput, GifByteType * BlueInput,
                   GifByteType * OutputBuffer,
                   GifColorType * OutputColorMap);

/* These used to live in the library header */
#define GIF_MESSAGE(Msg) fprintf(stderr, "\n%s: %s\n", PROGRAM_NAME, Msg)
#define GIF_EXIT(Msg)    { GIF_MESSAGE(Msg); exit(-3); }

#endif /* _GETARG_H */

/* end */
