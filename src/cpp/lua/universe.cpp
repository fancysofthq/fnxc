#include <cstdlib>

#include "fnx/lua/universe.hpp"
#include "fnx/utils/logging.hpp"

extern "C" {
#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"

#include "fnx/lua/luauser.h"
}

namespace FNX::Lua {

using namespace FNX::Utils::Logging;

// System Lua function bindings.
extern "C" {
void __fnx_luai__lock(lua_State *L) {
  auto universe = Universe::unsafe_find_by_state(L);

  trace(__func__) << "Acquiring system lock for Lua universe #"
                  << universe->id << "..." << std::endl;

  universe->lua_mutex.lock();

  trace(__func__) << "Acquired system lock for Lua universe #"
                  << universe->id << std::endl;
}

void __fnx_luai__unlock(lua_State *L) {
  auto universe = Universe::unsafe_find_by_state(L);
  universe->lua_mutex.unlock();
  trace(__func__) << "Released system lock for Lua universe #"
                  << universe->id << std::endl;
}

void __fnx_luai__userstateopen(lua_State *L) {
  void *id = ((void **)L)[0];
  trace(__func__) << "Lua universe #" << id << std::endl;
}

void __fnx_luai__userstateclose(lua_State *L) {
  void *id = ((void **)L)[0];
  trace(__func__) << "Lua universe #" << id << std::endl;
}

void __fnx_luai__userstatethread(lua_State *L, lua_State *L1) {
  void *id = ((void **)L)[0];
  trace(__func__) << "Lua universe #" << id << std::endl;
}

void __fnx_luai__userstatefree(lua_State *L, lua_State *L1) {
  void *id = ((void **)L)[0];
  trace(__func__) << "Lua universe #" << id << std::endl;
}

void __fnx_luai__userstateresume(lua_State *L, int n) {
  void *id = ((void **)L)[0];
  trace(__func__) << "Lua universe #" << id << std::endl;
}

void __fnx_luai__userstateyield(lua_State *L, int n) {
  void *id = ((void **)L)[0];
  trace(__func__) << "Lua universe #" << id << std::endl;
}
}

// User-space Lua function bindings.
extern "C" {
int __fnx_lua__emit(lua_State *L) {
  // TODO:
  luaL_checkstring(L, 1);
  lua_pushboolean(L, 1);
  return 1;
}

int __fnx_lua__sync(lua_State *L) {
  auto universe = Universe::unsafe_find_by_state(L);

  trace(__func__) << "Acquiring user lock for Lua universe #"
                  << universe->id << "..." << std::endl;

  universe->user_mutex.lock();

  trace(__func__) << "Acquired user lock for Lua universe #"
                  << universe->id << std::endl;

  // TODO:

  universe->user_mutex.unlock();
  trace(__func__) << "Released user lock for Lua universe #"
                  << universe->id << std::endl;

  return 1;
}
}

Universe::Universe() {
  // A `Universe()` call is guarnteed to be called from a thread-safe
  // function, thus we can easily ensure avoiding ID collisions from
  // within a single thread.
  //

  uintptr_t id = std::rand();

  while (map.contains(id))
    id = std::rand();

  map.emplace(id, this);

  // Now let's create a new Lua state.
  //

  _state = luaL_newstate();
  luaL_openlibs(_state); // Open standard libs

  // Let's add some C functions to the Lua universe!
  //

  // `nx.emit()` to emit Onyx code.
  lua_getglobal(_state, "nx");
  lua_pushcfunction(_state, __fnx_lua__emit);
  lua_setfield(_state, -2, "emit");

  // `nx.sync()` to synchronize between forked states.
  lua_pushcfunction(_state, __fnx_lua__sync);
  lua_setfield(_state, -3, "sync");

  // `nx.lock()`, ditto.
  lua_pushcfunction(_state, __fnx_lua__lock);
  lua_setfield(_state, -4, "lock");

  // `nx.unlock()`, ditto.
  lua_pushcfunction(_state, __fnx_lua__unlock);
  lua_setfield(_state, -5, "unlock");
}

/// DANGER: A Lua universe shall only be destroyed when no Lua
/// activity remains, because of mutices.
Universe::~Universe() {
  map.erase(id);
  lua_close(_state);
}

} // namespace FNX::Lua
