/*
** $Id: loadlib.c,v 1.111 2012/05/30 12:33:44 roberto Exp $
** Dynamic library loader for Lua
** See Copyright Notice in lua.h
**
** This module contains an implementation of loadlib for Unix systems
** that have dlfcn, an implementation for Windows, and a stub for other
** systems.
*/


/*
** if needed, includes windows header before everything else
*/
#if defined(_WIN32)
#include <windows.h>
#endif


#include <stdlib.h>
#include <string.h>


#define loadlib_c
#define LUA_LIB

#include "lua.h"

#include "lauxlib.h"
#include "lualib.h"


/*
** LUA_PATH and LUA_CPATH are the names of the environment
** variables that Lua check to set its paths.
*/
#if !defined(LUA_PATH)
#define LUA_PATH	"LUA_PATH"
#endif

#if !defined(LUA_CPATH)
#define LUA_CPATH	"LUA_CPATH"
#endif

#define LUA_PATHSUFFIX		"_" LUA_VERSION_MAJOR "_" LUA_VERSION_MINOR

#define LUA_PATHVERSION		LUA_PATH LUA_PATHSUFFIX
#define LUA_CPATHVERSION	LUA_CPATH LUA_PATHSUFFIX

/*
** LUA_PATH_SEP is the character that separates templates in a path.
** LUA_PATH_MARK is the string that marks the substitution points in a
** template.
** LUA_EXEC_DIR in a Windows path is replaced by the executable's
** directory.
** LUA_IGMARK is a mark to ignore all before it when building the
** luaopen_ function name.
*/
#if !defined (LUA_PATH_SEP)
#define LUA_PATH_SEP		";"
#endif
#if !defined (LUA_PATH_MARK)
#define LUA_PATH_MARK		"?"
#endif
#if !defined (LUA_EXEC_DIR)
#define LUA_EXEC_DIR		"!"
#endif
#if !defined (LUA_IGMARK)
#define LUA_IGMARK		"-"
#endif


/*
** LUA_CSUBSEP is the character that replaces dots in submodule names
** when searching for a C loader.
** LUA_LSUBSEP is the character that replaces dots in submodule names
** when searching for a Lua loader.
*/
#if !defined(LUA_CSUBSEP)
#define LUA_CSUBSEP		LUA_DIRSEP
#endif

#if !defined(LUA_LSUBSEP)
#define LUA_LSUBSEP		LUA_DIRSEP
#endif


/* prefix for open functions in C libraries */
#define LUA_POF		"luaopen_"

/* separator for open functions in C libraries */
#define LUA_OFSEP	"_"


/* table (in the registry) that keeps handles for all loaded C libraries */
#define CLIBS		"_CLIBS"

#define LIB_FAIL	"open"


/* error codes for ll_loadfunc */
#define ERRLIB		1
#define ERRFUNC		2

#define setprogdir(L)		((void)0)


/*
** system-dependent functions
*/
static void ll_unloadlib (void *lib);
static void *ll_load (lua_State *L, const char *path, int seeglb);
static lua_CFunction ll_sym (lua_State *L, void *lib, const char *sym);



#if defined(LUA_USE_DLOPEN)
/*
** {========================================================================
** This is an implementation of loadlib based on the dlfcn interface.
** The dlfcn interface is available in Linux, SunOS, Solaris, IRIX, FreeBSD,
** NetBSD, AIX 4.2, HPUX 11, and  probably most other Unix flavors, at least
** as an emulation layer on top of native functions.
** =========================================================================
*/

#include <dlfcn.h>

static void ll_unloadlib (void *lib) {
  dlclose(lib);
}


static void *ll_load (lua_State *L, const char *path, int seeglb) {
  void *lib = dlopen(path, RTLD_NOW | (seeglb ? RTLD_GLOBAL : RTLD_LOCAL));
  if (lib == NULL) lua_pushstring(L, dlerror());
  return lib;
}


static lua_CFunction ll_sym (lua_State *L, void *lib, const char *sym) {
  lua_CFunction f = (lua_CFunction)dlsym(lib, sym);
  if (f == NULL) lua_pushstring(L, dlerror());
  return f;
}

/* }====================================================== */



