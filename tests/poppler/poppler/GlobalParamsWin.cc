/* Written by Krzysztof Kowalczyk (http://blog.kowalczyk.info)
   but mostly based on xpdf code.
   
   // Copyright (C) 2010, 2012 Hib Eris <hib@hiberis.nl>
   // Copyright (C) 2012, 2013 Thomas Freitag <Thomas.Freitag@alfa.de>
   // Copyright (C) 2012 Suzuki Toshiya <mpsuzuki@hiroshima-u.ac.jp>
   // Copyright (C) 2012 Adrian Johnson <ajohnson@redneon.com>
   // Copyright (C) 2012 Mark Brand <mabrand@mabrand.nl>
   // Copyright (C) 2013 Adam Reichold <adamreichold@myopera.com>

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
#include <sys/stat.h>
#include "Object.h"
#include "Stream.h"
#include "Lexer.h"
#include "Parser.h"

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
#define DEFAULT_CID_FONT_AC1_MSWIN "MingLiU"   /* Adobe-CNS1 for Taiwan, HongKong */
#define DEFAULT_CID_FONT_AG1_MSWIN "SimSun"    /* Adobe-GB1 for PRC, Singapore */
#define DEFAULT_CID_FONT_AJ1_MSWIN "MS-Mincho" /* Adobe-Japan1 */
#define DEFAULT_CID_FONT_AJ2_MSWIN "MS-Mincho" /* Adobe-Japan2 (legacy) */
#define DEFAULT_CID_FONT_AK1_MSWIN "Batang"    /* Adobe-Korea1 */
#define DEFAULT_CID_FONT_MSWIN "ArialUnicode"  /* Unknown */

