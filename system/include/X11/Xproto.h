/* Definitions for the X window system used by server and c bindings */

/*
 * This packet-construction scheme makes the following assumptions:
 *
 * 1. The compiler is able
 * to generate code which addresses one- and two-byte quantities.
 * In the worst case, this would be done with bit-fields.  If bit-fields
 * are used it may be necessary to reorder the request fields in this file,
 * depending on the order in which the machine assigns bit fields to
 * machine words.  There may also be a problem with sign extension,
 * as K+R specify that bitfields are always unsigned.
 *
 * 2. 2- and 4-byte fields in packet structures must be ordered by hand
 * such that they are naturally-aligned, so that no compiler will ever
 * insert padding bytes.
 *
 * 3. All packets are hand-padded to a multiple of 4 bytes, for
 * the same reason.
 */

#ifndef XPROTO_H
#define XPROTO_H

/***********************************************************

Copyright 1987, 1998  The Open Group

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from The Open Group.


Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the name of Digital not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/

#include <X11/Xmd.h>
#include <X11/Xprotostr.h>

/*
 * Define constants for the sizes of the network packets.  The sz_ prefix is
 * used instead of something more descriptive so that the symbols are no more
 * than 32 characters in length (which causes problems for some compilers).
 */
#define sz_xSegment 8
#define sz_xPoint 4
#define sz_xRectangle 8
#define sz_xArc 12
#define sz_xConnClientPrefix 12
#define sz_xConnSetupPrefix 8
#define sz_xConnSetup 32
#define sz_xPixmapFormat 8
#define sz_xDepth 8
#define sz_xVisualType 24
#define sz_xWindowRoot 40
#define sz_xTimecoord 8
#define sz_xHostEntry 4
#define sz_xCharInfo 12
#define sz_xFontProp 8
#define sz_xTextElt 2
#define sz_xColorItem 12
#define sz_xrgb 8
#define sz_xGenericReply 32
#define sz_xGetWindowAttributesReply 44
#define sz_xGetGeometryReply 32
#define sz_xQueryTreeReply 32
#define sz_xInternAtomReply 32
#define sz_xGetAtomNameReply 32
#define sz_xGetPropertyReply 32
#define sz_xListPropertiesReply 32
#define sz_xGetSelectionOwnerReply 32
#define sz_xGrabPointerReply 32
#define sz_xQueryPointerReply 32
#define sz_xGetMotionEventsReply 32
#define sz_xTranslateCoordsReply 32
#define sz_xGetInputFocusReply 32
#define sz_xQueryKeymapReply 40
#define sz_xQueryFontReply 60
#define sz_xQueryTextExtentsReply 32
#define sz_xListFontsReply 32
#define sz_xGetFontPathReply 32
#define sz_xGetImageReply 32
#define sz_xListInstalledColormapsReply 32
#define sz_xAllocColorReply 32
#define sz_xAllocNamedColorReply 32
#define sz_xAllocColorCellsReply 32
#define sz_xAllocColorPlanesReply 32
#define sz_xQueryColorsReply 32
#define sz_xLookupColorReply 32
#define sz_xQueryBestSizeReply 32
#define sz_xQueryExtensionReply 32
#define sz_xListExtensionsReply 32
#define sz_xSetMappingReply 32
#define sz_xGetKeyboardControlReply 52
#define sz_xGetPointerControlReply 32
#define sz_xGetScreenSaverReply 32
#define sz_xListHostsReply 32
#define sz_xSetModifierMappingReply 32
#define sz_xError 32
#define sz_xEvent 32
#define sz_xKeymapEvent 32
#define sz_xReq 4
#define sz_xResourceReq 8
#define sz_xCreateWindowReq 32
#define sz_xChangeWindowAttributesReq 12
#define sz_xChangeSaveSetReq 8
#define sz_xReparentWindowReq 16
#define sz_xConfigureWindowReq 12
#define sz_xCirculateWindowReq 8
#define sz_xInternAtomReq 8
#define sz_xChangePropertyReq 24
#define sz_xDeletePropertyReq 12
#define sz_xGetPropertyReq 24
#define sz_xSetSelectionOwnerReq 16
#define sz_xConvertSelectionReq 24
#define sz_xSendEventReq 44
#define sz_xGrabPointerReq 24
#define sz_xGrabButtonReq 24
#define sz_xUngrabButtonReq 12
#define sz_xChangeActivePointerGrabReq 16
#define sz_xGrabKeyboardReq 16
#define sz_xGrabKeyReq 16
#define sz_xUngrabKeyReq 12
#define sz_xAllowEventsReq 8
#define sz_xGetMotionEventsReq 16
#define sz_xTranslateCoordsReq 16
#define sz_xWarpPointerReq 24
#define sz_xSetInputFocusReq 12
#define sz_xOpenFontReq 12
#define sz_xQueryTextExtentsReq 8
#define sz_xListFontsReq 8
#define sz_xSetFontPathReq 8
#define sz_xCreatePixmapReq 16
#define sz_xCreateGCReq 16
#define sz_xChangeGCReq 12
#define sz_xCopyGCReq 16
#define sz_xSetDashesReq 12
#define sz_xSetClipRectanglesReq 12
#define sz_xCopyAreaReq 28
#define sz_xCopyPlaneReq 32
#define sz_xPolyPointReq 12
#define sz_xPolySegmentReq 12
#define sz_xFillPolyReq 16
#define sz_xPutImageReq 24
#define sz_xGetImageReq 20
#define sz_xPolyTextReq 16
#define sz_xImageTextReq 16
#define sz_xCreateColormapReq 16
#define sz_xCopyColormapAndFreeReq 12
#define sz_xAllocColorReq 16
#define sz_xAllocNamedColorReq 12
#define sz_xAllocColorCellsReq 12
#define sz_xAllocColorPlanesReq 16
#define sz_xFreeColorsReq 12
#define sz_xStoreColorsReq 8
#define sz_xStoreNamedColorReq 16
#define sz_xQueryColorsReq 8
#define sz_xLookupColorReq 12
#define sz_xCreateCursorReq 32
#define sz_xCreateGlyphCursorReq 32
#define sz_xRecolorCursorReq 20
#define sz_xQueryBestSizeReq 12
#define sz_xQueryExtensionReq 8
#define sz_xChangeKeyboardControlReq 8
#define sz_xBellReq 4
#define sz_xChangePointerControlReq 12
#define sz_xSetScreenSaverReq 12
#define sz_xChangeHostsReq 8
#define sz_xListHostsReq 4
#define sz_xChangeModeReq 4
#define sz_xRotatePropertiesReq 12
#define sz_xReply 32
#define sz_xGrabKeyboardReply 32
#define sz_xListFontsWithInfoReply 60
#define sz_xSetPointerMappingReply 32
#define sz_xGetKeyboardMappingReply 32
#define sz_xGetPointerMappingReply 32
#define sz_xGetModifierMappingReply 32
#define sz_xListFontsWithInfoReq 8
#define sz_xPolyLineReq 12
#define sz_xPolyArcReq 12
#define sz_xPolyRectangleReq 12
#define sz_xPolyFillRectangleReq 12
#define sz_xPolyFillArcReq 12
#define sz_xPolyText8Req 16
#define sz_xPolyText16Req 16
#define sz_xImageText8Req 16
#define sz_xImageText16Req 16
#define sz_xSetPointerMappingReq 4
#define sz_xForceScreenSaverReq 4
#define sz_xSetCloseDownModeReq 4
#define sz_xClearAreaReq 16
#define sz_xSetAccessControlReq 4
#define sz_xGetKeyboardMappingReq 8
#define sz_xSetModifierMappingReq 4
#define sz_xPropIconSize 24
#define sz_xChangeKeyboardMappingReq 8


/* For the purpose of the structure definitions in this file,
we must redefine the following types in terms of Xmd.h's types, which may
include bit fields.  All of these are #undef'd at the end of this file,
restoring the definitions in X.h.  */

#define Window CARD32
#define Drawable CARD32
#define Font CARD32
#define Pixmap CARD32
#define Cursor CARD32
#define Colormap CARD32
#define GContext CARD32
#define Atom CARD32
#define VisualID CARD32
#define Time CARD32
#define KeyCode CARD8
#define KeySym CARD32

