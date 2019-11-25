#pragma once

#include <stack>

#include "./token.hpp"

using namespace std;

namespace Onyx {
// Lexer takes an input file and lexes it into series of tokens,
// which are then feed to the Parser. The lexer is also responsible
// for evaluating immediate macros.
class Lexer {
  // The stream to read input from.
  wistream *_input;

  wchar_t _codepoint;
  Location _location;

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

  Lexer(wistream *input);

  shared_ptr<Token::Base> lex();

private:
  enum State {
    Empty,
    Space,
    Reference, // An identifier or a type constant
    Expression,
    Access,
    AccessMod,
    Binop,
    Unop
  };

  // Contrinue lexing a decimal literal,
  // which may be either an integer or a float.
  shared_ptr<Token::Base>
  lex_decimal(string significand, bool is_float = false);

  shared_ptr<Token::NonDecimalNumber> lex_nondecimal();

  // Read the next codepoint from the input and return the previous one.
  wchar_t read();

  void raise(wstring = L"Unexpected symbol");

  shared_ptr<Token::Control> yield_control(Token::Control::Kind);
  shared_ptr<Token::Id> yield_id(Token::Id::Kind, wstring);

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