static struct {
    const char *name;
    const char *t1FileName;
    const char *ttFileName;
    GBool warnIfMissing;
} displayFontTab[] = {
    {"Courier",               "n022003l.pfb", "cour.ttf", gTrue},
    {"Courier-Bold",          "n022004l.pfb", "courbd.ttf", gTrue},
    {"Courier-BoldOblique",   "n022024l.pfb", "courbi.ttf", gTrue},
    {"Courier-Oblique",       "n022023l.pfb", "couri.ttf", gTrue},
    {"Helvetica",             "n019003l.pfb", "arial.ttf", gTrue},
    {"Helvetica-Bold",        "n019004l.pfb", "arialbd.ttf", gTrue},
    {"Helvetica-BoldOblique", "n019024l.pfb", "arialbi.ttf", gTrue},
    {"Helvetica-Oblique",     "n019023l.pfb", "ariali.ttf", gTrue},
    // TODO: not sure if "symbol.ttf" is right
    {"Symbol",                "s050000l.pfb", "symbol.ttf", gTrue},
    {"Times-Bold",            "n021004l.pfb", "timesbd.ttf", gTrue},
    {"Times-BoldItalic",      "n021024l.pfb", "timesbi.ttf", gTrue},
    {"Times-Italic",          "n021023l.pfb", "timesi.ttf", gTrue},
    {"Times-Roman",           "n021003l.pfb", "times.ttf", gTrue},
    // TODO: not sure if "wingding.ttf" is right
    {"ZapfDingbats",          "d050000l.pfb", "wingding.ttf", gTrue},

    // those seem to be frequently accessed by PDF files and I kind of guess
    // which font file do the refer to
    {"Palatino", NULL, "pala.ttf", gTrue},
    {"Palatino-Roman", NULL, "pala.ttf", gTrue},
    {"Palatino-Bold", NULL, "palab.ttf", gTrue},
    {"Palatino-Italic", NULL, "palai.ttf", gTrue},
    {"Palatino,Italic", NULL, "palai.ttf", gTrue},
    {"Palatino-BoldItalic", NULL, "palabi.ttf", gTrue},

    {"ArialBlack",        NULL, "arialbd.ttf", gTrue},

    {"ArialNarrow", NULL, "arialn.ttf", gTrue},
    {"ArialNarrow,Bold", NULL, "arialnb.ttf", gTrue},
    {"ArialNarrow,Italic", NULL, "arialni.ttf", gTrue},
    {"ArialNarrow,BoldItalic", NULL, "arialnbi.ttf", gTrue},
    {"ArialNarrow-Bold", NULL, "arialnb.ttf", gTrue},
    {"ArialNarrow-Italic", NULL, "arialni.ttf", gTrue},
    {"ArialNarrow-BoldItalic", NULL, "arialnbi.ttf", gTrue},

    {"HelveticaNarrow", NULL, "arialn.ttf", gTrue},
    {"HelveticaNarrow,Bold", NULL, "arialnb.ttf", gTrue},
    {"HelveticaNarrow,Italic", NULL, "arialni.ttf", gTrue},
    {"HelveticaNarrow,BoldItalic", NULL, "arialnbi.ttf", gTrue},
    {"HelveticaNarrow-Bold", NULL, "arialnb.ttf", gTrue},
    {"HelveticaNarrow-Italic", NULL, "arialni.ttf", gTrue},
    {"HelveticaNarrow-BoldItalic", NULL, "arialnbi.ttf", gTrue},

    {"BookAntiqua", NULL, "bkant.ttf", gTrue},
    {"BookAntiqua,Bold", NULL, "bkant.ttf", gTrue},
    {"BookAntiqua,Italic", NULL, "bkant.ttf", gTrue},
    {"BookAntiqua,BoldItalic", NULL, "bkant.ttf", gTrue},
    {"BookAntiqua-Bold", NULL, "bkant.ttf", gTrue},
    {"BookAntiqua-Italic", NULL, "bkant.ttf", gTrue},
    {"BookAntiqua-BoldItalic", NULL, "bkant.ttf", gTrue},

    {"Verdana", NULL, "verdana.ttf", gTrue},
    {"Verdana,Bold", NULL, "verdanab.ttf", gTrue},
    {"Verdana,Italic", NULL, "verdanai.ttf", gTrue},
    {"Verdana,BoldItalic", NULL, "verdanaz.ttf", gTrue},
    {"Verdana-Bold", NULL, "verdanab.ttf", gTrue},
    {"Verdana-Italic", NULL, "verdanai.ttf", gTrue},
    {"Verdana-BoldItalic", NULL, "verdanaz.ttf", gTrue},

    {"Tahoma", NULL, "tahoma.ttf", gTrue},
    {"Tahoma,Bold", NULL, "tahomabd.ttf", gTrue},
    {"Tahoma,Italic", NULL, "tahoma.ttf", gTrue},
    {"Tahoma,BoldItalic", NULL, "tahomabd.ttf", gTrue},
    {"Tahoma-Bold", NULL, "tahomabd.ttf", gTrue},
    {"Tahoma-Italic", NULL, "tahoma.ttf", gTrue},
    {"Tahoma-BoldItalic", NULL, "tahomabd.ttf", gTrue},

    {"CCRIKH+Verdana", NULL, "verdana.ttf", gTrue},
    {"CCRIKH+Verdana,Bold", NULL, "verdanab.ttf", gTrue},
    {"CCRIKH+Verdana,Italic", NULL, "verdanai.ttf", gTrue},
    {"CCRIKH+Verdana,BoldItalic", NULL, "verdanaz.ttf", gTrue},
    {"CCRIKH+Verdana-Bold", NULL, "verdanab.ttf", gTrue},
    {"CCRIKH+Verdana-Italic", NULL, "verdanai.ttf", gTrue},
    {"CCRIKH+Verdana-BoldItalic", NULL, "verdanaz.ttf", gTrue},

    {"Georgia", NULL, "georgia.ttf", gTrue},
    {"Georgia,Bold", NULL, "georgiab.ttf", gTrue},
    {"Georgia,Italic", NULL, "georgiai.ttf", gTrue},
    {"Georgia,BoldItalic", NULL, "georgiaz.ttf", gTrue},
    {"Georgia-Bold", NULL, "georgiab.ttf", gTrue},
    {"Georgia-Italic", NULL, "georgiai.ttf", gTrue},
    {"Georgia-BoldItalic", NULL, "georgiaz.ttf", gTrue},

    // fallback for Adobe CID fonts:
    {"MingLiU", NULL, "mingliu.ttf", gFalse},
    {"SimSun", NULL, "simsun.ttf", gFalse},
    {"MS-Mincho", NULL, "msmincho.ttf", gFalse},
    {"Batang", NULL, "batang.ttf", gFalse},
    {"ArialUnicode", NULL, "arialuni.ttf", gTrue},
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

void SysFontList::scanWindowsFonts(GooString *winFontDir) {
  OSVERSIONINFO version;
  const char *path;
  DWORD idx, valNameLen, dataLen, type;
  HKEY regKey;
  char valName[1024], data[1024];
  int n, fontNum;
  char *p0, *p1;
  GooString *fontPath;

  version.dwOSVersionInfoSize = sizeof(version);
  GetVersionEx(&version);
  if (version.dwPlatformId == VER_PLATFORM_WIN32_NT) {
    path = "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Fonts\\";
  } else {
    path = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Fonts\\";
  }
  if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, path, 0,
		   KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS,
		   &regKey) == ERROR_SUCCESS) {
    idx = 0;
    while (1) {
      valNameLen = sizeof(valName) - 1;
      dataLen = sizeof(data) - 1;
      if (RegEnumValue(regKey, idx, valName, &valNameLen, NULL,
		       &type, (LPBYTE)data, &dataLen) != ERROR_SUCCESS) {
	break;
      }
      if (type == REG_SZ &&
	  valNameLen > 0 && valNameLen < sizeof(valName) &&
	  dataLen > 0 && dataLen < sizeof(data)) {
	valName[valNameLen] = '\0';
	data[dataLen] = '\0';
	n = strlen(data);
	if (!strcasecmp(data + n - 4, ".ttf") ||
	    !strcasecmp(data + n - 4, ".ttc") ||
	    !strcasecmp(data + n - 4, ".otf")) {
	  fontPath = new GooString(data);
	  if (!(dataLen >= 3 && data[1] == ':' && data[2] == '\\')) {
	    fontPath->insert(0, '\\');
	    fontPath->insert(0, winFontDir);
		fontPath->append('\0');
	  }
	  p0 = valName;
	  fontNum = 0;
	  while (*p0) {
	    p1 = strstr(p0, " & ");
	    if (p1) {
	      *p1 = '\0';
	      p1 = p1 + 3;
	    } else {
	      p1 = p0 + strlen(p0);
	    }
	    fonts->append(makeWindowsFont(p0, fontNum,
					  fontPath->getCString()));
	    p0 = p1;
	    ++fontNum;
	  }
	  delete fontPath;
	}
      }
      ++idx;
    }
    RegCloseKey(regKey);
  }
}