#elif defined(LUA_DL_DLL)
/*
** {======================================================================
** This is an implementation of loadlib for Windows using native functions.
** =======================================================================
*/

#undef setprogdir

/*
** optional flags for LoadLibraryEx
*/
#if !defined(LUA_LLE_FLAGS)
#define LUA_LLE_FLAGS	0
#endif


static void setprogdir (lua_State *L) {
  char buff[MAX_PATH + 1];
  char *lb;
  DWORD nsize = sizeof(buff)/sizeof(char);
  DWORD n = GetModuleFileNameA(NULL, buff, nsize);
  if (n == 0 || n == nsize || (lb = strrchr(buff, '\\')) == NULL)
    luaL_error(L, "unable to get ModuleFileName");
  else {
    *lb = '\0';
    luaL_gsub(L, lua_tostring(L, -1), LUA_EXEC_DIR, buff);
    lua_remove(L, -2);  /* remove original string */
  }
}


static void pusherror (lua_State *L) {
  int error = GetLastError();
  char buffer[128];
  if (FormatMessageA(FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM,
      NULL, error, 0, buffer, sizeof(buffer)/sizeof(char), NULL))
    lua_pushstring(L, buffer);
  else
    lua_pushfstring(L, "system error %d\n", error);
}

static void ll_unloadlib (void *lib) {
  FreeLibrary((HMODULE)lib);
}


static void *ll_load (lua_State *L, const char *path, int seeglb) {
  HMODULE lib = LoadLibraryExA(path, NULL, LUA_LLE_FLAGS);
  (void)(seeglb);  /* not used: symbols are 'global' by default */
  if (lib == NULL) pusherror(L);
  return lib;
}


static lua_CFunction ll_sym (lua_State *L, void *lib, const char *sym) {
  lua_CFunction f = (lua_CFunction)GetProcAddress((HMODULE)lib, sym);
  if (f == NULL) pusherror(L);
  return f;
}

/* }====================================================== */


#else
/*
** {======================================================
** Fallback for other systems
** =======================================================
*/

#undef LIB_FAIL
#define LIB_FAIL	"absent"


#define DLMSG	"dynamic libraries not enabled; check your Lua installation"


static void ll_unloadlib (void *lib) {
  (void)(lib);  /* not used */
}


static void *ll_load (lua_State *L, const char *path, int seeglb) {
  (void)(path); (void)(seeglb);  /* not used */
  lua_pushliteral(L, DLMSG);
  return NULL;
}


static lua_CFunction ll_sym (lua_State *L, void *lib, const char *sym) {
  (void)(lib); (void)(sym);  /* not used */
  lua_pushliteral(L, DLMSG);
  return NULL;
}

/* }====================================================== */
#endif


static void *ll_checkclib (lua_State *L, const char *path) {
  void *plib;
  lua_getfield(L, LUA_REGISTRYINDEX, CLIBS);
  lua_getfield(L, -1, path);
  plib = lua_touserdata(L, -1);  /* plib = CLIBS[path] */
  lua_pop(L, 2);  /* pop CLIBS table and 'plib' */
  return plib;
}


static void ll_addtoclib (lua_State *L, const char *path, void *plib) {
  lua_getfield(L, LUA_REGISTRYINDEX, CLIBS);
  lua_pushlightuserdata(L, plib);
  lua_pushvalue(L, -1);
  lua_setfield(L, -3, path);  /* CLIBS[path] = plib */
  lua_rawseti(L, -2, luaL_len(L, -2) + 1);  /* CLIBS[#CLIBS + 1] = plib */
  lua_pop(L, 1);  /* pop CLIBS table */
}


/*
** __gc tag method for CLIBS table: calls 'll_unloadlib' for all lib
** handles in list CLIBS
*/
static int gctm (lua_State *L) {
  int n = luaL_len(L, 1);
  for (; n >= 1; n--) {  /* for each handle, in reverse order */
    lua_rawgeti(L, 1, n);  /* get handle CLIBS[n] */
    ll_unloadlib(lua_touserdata(L, -1));
    lua_pop(L, 1);  /* pop handle */
  }
  return 0;
}