#define X_TCP_PORT 6000     /* add display number */

#define xTrue        1
#define xFalse       0


typedef CARD16 KeyButMask;

/***************** 
   connection setup structure.  This is followed by
   numRoots xWindowRoot structs.
*****************/

typedef struct {
    CARD8	byteOrder;
    BYTE	pad;
    CARD16	majorVersion B16, minorVersion B16;
    CARD16	nbytesAuthProto B16;	/* Authorization protocol */
    CARD16	nbytesAuthString B16;	/* Authorization string */
    CARD16	pad2 B16;
} xConnClientPrefix;

typedef struct {
    CARD8          success;
    BYTE           lengthReason; /*num bytes in string following if failure */
    CARD16         majorVersion B16, 
                   minorVersion B16;
    CARD16         length B16;  /* 1/4 additional bytes in setup info */
} xConnSetupPrefix;


typedef struct {
    CARD32         release B32;
    CARD32         ridBase B32, 
                   ridMask B32;
    CARD32         motionBufferSize B32;
    CARD16         nbytesVendor B16;  /* number of bytes in vendor string */
    CARD16         maxRequestSize B16;
    CARD8          numRoots;          /* number of roots structs to follow */
    CARD8          numFormats;        /* number of pixmap formats */
    CARD8          imageByteOrder;        /* LSBFirst, MSBFirst */
    CARD8          bitmapBitOrder;        /* LeastSignificant, MostSign...*/
    CARD8          bitmapScanlineUnit,     /* 8, 16, 32 */
                   bitmapScanlinePad;     /* 8, 16, 32 */
    KeyCode	   minKeyCode, maxKeyCode;
    CARD32	   pad2 B32;
} xConnSetup;

typedef struct {
    CARD8          depth;
    CARD8          bitsPerPixel;
    CARD8          scanLinePad;
    CARD8          pad1;
    CARD32	   pad2 B32;
} xPixmapFormat;

/* window root */

typedef struct {
    CARD8 	depth;
    CARD8 	pad1;
    CARD16	nVisuals B16;  /* number of xVisualType structures following */
    CARD32	pad2 B32;
    } xDepth;

typedef struct {
    VisualID visualID B32;
#if defined(__cplusplus) || defined(c_plusplus)
    CARD8 c_class;
#else
    CARD8 class;
#endif
    CARD8 bitsPerRGB;
    CARD16 colormapEntries B16;
    CARD32 redMask B32, greenMask B32, blueMask B32;
    CARD32 pad B32;
    } xVisualType;

typedef struct {
    Window         windowId B32;
    Colormap       defaultColormap B32;
    CARD32         whitePixel B32, blackPixel B32;
    CARD32         currentInputMask B32;   
    CARD16         pixWidth B16, pixHeight B16;
    CARD16         mmWidth B16, mmHeight B16;
    CARD16         minInstalledMaps B16, maxInstalledMaps B16;
    VisualID       rootVisualID B32;
    CARD8          backingStore;
    BOOL           saveUnders;
    CARD8          rootDepth;
    CARD8          nDepths;  /* number of xDepth structures following */
} xWindowRoot;


/*****************************************************************
 * Structure Defns
 *   Structures needed for replies 
 *****************************************************************/

/* Used in GetMotionEvents */

typedef struct {
    CARD32 time B32;
    INT16 x B16, y B16;
} xTimecoord;

typedef struct {
    CARD8 family;
    BYTE pad;
    CARD16 length B16;
} xHostEntry;

typedef struct {
    INT16 leftSideBearing B16,
	  rightSideBearing B16,
	  characterWidth B16,
	  ascent B16,
	  descent B16;
    CARD16 attributes B16;
} xCharInfo;

typedef struct {
    Atom name B32;
    CARD32 value B32;
} xFontProp;

/*
 * non-aligned big-endian font ID follows this struct
 */
typedef struct {           /* followed by string */
    CARD8 len;	/* number of *characters* in string, or FontChange (255)
		   for font change, or 0 if just delta given */
    INT8 delta;
} xTextElt;


typedef struct {        
    CARD32 pixel B32;
    CARD16 red B16, green B16, blue B16;
    CARD8 flags;  /* DoRed, DoGreen, DoBlue booleans */
    CARD8 pad;
} xColorItem;


typedef struct {
    CARD16 red B16, green B16, blue B16, pad B16;
} xrgb;

typedef CARD8 KEYCODE;


/*****************
 * XRep:
 *    meant to be 32 byte quantity 
 *****************/

/* GenericReply is the common format of all replies.  The "data" items
   are specific to each individual reply type. */

typedef struct {	
    BYTE type;              /* X_Reply */
    BYTE data1;             /* depends on reply type */
    CARD16 sequenceNumber B16;  /* of last request received by server */
    CARD32 length B32;      /* 4 byte quantities beyond size of GenericReply */
    CARD32 data00 B32;
    CARD32 data01 B32;
    CARD32 data02 B32;
    CARD32 data03 B32;
    CARD32 data04 B32;
    CARD32 data05 B32;
    } xGenericReply;

/* Individual reply formats. */

typedef struct {
    BYTE type;  /* X_Reply */
    CARD8 backingStore;
    CARD16 sequenceNumber B16;
    CARD32 length B32;	/* NOT 0; this is an extra-large reply */
    VisualID visualID B32;
#if defined(__cplusplus) || defined(c_plusplus)
    CARD16 c_class B16;
#else
    CARD16 class B16;
#endif
    CARD8 bitGravity;
    CARD8 winGravity;
    CARD32 backingBitPlanes B32;
    CARD32 backingPixel B32;
    BOOL saveUnder;
    BOOL mapInstalled;
    CARD8 mapState;
    BOOL override;
    Colormap colormap B32;
    CARD32 allEventMasks B32;
    CARD32 yourEventMask B32;
    CARD16 doNotPropagateMask B16;
    CARD16 pad B16;
    } xGetWindowAttributesReply;

typedef struct {
    BYTE type;   /* X_Reply */
    CARD8 depth;
    CARD16 sequenceNumber B16;
    CARD32 length B32;  /* 0 */
    Window root B32;
    INT16 x B16, y B16;
    CARD16 width B16, height B16;
    CARD16 borderWidth B16;
    CARD16 pad1 B16;
    CARD32 pad2 B32;
    CARD32 pad3 B32;
    } xGetGeometryReply;

typedef struct {
    BYTE type;  /* X_Reply */
    BYTE pad1;
    CARD16 sequenceNumber B16;
    CARD32 length B32;
    Window root B32, parent B32;
    CARD16 nChildren B16;
    CARD16 pad2 B16;
    CARD32 pad3 B32;
    CARD32 pad4 B32;
    CARD32 pad5 B32;
    } xQueryTreeReply;

typedef struct {
    BYTE type;  /* X_Reply */
    BYTE pad1;
    CARD16 sequenceNumber B16;
    CARD32 length B32; /* 0 */
    Atom atom B32;
    CARD32 pad2 B32;
    CARD32 pad3 B32;
    CARD32 pad4 B32;
    CARD32 pad5 B32;
    CARD32 pad6 B32;
    } xInternAtomReply;

typedef struct {
    BYTE type;  /* X_Reply */
    BYTE pad1;
    CARD16 sequenceNumber B16;
    CARD32 length B32;  /* of additional bytes */
    CARD16 nameLength B16;  /* # of characters in name */
    CARD16 pad2 B16;
    CARD32 pad3 B32;
    CARD32 pad4 B32;
    CARD32 pad5 B32;
    CARD32 pad6 B32;
    CARD32 pad7 B32;
    } xGetAtomNameReply;

typedef struct {
    BYTE type;  /* X_Reply */
    CARD8 format;
    CARD16 sequenceNumber B16;
    CARD32 length B32; /* of additional bytes */
    Atom propertyType B32;
    CARD32 bytesAfter B32;
    CARD32 nItems B32; /* # of 8, 16, or 32-bit entities in reply */
    CARD32 pad1 B32;
    CARD32 pad2 B32;
    CARD32 pad3 B32;
    } xGetPropertyReply;

