/*
 * XpdfPluginAPI.h
 *
 * Copyright 2004 Glyph & Cog, LLC
 */

#ifndef XPDFPLUGINAPI_H
#define XPDFPLUGINAPI_H

#ifdef _WIN32
#include <windows.h>
#else
#define Object XtObject
#include <X11/Intrinsic.h>
#undef Object
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*------------------------------------------------------------------------
 * Macros
 *------------------------------------------------------------------------*/

/*
 * The current API version.
 */
#define xpdfPluginAPIVersion 1

#ifdef _WIN32
#  ifdef __cplusplus
#    define PLUGINFUNC(retType) extern "C" __declspec(dllexport) retType
#  else
#    define PLUGINFUNC(retType) extern __declspec(dllexport) retType
#  endif
#else
#  ifdef __cplusplus
#    define PLUGINFUNC(retType) extern "C" retType
#  else
#    define PLUGINFUNC(retType) extern retType
#  endif
#endif

/*------------------------------------------------------------------------
 * Plugin setup/cleanup
 *------------------------------------------------------------------------*/

/*
 * All plugins are required to implement two functions:
 *
 * -- Initialize the plugin.  Returns non-zero if successful.
 * PLUGINFUNC(XpdfBool) xpdfInitPlugin(void);
 *
 * -- Free the plugin.
 * PLUGINFUNC(void) xpdfFreePlugin(void);
 */

/*------------------------------------------------------------------------
 * Types
 *------------------------------------------------------------------------*/

/*
 * Standard C boolean -- zero = false, non-zero = true.
 */
typedef int XpdfBool;
#define xpdfTrue  1
#define xpdfFalse 0

/*
 * PDF document handle.
 */
typedef struct _XpdfDoc *XpdfDoc;

/*
 * PDF object handle.
 */
typedef struct _XpdfObject *XpdfObject;

/*
 * Document access permissions.  Any of these can be bitwise 'or'ed
 * together.  If xpdfPermissionOpen is not included, the document
 * cannot be opened at all, and the other bits are ignored.
 */
typedef unsigned int XpdfPermission;
#define xpdfPermissionOpen     (1 << 0)
#define xpdfPermissionPrint    (1 << 2)
#define xpdfPermissionChange   (1 << 3)
#define xpdfPermissionCopy     (1 << 4)
#define xpdfPermissionNotes    (1 << 5)

/*------------------------------------------------------------------------
 * Security handler
 *------------------------------------------------------------------------*/

/*
 * XpdfSecurityHandler - a security handler plugin should create one
 * of these and pass it to xpdfRegisterSecurityHandler.
 */
#ifdef __cplusplus
struct XpdfSecurityHandler {
#else
typedef struct {
#endif

  /*
   * Version of the security handler spec (this document) -- use
   * xpdfPluginAPIVersion.
   */
  int version;

  /*
   * Security handler name.
   */
  char *name;

  /*
   * Any global data the security handler needs.  XpdfViewer will pass
   * this pointer to all handler functions as the <handlerData>
   * argument.
   */
  void *handlerData;

  /*
   * Allocate and initialize data for a new document.  XpdfViewer will
   * pass the returned pointer to all other handler functions as the
   * <docData> argument.  Returns non-zero if successful.
   */
  XpdfBool (*newDoc)(void *handlerData, XpdfDoc doc,
		     XpdfObject encryptDict, void **docData);

  /*
   * Free the data allocated by newDoc.
   */
  void (*freeDoc)(void *handlerData, void *docData);

  /*
   * Construct authorization data based on the supplied owner and user
   * passwords (either or both of which may be NULL).  This function
   * is called in "batch" mode, i.e., if the password was supplied on
   * the command line or via an Xpdf library API.  It should not
   * generate any user interaction (e.g., a password dialog).  It is
   * not required to support this function: the makeAuthData function
   * pointer can be set to NULL.  Returns non-zero if successful.
   */
  XpdfBool (*makeAuthData)(void *handlerData, void *docData,
			   char *ownerPassword, char *userPassword,
			   void **authData);

  /*
   * Request any needed information (e.g., a password) from the user,
   * and construct an authorization data object.  Returns non-zero if
   * successful.
   */
  XpdfBool (*getAuthData)(void *handlerData, void *docData,
			  void **authData);

  /*
   * Free the data allocated by getAuthData.
   */
  void (*freeAuthData)(void *handlerData, void *docData,
		       void *authData);

  /*
   * Request permission to access the document.  This returns all
   * permissions granted by authData.
   */
  XpdfPermission (*authorize)(void *handlerData, void *docData,
			      void *authData);

  /*
   * Get the decryption key and algorithm version associated with the
   * document.  Returns non-zero if successful.
   */
  XpdfBool (*getKey)(void *handlerData, void *docData,
		     char **key, int *keyLen, int *cryptVersion);

  /*
   * Free the data allocated by getKey.
   */
  void (*freeKey)(void *handlerData, void *docData,
		  char *key, int keyLen);

#ifdef __cplusplus
};
#else
} XpdfSecurityHandler;
#endif

