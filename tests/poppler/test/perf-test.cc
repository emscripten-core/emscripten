/* Copyright Krzysztof Kowalczyk 2006-2007
   Copyright Hib Eris <hib@hiberis.nl> 2008
   License: GPLv2 */
/*
  A tool to stress-test poppler rendering and measure rendering times for
  very simplistic performance measuring.

  TODO:
   * make it work with cairo output as well
   * print more info about document like e.g. enumarate images,
     streams, compression, encryption, password-protection. Each should have
     a command-line arguments to turn it on/off
   * never over-write file given as -out argument (optionally, provide -force
     option to force writing the -out file). It's way too easy too lose results
     of a previous run.
*/

#ifdef _MSC_VER
// this sucks but I don't know any other way
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif

#ifdef _WIN32
#include <windows.h>
#else
#include <strings.h>
#endif

// Define COPY_FILE if you want the file to be copied to a local disk first
// before it's tested. This is desired if a file is on a slow drive.
// Currently copying only works on Windows.
// Not enabled by default.
//#define COPY_FILE 1

#include <assert.h>
#include <config.h>
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#ifdef HAVE_DIRENT_H
#include <dirent.h>
#endif

#include "Error.h"
#include "ErrorCodes.h"
#include "goo/GooString.h"
#include "goo/GooList.h"
#include "goo/GooTimer.h"
#include "GlobalParams.h"
#include "splash/SplashBitmap.h"
#include "Object.h" /* must be included before SplashOutputDev.h because of sloppiness in SplashOutputDev.h */
#include "SplashOutputDev.h"
#include "TextOutputDev.h"
#include "PDFDoc.h"
#include "Link.h"

#ifdef _MSC_VER
#define strdup _strdup
#define strcasecmp _stricmp
#endif

#define dimof(X)    (sizeof(X)/sizeof((X)[0]))

#define INVALID_PAGE_NO     -1

/* Those must be implemented in order to provide preview during execution.
   They can be no-ops. An implementation for windows is in
   perf-test-preview-win.cc
*/
extern void PreviewBitmapInit(void);
extern void PreviewBitmapDestroy(void);
extern void PreviewBitmapSplash(SplashBitmap *bmpSplash);

class PdfEnginePoppler {
public:
    PdfEnginePoppler();
    ~PdfEnginePoppler();

    const char *fileName(void) const { return _fileName; };

    void setFileName(const char *fileName) {
        assert(!_fileName);
        _fileName = (char*)strdup(fileName);
    }

    int pageCount(void) const { return _pageCount; }

    bool load(const char *fileName);
    SplashBitmap *renderBitmap(int pageNo, double zoomReal, int rotation);

    SplashOutputDev *   outputDevice();
private:
    char *              _fileName;
    int                 _pageCount;

    PDFDoc *            _pdfDoc;
    SplashOutputDev *   _outputDev;
};

typedef struct StrList {
    struct StrList *next;
    char *          str;
} StrList;

/* List of all command-line arguments that are not switches.
   We assume those are:
     - names of PDF files
     - names of a file with a list of PDF files
     - names of directories with PDF files
*/
static StrList *gArgsListRoot = NULL;

/* Names of all command-line switches we recognize */
#define TIMINGS_ARG         "-timings"
#define RESOLUTION_ARG      "-resolution"
#define RECURSIVE_ARG       "-recursive"
#define OUT_ARG             "-out"
#define PREVIEW_ARG         "-preview"
#define SLOW_PREVIEW_ARG    "-slowpreview"
#define LOAD_ONLY_ARG       "-loadonly"
#define PAGE_ARG            "-page"
#define TEXT_ARG            "-text"

/* Should we record timings? True if -timings command-line argument was given. */
static bool gfTimings = false;

/* If true, we use render each page at resolution 'gResolutionX'/'gResolutionY'.
   If false, we render each page at its native resolution.
   True if -resolution NxM command-line argument was given. */
static bool gfForceResolution = false;
static int  gResolutionX = 0;
static int  gResolutionY = 0;
/* If NULL, we output the log info to stdout. If not NULL, should be a name
   of the file to which we output log info.
   Controled by -out command-line argument. */
static char *   gOutFileName = NULL;
/* FILE * correspondig to gOutFileName or stdout if gOutFileName is NULL or
   was invalid name */
static FILE *   gOutFile = NULL;
/* FILE * correspondig to gOutFileName or stderr if gOutFileName is NULL or
   was invalid name */
static FILE *   gErrFile = NULL;

/* If True and a directory is given as a command-line argument, we'll process
   pdf files in sub-directories as well.
   Controlled by -recursive command-line argument */
