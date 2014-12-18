#include <stdio.h>
#include <string.h>
#include "luas.h"
#include "macro.h"
#include "logger.h"
#include "utils.h"
#include "asset.h"

#define SEN_LTAG "SEN:Lua"
static lua_State* g_Lua = 0;

//---------------------------------------------------------------
static int  lua_loader(lua_State *L);
static void lua_register_loader(lua_CFunction func);
static int  lua_exec_function(int numArgs);
static int  lua_load_buffer(const char* chunk, int chunkSize, const char* chunkName);

void
sen_lua_init()
{
  _logfi("+New lua state");
  g_Lua = luaL_newstate();
  sen_assert_msg(g_Lua, "Failed to create lua state");
  luaL_openlibs(g_Lua);
  lua_register_loader(lua_loader);
}

void
sen_lua_close()
{
  lua_close(g_Lua);
}

void lua_error_exit(const char *msg)
{
#ifdef SEN_DEBUG
  _logfe("[LUA ERROR]:\n  %s: %s\n\n", msg, lua_tostring(g_Lua, -1) );
  lua_pop(g_Lua, 1);
#else
  UNUSED(msg);
#endif
  exit(EXIT_FAILURE);
}

void lua_register_loader(lua_CFunction func)
{
  int i;
  sen_assert(func);
  lua_getglobal(g_Lua, "package");
  lua_getfield(g_Lua, -1, "loaders");
  lua_pushcfunction(g_Lua, func);
  for (i = (int)(lua_objlen(g_Lua, -2) + 1); i > 2; --i)
  {
      lua_rawgeti(g_Lua, -2, i - 1);
      lua_rawseti(g_Lua, -3, i);
  }
  lua_rawseti(g_Lua, -2, 2);
  lua_setfield(g_Lua, -2, "loaders");
  lua_pop(g_Lua, 1);
}

char * strrstr( char * s1, char * s2 )
{
    char * ss1;
    char * sss1;
    char * sss2;
    if( *( s2 ) == '\0' )
    {
        return s1;
    }
    ss1 = s1 + strlen( s1 );
    while( ss1 != s1 )
    {
        --ss1;

        for( sss1 = ss1, sss2 = s2; ; )
        {
            if( *( sss1++ ) != *( sss2++ ) )
            {
                break;
            }
            else if ( * sss2 == '\0' )
            {
                return ss1;
            }
        }
    }
    return NULL;
}

char* substring(const char* str, size_t begin, size_t len)
{
  size_t lens = strlen(str);
  if (str == 0 || lens == 0 || lens < begin || lens < (begin+len))
    return 0;

  return sen_strndup(str + begin, len);
}

int lua_loader(lua_State *L)
{

  const char *lstr = luaL_checkstring(L, 1);
  char* filename; char* c_pos;
  const char* searchpath;
  int spLen;
  int begin;
  char* c_next;
  int next;
  asset_t* script_asset = NULL;
  char fn_buffer[256];
  char* prefix;
  int pos;
  char* tmp;

  sen_assert(lstr);
  filename = sen_strdup(lstr);

  c_pos = strrstr(filename, ".lua");

  if (c_pos) {
    int pos = c_pos - filename;
    if ( pos == (int)strlen(filename)-4 )
      filename [pos] = '\0';
  }

  c_pos =  strchr(filename,'.');
  while (c_pos != NULL)
  {
    filename[c_pos-filename] = '/';
    c_pos = strchr(c_pos + 1, '.');
  }

  lua_getglobal(L, "package");
  lua_getfield(L, -1, "path");
  searchpath = lua_tostring(L, -1);
  spLen = strlen(searchpath);
  lua_pop(L, 1);
  begin = 0;
  c_next = strchr(searchpath, ';');
  next = c_next ? c_next - searchpath : -1;

  do
  {
      if (next < 0 )
          next = strlen(searchpath);

      prefix = substring(searchpath, begin, next - begin );
      if (prefix[0] == '.' && prefix[1] == '/')
      {
        char* tmp = prefix;
        prefix = substring(prefix, 2, strlen(prefix) - 2);
        free(tmp);
      }

      c_pos = strstr(prefix,"?.lua");
      pos = c_pos ? c_pos - prefix : -1;

      tmp = substring(prefix, 0, pos);
      free(prefix);
      *fn_buffer = '\0';
      if (tmp) {
        sprintf(fn_buffer, "%.50s%.200s.lua", tmp, filename);
        free(tmp);
      }
      if (asset_exists(fn_buffer)) {
        script_asset = asset_new(fn_buffer); break;
      }

      begin = next + 1;
      c_next = strchr(searchpath+begin, ';');
      next = c_next ? c_next - searchpath : -1;

  } while (begin < spLen);

  if (script_asset)
  {
    lua_load_buffer((char*)script_asset->data,
                    (int)script_asset->size,
                    (char*)fn_buffer );
      
    asset_delete(script_asset);
  }
  else
  {
    _logfe("Failed to load data from %s", filename);
    free(filename);
    return 0;
  }
  free(filename);
  return 1;
}

int
sen_lua_execFile(const char* filename)
{
  char buff[256];
  sen_assert(filename);
  sprintf(buff, "require \"%.200s\"", filename);
  return sen_lua_execString(buff);

}

int
sen_lua_execString(const char* code)
{
  sen_assert(code);
  luaL_loadstring(g_Lua, code);
  return lua_exec_function(0);
}


int lua_exec_function(int numArgs)
{
  int error = 0;
  int ret = 0;
  int functionIndex = -(numArgs + 1);
  if (!lua_isfunction(g_Lua, functionIndex))
  {
      _logfe("stack [%d] != function", functionIndex);
      lua_pop(g_Lua, numArgs + 1);
      return 0;
  }


  error = lua_pcall(g_Lua, numArgs, 1, 0);

  if (error)
  {
    lua_error_exit("Failed to call function");
    return 0;
  }

  if (lua_isnumber(g_Lua, -1))
  {
      ret = (int)lua_tointeger(g_Lua, -1);
  }
  else if (lua_isboolean(g_Lua, -1))
  {
      ret = (int)lua_toboolean(g_Lua, -1);
  }
  lua_pop(g_Lua, 1);

  return ret;
}

int
lua_load_buffer(const char* chunk,
                int chunkSize,
                const char* chunkName)
{
  int ret = luaL_loadbuffer(g_Lua, chunk, chunkSize, chunkName);
  if (ret)
  {
    lua_error_exit("load lua chunk failed");
  }
  return ret;
}
