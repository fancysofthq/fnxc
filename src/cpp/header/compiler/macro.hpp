#pragma once

#include <iostream>
#include <optional>

using namespace std;

namespace Onyx {
namespace Compiler {
// A macro interpreter instance per
// compilation unit and entity specialization.
//
// It gets instantiated only when actually needed
// (i.e. a macro is met in the source code).
//
// This header file does not define the actual engine
// used for macro evaluation.
class Macro {
  // Some sort of interpreter state, e.g. a Lua instance.
  void *_state;

  // Set to true if currently emitting Onyx code
  // residing within a macro expression, for example:
  //
  // ```
  // # The expression is incomplete, thus raw Onyx code within it
  // # is treated as to-be-emitted by the macro interpreter.
  // {% for e, i in ... %}
  //   puts({{ e }})
  // ^ Not emitting yet, thus `false`
  //
  //   puts({{ e }})
  //   ^ Began Onyx code; set to `true`
  //
  //   puts({{ e }})
  //        ^ Macro code; set back to `false`
  //
  //   puts({{ e }})
  //               ^ Set to `true` again
  // ```
  bool _is_expression_emitted_onyx_code;

  bool _is_incomplete;

public:
  // The buffered macro code to evaluate.
  iostream input = iostream(NULL);

  // The evaluated Onyx code stream.
  iostream output = iostream(NULL);

  // Whether the macro statement is not complete.
  // If it's not, an Onyx code within such a statement
  // would be implicitly emitted by the resulting
  // completed macro rather than treated immediately
  // as a bare Onyx code. For example:
  //
  // ```nx
  // {% for i = 0, 1, 3 %}
  //   # At this point, `for` is incomplete,
  //   # as it expects proper ending.
  //   puts({{ i }})
  // {% end %}
  //
  // # The same as:
  // #
  //
  // {% for i = 0, 1, 3 %}
  //   {% emit("puts(") %}
  //   {% emit(i) %}
  //   {% emit(")\n") %}
  // {% end %}
  // ```
  bool is_incomplete();

  // A macro evaluation error other than incomplete statement.
  optional<string> error;

  // Return `true` if given Onyx character needs
  // escape before being put into the macro buffer.
  // For example, quotes (`"`).
  static bool needs_escape(char);

  Macro();
  ~Macro();

  // Evaluate the buffered macro code.
  // It's usually triggered on closing macro brackets.
  // It should be preceded by fulfilling the `input`.
  void eval();

  void begin_emit();

  // A lexer would treat further Onyx code
  // as implicitly emitted by a macro.
  // This is true until `end_implicit_emit` is called.
  // See `is_incomplete` docs for example.
  void begin_implicit_emit();

  // // Make sure that the macro is currently
  // // in the implicit emission mode.
  // void ensure_implicit_emit();

  // End an implicit emission block.
  void end_implicit_emit();

  void begin_explicit_emit();
  void end_explicit_emit();

  void end_emit();

private:
  void _init();
  void _end_onyx_code();
};
} // namespace Compiler
} // namespace Onyx
