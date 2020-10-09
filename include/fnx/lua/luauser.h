#include "lua.h"

// Note how built-in (i.e. non-userspace) functions are prefixed
// with `__fnx_luai__` instead of `__fnx_lua__`.
//

#define lua_lock(L) __fnx_luai__lock(L)
#define lua_unlock(L) __fnx_luai__unlock(L)
#define luai_userstateopen(L) __fnx_luai__userstateopen(L)
#define luai_userstateclose(L) __fnx_luai__userstateclose(L)
#define luai_userstatethread(L, L1)                                 \
  __fnx_luai__userstatethread(L, L1)
#define luai_userstatefree(L, L1) __fnx_luai__userstatefree(L, L1)
#define luai_userstateresume(L, n) __fnx_luai__userstateresume(L, n)
#define luai_userstateyield(L, n) __fnx_luai__userstateyield(L, n)

void __fnx_luai__lock(lua_State *);
void __fnx_luai__unlock(lua_State *);
void __fnx_luai__userstateopen(lua_State *);
void __fnx_luai__userstateclose(lua_State *);
void __fnx_luai__userstatethread(lua_State *, lua_State *);
void __fnx_luai__userstatefree(lua_State *, lua_State *);
void __fnx_luai__userstateresume(lua_State *, int);
void __fnx_luai__userstateyield(lua_State *, int);
