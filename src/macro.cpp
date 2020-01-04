#include <cstring>

extern "C" {
#include "lauxlib.h"
#include "lualib.h"
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

Macro::Macro() { init(); };
Macro::~Macro() { lua_close(_state); }

void Macro::eval() {
  wchar_t buff[256];

  while (input.getline(buff, sizeof(buff))) {
    int error = luaL_loadbuffer(_state, (char *)buff, wcslen(buff), "line");
  }
}

bool Macro::onyx_code_needs_escape(wchar_t c) { return c == '"'; }

void Macro::begin_emit() { input << "\nemit('' .. "; }

void Macro::ensure_begin_emitting_onyx_code() {
  if (_is_current_emit_onyx_code)
    return;

  _is_current_emit_onyx_code = true;
  input << " .. \"";
}

void Macro::begin_emitting_expression() {
  end_onyx_code();
  input << " .. (";
}

void Macro::end_emitting_expression() { input << ')'; }

void Macro::end_emit() {
  if (_is_current_emit_onyx_code)
    end_onyx_code();

  input << ")\n";
}

void Macro::reset() {
  lua_close(_state);
  init();
}

void Macro::init() {
  _state = luaL_newstate();

  luaL_openlibs(_state);
  lua_pushcfunction(_state, lua_emit);
  lua_setglobal(_state, "emit");

  _is_current_emit_onyx_code = false;

  input.clear();
  output.clear();
  is_incomplete = false;
  error = nullptr;
}

void Macro::end_onyx_code() {
  if (!_is_current_emit_onyx_code)
    return;

  _is_current_emit_onyx_code = false;
  input << '"';
}
} // namespace Onyx
