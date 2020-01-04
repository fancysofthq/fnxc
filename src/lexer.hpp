#pragma once

#include <stack>

#include "./macro.hpp"
#include "./token.hpp"

using namespace std;

namespace Onyx {
// Lexer takes an input file and lexes it into series of tokens,
// which are then feed to the Parser. The lexer is also responsible
// for evaluating immediate macros.
class Lexer {
  // The Onyx file stream to read input from.
  wistream *_input;

  // A `Macro` class instance for macro evaluation.
  Macro *_macro;

  // The last read codepoint in UTF-8 encoding.
  wchar_t _codepoint;

  // The next token location.
  Location _location;

  // Set to `true` when currently reading from code
  // emitted by a macro, i.e. after macro evaluation.
  bool _is_reading_from_macro;

  // // Set to `true` when a macro statement is not complete,
  // // so the following Onyx code is treated as emitted.
  // //
  // // ```
  // // {% for s, _ in ipairs({"foo", "bar"}) do %}
  // //   `puts({{ s }}) # This line is a macro expression, evaluated as
  // //                  # `` emit("`puts(" .. s .. ")\n") ``
  // // {% end %}
  // // ```
  // bool _is_macro_emitted_code;

public:
  struct Cursor {
    uint row;
    uint col;

    Cursor(uint row, uint col);
  };

  Cursor cursor = Cursor(0, 0);

  struct Error {
    const Location location;
    const wstring message;

    Error(const Location loc, const wstring msg);
  };

  Lexer(wistream *input, Macro *);

  shared_ptr<Token::Base> lex();

private:
  // Contrinue lexing a decimal literal,
  // which may be either an integer or a float.
  shared_ptr<Token::Base>
  lex_decimal(vector<char> significand, bool is_float = false);

  // Lex a non-decimal number.
  shared_ptr<Token::NonDecimalNumber> lex_nondecimal();

  // Read the next codepoint from the input and return the previous one.
  wchar_t read();

  // Raise a lexing error.
  void raise(wstring = L"Unexpected symbol");

  shared_ptr<Token::Control> yield_control(Token::Control::Kind);
  shared_ptr<Token::Value> yield_value(Token::Value::Kind, wstring);

  bool is(wchar_t);

  bool is_underscore();
  bool is_digit();
  bool is_binary();
  bool is_octadecimal();
  bool is_hexadecimal();

  bool is_lowercase_latin();
  bool is_uppercase_latin();
  bool is_latin();

  bool is_lowercase_greek();
  bool is_uppercase_greek();
  bool is_greek();

  bool is_alpha();
  bool is_alphanum();

  bool is_op();
};
}; // namespace Onyx
