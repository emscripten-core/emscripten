/* Written by Krzysztof Kowalczyk (http://blog.kowalczyk.info)
   but mostly based on xpdf code.
   
   // Copyright (C) 2010 Hib Eris <hib@hiberis.nl>

TODO: instead of a fixed mapping defined in displayFontTab, it could
scan the whole fonts directory, parse TTF files and build font
description for all fonts available in Windows. That's how MuPDF works.
*/

#ifndef PACKAGE_NAME
#include <config.h>
#endif

#ifdef USE_GCC_PRAGMAS
#pragma implementation
#endif

#include <windows.h>
#if !(_WIN32_IE >= 0x0500)
#error "_WIN32_IE must be defined >= 0x0500 for SHGFP_TYPE_CURRENT from shlobj.h"
#endif
#include <shlobj.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <assert.h>

#include "goo/gmem.h"
#include "goo/GooString.h"
#include "goo/GooList.h"
#include "goo/GooHash.h"
#include "goo/gfile.h"
#include "Error.h"
#include "NameToCharCode.h"
#include "CharCodeToUnicode.h"
#include "UnicodeMap.h"
#include "CMap.h"
#include "BuiltinFontTables.h"
#include "FontEncodingTables.h"
#include "GlobalParams.h"
#include "GfxFont.h"

#if MULTITHREADED
#  define lockGlobalParams            gLockMutex(&mutex)
#  define lockUnicodeMapCache         gLockMutex(&unicodeMapCacheMutex)
#  define lockCMapCache               gLockMutex(&cMapCacheMutex)
#  define unlockGlobalParams          gUnlockMutex(&mutex)
#  define unlockUnicodeMapCache       gUnlockMutex(&unicodeMapCacheMutex)
#  define unlockCMapCache             gUnlockMutex(&cMapCacheMutex)
#else
#  define lockGlobalParams
#  define lockUnicodeMapCache
#  define lockCMapCache
#  define unlockGlobalParams
#  define unlockUnicodeMapCache
#  define unlockCMapCache
#endif

#define DEFAULT_SUBSTITUTE_FONT "Helvetica"