static bool gfRecursive = false;

/* If true, preview rendered image. To make sure that they're being rendered correctly. */
static bool gfPreview = false;

/* 1 second (1000 milliseconds) */
#define SLOW_PREVIEW_TIME 1000

/* If true, preview rendered image in a slow mode i.e. delay displaying for
   SLOW_PREVIEW_TIME. This is so that a human has enough time to see if the
   PDF renders ok. In release mode on fast processor pages take only ~100-200 ms
   to render and they go away too quickly to be inspected by a human. */
static bool gfSlowPreview = false;

/* If true, we only dump the text, not render */
static bool gfTextOnly = false;

#define PAGE_NO_NOT_GIVEN -1

/* If equals PAGE_NO_NOT_GIVEN, we're in default mode where we render all pages.
   If different, will only render this page */
static int  gPageNo = PAGE_NO_NOT_GIVEN;
/* If true, will only load the file, not render any pages. Mostly for
   profiling load time */
static bool gfLoadOnly = false;

#define PDF_FILE_DPI 72

#define MAX_FILENAME_SIZE 1024

/* DOS is 0xd 0xa */
#define DOS_NEWLINE "\x0d\x0a"
/* Mac is single 0xd */
#define MAC_NEWLINE "\x0d"
/* Unix is single 0xa (10) */
#define UNIX_NEWLINE "\x0a"
#define UNIX_NEWLINE_C 0xa

#ifdef _WIN32
  #define DIR_SEP_CHAR '\\'
  #define DIR_SEP_STR  "\\"
#else
  #define DIR_SEP_CHAR '/'
  #define DIR_SEP_STR  "/"
#endif

void memzero(void *data, size_t len)
{
    memset(data, 0, len);
}

void *zmalloc(size_t len)
{
    void *data = malloc(len);
    if (data)
        memzero(data, len);
    return data;
}

/* Concatenate 4 strings. Any string can be NULL.
   Caller needs to free() memory. */
char *str_cat4(const char *str1, const char *str2, const char *str3, const char *str4)
{
    char *str;
    char *tmp;
    size_t str1_len = 0;
    size_t str2_len = 0;
    size_t str3_len = 0;
    size_t str4_len = 0;

    if (str1)
        str1_len = strlen(str1);
    if (str2)
        str2_len = strlen(str2);
    if (str3)
        str3_len = strlen(str3);
    if (str4)
        str4_len = strlen(str4);

    str = (char*)zmalloc(str1_len + str2_len + str3_len + str4_len + 1);
    if (!str)
        return NULL;

    tmp = str;
    if (str1) {
        memcpy(tmp, str1, str1_len);
        tmp += str1_len;
    }
    if (str2) {
        memcpy(tmp, str2, str2_len);
        tmp += str2_len;
    }
    if (str3) {
        memcpy(tmp, str3, str3_len);
        tmp += str3_len;
    }
    if (str4) {
        memcpy(tmp, str4, str1_len);
    }
    return str;
}

char *str_dup(const char *str)
{
    return str_cat4(str, NULL, NULL, NULL);
}

bool str_eq(const char *str1, const char *str2)
{
    if (!str1 && !str2)
        return true;
    if (!str1 || !str2)
        return false;
    if (0 == strcmp(str1, str2))
        return true;
    return false;
}

bool str_ieq(const char *str1, const char *str2)
{
    if (!str1 && !str2)
        return true;
    if (!str1 || !str2)
        return false;
    if (0 == strcasecmp(str1, str2))
        return true;
    return false;
}

bool str_endswith(const char *txt, const char *end)
{
    size_t end_len;
    size_t txt_len;

    if (!txt || !end)
        return false;

    txt_len = strlen(txt);
    end_len = strlen(end);
    if (end_len > txt_len)
        return false;
    if (str_eq(txt+txt_len-end_len, end))
        return true;
    return false;
}

/* TODO: probably should move to some other file and change name to
   sleep_milliseconds */
void sleep_milliseconds(int milliseconds)
{
#ifdef _WIN32
    Sleep((DWORD)milliseconds);
#else
    struct timespec tv;
    int             secs, nanosecs;
    secs = milliseconds / 1000;
    nanosecs = (milliseconds - (secs * 1000)) * 1000;
    tv.tv_sec = (time_t) secs;
    tv.tv_nsec = (long) nanosecs;
    while (1)
    {
        int rval = nanosleep(&tv, &tv);
        if (rval == 0)
            /* Completed the entire sleep time; all done. */
            return;
        else if (errno == EINTR)
            /* Interrupted by a signal. Try again. */
            continue;
        else
            /* Some other error; bail out. */
            return;
    }
    return;
#endif
}

