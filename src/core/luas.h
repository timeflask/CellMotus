#ifndef __luas_H_
#define __luas_H_
#include "config.h"
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
//#include "luajit.h"


int
sen_lua_execFile(const char* filename);

int
sen_lua_execString(const char* code);

//---------------------------------------------------------------
void
sen_lua_init();

void
sen_lua_close();

#endif
