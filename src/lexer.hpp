#pragma once

#include "./token.hpp"

using namespace std;

namespace Onyx {
// Lexer takes an input file and lexes it into series of tokens,
// which are then feed to the Parser. The lexer is also responsible
// for evaluating immediate macros.
class Lexer {
  wifstream *_input;
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

  Lexer(wifstream *input);

  shared_ptr<Token::Base> lex();

private:
  // Read the next codepoint from the input.
  wchar_t read_codepoint();

  bool is_underscore();
  bool is_digit();
  bool is_lowercase_latin();
};
}; // namespace Onyx