#ifndef _MSC_VER
void strcpy_s(char* dst, size_t dst_size, const char* src)
{
    size_t src_size = strlen(src) + 1;
    if (src_size <= dst_size)
        memcpy(dst, src, src_size);
    else {
        if (dst_size > 0) {
            memcpy(dst, src, dst_size);
            dst[dst_size-1] = 0;
        }
    }
}

void strcat_s(char *dst, size_t dst_size, const char* src)
{
    size_t dst_len = strlen(dst);
    if (dst_len >= dst_size) {
        if (dst_size > 0)
            dst[dst_size-1] = 0;
        return;
    }
    strcpy_s(dst+dst_len, dst_size - dst_len, src);
}
#endif

static SplashColorMode gSplashColorMode = splashModeBGR8;

static SplashColor splashColRed;
static SplashColor splashColGreen;
static SplashColor splashColBlue;
static SplashColor splashColWhite;
static SplashColor splashColBlack;

#define SPLASH_COL_RED_PTR (SplashColorPtr)&(splashColRed[0])
#define SPLASH_COL_GREEN_PTR (SplashColorPtr)&(splashColGreen[0])
#define SPLASH_COL_BLUE_PTR (SplashColorPtr)&(splashColBlue[0])
#define SPLASH_COL_WHITE_PTR (SplashColorPtr)&(splashColWhite[0])
#define SPLASH_COL_BLACK_PTR (SplashColorPtr)&(splashColBlack[0])

static SplashColorPtr  gBgColor = SPLASH_COL_WHITE_PTR;

static void splashColorSet(SplashColorPtr col, Guchar red, Guchar green, Guchar blue, Guchar alpha)
{
    switch (gSplashColorMode)
    {
        case splashModeBGR8:
            col[0] = blue;
            col[1] = green;
            col[2] = red;
            break;
        case splashModeRGB8:
            col[0] = red;
            col[1] = green;
            col[2] = blue;
            break;
        default:
            assert(0);
            break;
    }
}

void SplashColorsInit(void)
{
    splashColorSet(SPLASH_COL_RED_PTR, 0xff, 0, 0, 0);
    splashColorSet(SPLASH_COL_GREEN_PTR, 0, 0xff, 0, 0);
    splashColorSet(SPLASH_COL_BLUE_PTR, 0, 0, 0xff, 0);
    splashColorSet(SPLASH_COL_BLACK_PTR, 0, 0, 0, 0);
    splashColorSet(SPLASH_COL_WHITE_PTR, 0xff, 0xff, 0xff, 0);
}

PdfEnginePoppler::PdfEnginePoppler() : 
   _fileName(0)
   , _pageCount(INVALID_PAGE_NO) 
   , _pdfDoc(NULL)
   , _outputDev(NULL)
{
}

PdfEnginePoppler::~PdfEnginePoppler()
{
    free(_fileName);
    delete _outputDev;
    delete _pdfDoc;
}

bool PdfEnginePoppler::load(const char *fileName)
{
    setFileName(fileName);
    /* note: don't delete fileNameStr since PDFDoc takes ownership and deletes them itself */
    GooString *fileNameStr = new GooString(fileName);
    if (!fileNameStr) return false;

    _pdfDoc = new PDFDoc(fileNameStr, NULL, NULL, (void*)NULL);
    if (!_pdfDoc->isOk()) {
        return false;
    }
    _pageCount = _pdfDoc->getNumPages();
    return true;
}

SplashOutputDev * PdfEnginePoppler::outputDevice() {
    if (!_outputDev) {
        GBool bitmapTopDown = gTrue;
        _outputDev = new SplashOutputDev(gSplashColorMode, 4, gFalse, gBgColor, bitmapTopDown);
        if (_outputDev)
            _outputDev->startDoc(_pdfDoc->getXRef());
    }
    return _outputDev;
}

SplashBitmap *PdfEnginePoppler::renderBitmap(int pageNo, double zoomReal, int rotation)
{
    assert(outputDevice());
    if (!outputDevice()) return NULL;

    double hDPI = (double)PDF_FILE_DPI * zoomReal * 0.01;
    double vDPI = (double)PDF_FILE_DPI * zoomReal * 0.01;
    GBool  useMediaBox = gFalse;
    GBool  crop        = gTrue;
    GBool  doLinks     = gTrue;
    _pdfDoc->displayPage(_outputDev, pageNo, hDPI, vDPI, rotation, useMediaBox, 
        crop, doLinks, NULL, NULL);

    SplashBitmap* bmp = _outputDev->takeBitmap();
    return bmp;
}