static int ll_loadfunc (lua_State *L, const char *path, const char *sym) {
  void *reg = ll_checkclib(L, path);  /* check loaded C libraries */
  if (reg == NULL) {  /* must load library? */
    reg = ll_load(L, path, *sym == '*');
    if (reg == NULL) return ERRLIB;  /* unable to load library */
    ll_addtoclib(L, path, reg);
  }
  if (*sym == '*') {  /* loading only library (no function)? */
    lua_pushboolean(L, 1);  /* return 'true' */
    return 0;  /* no errors */
  }
  else {
    lua_CFunction f = ll_sym(L, reg, sym);
    if (f == NULL)
      return ERRFUNC;  /* unable to find function */
    lua_pushcfunction(L, f);  /* else create new function */
    return 0;  /* no errors */
  }
}


static int ll_loadlib (lua_State *L) {
  const char *path = luaL_checkstring(L, 1);
  const char *init = luaL_checkstring(L, 2);
  int stat = ll_loadfunc(L, path, init);
  if (stat == 0)  /* no errors? */
    return 1;  /* return the loaded function */
  else {  /* error; error message is on stack top */
    lua_pushnil(L);
    lua_insert(L, -2);
    lua_pushstring(L, (stat == ERRLIB) ?  LIB_FAIL : "init");
    return 3;  /* return nil, error message, and where */
  }
}



/*
** {======================================================
** 'require' function
** =======================================================
*/


static int readable (const char *filename) {
  FILE *f = fopen(filename, "r");  /* try to open file */
  if (f == NULL) return 0;  /* open failed */
  fclose(f);
  return 1;
}


static const char *pushnexttemplate (lua_State *L, const char *path) {
  const char *l;
  while (*path == *LUA_PATH_SEP) path++;  /* skip separators */
  if (*path == '\0') return NULL;  /* no more templates */
  l = strchr(path, *LUA_PATH_SEP);  /* find next separator */
  if (l == NULL) l = path + strlen(path);
  lua_pushlstring(L, path, l - path);  /* template */
  return l;
}


static const char *searchpath (lua_State *L, const char *name,
                                             const char *path,
                                             const char *sep,
                                             const char *dirsep) {
  luaL_Buffer msg;  /* to build error message */
  luaL_buffinit(L, &msg);
  if (*sep != '\0')  /* non-empty separator? */
    name = luaL_gsub(L, name, sep, dirsep);  /* replace it by 'dirsep' */
  while ((path = pushnexttemplate(L, path)) != NULL) {
    const char *filename = luaL_gsub(L, lua_tostring(L, -1),
                                     LUA_PATH_MARK, name);
    lua_remove(L, -2);  /* remove path template */
    if (readable(filename))  /* does file exist and is readable? */
      return filename;  /* return that file name */
    lua_pushfstring(L, "\n\tno file " LUA_QS, filename);
    lua_remove(L, -2);  /* remove file name */
    luaL_addvalue(&msg);  /* concatenate error msg. entry */
  }
  luaL_pushresult(&msg);  /* create error message */
  return NULL;  /* not found */
}


static int ll_searchpath (lua_State *L) {
  const char *f = searchpath(L, luaL_checkstring(L, 1),
                                luaL_checkstring(L, 2),
                                luaL_optstring(L, 3, "."),
                                luaL_optstring(L, 4, LUA_DIRSEP));
  if (f != NULL) return 1;
  else {  /* error message is on top of the stack */
    lua_pushnil(L);
    lua_insert(L, -2);
    return 2;  /* return nil + error message */
  }
}


static const char *findfile (lua_State *L, const char *name,
                                           const char *pname,
                                           const char *dirsep) {
  const char *path;
  lua_getfield(L, lua_upvalueindex(1), pname);
  path = lua_tostring(L, -1);
  if (path == NULL)
    luaL_error(L, LUA_QL("package.%s") " must be a string", pname);
  return searchpath(L, name, path, ".", dirsep);
}


static int checkload (lua_State *L, int stat, const char *filename) {
  if (stat) {  /* module loaded successfully? */
    lua_pushstring(L, filename);  /* will be 2nd argument to module */
    return 2;  /* return open function and file name */
  }
  else
    return luaL_error(L, "error loading module " LUA_QS
                         " from file " LUA_QS ":\n\t%s",
                          lua_tostring(L, 1), filename, lua_tostring(L, -1));
}


