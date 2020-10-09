#pragma once

#include <istream>
#include <optional>
#include <ostream>
#include <set>
#include <stdexcept>
#include <stdint.h>
#include <variant>

#include "./location.hpp"
#include "./position.hpp"
#include "./radix.hpp"

namespace FNX {

/// A basic lexer.
class Lexer {
public:
  struct Error : std::logic_error {
    Error(std::string msg);
  };

  struct EOFError : Error {
    EOFError();
  };

  struct ExpectationError : Error {
    std::optional<std::set<char32_t>> expected_codepoints;
    std::optional<std::string> expected_message;

    ExpectationError(std::set<char32_t>);
    ExpectationError(std::string);
  };

  Lexer(std::basic_istream<char8_t> *);

protected:
  std::basic_istream<char8_t> *_input;

  /// Current @b byte offset in _input.
  ///
  /// @note Consequently, the file size limit is 4GiB.
  uint32_t _offset;

  /// Current cursor position in _input.
  ///
  /// Points to the position after the latest read code point.
  ///
  /// @code{.plain}
  ///   # When read `a`, the cursor would be
  ///   # at 0:1 with 1 byte offset.
  ///   ab
  ///
  ///   # Ditto. Note that `å` is two code units,
  ///   # so the offset would be 2 bytes.
  ///   åb
  /// @endcode
  Position _cursor;

  /// The previous _cursor value.
  Position _previous_cursor;

  /// The latest yielded cursor position. In conjunction with
  /// _cursor, it is used to construct a yielded token's location.
  Position _latest_yielded_cursor;

  /// The latest read UCS code point.
  char32_t _code_point;

  /// The previous _code_point value.
  char32_t _previous_code_point;

  /// Advance the input stream, updating the _code_point.
  ///
  /// Moves @link _cursor @endlink forward and updates @link
  /// _previous_cursor @endlink accordingly.
  ///
  /// In case of `EOFError` thrown, only  @link _code_point @endlink
  /// and  @link _previous_code_point @endlink are updated.
  ///
  /// @param throw_on_eof If true, would throw on EOF.
  /// @throw EOFError if `throw_on_eof` is `true` and EOF is met.
  /// @return The previous @link _code_point @endlink value.
  char32_t _advance();

  /// Consume one of the code points, advancing in case of success.
  ///
  /// @throw ExpectationError.
  /// @return A consumed code point.
  char32_t _consume(std::set<char32_t>);

  /// Consume the code point, advancing in case of success.
  ///
  /// @throw ExpectationError
  /// @return A consumed code point.
  char32_t _consume(char32_t);

  /// Update _latest_yielded_cursor to _previousious_cursor and
  /// return location for the next token to be yielded.
  ///
  /// @return Location comprised of the previous
  /// _latest_yielded_cursor and current _previousious_cursor values.
  Location _reloc();

#pragma region Match subroutines

  /// Check if the input is EOF.
  bool _is_eof();

  /// Match the code point exactly.
  bool _is(char32_t);

  /// Match one of the code points.
  /// @return The matched code unit, if any.
  std::optional<char32_t> _is(std::set<char32_t>);

  /// Match characters U+0020 (space), U+0009 (horizontal tab) and
  /// U+0013 (vertical tab).
  ///
  /// @todo Also match Unicode characters with @c Zs category.
  /// @see @link Onyx::Token::Punctuation::WhiteSpace @endlink
  bool _is_space();

  /// Match a line breaking (End-Of-Line, EOL) character.
  /// For now, it only matches system-specific newline.
  ///
  /// @todo Match EOL in Unicode-correct way.
  /// @see https://www.unicode.org/reports/tr14/
  bool _is_eol();

  /// Match an ASCII digit in given radix, case insensitive.
  bool _is_num(Radix);

  /// Match @c /0-1/ .
  bool _is_binary();

  /// Match @c /0-7/ .
  bool _is_octal();

  /// Match @c /0-9/ .
  bool _is_decimal();

  /// Match @c /[0-9a-fA-F]/ .
  bool _is_hexadecimal();

  /// Match @c /a-z/ .
  bool _is_latin_lowercase();

  /// Match @c /A-Z/ .
  bool _is_latin_uppercase();

  /// Match @c /[a-zA-Z]/ .
  bool _is_latin_alpha();

  /// Match @c /α-ω/ ( @c \\x03B1-\\x03C9 ).
  bool _is_greek_lowercase();

  /// Match @c /Α-Ω/ ( @c \\x0391-\\x03A9 ).
  bool _is_greek_uppercase();

  /// Match @c /[α-ωΑ-Ω]/ .
  bool _is_greek_alpha();

#pragma endregion
};

} // namespace FNX