/*------------------------------------------------------------------------*/

typedef struct {
  int version;

/*------------------------------------------------------------------------
 * Document access functions
 *------------------------------------------------------------------------*/

/*
 * Get a document's info dictionary.  (The returned object must be
 * freed with xpdfFreeObj.)
 */
XpdfObject (*_xpdfGetInfoDict)(XpdfDoc doc);

/*
 * Get a document's catalog ("root") dictionary.  (The returned object
 * must be freed with xpdfFreeObj.)
 */
XpdfObject (*_xpdfGetCatalog)(XpdfDoc doc);

#ifdef _WIN32

/*
 * Get the handle for the viewer window associated with the specified
 * document.  [Win32 only]
 */
HWND (*_xpdfWin32GetWindow)(XpdfDoc doc);

#else

/*
 * Get the Motif widget for the viewer window associated with the
 * specified document.  [X only]
 */
Widget (*_xpdfXGetWindow)(XpdfDoc doc);

#endif

/*------------------------------------------------------------------------
 * Object access functions
 *------------------------------------------------------------------------*/

/*
 * Check an object's type.
 */
XpdfBool (*_xpdfObjIsBool)(XpdfObject obj);
XpdfBool (*_xpdfObjIsInt)(XpdfObject obj);
XpdfBool (*_xpdfObjIsReal)(XpdfObject obj);
XpdfBool (*_xpdfObjIsString)(XpdfObject obj);
XpdfBool (*_xpdfObjIsName)(XpdfObject obj);
XpdfBool (*_xpdfObjIsNull)(XpdfObject obj);
XpdfBool (*_xpdfObjIsArray)(XpdfObject obj);
XpdfBool (*_xpdfObjIsDict)(XpdfObject obj);
XpdfBool (*_xpdfObjIsStream)(XpdfObject obj);
XpdfBool (*_xpdfObjIsRef)(XpdfObject obj);

/*
 * Value access.
 * (Objects returned by xpdfArrayGet and xpdfDictGet must be freed
 * with xpdfFreeObj.)
 */
XpdfBool (*_xpdfBoolValue)(XpdfObject obj);
int (*_xpdfIntValue)(XpdfObject obj);
double (*_xpdfRealValue)(XpdfObject obj);
int (*_xpdfStringLength)(XpdfObject obj);
char *(*_xpdfStringValue)(XpdfObject obj);
char *(*_xpdfNameValue)(XpdfObject obj);
int (*_xpdfArrayLength)(XpdfObject obj);
XpdfObject (*_xpdfArrayGet)(XpdfObject obj, int idx);
XpdfObject (*_xpdfDictGet)(XpdfObject obj, char *key);

/*
 * Object destruction.  NB: *all* objects must be freed after use.
 */
void (*_xpdfFreeObj)(XpdfObject obj);

/*------------------------------------------------------------------------
 * Memory allocation functions
 *------------------------------------------------------------------------*/

void *(*_xpdfMalloc)(int size);
void *(*_xpdfRealloc)(void *p, int size);
void (*_xpdfFree)(void *p);

/*------------------------------------------------------------------------
 * Security handler functions
 *------------------------------------------------------------------------*/

/*
 * Register a new security handler.
 */
void (*_xpdfRegisterSecurityHandler)(XpdfSecurityHandler *handler);

/*------------------------------------------------------------------------*/

} XpdfPluginVecTable;