typedef struct {
    BYTE type;  /* X_Reply */
    BYTE pad1;
    CARD16 sequenceNumber B16;
    CARD32 length B32;
    CARD16 nProperties B16;
    CARD16 pad2 B16;
    CARD32 pad3 B32;
    CARD32 pad4 B32;
    CARD32 pad5 B32;
    CARD32 pad6 B32;
    CARD32 pad7 B32;
    } xListPropertiesReply;

typedef struct {
    BYTE type;  /* X_Reply */
    BYTE pad1;
    CARD16 sequenceNumber B16;
    CARD32 length B32;  /* 0 */
    Window owner B32;
    CARD32 pad2 B32;
    CARD32 pad3 B32;
    CARD32 pad4 B32;
    CARD32 pad5 B32;
    CARD32 pad6 B32;
    } xGetSelectionOwnerReply;

typedef struct {
    BYTE type;  /* X_Reply */
    BYTE status;
    CARD16 sequenceNumber B16;
    CARD32 length B32;  /* 0 */
    CARD32 pad1 B32;
    CARD32 pad2 B32;
    CARD32 pad3 B32;
    CARD32 pad4 B32;
    CARD32 pad5 B32;
    CARD32 pad6 B32;
    } xGrabPointerReply;

typedef xGrabPointerReply xGrabKeyboardReply;

typedef struct {
    BYTE type;  /* X_Reply */
    BOOL sameScreen;
    CARD16 sequenceNumber B16;
    CARD32 length B32;  /* 0 */
    Window root B32, child B32;
    INT16 rootX B16, rootY B16, winX B16, winY B16;
    CARD16 mask B16;
    CARD16 pad1 B16;
    CARD32 pad B32;
    } xQueryPointerReply;

typedef struct {
    BYTE type;  /* X_Reply */
    BYTE pad1;
    CARD16 sequenceNumber B16;
    CARD32 length B32;
    CARD32 nEvents B32;
    CARD32 pad2 B32;
    CARD32 pad3 B32;
    CARD32 pad4 B32;
    CARD32 pad5 B32;
    CARD32 pad6 B32;
    } xGetMotionEventsReply;

typedef struct {
    BYTE type;  /* X_Reply */
    BOOL sameScreen;
    CARD16 sequenceNumber B16;
    CARD32 length B32; /* 0 */
    Window child B32;
    INT16 dstX B16, dstY B16;
    CARD32 pad2 B32;
    CARD32 pad3 B32;
    CARD32 pad4 B32;
    CARD32 pad5 B32;
    } xTranslateCoordsReply;

typedef struct {
    BYTE type;  /* X_Reply */
    CARD8 revertTo;
    CARD16 sequenceNumber B16;
    CARD32 length B32;  /* 0 */
    Window focus B32;
    CARD32 pad1 B32;
    CARD32 pad2 B32;
    CARD32 pad3 B32;
    CARD32 pad4 B32;
    CARD32 pad5 B32;
    } xGetInputFocusReply;

typedef struct {
    BYTE type;  /* X_Reply */
    BYTE pad1;
    CARD16 sequenceNumber B16;
    CARD32 length B32;  /* 2, NOT 0; this is an extra-large reply */
    BYTE map[32];
    } xQueryKeymapReply;

/* Warning: this MUST match (up to component renaming) xListFontsWithInfoReply */
typedef struct _xQueryFontReply {
    BYTE type;  /* X_Reply */
    BYTE pad1;
    CARD16 sequenceNumber B16;
    CARD32 length B32;  /* definitely > 0, even if "nCharInfos" is 0 */
    xCharInfo minBounds; 
#ifndef WORD64
    CARD32 walign1 B32;
#endif
    xCharInfo maxBounds; 
#ifndef WORD64
    CARD32 walign2 B32;
#endif
    CARD16 minCharOrByte2 B16, maxCharOrByte2 B16;
    CARD16 defaultChar B16;
    CARD16 nFontProps B16;  /* followed by this many xFontProp structures */
    CARD8 drawDirection;
    CARD8 minByte1, maxByte1;
    BOOL allCharsExist;
    INT16 fontAscent B16, fontDescent B16;
    CARD32 nCharInfos B32; /* followed by this many xCharInfo structures */
} xQueryFontReply;

typedef struct {
    BYTE type;  /* X_Reply */
    CARD8 drawDirection;
    CARD16 sequenceNumber B16;
    CARD32 length B32;  /* 0 */
    INT16 fontAscent B16, fontDescent B16;
    INT16 overallAscent B16, overallDescent B16;
    INT32 overallWidth B32, overallLeft B32, overallRight B32;
    CARD32 pad B32;
    } xQueryTextExtentsReply;

typedef struct {
    BYTE type;  /* X_Reply */
    BYTE pad1;
    CARD16 sequenceNumber B16;
    CARD32 length B32;
    CARD16 nFonts B16;
    CARD16 pad2 B16;
    CARD32 pad3 B32;
    CARD32 pad4 B32;
    CARD32 pad5 B32;
    CARD32 pad6 B32;
    CARD32 pad7 B32;
    } xListFontsReply;

/* Warning: this MUST match (up to component renaming) xQueryFontReply */
typedef struct {
    BYTE type;  /* X_Reply */
    CARD8 nameLength;  /* 0 indicates end-of-reply-sequence */
    CARD16 sequenceNumber B16;
    CARD32 length B32;  /* definitely > 0, even if "nameLength" is 0 */
    xCharInfo minBounds; 
#ifndef WORD64
    CARD32 walign1 B32;
#endif
    xCharInfo maxBounds; 
#ifndef WORD64
    CARD32 walign2 B32;
#endif
    CARD16 minCharOrByte2 B16, maxCharOrByte2 B16;
    CARD16 defaultChar B16;
    CARD16 nFontProps B16;  /* followed by this many xFontProp structures */
    CARD8 drawDirection;
    CARD8 minByte1, maxByte1;
    BOOL allCharsExist;
    INT16 fontAscent B16, fontDescent B16;
    CARD32 nReplies B32;   /* hint as to how many more replies might be coming */
} xListFontsWithInfoReply;

typedef struct {
    BYTE type;  /* X_Reply */
    BYTE pad1;
    CARD16 sequenceNumber B16;
    CARD32 length B32;
    CARD16 nPaths B16;
    CARD16 pad2 B16;
    CARD32 pad3 B32;
    CARD32 pad4 B32;
    CARD32 pad5 B32;
    CARD32 pad6 B32;
    CARD32 pad7 B32;
    } xGetFontPathReply;

typedef struct {
    BYTE type;  /* X_Reply */
    CARD8 depth;
    CARD16 sequenceNumber B16;
    CARD32 length B32;
    VisualID visual B32;
    CARD32 pad3 B32;
    CARD32 pad4 B32;
    CARD32 pad5 B32;
    CARD32 pad6 B32;
    CARD32 pad7 B32;
    } xGetImageReply;

typedef struct {
    BYTE type;  /* X_Reply */
    BYTE pad1;
    CARD16 sequenceNumber B16;
    CARD32 length B32;
    CARD16 nColormaps B16;
    CARD16 pad2 B16;
    CARD32 pad3 B32;
    CARD32 pad4 B32;
    CARD32 pad5 B32;
    CARD32 pad6 B32;
    CARD32 pad7 B32;
    } xListInstalledColormapsReply;

typedef struct {
    BYTE type; /* X_Reply */
    BYTE pad1;
    CARD16 sequenceNumber B16;
    CARD32 length B32;   /* 0 */
    CARD16 red B16, green B16, blue B16;
    CARD16 pad2 B16;
    CARD32 pixel B32;
    CARD32 pad3 B32;
    CARD32 pad4 B32;
    CARD32 pad5 B32;
    } xAllocColorReply;

typedef struct {
    BYTE type; /* X_Reply */
    BYTE pad1;
    CARD16 sequenceNumber B16;
    CARD32 length B32;  /* 0 */
    CARD32 pixel B32;
    CARD16 exactRed B16, exactGreen B16, exactBlue B16;
    CARD16 screenRed B16, screenGreen B16, screenBlue B16;
    CARD32 pad2 B32;
    CARD32 pad3 B32;
    } xAllocNamedColorReply;