struct FindFileState {
    char path[MAX_FILENAME_SIZE];
    char dirpath[MAX_FILENAME_SIZE]; /* current dir path */
    char pattern[MAX_FILENAME_SIZE]; /* search pattern */
    const char *bufptr;
#ifdef _WIN32
    WIN32_FIND_DATA fileinfo;
    HANDLE dir;
#else
    DIR *dir;
#endif
};

#ifdef _WIN32
#include <windows.h>
#include <sys/timeb.h>
#include <direct.h>

__inline char *getcwd(char *buffer, int maxlen)
{
    return _getcwd(buffer, maxlen);
}

int fnmatch(const char *pattern, const char *string, int flags)
{
    int prefix_len;
    const char *star_pos = strchr(pattern, '*');
    if (!star_pos)
        return strcmp(pattern, string) != 0;

    prefix_len = (int)(star_pos-pattern);
    if (0 == prefix_len)
        return 0;

    if (0 == _strnicmp(pattern, string, prefix_len))
        return 0;

    return 1;
}

#else
#include <fnmatch.h>
#endif

#ifdef _WIN32
/* on windows to query dirs we need foo\* to get files in this directory.
    foo\ always fails and foo will return just info about foo directory,
    not files in this directory */
static void win_correct_path_for_FindFirstFile(char *path, int path_max_len)
{
    int path_len = strlen(path);
    if (path_len >= path_max_len-4)
        return;
    if (DIR_SEP_CHAR != path[path_len])
        path[path_len++] = DIR_SEP_CHAR;
    path[path_len++] = '*';
    path[path_len] = 0;
}
#endif

FindFileState *find_file_open(const char *path, const char *pattern)
{
    FindFileState *s;

    s = (FindFileState*)malloc(sizeof(FindFileState));
    if (!s)
        return NULL;
    strcpy_s(s->path, sizeof(s->path), path);
    strcpy_s(s->dirpath, sizeof(s->path), path);
#ifdef _WIN32
    win_correct_path_for_FindFirstFile(s->path, sizeof(s->path));
#endif
    strcpy_s(s->pattern, sizeof(s->pattern), pattern);
    s->bufptr = s->path;
#ifdef _WIN32
    s->dir = INVALID_HANDLE_VALUE;
#else
    s->dir = NULL;
#endif
    return s;
}

#if 0 /* re-enable if we #define USE_OWN_GET_AUTH_DATA */
void *StandardSecurityHandler::getAuthData()
{
    return NULL;
}
#endif

char *makepath(char *buf, int buf_size, const char *path,
               const char *filename)
{
    strcpy_s(buf, buf_size, path);
    int len = strlen(path);
    if (len > 0 && path[len - 1] != DIR_SEP_CHAR && len + 1 < buf_size) {
        buf[len++] = DIR_SEP_CHAR;
        buf[len] = '\0';
    }
    strcat_s(buf, buf_size, filename);
    return buf;
}

#ifdef _WIN32
static int skip_matching_file(const char *filename)
{
    if (0 == strcmp(".", filename))
        return 1;
    if (0 == strcmp("..", filename))
        return 1;
    return 0;
}
#endif

int find_file_next(FindFileState *s, char *filename, int filename_size_max)
{
#ifdef _WIN32
    int    fFound;
    if (INVALID_HANDLE_VALUE == s->dir) {
        s->dir = FindFirstFile(s->path, &(s->fileinfo));
        if (INVALID_HANDLE_VALUE == s->dir)
            return -1;
        goto CheckFile;
    }

    while (1) {
        fFound = FindNextFile(s->dir, &(s->fileinfo));
        if (!fFound)
            return -1;
CheckFile:
        if (skip_matching_file(s->fileinfo.cFileName))
            continue;
        if (0 == fnmatch(s->pattern, s->fileinfo.cFileName, 0) ) {
            makepath(filename, filename_size_max, s->dirpath, s->fileinfo.cFileName);
            return 0;
        }
    }
#else
    struct dirent *dirent;
    const char *p;
    char *q;

    if (s->dir == NULL)
        goto redo;

    for (;;) {
        dirent = readdir(s->dir);
        if (dirent == NULL) {
        redo:
            if (s->dir) {
                closedir(s->dir);
                s->dir = NULL;
            }
            p = s->bufptr;
            if (*p == '\0')
                return -1;
            /* CG: get_str(&p, s->dirpath, sizeof(s->dirpath), ":") */
            q = s->dirpath;
            while (*p != ':' && *p != '\0') {
                if ((q - s->dirpath) < (int)sizeof(s->dirpath) - 1)
                    *q++ = *p;
                p++;
            }
            *q = '\0';
            if (*p == ':')
                p++;
            s->bufptr = p;
            s->dir = opendir(s->dirpath);
            if (!s->dir)
                goto redo;
        } else {
            if (fnmatch(s->pattern, dirent->d_name, 0) == 0) {
                makepath(filename, filename_size_max,
                         s->dirpath, dirent->d_name);
                return 0;
            }
        }
    }
#endif
}

