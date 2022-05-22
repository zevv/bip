#ifndef api_h
#define api_h

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

int l_traceback(lua_State *L);
int l_gain(lua_State *L);
int l_pan(lua_State *L);
int l_bip(lua_State *L);
int l_hash(lua_State *L);

#endif