typedef struct {
    BYTE type;  /* X_Reply */
    BYTE pad1;
    CARD16 sequenceNumber B16;
    CARD32 length B32;
    CARD16 nPixels B16, nMasks B16;
    CARD32 pad3 B32;
    CARD32 pad4 B32;
    CARD32 pad5 B32;
    CARD32 pad6 B32;
    CARD32 pad7 B32;
    } xAllocColorCellsReply;

typedef struct {
    BYTE type; /* X_Reply */
    BYTE pad1;
    CARD16 sequenceNumber B16;
    CARD32 length B32;
    CARD16 nPixels B16;
    CARD16 pad2 B16;
    CARD32 redMask B32, greenMask B32, blueMask B32;
    CARD32 pad3 B32;
    CARD32 pad4 B32;
    } xAllocColorPlanesReply;

typedef struct {
    BYTE type; /* X_Reply */
    BYTE pad1;
    CARD16 sequenceNumber B16;
    CARD32 length B32;
    CARD16 nColors B16;
    CARD16 pad2 B16;
    CARD32 pad3 B32;
    CARD32 pad4 B32;
    CARD32 pad5 B32;
    CARD32 pad6 B32;
    CARD32 pad7 B32;
    } xQueryColorsReply;

typedef struct {
    BYTE type;  /* X_Reply */
    BYTE pad1;
    CARD16 sequenceNumber B16;
    CARD32 length B32;  /* 0 */
    CARD16 exactRed B16, exactGreen B16, exactBlue B16;
    CARD16 screenRed B16, screenGreen B16, screenBlue B16;
    CARD32 pad3 B32;
    CARD32 pad4 B32;
    CARD32 pad5 B32;
    } xLookupColorReply;

typedef struct {
    BYTE type;  /* X_Reply */
    BYTE pad1;
    CARD16 sequenceNumber B16;
    CARD32 length B32;  /* 0 */
    CARD16 width B16, height B16;
    CARD32 pad3 B32;
    CARD32 pad4 B32;
    CARD32 pad5 B32;
    CARD32 pad6 B32;
    CARD32 pad7 B32;
    } xQueryBestSizeReply;

typedef struct {
    BYTE type;  /* X_Reply */
    BYTE pad1;
    CARD16 sequenceNumber B16;
    CARD32 length B32; /* 0 */
    BOOL  present;
    CARD8 major_opcode;
    CARD8 first_event;
    CARD8 first_error;
    CARD32 pad3 B32;
    CARD32 pad4 B32;
    CARD32 pad5 B32;
    CARD32 pad6 B32;
    CARD32 pad7 B32;
    } xQueryExtensionReply;

typedef struct {
    BYTE type;  /* X_Reply */
    CARD8 nExtensions;
    CARD16 sequenceNumber B16;
    CARD32 length B32;
    CARD32 pad2 B32;
    CARD32 pad3 B32;
    CARD32 pad4 B32;
    CARD32 pad5 B32;
    CARD32 pad6 B32;
    CARD32 pad7 B32;
    } xListExtensionsReply;


typedef struct {
    BYTE   type;  /* X_Reply */
    CARD8  success;
    CARD16 sequenceNumber B16;
    CARD32 length B32;
    CARD32 pad2 B32;
    CARD32 pad3 B32;
    CARD32 pad4 B32;
    CARD32 pad5 B32;
    CARD32 pad6 B32;
    CARD32 pad7 B32;
    } xSetMappingReply;
typedef xSetMappingReply xSetPointerMappingReply;
typedef xSetMappingReply xSetModifierMappingReply;

typedef struct {
    BYTE type;  /* X_Reply */
    CARD8 nElts;  /* how many elements does the map have */
    CARD16 sequenceNumber B16;
    CARD32 length B32;
    CARD32 pad2 B32;
    CARD32 pad3 B32;
    CARD32 pad4 B32;
    CARD32 pad5 B32;
    CARD32 pad6 B32;
    CARD32 pad7 B32;
    } xGetPointerMappingReply;

typedef struct {
    BYTE type;
    CARD8 keySymsPerKeyCode;
    CARD16 sequenceNumber B16;
    CARD32 length B32;
    CARD32 pad2 B32;
    CARD32 pad3 B32;
    CARD32 pad4 B32;
    CARD32 pad5 B32;
    CARD32 pad6 B32;
    CARD32 pad7 B32;
} xGetKeyboardMappingReply;    

typedef struct {
    BYTE type;
    CARD8 numKeyPerModifier;
    CARD16 sequenceNumber B16;
    CARD32 length B32;
    CARD32 pad1 B32;
    CARD32 pad2 B32;
    CARD32 pad3 B32;
    CARD32 pad4 B32;
    CARD32 pad5 B32;
    CARD32 pad6 B32;
} xGetModifierMappingReply;

typedef struct {
    BYTE type;  /* X_Reply */
    BOOL globalAutoRepeat;
    CARD16 sequenceNumber B16;
    CARD32 length B32;  /* 5 */
    CARD32 ledMask B32;
    CARD8 keyClickPercent, bellPercent;
    CARD16 bellPitch B16, bellDuration B16;
    CARD16 pad B16;
    BYTE map[32];  /* bit masks start here */
    } xGetKeyboardControlReply;

typedef struct {
    BYTE type;  /* X_Reply */
    BYTE pad1;
    CARD16 sequenceNumber B16;
    CARD32 length B32;  /* 0 */
    CARD16 accelNumerator B16, accelDenominator B16;
    CARD16 threshold B16;
    CARD16 pad2 B16;
    CARD32 pad3 B32;
    CARD32 pad4 B32;
    CARD32 pad5 B32;
    CARD32 pad6 B32;
    } xGetPointerControlReply;

typedef struct {
    BYTE type;  /* X_Reply */
    BYTE pad1;
    CARD16 sequenceNumber B16;
    CARD32 length B32;  /* 0 */
    CARD16 timeout B16, interval B16;
    BOOL preferBlanking;
    BOOL allowExposures;
    CARD16 pad2 B16;
    CARD32 pad3 B32;
    CARD32 pad4 B32;
    CARD32 pad5 B32;
    CARD32 pad6 B32;
    } xGetScreenSaverReply;

typedef struct {
    BYTE type;  /* X_Reply */
    BOOL enabled;
    CARD16 sequenceNumber B16;
    CARD32 length B32;
    CARD16 nHosts B16;
    CARD16 pad1 B16;
    CARD32 pad3 B32;
    CARD32 pad4 B32;
    CARD32 pad5 B32;
    CARD32 pad6 B32;
    CARD32 pad7 B32;
    } xListHostsReply;




/*****************************************************************
 * Xerror
 *    All errors  are 32 bytes 
 *****************************************************************/

typedef struct {
    BYTE type;                  /* X_Error */
    BYTE errorCode;
    CARD16 sequenceNumber B16;       /* the nth request from this client */
    CARD32 resourceID B32;
    CARD16 minorCode B16;
    CARD8 majorCode;
    BYTE pad1;
    CARD32 pad3 B32;
    CARD32 pad4 B32;
    CARD32 pad5 B32;
    CARD32 pad6 B32;
    CARD32 pad7 B32;
} xError;

/*****************************************************************
 * xEvent
 *    All events are 32 bytes
 *****************************************************************/

