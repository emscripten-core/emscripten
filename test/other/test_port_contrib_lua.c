/*
 * Copyright 2025 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <stdio.h>

int main() {
  // Create a new Lua state
  lua_State *L = luaL_newstate();

  // Load standard Lua libraries
  luaL_openlibs(L);
  
  char const *chunks[3] = {
    "print('hello world')",                                // test Basic Library
    "print(string.upper('hello world'))",                  // test string library
    "print('sqrt(16)=' .. math.tointeger(math.sqrt(16)))", // test math library
  };

  // Iterate over each chunk of Lua code and execute it
  for (size_t i = 0; i <= 3; i++) {
    if (luaL_dostring(L, chunks[i]) != LUA_OK) {
      printf("Error running Lua code: %s\n", lua_tostring(L, -1));
      lua_pop(L, 1); // Remove the error message from the stack
      lua_close(L);
      return 1; // returning error so the test can fail
    }
  }

  // Close the Lua state
  lua_close(L);

  return 0;
}