void find_file_close(FindFileState *s)
{
#ifdef _WIN32
    if (INVALID_HANDLE_VALUE != s->dir)
       FindClose(s->dir);
#else
    if (s->dir)
        closedir(s->dir);
#endif
    free(s);
}

int StrList_Len(StrList **root)
{
    int         len = 0;
    StrList *   cur;
    assert(root);
    if (!root)
        return 0;
    cur = *root;
    while (cur) {
        ++len;
        cur = cur->next;
    }
    return len;
}

int StrList_InsertAndOwn(StrList **root, char *txt)
{
    StrList *   el;
    assert(root && txt);
    if (!root || !txt)
        return false;

    el = (StrList*)malloc(sizeof(StrList));
    if (!el)
        return false;
    el->str = txt;
    el->next = *root;
    *root = el;
    return true;
}

int StrList_Insert(StrList **root, char *txt)
{
    char *txtDup;

    assert(root && txt);
    if (!root || !txt)
        return false;
    txtDup = str_dup(txt);
    if (!txtDup)
        return false;

    if (!StrList_InsertAndOwn(root, txtDup)) {
        free((void*)txtDup);
        return false;
    }
    return true;
}

StrList* StrList_RemoveHead(StrList **root)
{
    StrList *tmp;
    assert(root);
    if (!root)
        return NULL;

    if (!*root)
        return NULL;
    tmp = *root;
    *root = tmp->next;
    tmp->next = NULL;
    return tmp;
}

void StrList_FreeElement(StrList *el)
{
    if (!el)
        return;
    free((void*)el->str);
    free((void*)el);
}

void StrList_Destroy(StrList **root)
{
    StrList *   cur;
    StrList *   next;

    if (!root)
        return;
    cur = *root;
    while (cur) {
        next = cur->next;
        StrList_FreeElement(cur);
        cur = next;
    }
    *root = NULL;
}

#ifndef _WIN32
void OutputDebugString(const char *txt)
{
    /* do nothing */
}
#define _snprintf snprintf
#define _vsnprintf vsnprintf
#endif

void my_error(int pos, char *msg, va_list args) {
#if 0
    char        buf[4096], *p = buf;

    // NB: this can be called before the globalParams object is created
    if (globalParams && globalParams->getErrQuiet()) {
        return;
    }

    if (pos >= 0) {
        p += _snprintf(p, sizeof(buf)-1, "Error (%d): ", pos);
        *p   = '\0';
        OutputDebugString(p);
    } else {
        OutputDebugString("Error: ");
    }

    p = buf;
    p += _vsnprintf(p, sizeof(buf) - 1, msg, args);
    while ( p > buf  &&  isspace(p[-1]) )
            *--p = '\0';
    *p++ = '\r';
    *p++ = '\n';
    *p   = '\0';
    OutputDebugString(buf);

    if (pos >= 0) {
        p += _snprintf(p, sizeof(buf)-1, "Error (%d): ", pos);
        *p   = '\0';
        OutputDebugString(buf);
        if (gErrFile)
            fprintf(gErrFile, buf);
    } else {
        OutputDebugString("Error: ");
        if (gErrFile)
            fprintf(gErrFile, "Error: ");
    }
#endif
#if 0
    p = buf;
    va_start(args, msg);
    p += _vsnprintf(p, sizeof(buf) - 3, msg, args);
    while ( p > buf  &&  isspace(p[-1]) )
            *--p = '\0';
    *p++ = '\r';
    *p++ = '\n';
    *p   = '\0';
    OutputDebugString(buf);
    if (gErrFile)
        fprintf(gErrFile, buf);
    va_end(args);
#endif
}

void LogInfo(char *fmt, ...)
{
    va_list args;
    char        buf[4096], *p = buf;

    p = buf;
    va_start(args, fmt);
    p += _vsnprintf(p, sizeof(buf) - 1, fmt, args);
    *p   = '\0';
    fprintf(gOutFile, "%s", buf);
    va_end(args);
    fflush(gOutFile);
}