typedef struct _xEvent {
    union {
	struct {
	    BYTE type;
	    BYTE detail;
	    CARD16 sequenceNumber B16;
	    } u;
	struct {
            CARD32 pad00 B32;
	    Time time B32;
	    Window root B32, event B32, child B32;
	    INT16 rootX B16, rootY B16, eventX B16, eventY B16;
	    KeyButMask state B16;
	    BOOL sameScreen;		
	    BYTE pad1;
	} keyButtonPointer;
	struct {
            CARD32 pad00 B32;
            Time time B32;
	    Window root B32, event B32, child B32;
	    INT16 rootX B16, rootY B16, eventX B16, eventY B16;
	    KeyButMask state B16;
	    BYTE mode; 			/* really XMode */
	    BYTE flags;		/* sameScreen and focus booleans, packed together */
#define ELFlagFocus        (1<<0)
#define ELFlagSameScreen   (1<<1)
	} enterLeave;
	struct {
            CARD32 pad00 B32;
	    Window window B32;
	    BYTE mode; 			/* really XMode */
	    BYTE pad1, pad2, pad3;
	} focus;
	struct {
            CARD32 pad00 B32;
	    Window window B32;
	    CARD16 x B16, y B16, width B16, height B16;
	    CARD16 count B16;
	    CARD16 pad2 B16;
	} expose;
	struct {
            CARD32 pad00 B32;
	    Drawable drawable B32;
	    CARD16 x B16, y B16, width B16, height B16;
	    CARD16 minorEvent B16;
	    CARD16 count B16;
	    BYTE majorEvent;
	    BYTE pad1, pad2, pad3;
	} graphicsExposure;
	struct {
            CARD32 pad00 B32;
	    Drawable drawable B32;
	    CARD16 minorEvent B16;
	    BYTE majorEvent;
	    BYTE bpad;
	} noExposure;
	struct {
            CARD32 pad00 B32;
	    Window window B32;
	    CARD8 state;
	    BYTE pad1, pad2, pad3;
	} visibility;
	struct {
            CARD32 pad00 B32;
	    Window parent B32, window B32;
	    INT16 x B16, y B16;
	    CARD16 width B16, height B16, borderWidth B16;
	    BOOL override;
	    BYTE bpad;
        } createNotify;
/*
 * The event fields in the structures for DestroyNotify, UnmapNotify,
 * MapNotify, ReparentNotify, ConfigureNotify, CirculateNotify, GravityNotify,
 * must be at the same offset because server internal code is depending upon
 * this to patch up the events before they are delivered.
 * Also note that MapRequest, ConfigureRequest and CirculateRequest have
 * the same offset for the event window.
 */
	struct {
            CARD32 pad00 B32;
	    Window event B32, window B32;
	} destroyNotify;
	struct {
            CARD32 pad00 B32;
	    Window event B32, window B32;
	    BOOL fromConfigure;
	    BYTE pad1, pad2, pad3;
        } unmapNotify;
	struct {
            CARD32 pad00 B32;
	    Window event B32, window B32;
	    BOOL override;
	    BYTE pad1, pad2, pad3;
        } mapNotify;
	struct {
            CARD32 pad00 B32;
	    Window parent B32, window B32;
        } mapRequest;
	struct {
            CARD32 pad00 B32;
	    Window event B32, window B32, parent B32;
	    INT16 x B16, y B16;
	    BOOL override;
	    BYTE pad1, pad2, pad3;
	} reparent;
	struct {
            CARD32 pad00 B32;
	    Window event B32, window B32, aboveSibling B32;
	    INT16 x B16, y B16;
	    CARD16 width B16, height B16, borderWidth B16;
	    BOOL override;		
	    BYTE bpad;
	} configureNotify;
	struct {
            CARD32 pad00 B32;
	    Window parent B32, window B32, sibling B32;
	    INT16 x B16, y B16;
	    CARD16 width B16, height B16, borderWidth B16;
	    CARD16 valueMask B16;
	    CARD32 pad1 B32;
	} configureRequest;
	struct {
            CARD32 pad00 B32;
	    Window event B32, window B32;
	    INT16 x B16, y B16;
	    CARD32 pad1 B32, pad2 B32, pad3 B32, pad4 B32;
	} gravity;
	struct {
            CARD32 pad00 B32;
	    Window window B32;
	    CARD16 width B16, height B16;
	} resizeRequest;
	struct {
/* The event field in the circulate record is really the parent when this
   is used as a CirculateRequest instead of a CirculateNotify */
            CARD32 pad00 B32;
	    Window event B32, window B32, parent B32;
	    BYTE place;			/* Top or Bottom */
	    BYTE pad1, pad2, pad3;
	} circulate;
	struct {
            CARD32 pad00 B32;
	    Window window B32;
	    Atom atom B32;
	    Time time B32;
	    BYTE state;			/* NewValue or Deleted */
	    BYTE pad1;
	    CARD16 pad2 B16;
	} property;
	struct {
            CARD32 pad00 B32;
            Time time B32;     
	    Window window B32;
	    Atom atom B32;
	} selectionClear;
	struct {
            CARD32 pad00 B32;
            Time time B32;    
	    Window owner B32, requestor B32;
	    Atom selection B32, target B32, property B32;
	} selectionRequest;
	struct {
            CARD32 pad00 B32;
            Time time B32;   
	    Window requestor B32;
	    Atom selection B32, target B32, property B32;
	} selectionNotify;
	struct {
            CARD32 pad00 B32;
	    Window window B32;
	    Colormap colormap B32;
#if defined(__cplusplus) || defined(c_plusplus)
	    BOOL c_new;
#else
	    BOOL new;
#endif
	    BYTE state;			/* Installed or UnInstalled */
	    BYTE pad1, pad2;
	} colormap;
	struct {
	    CARD32 pad00 B32;
	    CARD8 request;
	    KeyCode firstKeyCode;
	    CARD8 count;
	    BYTE pad1;
	} mappingNotify;
	struct {
            CARD32 pad00 B32;
	    Window window B32;
	    union {
		struct {
		    Atom type B32;
		    INT32 longs0 B32;
		    INT32 longs1 B32;
		    INT32 longs2 B32;
		    INT32 longs3 B32;
		    INT32 longs4 B32;
		} l;
		struct {
		    Atom type B32;
		    INT16 shorts0 B16;
		    INT16 shorts1 B16;
		    INT16 shorts2 B16;
		    INT16 shorts3 B16;
		    INT16 shorts4 B16;
		    INT16 shorts5 B16;
		    INT16 shorts6 B16;
		    INT16 shorts7 B16;
		    INT16 shorts8 B16;
		    INT16 shorts9 B16;
		} s;
		struct {
		    Atom type B32;
		    INT8 bytes[20];
		} b;
	    } u; 
	} clientMessage;
    } u;
} xEvent;

/*********************************************************
 *
 * Generic event
 * 
 * Those events are not part of the core protocol spec and can be used by
 * various extensions.
 * type is always GenericEvent
 * extension is the minor opcode of the extension the event belongs to.
 * evtype is the actual event type, unique __per extension__. 
 *
 * GenericEvents can be longer than 32 bytes, with the length field
 * specifying the number of 4 byte blocks after the first 32 bytes. 
 *
 *
 */
typedef struct 
{
    BYTE    type;
    CARD8   extension;
    CARD16  sequenceNumber B16;
    CARD32  length B32;
    CARD16  evtype B16;
    CARD16  pad2 B16;
    CARD32  pad3 B32;
    CARD32  pad4 B32;
    CARD32  pad5 B32;
    CARD32  pad6 B32;
    CARD32  pad7 B32;
} xGenericEvent;



/* KeymapNotify events are not included in the above union because they
   are different from all other events: they do not have a "detail"
   or "sequenceNumber", so there is room for a 248-bit key mask. */

typedef struct {
    BYTE type;
    BYTE map[31];
    } xKeymapEvent;

#define XEventSize (sizeof(xEvent))

/* XReply is the union of all the replies above whose "fixed part"
fits in 32 bytes.  It does NOT include GetWindowAttributesReply,
QueryFontReply, QueryKeymapReply, or GetKeyboardControlReply 
ListFontsWithInfoReply */

typedef union {
    xGenericReply generic;
    xGetGeometryReply geom;
    xQueryTreeReply tree;
    xInternAtomReply atom;
    xGetAtomNameReply atomName;
    xGetPropertyReply property;
    xListPropertiesReply listProperties;
    xGetSelectionOwnerReply selection;
    xGrabPointerReply grabPointer;
    xGrabKeyboardReply grabKeyboard;
    xQueryPointerReply pointer;
    xGetMotionEventsReply motionEvents;
    xTranslateCoordsReply coords;
    xGetInputFocusReply inputFocus;
    xQueryTextExtentsReply textExtents;
    xListFontsReply fonts;
    xGetFontPathReply fontPath;
    xGetImageReply image;
    xListInstalledColormapsReply colormaps;
    xAllocColorReply allocColor;
    xAllocNamedColorReply allocNamedColor;
    xAllocColorCellsReply colorCells;
    xAllocColorPlanesReply colorPlanes;
    xQueryColorsReply colors;
    xLookupColorReply lookupColor;
    xQueryBestSizeReply bestSize;
    xQueryExtensionReply extension;
    xListExtensionsReply extensions;
    xSetModifierMappingReply setModifierMapping;
    xGetModifierMappingReply getModifierMapping;
    xSetPointerMappingReply setPointerMapping;
    xGetKeyboardMappingReply getKeyboardMapping;
    xGetPointerMappingReply getPointerMapping;
    xGetPointerControlReply pointerControl;
    xGetScreenSaverReply screenSaver;
    xListHostsReply hosts;
    xError error;
    xEvent event;
} xReply;