static struct {
    char *name;
    char *t1FileName;
    char *ttFileName;
} displayFontTab[] = {
    {"Courier",               "n022003l.pfb", "cour.ttf"},
    {"Courier-Bold",          "n022004l.pfb", "courbd.ttf"},
    {"Courier-BoldOblique",   "n022024l.pfb", "courbi.ttf"},
    {"Courier-Oblique",       "n022023l.pfb", "couri.ttf"},
    {"Helvetica",             "n019003l.pfb", "arial.ttf"},
    {"Helvetica-Bold",        "n019004l.pfb", "arialbd.ttf"},
    {"Helvetica-BoldOblique", "n019024l.pfb", "arialbi.ttf"},
    {"Helvetica-Oblique",     "n019023l.pfb", "ariali.ttf"},
    // TODO: not sure if "symbol.ttf" is right
    {"Symbol",                "s050000l.pfb", "symbol.ttf"},
    {"Times-Bold",            "n021004l.pfb", "timesbd.ttf"},
    {"Times-BoldItalic",      "n021024l.pfb", "timesbi.ttf"},
    {"Times-Italic",          "n021023l.pfb", "timesi.ttf"},
    {"Times-Roman",           "n021003l.pfb", "times.ttf"},
    // TODO: not sure if "wingding.ttf" is right
    {"ZapfDingbats",          "d050000l.pfb", "wingding.ttf"},

    // those seem to be frequently accessed by PDF files and I kind of guess
    // which font file do the refer to
    {"Palatino", NULL, "pala.ttf"},
    {"Palatino-Roman", NULL, "pala.ttf"},
    {"Palatino-Bold", NULL, "palab.ttf"},
    {"Palatino-Italic", NULL, "palai.ttf"},
    {"Palatino,Italic", NULL, "palai.ttf"},
    {"Palatino-BoldItalic", NULL, "palabi.ttf"},

    {"ArialBlack",        NULL, "arialbd.ttf"},

    {"ArialNarrow", NULL, "arialn.ttf"},
    {"ArialNarrow,Bold", NULL, "arialnb.ttf"},
    {"ArialNarrow,Italic", NULL, "arialni.ttf"},
    {"ArialNarrow,BoldItalic", NULL, "arialnbi.ttf"},
    {"ArialNarrow-Bold", NULL, "arialnb.ttf"},
    {"ArialNarrow-Italic", NULL, "arialni.ttf"},
    {"ArialNarrow-BoldItalic", NULL, "arialnbi.ttf"},

    {"HelveticaNarrow", NULL, "arialn.ttf"},
    {"HelveticaNarrow,Bold", NULL, "arialnb.ttf"},
    {"HelveticaNarrow,Italic", NULL, "arialni.ttf"},
    {"HelveticaNarrow,BoldItalic", NULL, "arialnbi.ttf"},
    {"HelveticaNarrow-Bold", NULL, "arialnb.ttf"},
    {"HelveticaNarrow-Italic", NULL, "arialni.ttf"},
    {"HelveticaNarrow-BoldItalic", NULL, "arialnbi.ttf"},

    {"BookAntiqua", NULL, "bkant.ttf"},
    {"BookAntiqua,Bold", NULL, "bkant.ttf"},
    {"BookAntiqua,Italic", NULL, "bkant.ttf"},
    {"BookAntiqua,BoldItalic", NULL, "bkant.ttf"},
    {"BookAntiqua-Bold", NULL, "bkant.ttf"},
    {"BookAntiqua-Italic", NULL, "bkant.ttf"},
    {"BookAntiqua-BoldItalic", NULL, "bkant.ttf"},

    {"Verdana", NULL, "verdana.ttf"},
    {"Verdana,Bold", NULL, "verdanab.ttf"},
    {"Verdana,Italic", NULL, "verdanai.ttf"},
    {"Verdana,BoldItalic", NULL, "verdanaz.ttf"},
    {"Verdana-Bold", NULL, "verdanab.ttf"},
    {"Verdana-Italic", NULL, "verdanai.ttf"},
    {"Verdana-BoldItalic", NULL, "verdanaz.ttf"},

    {"Tahoma", NULL, "tahoma.ttf"},
    {"Tahoma,Bold", NULL, "tahomabd.ttf"},
    {"Tahoma,Italic", NULL, "tahoma.ttf"},
    {"Tahoma,BoldItalic", NULL, "tahomabd.ttf"},
    {"Tahoma-Bold", NULL, "tahomabd.ttf"},
    {"Tahoma-Italic", NULL, "tahoma.ttf"},
    {"Tahoma-BoldItalic", NULL, "tahomabd.ttf"},

    {"CCRIKH+Verdana", NULL, "verdana.ttf"},
    {"CCRIKH+Verdana,Bold", NULL, "verdanab.ttf"},
    {"CCRIKH+Verdana,Italic", NULL, "verdanai.ttf"},
    {"CCRIKH+Verdana,BoldItalic", NULL, "verdanaz.ttf"},
    {"CCRIKH+Verdana-Bold", NULL, "verdanab.ttf"},
    {"CCRIKH+Verdana-Italic", NULL, "verdanai.ttf"},
    {"CCRIKH+Verdana-BoldItalic", NULL, "verdanaz.ttf"},

    {"Georgia", NULL, "georgia.ttf"},
    {"Georgia,Bold", NULL, "georgiab.ttf"},
    {"Georgia,Italic", NULL, "georgiai.ttf"},
    {"Georgia,BoldItalic", NULL, "georgiaz.ttf"},
    {"Georgia-Bold", NULL, "georgiab.ttf"},
    {"Georgia-Italic", NULL, "georgiai.ttf"},
    {"Georgia-BoldItalic", NULL, "georgiaz.ttf"},

    {NULL}
};

#define FONTS_SUBDIR "\\fonts"