static int searcher_Lua (lua_State *L) {
  const char *filename;
  const char *name = luaL_checkstring(L, 1);
  filename = findfile(L, name, "path", LUA_LSUBSEP);
  if (filename == NULL) return 1;  /* module not found in this path */
  return checkload(L, (luaL_loadfile(L, filename) == LUA_OK), filename);
}


static int loadfunc (lua_State *L, const char *filename, const char *modname) {
  const char *funcname;
  const char *mark;
  modname = luaL_gsub(L, modname, ".", LUA_OFSEP);
  mark = strchr(modname, *LUA_IGMARK);
  if (mark) {
    int stat;
    funcname = lua_pushlstring(L, modname, mark - modname);
    funcname = lua_pushfstring(L, LUA_POF"%s", funcname);
    stat = ll_loadfunc(L, filename, funcname);
    if (stat != ERRFUNC) return stat;
    modname = mark + 1;  /* else go ahead and try old-style name */
  }
  funcname = lua_pushfstring(L, LUA_POF"%s", modname);
  return ll_loadfunc(L, filename, funcname);
}


static int searcher_C (lua_State *L) {
  const char *name = luaL_checkstring(L, 1);
  const char *filename = findfile(L, name, "cpath", LUA_CSUBSEP);
  if (filename == NULL) return 1;  /* module not found in this path */
  return checkload(L, (loadfunc(L, filename, name) == 0), filename);
}


static int searcher_Croot (lua_State *L) {
  const char *filename;
  const char *name = luaL_checkstring(L, 1);
  const char *p = strchr(name, '.');
  int stat;
  if (p == NULL) return 0;  /* is root */
  lua_pushlstring(L, name, p - name);
  filename = findfile(L, lua_tostring(L, -1), "cpath", LUA_CSUBSEP);
  if (filename == NULL) return 1;  /* root not found */
  if ((stat = loadfunc(L, filename, name)) != 0) {
    if (stat != ERRFUNC)
      return checkload(L, 0, filename);  /* real error */
    else {  /* open function not found */
      lua_pushfstring(L, "\n\tno module " LUA_QS " in file " LUA_QS,
                         name, filename);
      return 1;
    }
  }
  lua_pushstring(L, filename);  /* will be 2nd argument to module */
  return 2;
}


static int searcher_preload (lua_State *L) {
  const char *name = luaL_checkstring(L, 1);
  lua_getfield(L, LUA_REGISTRYINDEX, "_PRELOAD");
  lua_getfield(L, -1, name);
  if (lua_isnil(L, -1))  /* not found? */
    lua_pushfstring(L, "\n\tno field package.preload['%s']", name);
  return 1;
}


static void findloader (lua_State *L, const char *name) {
  int i;
  luaL_Buffer msg;  /* to build error message */
  luaL_buffinit(L, &msg);
  lua_getfield(L, lua_upvalueindex(1), "searchers");  /* will be at index 3 */
  if (!lua_istable(L, 3))
    luaL_error(L, LUA_QL("package.searchers") " must be a table");
  /*  iterate over available searchers to find a loader */
  for (i = 1; ; i++) {
    lua_rawgeti(L, 3, i);  /* get a searcher */
    if (lua_isnil(L, -1)) {  /* no more searchers? */
      lua_pop(L, 1);  /* remove nil */
      luaL_pushresult(&msg);  /* create error message */
      luaL_error(L, "module " LUA_QS " not found:%s",
                    name, lua_tostring(L, -1));
    }
    lua_pushstring(L, name);
    lua_call(L, 1, 2);  /* call it */
    if (lua_isfunction(L, -2))  /* did it find a loader? */
      return;  /* module loader found */
    else if (lua_isstring(L, -2)) {  /* searcher returned error message? */
      lua_pop(L, 1);  /* remove extra return */
      luaL_addvalue(&msg);  /* concatenate error message */
    }
    else
      lua_pop(L, 2);  /* remove both returns */
  }
}