/*****************************************************************
 * REQUESTS
 *****************************************************************/


/* Request structure */

typedef struct _xReq {
	CARD8 reqType;
	CARD8 data;            /* meaning depends on request type */
	CARD16 length B16;         /* length in 4 bytes quantities 
				  of whole request, including this header */
} xReq;

/*****************************************************************
 *  structures that follow request. 
 *****************************************************************/

/* ResourceReq is used for any request which has a resource ID 
   (or Atom or Time) as its one and only argument.  */

typedef struct {
    CARD8 reqType;
    BYTE pad;
    CARD16 length B16;
    CARD32 id B32;  /* a Window, Drawable, Font, GContext, Pixmap, etc. */
    } xResourceReq;

typedef struct {
    CARD8 reqType;
    CARD8 depth;
    CARD16 length B16;
    Window wid B32, parent B32;
    INT16 x B16, y B16;
    CARD16 width B16, height B16, borderWidth B16;  
#if defined(__cplusplus) || defined(c_plusplus)
    CARD16 c_class B16;
#else
    CARD16 class B16;
#endif
    VisualID visual B32;
    CARD32 mask B32;
} xCreateWindowReq;

typedef struct {
    CARD8 reqType;
    BYTE pad;
    CARD16 length B16;
    Window window B32;
    CARD32 valueMask B32; 
} xChangeWindowAttributesReq;

typedef struct {
    CARD8 reqType;
    BYTE mode;
    CARD16 length B16;
    Window window B32;
} xChangeSaveSetReq;

typedef struct {
    CARD8 reqType;
    BYTE pad;
    CARD16 length B16;
    Window window B32, parent B32;
    INT16 x B16, y B16;
} xReparentWindowReq;

typedef struct {
    CARD8 reqType;
    CARD8 pad;
    CARD16 length B16;
    Window window B32;
    CARD16 mask B16;
    CARD16 pad2 B16;
} xConfigureWindowReq;

typedef struct {
    CARD8 reqType;
    CARD8 direction;
    CARD16 length B16;
    Window window B32;
} xCirculateWindowReq;

typedef struct {    /* followed by padded string */
    CARD8 reqType;
    BOOL onlyIfExists;
    CARD16 length B16;
    CARD16 nbytes  B16;    /* number of bytes in string */
    CARD16 pad B16;
} xInternAtomReq;

typedef struct {
    CARD8 reqType;
    CARD8 mode;
    CARD16 length B16;
    Window window B32;
    Atom property B32, type B32;
    CARD8 format;
    BYTE pad[3];
    CARD32 nUnits B32;     /* length of stuff following, depends on format */
} xChangePropertyReq;

typedef struct {
    CARD8 reqType;
    BYTE pad;
    CARD16 length B16;
    Window window B32;
    Atom property B32;
} xDeletePropertyReq;

typedef struct {
    CARD8 reqType;
#if defined(__cplusplus) || defined(c_plusplus)
    BOOL c_delete;
#else
    BOOL delete;
#endif
    CARD16 length B16;
    Window window B32;
    Atom property B32, type B32;
    CARD32 longOffset B32;
    CARD32 longLength B32;
} xGetPropertyReq;
 
typedef struct {
    CARD8 reqType;
    BYTE pad;
    CARD16 length B16;
    Window window B32;
    Atom selection B32;
    Time time B32;
} xSetSelectionOwnerReq;

typedef struct {
    CARD8 reqType;
    BYTE pad;
    CARD16 length B16;
    Window requestor B32;
    Atom selection B32, target B32, property B32;
    Time time B32;
    } xConvertSelectionReq;

typedef struct {
    CARD8 reqType;
    BOOL propagate;
    CARD16 length B16;
    Window destination B32;
    CARD32 eventMask B32;
#ifdef WORD64
    /* the structure should have been quad-aligned */
    BYTE eventdata[SIZEOF(xEvent)];
#else
    xEvent event;
#endif /* WORD64 */
} xSendEventReq;

typedef struct {
    CARD8 reqType;
    BOOL ownerEvents;
    CARD16 length B16;
    Window grabWindow B32;
    CARD16 eventMask B16;
    BYTE pointerMode, keyboardMode;
    Window confineTo B32;
    Cursor cursor B32;
    Time time B32;
} xGrabPointerReq;

typedef struct {
    CARD8 reqType;
    BOOL ownerEvents;
    CARD16 length B16;
    Window grabWindow B32;
    CARD16 eventMask B16;
    BYTE pointerMode, keyboardMode;
    Window confineTo B32;
    Cursor cursor B32;
    CARD8 button;
    BYTE pad;
    CARD16 modifiers B16;
} xGrabButtonReq;

typedef struct {
    CARD8 reqType;
    CARD8 button;
    CARD16 length B16;
    Window grabWindow B32;
    CARD16 modifiers B16;
    CARD16 pad B16;
} xUngrabButtonReq;

typedef struct {
    CARD8 reqType;
    BYTE pad;
    CARD16 length B16;
    Cursor cursor B32;
    Time time B32;
    CARD16 eventMask B16;
    CARD16 pad2 B16;
} xChangeActivePointerGrabReq;

typedef struct {
    CARD8 reqType;
    BOOL ownerEvents;
    CARD16 length B16;
    Window grabWindow B32;
    Time time B32;
    BYTE pointerMode, keyboardMode;  
    CARD16 pad B16;
} xGrabKeyboardReq;

typedef struct {
    CARD8 reqType;
    BOOL ownerEvents;
    CARD16 length B16;
    Window grabWindow B32;
    CARD16 modifiers B16;
    CARD8 key;
    BYTE pointerMode, keyboardMode;  
    BYTE pad1, pad2, pad3;
} xGrabKeyReq;

typedef struct {
    CARD8 reqType;
    CARD8 key;
    CARD16 length B16;
    Window grabWindow B32;
    CARD16 modifiers B16;
    CARD16 pad B16;
} xUngrabKeyReq;

typedef struct {
    CARD8 reqType;
    CARD8 mode;
    CARD16 length B16;
    Time time B32;
} xAllowEventsReq;

typedef struct {
    CARD8 reqType;
    BYTE pad;
    CARD16 length B16;
    Window window B32;
    Time start B32, stop B32;
} xGetMotionEventsReq;

typedef struct {
    CARD8 reqType;
    BYTE pad;
    CARD16 length B16;
    Window srcWid B32, dstWid B32;
    INT16 srcX B16, srcY B16;
} xTranslateCoordsReq;

typedef struct {
    CARD8 reqType;
    BYTE pad;
    CARD16 length B16;
    Window srcWid B32, dstWid B32;
    INT16 srcX B16, srcY B16;
    CARD16 srcWidth B16, srcHeight B16;
    INT16 dstX B16, dstY B16;
} xWarpPointerReq;

typedef struct {
    CARD8 reqType;
    CARD8 revertTo;
    CARD16 length B16;
    Window focus B32;
    Time time B32;
} xSetInputFocusReq;

typedef struct {
    CARD8 reqType;
    BYTE pad;
    CARD16 length B16;
    Font fid B32;
    CARD16 nbytes B16;
    BYTE pad1, pad2;	/* string follows on word boundary */
} xOpenFontReq;

typedef struct {
    CARD8 reqType;
    BOOL oddLength;
    CARD16 length B16;
    Font fid B32;
    } xQueryTextExtentsReq;

typedef struct {
    CARD8 reqType;
    BYTE pad;
    CARD16 length B16;
    CARD16 maxNames B16;
    CARD16 nbytes B16;  /* followed immediately by string bytes */
} xListFontsReq;

typedef xListFontsReq xListFontsWithInfoReq;

