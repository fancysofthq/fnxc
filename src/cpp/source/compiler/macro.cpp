#include <cstring>

extern "C" {
#include "../../../lib/lua/lauxlib.h"
#include "../../../lib/lua/lua.h"
#include "../../../lib/lua/lualib.h"
}

#include "./macro.hpp"

extern "C" {
static int lua_emit(lua_State *state) {
  luaL_checkstring(state, 1);
  lua_pushboolean(state, 1);
  return 1;
}
}

namespace Onyx {

Macro::Macro() {
  _state = luaL_newstate();

  luaopen_table((lua_State *)_state);
  luaopen_string((lua_State *)_state);

  lua_pushcfunction((lua_State *)_state, lua_emit);
  lua_setglobal((lua_State *)_state, "emit");

  _is_expression_emitted_onyx_code = false;

  input.clear();
  output.clear();

  is_incomplete = false;
  error = nullptr;
};

Macro::~Macro() { lua_close((lua_State *)_state); }

void Macro::eval() {
  char buff[256];

  while (input.getline(buff, sizeof(buff))) {
    int error = luaL_loadbuffer(
        (lua_State *)_state, (char *)buff, strlen(buff), "line");
  }
}

bool Macro::does_onyx_code_need_escape(char c) { return c == '"'; }

void Macro::begin_emit() { input << "\nemit('' .. "; }

void Macro::ensure_begin_emitting_onyx_code() {
  if (_is_expression_emitted_onyx_code)
    return;

  _is_expression_emitted_onyx_code = true;
  input << " .. \"";
}

void Macro::begin_emitting_expression() {
  _end_onyx_code();
  input << " .. (";
}

void Macro::end_emitting_expression() { input << ')'; }

void Macro::end_emit() {
  if (_is_expression_emitted_onyx_code)
    _end_onyx_code();

  input << ")\n";
}

void Macro::_end_onyx_code() {
  if (!_is_expression_emitted_onyx_code)
    return;

  _is_expression_emitted_onyx_code = false;
  input << '"';
}
} // namespace Onyx
