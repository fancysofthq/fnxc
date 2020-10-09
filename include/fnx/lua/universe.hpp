#pragma once

#include <mutex>
#include <unordered_map>
#include <vadefs.h>

extern "C" {
#include "lua.h"

// The naming scheme for user-space Lua functions (i.e. those
// available under the `nx.` table) is `__fnx_lua__*`. For example,
// `__fnx_lua__emit` for `nx.emit`. When the function is further
// namespaced, it is also separated by double underscores, e.g.
// `__fnx_lua__foo__bar_baz` for `nx.foo.bar.baz`.
//

/// @c nx.emit() , the function to emit Onyx code.
int __fnx_lua__emit(lua_State *state);

/// @c nx.sync() .
int __fnx_lua__sync(lua_State *state);

/// @c nx.lock() .
int __fnx_lua__lock(lua_State *state);

/// @c nx.unlock() .
int __fnx_lua__unlock(lua_State *state);
}

namespace FNX {
namespace Lua {

/// A Lua universe wrapper.
///
/// A universe may have multiple Lua states forked with
/// @c lua_newthread . Each forked state has the parent universe ID
/// in its @c LUA_EXTRASPACE .
///
/// @see http://lua-users.org/wiki/ThreadsTutorial
class Universe {
public:
  /// DANGER: The behaviour is undefined
  /// if the universe is not found.
  static Universe *unsafe_find_by_state(lua_State *);

  /// DANGER: The behaviour is undefined
  /// if the universe is not found.
  static Universe *unsafe_find_by_id(uintptr_t);

  /// The Lua universes map.
  static std::unordered_map<uintptr_t, Universe> map;

  /// Guaranteed to be unique. Shall not be changed.
  uintptr_t id;

  /// The mutex used by internal Lua routines.
  ///
  /// > However, be sure to use a different mutex than the one used
  /// by Lua in order to avoid potential deadlocks.
  std::mutex lua_mutex;

  /// The mutex used by Lua user-space functions.
  std::mutex user_mutex;

  /// Creates a new @c lua_State internally.
  /// DANGER: Is not thread-safe, must be called from within a
  /// thread-safe context.
  Universe();

  /// DANGER: Is not thread-safe, must be called from within a
  /// thread-safe context.
  ~Universe();

  lua_State *fork();

private:
  lua_State *_state;
};

} // namespace Lua
} // namespace FNX