typedef struct {
    CARD8 reqType;
    BYTE pad;
    CARD16 length B16;
    CARD16 nFonts B16;
    BYTE pad1, pad2;	/* LISTofSTRING8 follows on word boundary */
} xSetFontPathReq;

typedef struct {
    CARD8 reqType;
    CARD8 depth;
    CARD16 length B16;
    Pixmap pid B32;
    Drawable drawable B32;
    CARD16 width B16, height B16;
} xCreatePixmapReq;

typedef struct {
    CARD8 reqType;
    BYTE pad;
    CARD16 length B16;
    GContext gc B32;
    Drawable drawable B32;
    CARD32 mask B32;
} xCreateGCReq;

typedef struct {
    CARD8 reqType;
    BYTE pad;
    CARD16 length B16;
    GContext gc B32;
    CARD32 mask B32;
} xChangeGCReq;    

typedef struct {
    CARD8 reqType;
    BYTE pad;
    CARD16 length B16;
    GContext srcGC B32, dstGC B32;
    CARD32 mask B32;
} xCopyGCReq;    

typedef struct {
    CARD8 reqType;
    BYTE pad;
    CARD16 length B16;
    GContext gc B32;
    CARD16 dashOffset B16;
    CARD16 nDashes B16;        /* length LISTofCARD8 of values following */
} xSetDashesReq;    

typedef struct {
    CARD8 reqType;
    BYTE ordering;
    CARD16 length B16;
    GContext gc B32;
    INT16 xOrigin B16, yOrigin B16;
} xSetClipRectanglesReq;    

typedef struct {
    CARD8 reqType;
    BOOL exposures;
    CARD16 length B16;
    Window window B32;
    INT16 x B16, y B16;
    CARD16 width B16, height B16;
} xClearAreaReq;

typedef struct {
    CARD8 reqType;
    BYTE pad;
    CARD16 length B16;
    Drawable srcDrawable B32, dstDrawable B32;
    GContext gc B32;
    INT16 srcX B16, srcY B16, dstX B16, dstY B16;
    CARD16 width B16, height B16;
} xCopyAreaReq;    

typedef struct {
    CARD8 reqType;
    BYTE pad;
    CARD16 length B16;
    Drawable srcDrawable B32, dstDrawable B32;
    GContext gc B32;
    INT16 srcX B16, srcY B16, dstX B16, dstY B16;
    CARD16 width B16, height B16;
    CARD32 bitPlane B32;
} xCopyPlaneReq;    

typedef struct {
    CARD8 reqType;
    BYTE coordMode;
    CARD16 length B16;
    Drawable drawable B32;
    GContext gc B32;
} xPolyPointReq;    

typedef xPolyPointReq xPolyLineReq;  /* same request structure */

/* The following used for PolySegment, PolyRectangle, PolyArc, PolyFillRectangle, PolyFillArc */

typedef struct {
    CARD8 reqType;
    BYTE pad;
    CARD16 length B16;
    Drawable drawable B32;
    GContext gc B32;
} xPolySegmentReq;    

typedef xPolySegmentReq xPolyArcReq;
typedef xPolySegmentReq xPolyRectangleReq;
typedef xPolySegmentReq xPolyFillRectangleReq;
typedef xPolySegmentReq xPolyFillArcReq;

typedef struct _FillPolyReq {
    CARD8 reqType;
    BYTE pad;
    CARD16 length B16;
    Drawable drawable B32;
    GContext gc B32;
    BYTE shape;
    BYTE coordMode;
    CARD16 pad1 B16;
} xFillPolyReq;    


typedef struct _PutImageReq {
    CARD8 reqType;
    CARD8 format;
    CARD16 length B16;
    Drawable drawable B32;
    GContext gc B32;
    CARD16 width B16, height B16;
    INT16 dstX B16, dstY B16;
    CARD8 leftPad;
    CARD8 depth;
    CARD16 pad B16;
} xPutImageReq;    

typedef struct {
    CARD8 reqType;
    CARD8 format;
    CARD16 length B16;
    Drawable drawable B32;
    INT16 x B16, y B16;
    CARD16 width B16, height B16;
    CARD32 planeMask B32;
} xGetImageReq;    

/* the following used by PolyText8 and PolyText16 */

typedef struct {
    CARD8 reqType;
    CARD8 pad;
    CARD16 length B16;
    Drawable drawable B32;
    GContext gc B32;
    INT16 x B16, y B16;		/* items (xTextElt) start after struct */
} xPolyTextReq;    

typedef xPolyTextReq xPolyText8Req;
typedef xPolyTextReq xPolyText16Req;

typedef struct {
    CARD8 reqType;
    BYTE nChars;
    CARD16 length B16;
    Drawable drawable B32;
    GContext gc B32;
    INT16 x B16, y B16;
} xImageTextReq;    

typedef xImageTextReq xImageText8Req;
typedef xImageTextReq xImageText16Req;

typedef struct {
    CARD8 reqType;
    BYTE alloc;
    CARD16 length B16;
    Colormap mid B32;
    Window window B32;
    VisualID visual B32;
} xCreateColormapReq;    

typedef struct {
    CARD8 reqType;
    BYTE pad;
    CARD16 length B16;
    Colormap mid B32;
    Colormap srcCmap B32;
} xCopyColormapAndFreeReq;    

typedef struct {
    CARD8 reqType;
    BYTE pad;
    CARD16 length B16;
    Colormap cmap B32;
    CARD16 red B16, green B16, blue B16;
    CARD16 pad2 B16;
} xAllocColorReq;    

typedef struct {
    CARD8 reqType;
    BYTE pad;
    CARD16 length B16;
    Colormap cmap B32;
    CARD16 nbytes B16;  /* followed by structure */
    BYTE pad1, pad2;
} xAllocNamedColorReq;    

typedef struct {
    CARD8 reqType;
    BOOL contiguous;
    CARD16 length B16;
    Colormap cmap B32;
    CARD16 colors B16, planes B16;
} xAllocColorCellsReq;    

typedef struct {
    CARD8 reqType;
    BOOL contiguous;
    CARD16 length B16;
    Colormap cmap B32;
    CARD16 colors B16, red B16, green B16, blue B16;
} xAllocColorPlanesReq;    

typedef struct {
    CARD8 reqType;
    BYTE pad;
    CARD16 length B16;
    Colormap cmap B32;
    CARD32 planeMask B32;
} xFreeColorsReq;    

typedef struct {
    CARD8 reqType;
    BYTE pad;
    CARD16 length B16;
    Colormap cmap B32;
} xStoreColorsReq;    

typedef struct {
    CARD8 reqType;
    CARD8 flags;   /* DoRed, DoGreen, DoBlue, as in xColorItem */
    CARD16 length B16;
    Colormap cmap B32;
    CARD32 pixel B32;
    CARD16 nbytes B16;  /* number of name string bytes following structure */
    BYTE pad1, pad2;
    } xStoreNamedColorReq;

typedef struct {
    CARD8 reqType;
    BYTE pad;
    CARD16 length B16;
    Colormap cmap B32;
} xQueryColorsReq;    

typedef struct {    /* followed  by string of length len */
    CARD8 reqType;
    BYTE pad;
    CARD16 length B16;
    Colormap cmap B32;
    CARD16 nbytes B16;  /* number of string bytes following structure*/
    BYTE pad1, pad2;
} xLookupColorReq;    

typedef struct {
    CARD8 reqType;
    BYTE pad;
    CARD16 length B16;
    Cursor cid B32;
    Pixmap source B32, mask B32;
    CARD16 foreRed B16, foreGreen B16, foreBlue B16;
    CARD16 backRed B16, backGreen B16, backBlue B16;
    CARD16 x B16, y B16;
} xCreateCursorReq;    

typedef struct {
    CARD8 reqType;
    BYTE pad;
    CARD16 length B16;
    Cursor cid B32;
    Font source B32, mask B32;
    CARD16 sourceChar B16, maskChar B16;
    CARD16 foreRed B16, foreGreen B16, foreBlue B16;
    CARD16 backRed B16, backGreen B16, backBlue B16;
} xCreateGlyphCursorReq;    