#ifdef _WIN32

extern __declspec(dllexport) XpdfPluginVecTable xpdfPluginVecTable;

#define xpdfPluginSetup \
  extern __declspec(dllexport) \
  XpdfPluginVecTable xpdfPluginVecTable = {xpdfPluginAPIVersion};

#else

extern XpdfPluginVecTable xpdfPluginVecTable;

#define xpdfPluginSetup \
  XpdfPluginVecTable xpdfPluginVecTable = {xpdfPluginAPIVersion};

#endif

#define xpdfGetInfoDict (*xpdfPluginVecTable._xpdfGetInfoDict)
#define xpdfGetCatalog (*xpdfPluginVecTable._xpdfGetCatalog)
#ifdef _WIN32
#define xpdfWin32GetWindow (*xpdfPluginVecTable._xpdfWin32GetWindow)
#else
#define xpdfXGetWindow (*xpdfPluginVecTable._xpdfXGetWindow)
#endif
#define xpdfObjIsBool (*xpdfPluginVecTable._xpdfObjIsBool)
#define xpdfObjIsInt (*xpdfPluginVecTable._xpdfObjIsInt)
#define xpdfObjIsReal (*xpdfPluginVecTable._xpdfObjIsReal)
#define xpdfObjIsString (*xpdfPluginVecTable._xpdfObjIsString)
#define xpdfObjIsName (*xpdfPluginVecTable._xpdfObjIsName)
#define xpdfObjIsNull (*xpdfPluginVecTable._xpdfObjIsNull)
#define xpdfObjIsArray (*xpdfPluginVecTable._xpdfObjIsArray)
#define xpdfObjIsDict (*xpdfPluginVecTable._xpdfObjIsDict)
#define xpdfObjIsStream (*xpdfPluginVecTable._xpdfObjIsStream)
#define xpdfObjIsRef (*xpdfPluginVecTable._xpdfObjIsRef)
#define xpdfBoolValue (*xpdfPluginVecTable._xpdfBoolValue)
#define xpdfIntValue (*xpdfPluginVecTable._xpdfIntValue)
#define xpdfRealValue (*xpdfPluginVecTable._xpdfRealValue)
#define xpdfStringLength (*xpdfPluginVecTable._xpdfStringLength)
#define xpdfStringValue (*xpdfPluginVecTable._xpdfStringValue)
#define xpdfNameValue (*xpdfPluginVecTable._xpdfNameValue)
#define xpdfArrayLength (*xpdfPluginVecTable._xpdfArrayLength)
#define xpdfArrayGet (*xpdfPluginVecTable._xpdfArrayGet)
#define xpdfDictGet (*xpdfPluginVecTable._xpdfDictGet)
#define xpdfFreeObj (*xpdfPluginVecTable._xpdfFreeObj)
#define xpdfMalloc (*xpdfPluginVecTable._xpdfMalloc)
#define xpdfRealloc (*xpdfPluginVecTable._xpdfRealloc)
#define xpdfFree (*xpdfPluginVecTable._xpdfFree)
#define xpdfRegisterSecurityHandler (*xpdfPluginVecTable._xpdfRegisterSecurityHandler)

#ifdef __cplusplus
}
#endif

#endif