static int ll_require (lua_State *L) {
  const char *name = luaL_checkstring(L, 1);
  lua_settop(L, 1);  /* _LOADED table will be at index 2 */
  lua_getfield(L, LUA_REGISTRYINDEX, "_LOADED");
  lua_getfield(L, 2, name);  /* _LOADED[name] */
  if (lua_toboolean(L, -1))  /* is it there? */
    return 1;  /* package is already loaded */
  /* else must load package */
  lua_pop(L, 1);  /* remove 'getfield' result */
  findloader(L, name);
  lua_pushstring(L, name);  /* pass name as argument to module loader */
  lua_insert(L, -2);  /* name is 1st argument (before search data) */
  lua_call(L, 2, 1);  /* run loader to load module */
  if (!lua_isnil(L, -1))  /* non-nil return? */
    lua_setfield(L, 2, name);  /* _LOADED[name] = returned value */
  lua_getfield(L, 2, name);
  if (lua_isnil(L, -1)) {   /* module did not set a value? */
    lua_pushboolean(L, 1);  /* use true as result */
    lua_pushvalue(L, -1);  /* extra copy to be returned */
    lua_setfield(L, 2, name);  /* _LOADED[name] = true */
  }
  return 1;
}

/* }====================================================== */



/*
** {======================================================
** 'module' function
** =======================================================
*/
#if defined(LUA_COMPAT_MODULE)

/*
** changes the environment variable of calling function
*/
static void set_env (lua_State *L) {
  lua_Debug ar;
  if (lua_getstack(L, 1, &ar) == 0 ||
      lua_getinfo(L, "f", &ar) == 0 ||  /* get calling function */
      lua_iscfunction(L, -1))
    luaL_error(L, LUA_QL("module") " not called from a Lua function");
  lua_pushvalue(L, -2);  /* copy new environment table to top */
  lua_setupvalue(L, -2, 1);
  lua_pop(L, 1);  /* remove function */
}


static void dooptions (lua_State *L, int n) {
  int i;
  for (i = 2; i <= n; i++) {
    if (lua_isfunction(L, i)) {  /* avoid 'calling' extra info. */
      lua_pushvalue(L, i);  /* get option (a function) */
      lua_pushvalue(L, -2);  /* module */
      lua_call(L, 1, 0);
    }
  }
}


static void modinit (lua_State *L, const char *modname) {
  const char *dot;
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "_M");  /* module._M = module */
  lua_pushstring(L, modname);
  lua_setfield(L, -2, "_NAME");
  dot = strrchr(modname, '.');  /* look for last dot in module name */
  if (dot == NULL) dot = modname;
  else dot++;
  /* set _PACKAGE as package name (full module name minus last part) */
  lua_pushlstring(L, modname, dot - modname);
  lua_setfield(L, -2, "_PACKAGE");
}


static int ll_module (lua_State *L) {
  const char *modname = luaL_checkstring(L, 1);
  int lastarg = lua_gettop(L);  /* last parameter */
  luaL_pushmodule(L, modname, 1);  /* get/create module table */
  /* check whether table already has a _NAME field */
  lua_getfield(L, -1, "_NAME");
  if (!lua_isnil(L, -1))  /* is table an initialized module? */
    lua_pop(L, 1);
  else {  /* no; initialize it */
    lua_pop(L, 1);
    modinit(L, modname);
  }
  lua_pushvalue(L, -1);
  set_env(L);
  dooptions(L, lastarg);
  return 1;
}


static int ll_seeall (lua_State *L) {
  luaL_checktype(L, 1, LUA_TTABLE);
  if (!lua_getmetatable(L, 1)) {
    lua_createtable(L, 0, 1); /* create new metatable */
    lua_pushvalue(L, -1);
    lua_setmetatable(L, 1);
  }
  lua_pushglobaltable(L);
  lua_setfield(L, -2, "__index");  /* mt.__index = _G */
  return 0;
}

#endif
/* }====================================================== */



/* auxiliary mark (for internal use) */
#define AUXMARK		"\1"


/*
** return registry.LUA_NOENV as a boolean
*/
static int noenv (lua_State *L) {
  int b;
  lua_getfield(L, LUA_REGISTRYINDEX, "LUA_NOENV");
  b = lua_toboolean(L, -1);
  lua_pop(L, 1);  /* remove value */
  return b;
}