SysFontInfo *SysFontList::makeWindowsFont(char *name, int fontNum,
					  char *path) {
  int n;
  GBool bold, italic, oblique, fixedWidth;
  GooString *s;
  char c;
  int i;
  SysFontType type;
  GooString substituteName;

  n = strlen(name);
  bold = italic = oblique = fixedWidth = gFalse;

  // remove trailing ' (TrueType)'
  if (n > 11 && !strncmp(name + n - 11, " (TrueType)", 11)) {
    n -= 11;
  }

  // remove trailing ' (OpenType)'
  if (n > 11 && !strncmp(name + n - 11, " (OpenType)", 11)) {
    n -= 11;
  }

  // remove trailing ' Italic'
  if (n > 7 && !strncmp(name + n - 7, " Italic", 7)) {
    n -= 7;
    italic = gTrue;
  }

  // remove trailing ' Oblique'
  if (n > 7 && !strncmp(name + n - 8, " Oblique", 8)) {
    n -= 8;
    oblique = gTrue;
  }

  // remove trailing ' Bold'
  if (n > 5 && !strncmp(name + n - 5, " Bold", 5)) {
    n -= 5;
    bold = gTrue;
  }

  // remove trailing ' Regular'
  if (n > 5 && !strncmp(name + n - 8, " Regular", 8)) {
    n -= 8;
  }

  // the familyname cannot indicate whether a font is fixedWidth or not.
  // some well-known fixedWidth typeface family names or keyword are checked.
  if ( strstr(name, "Courier") ||
       strstr(name, "Fixed")   ||
      (strstr(name, "Mono") && !strstr(name, "Monotype")) ||
       strstr(name, "Typewriter") )
    fixedWidth = gTrue;
  else
    fixedWidth = gFalse;


  //----- normalize the font name
  s = new GooString(name, n);
  i = 0;
  while (i < s->getLength()) {
    c = s->getChar(i);
    if (c == ' ' || c == ',' || c == '-') {
      s->del(i);
    } else {
      ++i;
    }
  }

  if (!strcasecmp(path + strlen(path) - 4, ".ttc")) {
    type = sysFontTTC;
  } else {
    type = sysFontTTF;
  }

  return new SysFontInfo(s, bold, italic, oblique, fixedWidth,
                         new GooString(path), type, fontNum, substituteName.copy());
}