static void PrintUsageAndExit(int argc, char **argv)
{
    printf("Usage: pdftest [-preview|-slowpreview] [-loadonly] [-timings] [-text] [-resolution NxM] [-recursive] [-page N] [-out out.txt] pdf-files-to-process\n");
    for (int i=0; i < argc; i++) {
        printf("i=%d, '%s'\n", i, argv[i]);
    }
    exit(0);
}

static bool ShowPreview(void)
{
    if (gfPreview || gfSlowPreview)
        return true;
    return false;
}

static void RenderPdfAsText(const char *fileName)
{
    GooString *         fileNameStr = NULL;
    PDFDoc *            pdfDoc = NULL;
    GooString *         txt = NULL;
    int                 pageCount;
    double              timeInMs;

    assert(fileName);
    if (!fileName)
        return;

    LogInfo("started: %s\n", fileName);

    TextOutputDev * textOut = new TextOutputDev(NULL, gTrue, gFalse, gFalse);
    if (!textOut->isOk()) {
        delete textOut;
        return;
    }

    GooTimer msTimer;
    /* note: don't delete fileNameStr since PDFDoc takes ownership and deletes them itself */
    fileNameStr = new GooString(fileName);
    if (!fileNameStr)
        goto Exit;

    pdfDoc = new PDFDoc(fileNameStr, NULL, NULL, NULL);
    if (!pdfDoc->isOk()) {
        error(-1, "RenderPdfFile(): failed to open PDF file %s\n", fileName);
        goto Exit;
    }

    msTimer.stop();
    timeInMs = msTimer.getElapsed();
    LogInfo("load: %.2f ms\n", timeInMs);

    pageCount = pdfDoc->getNumPages();
    LogInfo("page count: %d\n", pageCount);

    for (int curPage = 1; curPage <= pageCount; curPage++) {
        if ((gPageNo != PAGE_NO_NOT_GIVEN) && (gPageNo != curPage))
            continue;

        msTimer.start();
        int rotate = 0;
        GBool useMediaBox = gFalse;
        GBool crop = gTrue;
        GBool doLinks = gFalse;
        pdfDoc->displayPage(textOut, curPage, 72, 72, rotate, useMediaBox, crop, doLinks);
        txt = textOut->getText(0.0, 0.0, 10000.0, 10000.0);
        msTimer.stop();
        timeInMs = msTimer.getElapsed();
        if (gfTimings)
            LogInfo("page %d: %.2f ms\n", curPage, timeInMs);
        printf("%s\n", txt->getCString());
        delete txt;
        txt = NULL;
    }

Exit:
    LogInfo("finished: %s\n", fileName);
    delete textOut;
    delete pdfDoc;
}

#ifdef _MSC_VER
#define POPPLER_TMP_NAME "c:\\poppler_tmp.pdf"
#else
#define POPPLER_TMP_NAME "/tmp/poppler_tmp.pdf"
#endif

static void RenderPdf(const char *fileName)
{
    const char *        fileNameSplash = NULL;
    PdfEnginePoppler *  engineSplash = NULL;
    int                 pageCount;
    double              timeInMs;

#ifdef COPY_FILE
    // TODO: fails if file already exists and has read-only attribute
    CopyFile(fileName, POPPLER_TMP_NAME, false);
    fileNameSplash = POPPLER_TMP_NAME;
#else
    fileNameSplash = fileName;
#endif
    LogInfo("started: %s\n", fileName);

    engineSplash = new PdfEnginePoppler();

    GooTimer msTimer;
    if (!engineSplash->load(fileNameSplash)) {
        LogInfo("failed to load splash\n");
        goto Error;
    }
    msTimer.stop();
    timeInMs = msTimer.getElapsed();
    LogInfo("load splash: %.2f ms\n", timeInMs);
    pageCount = engineSplash->pageCount();

    LogInfo("page count: %d\n", pageCount);
    if (gfLoadOnly)
        goto Error;

    for (int curPage = 1; curPage <= pageCount; curPage++) {
        if ((gPageNo != PAGE_NO_NOT_GIVEN) && (gPageNo != curPage))
            continue;

        SplashBitmap *bmpSplash = NULL;

        GooTimer msTimer;
        bmpSplash = engineSplash->renderBitmap(curPage, 100.0, 0);
        msTimer.stop();
        double timeInMs = msTimer.getElapsed();
        if (gfTimings) {
            if (!bmpSplash)
                LogInfo("page splash %d: failed to render\n", curPage);
            else
                LogInfo("page splash %d (%dx%d): %.2f ms\n", curPage, bmpSplash->getWidth(), bmpSplash->getHeight(), timeInMs);
        }

        if (ShowPreview()) {
            PreviewBitmapSplash(bmpSplash);
            if (gfSlowPreview)
                sleep_milliseconds(SLOW_PREVIEW_TIME);
        }
        delete bmpSplash;
    }
Error:
    delete engineSplash;
    LogInfo("finished: %s\n", fileName);
}