static void setpath (lua_State *L, const char *fieldname, const char *envname1,
                                   const char *envname2, const char *def) {
  const char *path = getenv(envname1);
  if (path == NULL)  /* no environment variable? */
    path = getenv(envname2);  /* try alternative name */
  if (path == NULL || noenv(L))  /* no environment variable? */
    lua_pushstring(L, def);  /* use default */
  else {
    /* replace ";;" by ";AUXMARK;" and then AUXMARK by default path */
    path = luaL_gsub(L, path, LUA_PATH_SEP LUA_PATH_SEP,
                              LUA_PATH_SEP AUXMARK LUA_PATH_SEP);
    luaL_gsub(L, path, AUXMARK, def);
    lua_remove(L, -2);
  }
  setprogdir(L);
  lua_setfield(L, -2, fieldname);
}


static const luaL_Reg pk_funcs[] = {
  {"loadlib", ll_loadlib},
  {"searchpath", ll_searchpath},
#if defined(LUA_COMPAT_MODULE)
  {"seeall", ll_seeall},
#endif
  {NULL, NULL}
};


static const luaL_Reg ll_funcs[] = {
#if defined(LUA_COMPAT_MODULE)
  {"module", ll_module},
#endif
  {"require", ll_require},
  {NULL, NULL}
};


static void createsearcherstable (lua_State *L) {
  static const lua_CFunction searchers[] =
    {searcher_preload, searcher_Lua, searcher_C, searcher_Croot, NULL};
  int i;
  /* create 'searchers' table */
  lua_createtable(L, sizeof(searchers)/sizeof(searchers[0]) - 1, 0);
  /* fill it with pre-defined searchers */
  for (i=0; searchers[i] != NULL; i++) {
    lua_pushvalue(L, -2);  /* set 'package' as upvalue for all searchers */
    lua_pushcclosure(L, searchers[i], 1);
    lua_rawseti(L, -2, i+1);
  }
}


LUAMOD_API int luaopen_package (lua_State *L) {
  /* create table CLIBS to keep track of loaded C libraries */
  luaL_getsubtable(L, LUA_REGISTRYINDEX, CLIBS);
  lua_createtable(L, 0, 1);  /* metatable for CLIBS */
  lua_pushcfunction(L, gctm);
  lua_setfield(L, -2, "__gc");  /* set finalizer for CLIBS table */
  lua_setmetatable(L, -2);
  /* create `package' table */
  luaL_newlib(L, pk_funcs);
  createsearcherstable(L);
#if defined(LUA_COMPAT_LOADERS)
  lua_pushvalue(L, -1);  /* make a copy of 'searchers' table */
  lua_setfield(L, -3, "loaders");  /* put it in field `loaders' */
#endif
  lua_setfield(L, -2, "searchers");  /* put it in field 'searchers' */
  /* set field 'path' */
  setpath(L, "path", LUA_PATHVERSION, LUA_PATH, LUA_PATH_DEFAULT);
  /* set field 'cpath' */
  setpath(L, "cpath", LUA_CPATHVERSION, LUA_CPATH, LUA_CPATH_DEFAULT);
  /* store config information */
  lua_pushliteral(L, LUA_DIRSEP "\n" LUA_PATH_SEP "\n" LUA_PATH_MARK "\n"
                     LUA_EXEC_DIR "\n" LUA_IGMARK "\n");
  lua_setfield(L, -2, "config");
  /* set field `loaded' */
  luaL_getsubtable(L, LUA_REGISTRYINDEX, "_LOADED");
  lua_setfield(L, -2, "loaded");
  /* set field `preload' */
  luaL_getsubtable(L, LUA_REGISTRYINDEX, "_PRELOAD");
  lua_setfield(L, -2, "preload");
  lua_pushglobaltable(L);
  lua_pushvalue(L, -2);  /* set 'package' as upvalue for next lib */
  luaL_setfuncs(L, ll_funcs, 1);  /* open lib into global table */
  lua_pop(L, 1);  /* pop global table */
  return 1;  /* return 'package' table */
}