static GooString* replaceSuffix(GooString *path,
                                const char* suffixA, const char* suffixB)
{
  int suffLenA = strlen(suffixA);
  int suffLenB = strlen(suffixB);
  int baseLenA = path->getLength() - suffLenA;
  int baseLenB = path->getLength() - suffLenB;

  if (!strcasecmp(path->getCString()+baseLenA, suffixA)) {
    path->del(baseLenA,suffLenA)->append(suffixB);
  } else if (!strcasecmp(path->getCString()+baseLenB, suffixB)) {
    path->del(baseLenB,suffLenB)->append(suffixA);
  }

  return path;
}


void GlobalParams::setupBaseFonts(char * dir)
{
    const char *dataRoot = popplerDataDir ? popplerDataDir : POPPLER_DATADIR;
    GooString *fileName = NULL;
    GooFile *file;

    if (baseFontsInitialized)
        return;
    baseFontsInitialized = true;

    char winFontDir[MAX_PATH];
    GetWindowsFontDir(winFontDir, sizeof(winFontDir));

    for (int i = 0; displayFontTab[i].name; ++i) {
        GooString  *fontName = new GooString(displayFontTab[i].name);
        if (fontFiles->lookup(fontName))
            continue;

        if (dir) {
            GooString *fontPath = appendToPath(new GooString(dir), displayFontTab[i].t1FileName);
            if (FileExists(fontPath->getCString()) ||
                FileExists(replaceSuffix(fontPath, ".pfb", ".pfa")->getCString())) {
                addFontFile(fontName, fontPath);
                continue;
            }
            delete fontPath;
        }

        if (winFontDir[0] && displayFontTab[i].ttFileName) {
            GooString *fontPath = appendToPath(new GooString(winFontDir), displayFontTab[i].ttFileName);
            if (FileExists(fontPath->getCString()) ||
                FileExists(replaceSuffix(fontPath, ".ttc", ".ttf")->getCString())) {
                addFontFile(fontName, fontPath);
                continue;
            }
            delete fontPath;
        }

        if (displayFontTab[i].warnIfMissing)
          error(errSyntaxError, -1, "No display font for '{0:s}'", fontName);
    }
    if (winFontDir[0]) {
      sysFonts->scanWindowsFonts(new GooString(winFontDir));
    }

    fileName = new GooString(dataRoot);
    fileName->append("/cidfmap");

    // try to open file
    file = GooFile::open(fileName);

    if (file != NULL) {
      Parser *parser;
      Object obj1, obj2;

      obj1.initNull();
      parser = new Parser(NULL,
	      new Lexer(NULL,
	      new FileStream(file, 0, gFalse, file->size(), &obj1)),
	      gTrue);
      obj1.free();
      parser->getObj(&obj1);
      while (!obj1.isEOF()) {
	parser->getObj(&obj2);
	if (obj1.isName()) {
	  // Substitutions
	  if (obj2.isDict()) {
	    Object obj3;
	    obj2.getDict()->lookup("Path", &obj3);
	    if (obj3.isString())
	      addFontFile(new GooString(obj1.getName()), obj3.getString()->copy());
	    obj3.free();
	  // Aliases
	  } else if (obj2.isName()) {
	    substFiles->add(new GooString(obj1.getName()), new GooString(obj2.getName()));
	  }
	}
	obj2.free();
	obj1.free();
	parser->getObj(&obj1);
	// skip trailing ';'
	while (obj1.isCmd(";")) {
	  obj1.free();
	  parser->getObj(&obj1);
	}
      }
      delete file;
      delete parser;
    }
}