static void GetWindowsFontDir(char *winFontDir, int cbWinFontDirLen)
{
    BOOL (__stdcall *SHGetSpecialFolderPathFunc)(HWND  hwndOwner,
                                                  LPTSTR lpszPath,
                                                  int    nFolder,
                                                  BOOL  fCreate);
    HRESULT (__stdcall *SHGetFolderPathFunc)(HWND  hwndOwner,
                                              int    nFolder,
                                              HANDLE hToken,
                                              DWORD  dwFlags,
                                              LPTSTR pszPath);

    // SHGetSpecialFolderPath isn't available in older versions of shell32.dll (Win95 and
    // WinNT4), so do a dynamic load of ANSI versions.
    winFontDir[0] = '\0';

    HMODULE hLib = LoadLibrary("shell32.dll");
    if (hLib) {
        SHGetFolderPathFunc = (HRESULT (__stdcall *)(HWND, int, HANDLE, DWORD, LPTSTR)) 
                              GetProcAddress(hLib, "SHGetFolderPathA");
        if (SHGetFolderPathFunc)
            (*SHGetFolderPathFunc)(NULL, CSIDL_FONTS, NULL, SHGFP_TYPE_CURRENT, winFontDir);

        if (!winFontDir[0]) {
            // Try an older function
            SHGetSpecialFolderPathFunc = (BOOL (__stdcall *)(HWND, LPTSTR, int, BOOL))
                                          GetProcAddress(hLib, "SHGetSpecialFolderPathA");
            if (SHGetSpecialFolderPathFunc)
                (*SHGetSpecialFolderPathFunc)(NULL, winFontDir, CSIDL_FONTS, FALSE);
        }
        FreeLibrary(hLib);
    }
    if (winFontDir[0])
        return;

    // Try older DLL
    hLib = LoadLibrary("SHFolder.dll");
    if (hLib) {
        SHGetFolderPathFunc = (HRESULT (__stdcall *)(HWND, int, HANDLE, DWORD, LPTSTR))
                              GetProcAddress(hLib, "SHGetFolderPathA");
        if (SHGetFolderPathFunc)
            (*SHGetFolderPathFunc)(NULL, CSIDL_FONTS, NULL, SHGFP_TYPE_CURRENT, winFontDir);
        FreeLibrary(hLib);
    }
    if (winFontDir[0])
        return;

    // Everything else failed so the standard fonts directory.
    GetWindowsDirectory(winFontDir, cbWinFontDirLen);                                                       
    if (winFontDir[0]) {
        strncat(winFontDir, FONTS_SUBDIR, cbWinFontDirLen);
        winFontDir[cbWinFontDirLen-1] = 0;
    }
}

static bool FileExists(const char *path)
{
    FILE * f = fopen(path, "rb");
    if (f) {
        fclose(f);
        return true;
    }
    return false;
}

static void AddFont(GooHash *displayFonts, char *fontName, GooString *fontPath, DisplayFontParamKind kind)
{
    DisplayFontParam *dfp = new DisplayFontParam(new GooString(fontName), kind);
    dfp->setFileName(fontPath);
    displayFonts->add(dfp->name, dfp);
}

void GlobalParams::setupBaseFonts(char * dir)
{
    if (baseFontsInitialized)
        return;
    baseFontsInitialized = true;

    char winFontDir[MAX_PATH];
    GetWindowsFontDir(winFontDir, sizeof(winFontDir));

    for (int i = 0; displayFontTab[i].name; ++i) {
        char *fontName = displayFontTab[i].name;
        if (displayFonts->lookup(fontName))
            continue;

        if (dir) {
            GooString *fontPath = appendToPath(new GooString(dir), displayFontTab[i].t1FileName);
            if (FileExists(fontPath->getCString())) {
                AddFont(displayFonts, fontName, fontPath, displayFontT1);
                continue;
            }
            delete fontPath;
        }

        if (winFontDir[0] && displayFontTab[i].ttFileName) {
            GooString *fontPath = appendToPath(new GooString(winFontDir), displayFontTab[i].ttFileName);
            if (FileExists(fontPath->getCString())) {
                AddFont(displayFonts, fontName, fontPath, displayFontTT);
                continue;
            }
            delete fontPath;
        }

        error(-1, "No display font for '%s'", fontName);
    }
}

static char *findSubstituteName(const char *origName)
{
    assert(origName);
    if (!origName) return NULL;
    /* TODO: try to at least guess bold/italic/bolditalic from the name */
    return DEFAULT_SUBSTITUTE_FONT;
}

/* Windows implementation of external font matching code */
DisplayFontParam *GlobalParams::getDisplayFont(GfxFont *font) {
    DisplayFontParam *  dfp;
    GooString *         fontName = font->getName();
    char *              substFontName = NULL;

    if (!fontName) return NULL;
    lockGlobalParams;
    setupBaseFonts(NULL);
    dfp = (DisplayFontParam *)displayFonts->lookup(fontName);
    if (!dfp) {
        substFontName = findSubstituteName(fontName->getCString());
        error(-1, "Couldn't find a font for '%s', subst is '%s'", fontName->getCString(), substFontName);
        dfp = (DisplayFontParam *)displayFonts->lookup(substFontName);
        assert(dfp);
    }
    unlockGlobalParams;
    return dfp;
}