static void RenderFile(const char *fileName)
{
    if (gfTextOnly) {
        RenderPdfAsText(fileName);
        return;
    }

    RenderPdf(fileName);
}

static bool ParseInteger(const char *start, const char *end, int *intOut)
{
    char            numBuf[16];
    int             digitsCount;
    const char *    tmp;

    assert(start && end && intOut);
    assert(end >= start);
    if (!start || !end || !intOut || (start > end))
        return false;

    digitsCount = 0;
    tmp = start;
    while (tmp <= end) {
        if (isspace(*tmp)) {
            /* do nothing, we allow whitespace */
        } else if (!isdigit(*tmp))
            return false;
        numBuf[digitsCount] = *tmp;
        ++digitsCount;
        if (digitsCount == dimof(numBuf)-3) /* -3 to be safe */
            return false;
        ++tmp;
    }
    if (0 == digitsCount)
        return false;
    numBuf[digitsCount] = 0;
    *intOut = atoi(numBuf);
    return true;
}

/* Given 'resolutionString' in format NxM (e.g. "100x200"), parse the string and put N
   into 'resolutionXOut' and M into 'resolutionYOut'.
   Return false if there was an error (e.g. string is not in the right format */
static bool ParseResolutionString(const char *resolutionString, int *resolutionXOut, int *resolutionYOut)
{
    const char *    posOfX;

    assert(resolutionString);
    assert(resolutionXOut);
    assert(resolutionYOut);
    if (!resolutionString || !resolutionXOut || !resolutionYOut)
        return false;
    *resolutionXOut = 0;
    *resolutionYOut = 0;
    posOfX = strchr(resolutionString, 'X');
    if (!posOfX)
        posOfX = strchr(resolutionString, 'x');
    if (!posOfX)
        return false;
    if (posOfX == resolutionString)
        return false;
    if (!ParseInteger(resolutionString, posOfX-1, resolutionXOut))
        return false;
    if (!ParseInteger(posOfX+1, resolutionString+strlen(resolutionString)-1, resolutionYOut))
        return false;
    return true;
}

static void ParseCommandLine(int argc, char **argv)
{
    char *      arg;

    if (argc < 2)
        PrintUsageAndExit(argc, argv);

    for (int i=1; i < argc; i++) {
        arg = argv[i];
        assert(arg);
        if ('-' == arg[0]) {
            if (str_ieq(arg, TIMINGS_ARG)) {
                gfTimings = true;
            } else if (str_ieq(arg, RESOLUTION_ARG)) {
                ++i;
                if (i == argc)
                    PrintUsageAndExit(argc, argv); /* expect a file name after that */
                if (!ParseResolutionString(argv[i], &gResolutionX, &gResolutionY))
                    PrintUsageAndExit(argc, argv);
                gfForceResolution = true;
            } else if (str_ieq(arg, RECURSIVE_ARG)) {
                gfRecursive = true;
            } else if (str_ieq(arg, OUT_ARG)) {
                /* expect a file name after that */
                ++i;
                if (i == argc)
                    PrintUsageAndExit(argc, argv);
                gOutFileName = str_dup(argv[i]);
            } else if (str_ieq(arg, PREVIEW_ARG)) {
                gfPreview = true;
            } else if (str_ieq(arg, TEXT_ARG)) {
                gfTextOnly = true;
            } else if (str_ieq(arg, SLOW_PREVIEW_ARG)) {
                gfSlowPreview = true;
            } else if (str_ieq(arg, LOAD_ONLY_ARG)) {
                gfLoadOnly = true;
            } else if (str_ieq(arg, PAGE_ARG)) {
                /* expect an integer after that */
                ++i;
                if (i == argc)
                    PrintUsageAndExit(argc, argv);
                gPageNo = atoi(argv[i]);
                if (gPageNo < 1)
                    PrintUsageAndExit(argc, argv);
            } else {
                /* unknown option */
                PrintUsageAndExit(argc, argv);
            }
        } else {
            /* we assume that this is not an option hence it must be
               a name of PDF/directory/file with PDF names */
            StrList_Insert(&gArgsListRoot, arg);
        }
    }
}

