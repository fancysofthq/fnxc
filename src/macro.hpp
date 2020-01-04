#pragma once

#include <iostream>

extern "C" {
#include "lua.h"
}

using namespace std;

namespace Onyx {
// The macro evaluator.
// Right now it uses Lua for evaluation.
class Macro {
  lua_State *_state;

  // Set to true if currently emitting an Onyx code, for example:
  //
  // ```
  // {% for e, i in ... %}
  //   `puts({{ e }})
  // ^ Not emitting yet, thus `false`
  //
  //   `puts({{ e }})
  //   ^ Set to `true`
  //
  //   `puts({{ e }})
  //         ^ Set back to `false`
  //
  //   `puts({{ e }})
  //               ^ Set to `true` again
  // ```
  bool _is_current_emit_onyx_code;

public:
  // The macro code to evaluate in UTF-8 encoding.
  wiostream input = wiostream(NULL);

  // The evaluated Onyx code stream in UTF-8 encoding.
  wiostream output = wiostream(NULL);

  // Whether is the macro statement not complete.
  bool is_incomplete;

  // A error other than incomplete statement.
  wstring error;

  Macro();
  ~Macro();

  // Evaluate the buffered macro code.
  void eval();

  bool onyx_code_needs_escape(wchar_t);

  void begin_emit();
  void ensure_begin_emitting_onyx_code();
  void begin_emitting_expression();
  void end_emitting_expression();
  void end_emit();

  // Reset the macro state.
  void reset();

private:
  void init();
  void end_onyx_code();
};

} // namespace Onyx
