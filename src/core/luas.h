#ifndef __luas_H_
#define __luas_H_
#include "config.h"
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
//#include "luajit.h"


SEN_DECL int
sen_lua_execFile(const char* filename);

SEN_DECL int
sen_lua_execString(const char* code);

//---------------------------------------------------------------
SEN_DECL void
sen_lua_init();

SEN_DECL void
sen_lua_close();

#endif