#if 0
void RenderFileList(char *pdfFileList)
{
    char *data = NULL;
    char *dataNormalized = NULL;
    char *pdfFileName;
    uint64_t fileSize;

    assert(pdfFileList);
    if (!pdfFileList)
        return;
    data = file_read_all(pdfFileList, &fileSize);
    if (!data) {
        error(-1, "couldn't load file '%s'", pdfFileList);
        return;
    }
    dataNormalized = str_normalize_newline(data, UNIX_NEWLINE);
    if (!dataNormalized) {
        error(-1, "couldn't normalize data of file '%s'", pdfFileList);
        goto Exit;
    }
    for (;;) {
        pdfFileName = str_split_iter(&dataNormalized, UNIX_NEWLINE_C);
        if (!pdfFileName)
            break;
        str_strip_ws_both(pdfFileName);
        if (str_empty(pdfFileName)) {
            free((void*)pdfFileName);
            continue;
        }
        RenderFile(pdfFileName);
        free((void*)pdfFileName);
    }
Exit:
    free((void*)dataNormalized);
    free((void*)data);
}
#endif

#ifdef _WIN32
#include <sys/types.h>
#include <sys/stat.h>

bool IsDirectoryName(char *path)
{
    struct _stat    buf;
    int             result;

    result = _stat(path, &buf );
    if (0 != result)
        return false;

    if (buf.st_mode & _S_IFDIR)
        return true;

    return false;
}

bool IsFileName(char *path)
{
    struct _stat    buf;
    int             result;

    result = _stat(path, &buf );
    if (0 != result)
        return false;

    if (buf.st_mode & _S_IFREG)
        return true;

    return false;
}
#else
bool IsDirectoryName(char *path)
{
    /* TODO: implement me */
    return false;
}

bool IsFileName(char *path)
{
    /* TODO: implement me */
    return true;
}
#endif

bool IsPdfFileName(char *path)
{
    if (str_endswith(path, ".pdf"))
        return true;
    return false;
}

static void RenderDirectory(char *path)
{
    FindFileState * ffs;
    char            filename[MAX_FILENAME_SIZE];
    StrList *       dirList = NULL;
    StrList *       el;

    StrList_Insert(&dirList, path);

    while (0 != StrList_Len(&dirList)) {
        el = StrList_RemoveHead(&dirList);
        ffs = find_file_open(el->str, "*");
        while (!find_file_next(ffs, filename, sizeof(filename))) {
            if (IsDirectoryName(filename)) {
                if (gfRecursive) {
                    StrList_Insert(&dirList, filename);
                }
            } else if (IsFileName(filename)) {
                if (IsPdfFileName(filename)) {
                    RenderFile(filename);
                }
            }
        }
        find_file_close(ffs);
        StrList_FreeElement(el);
    }
    StrList_Destroy(&dirList);
}

/* Render 'cmdLineArg', which can be:
   - directory name
   - name of PDF file
   - name of text file with names of PDF files
*/
static void RenderCmdLineArg(char *cmdLineArg)
{
    assert(cmdLineArg);
    if (!cmdLineArg)
        return;
    if (IsDirectoryName(cmdLineArg)) {
        RenderDirectory(cmdLineArg);
    } else if (IsFileName(cmdLineArg)) {
        if (IsPdfFileName(cmdLineArg))
            RenderFile(cmdLineArg);
#if 0
        else
            RenderFileList(cmdLineArg);
#endif
    } else {
        error(-1, "unexpected argument '%s'", cmdLineArg);
    }
}

int main(int argc, char **argv)
{
    setErrorFunction(my_error);
    ParseCommandLine(argc, argv);
    if (0 == StrList_Len(&gArgsListRoot))
        PrintUsageAndExit(argc, argv);
    assert(gArgsListRoot);

    SplashColorsInit();
    globalParams = new GlobalParams();
    if (!globalParams)
        return 1;
    globalParams->setErrQuiet(gFalse);
    globalParams->setBaseDir("");

    FILE * outFile = NULL;
    if (gOutFileName) {
        outFile = fopen(gOutFileName, "wb");
        if (!outFile) {
            printf("failed to open -out file %s\n", gOutFileName);
            return 1;
        }
        gOutFile = outFile;
    }
    else
        gOutFile = stdout;

    if (gOutFileName)
        gErrFile = outFile;
    else
        gErrFile = stderr;

    PreviewBitmapInit();

    StrList * curr = gArgsListRoot;
    while (curr) {
        RenderCmdLineArg(curr->str);
        curr = curr->next;
    }
    if (outFile)
        fclose(outFile);
    PreviewBitmapDestroy();
    StrList_Destroy(&gArgsListRoot);
    delete globalParams;
    free(gOutFileName);
    return 0;
}

