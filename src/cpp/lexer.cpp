#include <optional>

#include "../../include/fnx/lexer.hpp"

namespace FNX {

using namespace Utils;

char32_t Lexer::_advance() {
  if (_is_eof())
    throw EOFError();

  char8_t cu[4];

  cu[0] = _input->get();
  const auto size = UTF8::size_from_leading_byte(cu[0]);

  _previous_code_point = _code_point;

  if (size == 1) {
    _code_point = (char32_t)cu[0];
  } else {
    for (int i = 1; i < size; i++) {
      if (_is_eof())
        throw EOFError();

      cu[i] = _input->get();
    }

    _code_point = Utils::UTF8::to_code_point(cu);
  }

  _previous_cursor = _cursor;
  _cursor.offset += size;

  // DANGER: `\n` is only valid if the input is read in text mode.
  if (_code_point == '\n') {
    _cursor.col = 0;
    _cursor.row++;
  } else {
    _cursor.col++;
  }

  return _previous_code_point;
}

char32_t Lexer::_consume(std::set<char32_t> chars) {
  auto match = _is(chars);

  if (!match.has_value())
    throw ExpectationError(chars);

  _advance();
  return match.value();
}

char32_t Lexer::_consume(char32_t c) {
  if (!_is(c))
    throw ExpectationError(std::set{c});

  _advance();
  return c;
}

Location Lexer::_reloc() {
  Location loc(_latest_yielded_cursor, _previous_cursor);
  _latest_yielded_cursor = _previous_cursor;
  return loc;
}

#pragma region Match subroutines

bool Lexer::_is_eof() { return _input->eof(); }

bool Lexer::_is(char32_t c) { return _code_point == c; }

std::optional<char32_t> Lexer::_is(std::set<char32_t> chars) {
  if (chars.contains(_code_point))
    return _code_point;
  else
    return std::nullopt;
}

bool Lexer::_is_space() {
  return (
      _code_point == ' ' || _code_point == '\t' ||
      _code_point == '\v');
}

bool Lexer::_is_eol() { return _code_point == '\n'; }

bool Lexer::_is_num(Radix radix) {
  switch (radix) {
  case Radix::Binary:
    return _code_point == '0' || _code_point == '1';
  case Radix::Octal:
    return _code_point >= '0' && _code_point <= '7';
  case Radix::Decimal:
    return _code_point >= '0' && _code_point <= '9';
  case Radix::Hexadecimal:
    return (_code_point >= '0' && _code_point <= '9') ||
           (_code_point >= 'a' && _code_point <= 'f') ||
           (_code_point >= 'A' && _code_point <= 'F');
  }
}

bool Lexer::_is_binary() { return _is_num(Radix::Binary); }
bool Lexer::_is_decimal() { return _is_num(Radix::Decimal); }
bool Lexer::_is_octal() { return _is_num(Radix::Octal); }
bool Lexer::_is_hexadecimal() { return _is_num(Radix::Hexadecimal); }

bool Lexer::_is_latin_lowercase() {
  return _code_point >= 'a' && _code_point <= 'z';
}

bool Lexer::_is_latin_uppercase() {
  return _code_point >= 'A' && _code_point <= 'Z';
}

bool Lexer::_is_latin_alpha() {
  return _is_latin_lowercase() || _is_latin_uppercase();
}

bool Lexer::_is_greek_lowercase() {
  return _code_point >= 0x03B1 && _code_point <= 0x3C9;
}

bool Lexer::_is_greek_uppercase() {
  return (_code_point >= 0x0391 && _code_point < 0x03A2) || // Α-Ρ
         (_code_point >= 0x03A3 && _code_point <= 0x03A9);  // Σ-Ω
}

bool Lexer::_is_greek_alpha() {
  return _is_greek_lowercase() || _is_greek_uppercase();
}

#pragma endregion

} // namespace FNX