typedef struct {
    CARD8 reqType;
    BYTE pad;
    CARD16 length B16;
    Cursor cursor B32;
    CARD16 foreRed B16, foreGreen B16, foreBlue B16;
    CARD16 backRed B16, backGreen B16, backBlue B16;
} xRecolorCursorReq;    

typedef struct {
    CARD8 reqType;
#if defined(__cplusplus) || defined(c_plusplus)
    CARD8 c_class;
#else
    CARD8 class;
#endif
    CARD16 length B16;
    Drawable drawable B32;
    CARD16 width B16, height B16;
} xQueryBestSizeReq;    

typedef struct {
    CARD8 reqType;
    BYTE pad;
    CARD16 length B16;
    CARD16 nbytes B16;  /* number of string bytes following structure */
    BYTE pad1, pad2;
} xQueryExtensionReq;

typedef struct {
    CARD8   reqType;
    CARD8   numKeyPerModifier;
    CARD16  length B16;
} xSetModifierMappingReq;

typedef struct {
    CARD8 reqType;
    CARD8 nElts;  /* how many elements in the map */
    CARD16 length B16;
} xSetPointerMappingReq;

typedef struct {
    CARD8 reqType;
    BYTE pad;
    CARD16 length B16;
    KeyCode firstKeyCode;
    CARD8 count;
    CARD16 pad1 B16;
} xGetKeyboardMappingReq;    

typedef struct {
    CARD8 reqType;
    CARD8 keyCodes;
    CARD16 length B16;
    KeyCode firstKeyCode;
    CARD8 keySymsPerKeyCode;
    CARD16 pad1 B16;
} xChangeKeyboardMappingReq;

typedef struct {
    CARD8 reqType;
    BYTE pad;
    CARD16 length B16;
    CARD32 mask B32;
} xChangeKeyboardControlReq;    

typedef struct {
    CARD8 reqType;
    INT8 percent;  /* -100 to 100 */
    CARD16 length B16;
} xBellReq;    

typedef struct {
    CARD8 reqType;
    BYTE pad;
    CARD16 length B16;
    INT16 accelNum B16, accelDenum B16;
    INT16 threshold B16;             
    BOOL doAccel, doThresh;
} xChangePointerControlReq;    

typedef struct {
    CARD8 reqType;
    BYTE pad;
    CARD16 length B16;
    INT16 timeout B16, interval B16;
    BYTE preferBlank, allowExpose;  
    CARD16 pad2 B16;
} xSetScreenSaverReq;    

typedef struct {
    CARD8 reqType;
    BYTE mode;
    CARD16 length B16;
    CARD8 hostFamily;
    BYTE pad;
    CARD16 hostLength B16;
} xChangeHostsReq;    

typedef struct {
    CARD8 reqType;
    BYTE pad;
    CARD16 length B16;
    } xListHostsReq;

typedef struct {
    CARD8 reqType;
    BYTE mode;
    CARD16 length B16;
    } xChangeModeReq;

typedef xChangeModeReq xSetAccessControlReq;
typedef xChangeModeReq xSetCloseDownModeReq;
typedef xChangeModeReq xForceScreenSaverReq;

typedef struct { /* followed by LIST of ATOM */
    CARD8 reqType;
    BYTE pad;
    CARD16 length B16;
    Window window B32;
    CARD16 nAtoms B16;
    INT16 nPositions B16;
    } xRotatePropertiesReq;
    


/* Reply codes */

#define X_Reply		1		/* Normal reply */
#define X_Error		0		/* Error */

/* Request codes */

#define X_CreateWindow                  1              
#define X_ChangeWindowAttributes        2        
#define X_GetWindowAttributes           3     
#define X_DestroyWindow                 4
#define X_DestroySubwindows             5   
#define X_ChangeSaveSet                 6
#define X_ReparentWindow                7
#define X_MapWindow                     8
#define X_MapSubwindows                 9
#define X_UnmapWindow                  10
#define X_UnmapSubwindows              11  
#define X_ConfigureWindow              12  
#define X_CirculateWindow              13  
#define X_GetGeometry                  14
#define X_QueryTree                    15
#define X_InternAtom                   16
#define X_GetAtomName                  17
#define X_ChangeProperty               18 
#define X_DeleteProperty               19 
#define X_GetProperty                  20
#define X_ListProperties               21 
#define X_SetSelectionOwner            22    
#define X_GetSelectionOwner            23    
#define X_ConvertSelection             24   
#define X_SendEvent                    25
#define X_GrabPointer                  26
#define X_UngrabPointer                27
#define X_GrabButton                   28
#define X_UngrabButton                 29
#define X_ChangeActivePointerGrab      30          
#define X_GrabKeyboard                 31
#define X_UngrabKeyboard               32 
#define X_GrabKey                      33
#define X_UngrabKey                    34
#define X_AllowEvents                  35       
#define X_GrabServer                   36      
#define X_UngrabServer                 37        
#define X_QueryPointer                 38        
#define X_GetMotionEvents              39           
#define X_TranslateCoords              40                
#define X_WarpPointer                  41       
#define X_SetInputFocus                42         
#define X_GetInputFocus                43         
#define X_QueryKeymap                  44       
#define X_OpenFont                     45    
#define X_CloseFont                    46     
#define X_QueryFont                    47
#define X_QueryTextExtents             48     
#define X_ListFonts                    49  
#define X_ListFontsWithInfo    	       50 
#define X_SetFontPath                  51 
#define X_GetFontPath                  52 
#define X_CreatePixmap                 53        
#define X_FreePixmap                   54      
#define X_CreateGC                     55    
#define X_ChangeGC                     56    
#define X_CopyGC                       57  
#define X_SetDashes                    58     
#define X_SetClipRectangles            59             
#define X_FreeGC                       60  
#define X_ClearArea                    61             
#define X_CopyArea                     62    
#define X_CopyPlane                    63     
#define X_PolyPoint                    64     
#define X_PolyLine                     65    
#define X_PolySegment                  66       
#define X_PolyRectangle                67         
#define X_PolyArc                      68   
#define X_FillPoly                     69    
#define X_PolyFillRectangle            70             
#define X_PolyFillArc                  71       
#define X_PutImage                     72    
#define X_GetImage                     73 
#define X_PolyText8                    74     
#define X_PolyText16                   75      
#define X_ImageText8                   76      
#define X_ImageText16                  77       
#define X_CreateColormap               78          
#define X_FreeColormap                 79        
#define X_CopyColormapAndFree          80               
#define X_InstallColormap              81           
#define X_UninstallColormap            82             
#define X_ListInstalledColormaps       83                  
#define X_AllocColor                   84      
#define X_AllocNamedColor              85           
#define X_AllocColorCells              86           
#define X_AllocColorPlanes             87            
#define X_FreeColors                   88      
#define X_StoreColors                  89       
#define X_StoreNamedColor              90           
#define X_QueryColors                  91       
#define X_LookupColor                  92       
#define X_CreateCursor                 93        
#define X_CreateGlyphCursor            94             
#define X_FreeCursor                   95      
#define X_RecolorCursor                96         
#define X_QueryBestSize                97         
#define X_QueryExtension               98          
#define X_ListExtensions               99          
#define X_ChangeKeyboardMapping        100
#define X_GetKeyboardMapping           101
#define X_ChangeKeyboardControl        102                
#define X_GetKeyboardControl           103             
#define X_Bell                         104
#define X_ChangePointerControl         105
#define X_GetPointerControl            106
#define X_SetScreenSaver               107          
#define X_GetScreenSaver               108          
#define X_ChangeHosts                  109       
#define X_ListHosts                    110     
#define X_SetAccessControl             111               
#define X_SetCloseDownMode             112
#define X_KillClient                   113 
#define X_RotateProperties	       114
#define X_ForceScreenSaver	       115
#define X_SetPointerMapping            116
#define X_GetPointerMapping            117
#define X_SetModifierMapping	       118
#define X_GetModifierMapping	       119
#define X_NoOperation                  127

/* restore these definitions back to the typedefs in X.h */
#undef Window
#undef Drawable
#undef Font
#undef Pixmap
#undef Cursor
#undef Colormap
#undef GContext
#undef Atom
#undef VisualID
#undef Time
#undef KeyCode
#undef KeySym

#endif /* XPROTO_H */
