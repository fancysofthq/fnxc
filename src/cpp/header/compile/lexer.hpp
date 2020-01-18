#pragma once

#include "../utils/coroutines.hpp"
#include "./macro.hpp"
#include "./token.hpp"

using namespace std;

namespace Onyx {
namespace Compile {
// Lexer takes an compilation unit and lexes it into series of tokens,
// which are then fed to the Parser. The lexer is also responsible
// for evaluating immediate macros.
class Lexer {
  // The compilation unit.
  shared_ptr<Unit> _unit;

  // A file stream to read input from.
  unique_ptr<ifstream> _input;

  // A lazily instantiated `Macro` class
  // instance for macro evaluation.
  unique_ptr<Macro> _macro;

  // The last UTF-8 code unit read.
  char _codeunit;

  // Points to the position **after** the latest read character.
  //
  // ```
  // ab // If read `b`, the cursor would be at 0:2
  // ```
  Position _cursor = Position();

  // The previous cursor position.
  Position _prev_cursor = Position();

  // Location of the next token to be yielded.
  Location _location;

  // Set to `true` when currently reading from code
  // emitted by a macro, i.e. after macro evaluation.
  bool _is_reading_from_macro;

public:
  struct Error {
    enum Kind {
      Unexpected, // The lexer did not expect that codeunit
      FileError,  // Something's wrong with the unit file

      NumericUnknownRadix,
      NumericDecimalBeginsFromZero,
      NumericDotTooEarly, // E.g. `0x.1` is invalid
      NumericEmptyWholePart,
      NumericEmptyFractionPart, // E.g. `0x1.p` is invalid
      NumericEmptyExponent,
      NumericZeroExponent,
      NumericInvalidBitsize,

      CodepointUnknownEscapeSequence, // E.g. `'\h'`: what's that?
      CodepointOutOfRange,      // Must be in range 0-‭4294967295‬
      CodepointOutOfRangeASCII, // Must be in range 0-255
      CodepointMalformed,       // A UTF-8 error

      CharLinebreak, // Linebreaks aren't allowed in chars
      CharEmpty,     // Empty chars aren't allowed

      UnderscorePreceding,
      UnderscoreDuplicate,
      UnderscoreTrailing,

      ValueInvalidIntrinsic,
      ValueInvalidCID,
      ValueInvalidType,
    };

    Position position;
    Kind kind;

    Error(Position pos, Kind kind = Unexpected) :
        position(pos), kind(kind) {}
  };

  struct ExpectationError {
    Position position;
    set<char> expected;

    ExpectationError(Position pos, set<char> exp) :
        position(pos), expected(exp) {}
  };

  struct MacroError {
    Position position;
    string message;

    MacroError(Position pos, string msg) :
        position(pos), message(msg) {}
  };

  Lexer(shared_ptr<Unit>);

  // Lex the tokens.
  generator<shared_ptr<Token::Base>> lex();

private:
  // Read the next code point from the input
  // and return the previous one.
  //
  // Moves the `_cursor` position forward,
  // updates `_prev_cursor`.
  char _read(bool raise_on_eof = true);

  // Raise a lexing `Error` with current cursor location.
  void _err(Error::Kind = Error::Unexpected);

  // // Raise a error with expected codepoints.
  // void _err_expect(set<string> expected);

  // Raise a error with expected codepoints.
  void _err_expect(set<char> expected);

  // Raise a macro error.
  void _err_macro(string message);

  // Lex a raw codepoint, e.g. `a` or `\x61`.
  // Halts reading once encountered the *terminator*.
  // Returns `nullopt` if the terminator is immediately read.
  // Raises if *ascii_only* us `true` and encountered
  // an UTF-8 char with bytesize greater than one.
  optional<variant<Token::Linebreak, Token::Codepoint>>
  _lex_codepoint(char terminator, bool ascii_only);

  // Lex a single char literal, e.g. `'a'` or `'\x61`,
  // including wrapping quotes.
  generator<shared_ptr<Token::Base>> _lex_char_literal();

  // Lex a string literal, e.g. `"foo"` or `"\o{146}oo"`,
  // including wrapping quotes.
  generator<shared_ptr<Token::Base>> _lex_string_literal();

  // Lex a numeric literal, e.g. `42`, `0.5e-3` or `0x1.2p-10`.
  generator<shared_ptr<Token::Base>> _lex_numeric_literal();

  // Create or return a `_macro` instance.
  unique_ptr<Macro> _ensure_macro();

  // Set the `_location.end` position to
  // `_prev_cursor`; the copy of this object
  // would be returned from the function.
  //
  // Then sets the `_location.begin` position
  // to the same `_prev_cursor`.
  //
  // The function can be understood as
  // "reset the current location" or
  // "relocate the current location".
  Location _reloc();

  // Shortcut for a new control token.
  shared_ptr<Token::Control> _control(Token::Control::Kind);

  // Shortcut for a new value token.
  shared_ptr<Token::Value> _value(Token::Value::Kind, string);

  // Check if current `_codeunit` value
  // equals to the argument.
  bool _is(char);

  // Check if current `_codeunit` value
  // equals to any of the chars.
  bool _is(const set<char>);

  // Matching /0-1/.
  bool _is_binary();

  // Matching /0-7/.
  bool _is_octadecimal();

  // Matching /[0-9a-fA-F]/.
  bool _is_hexadecimal();

  // Matching /0-9/.
  bool _is_num();

  // Matching /a-z/.
  bool _is_lowercase();

  // Matching /A-Z/.
  bool _is_uppercase();

  // Matching /[a-zA-Z]/.
  bool _is_alpha();

  // Matching /[a-zA-Z0-9]/.
  bool _is_alphanum();

  // 0 to 127 ASCII.
  bool _is_ascii();

  // Return `true` if code unit is in the list
  // of allowed ASCII operators, e.g. `+` or `&`.
  bool _is_ascii_op();
};
}; // namespace Compile
} // namespace Onyx