static const char *findSubstituteName(GfxFont *font, GooHash *fontFiles,
                                      GooHash *substFiles,
                                      const char *origName)
{
    assert(origName);
    if (!origName) return NULL;
    GooString *name2 = new GooString(origName);
    int n = strlen(origName);
    // remove trailing "-Identity-H"
    if (n > 11 && !strcmp(name2->getCString() + n - 11, "-Identity-H")) {
      name2->del(n - 11, 11);
      n -= 11;
    }
    // remove trailing "-Identity-V"
    if (n > 11 && !strcmp(name2->getCString() + n - 11, "-Identity-V")) {
      name2->del(n - 11, 11);
      n -= 11;
    }
    GooString *substName = (GooString *)substFiles->lookup(name2);
    if (substName != NULL) {
      delete name2;
      return substName->getCString();
    }

    /* TODO: try to at least guess bold/italic/bolditalic from the name */
    delete name2;
    if (font->isCIDFont()) {
      GooString *collection = ((GfxCIDFont *)font)->getCollection();

      const char* name3 = NULL;
      if ( !collection->cmp("Adobe-CNS1") )
        name3 = DEFAULT_CID_FONT_AC1_MSWIN;
      else if ( !collection->cmp("Adobe-GB1") )
        name3 = DEFAULT_CID_FONT_AG1_MSWIN;
      else if ( !collection->cmp("Adobe-Japan1") )
        name3 = DEFAULT_CID_FONT_AJ1_MSWIN;
      else if ( !collection->cmp("Adobe-Japan2") )
        name3 = DEFAULT_CID_FONT_AJ2_MSWIN;
      else if ( !collection->cmp("Adobe-Korea1") )
        name3 = DEFAULT_CID_FONT_AK1_MSWIN;

      if (name3 && fontFiles->lookup(name3))
        return name3;

      if (fontFiles->lookup(DEFAULT_CID_FONT_MSWIN))
        return DEFAULT_CID_FONT_MSWIN;
    } 
    return DEFAULT_SUBSTITUTE_FONT;
}

/* Windows implementation of external font matching code */
GooString *GlobalParams::findSystemFontFile(GfxFont *font,
					  SysFontType *type,
					  int *fontNum, GooString *substituteFontName,
                                          GooString *base14Name) {
  SysFontInfo *fi;
  GooString *path = NULL;
  GooString *fontName = font->getName();
  if (!fontName) return NULL;
  lockGlobalParams;
  setupBaseFonts(NULL);

  // TODO: base14Name should be changed?
  // In the system using FontConfig, findSystemFontFile() uses
  // base14Name only for the creation of query pattern.

  if ((fi = sysFonts->find(fontName, gFalse, gFalse))) {
    path = fi->path->copy();
    *type = fi->type;
    *fontNum = fi->fontNum;
    if (substituteFontName)
      substituteFontName->Set(fi->substituteName->getCString());
  } else {
    GooString *substFontName = new GooString(findSubstituteName(font, fontFiles,
                                                                substFiles,
                                                                fontName->getCString()));
    GooString *path2 = NULL;
    error(errSyntaxError, -1, "Couldn't find a font for '{0:t}', subst is '{1:t}'", fontName, substFontName);
    if ((path2 = (GooString *)fontFiles->lookup(substFontName))) {
      path = new GooString(path2);
      if (substituteFontName)
	substituteFontName->Set(path->getCString());
      if (!strcasecmp(path->getCString() + path->getLength() - 4, ".ttc")) {
	*type = sysFontTTC;
      } else {
	*type = sysFontTTF;
      }
      *fontNum = 0;
    }
  }
  unlockGlobalParams;
  return path;
}
