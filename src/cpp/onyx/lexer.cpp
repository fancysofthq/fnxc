#include <algorithm>
#include <functional>
#include <limits>
#include <memory>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <stdint.h>
#include <string>
#include <string_view>
#include <type_traits>
#include <vadefs.h>
#include <variant>
#include <vector>
#include <xstddef>
#include <xstring>

#include "../../../include/fnx/errors.hpp"
#include "../../../include/fnx/onyx/lexer.hpp"
#include "../../../include/fnx/utils/logging.hpp"
#include "../../../include/fnx/utils/string.hpp"

#define CONCAT_(x, y) x##y
#define CONCAT(x, y) CONCAT_(x, y)

namespace FNX::Onyx {

/// Yield a variant of tokens as `AnyToken`.
#define YIELD_V(ARG, WRAPPER)                                       \
  co_yield std::visit(                                              \
      [this](auto &&tok) { return WRAPPER({tok}); }, ARG);

#define __YIELD_ALL_IMPL(CORO_NAME, ARG, WRAPPER)                   \
  auto CORO_NAME = ARG;                                             \
  while (!CORO_NAME.done())                                         \
    YIELD_V(CORO_NAME.next(), WRAPPER);

/// Create a coroutine and yield its result until it's done.
#define YIELD_ALL(ARG, WRAPPER)                                     \
  __YIELD_ALL_IMPL(CONCAT(__coro_, __COUNTER__), ARG, WRAPPER)

using namespace Utils::String;

using u8stream = std::basic_stringstream<char8_t>;
using u32stream = std::basic_stringstream<char32_t>;

Utils::generator<AnyToken> Lexer::lex() {
  while (!_input->eof()) {
    // Comments have the highest precedence, and even macros won't
    // work within them. It is not possible to have a freestanding
    // escaped comment sign, e.g. `\#`, as well as any escaped
    // sequences within the comment itself.
    //
    // A comment is terminated with a newline in the source.
    //

    if (_is('#')) {
      co_yield _lex_comment();
    }

    // Onyx macros have the next highest precedence, so we'll
    // attempt lexing them now. A macro, however, may be preceded
    // with a backslash, so we have to lex a backslash first. Note
    // that a block-opening curly bracket can not be escaped.
    //

    else if (_is('\\')) {
      _advance(); // Consume '\'

      // A EOL may be escaped.
      if (_is_eol()) {
        _advance(); // Consume EOL
        co_yield _punct(Token::Punctuation::EEol);
      }

      // '\!' denotes a final macro.
      else if (_is('!')) {
        _advance(); // Consume '!'
        _consume('{');

        if (_consume({'%', '{'}) == '%') {
          co_yield _punct(Token::Punctuation::FinalMacroOpen);
        } else {
          co_yield _punct(
              Token::Punctuation::FinalEmittingMacroOpen);
        }
      }

      // '\{' denotes a delayed macro.
      else if (_is('{')) {
        _advance(); // Consume '{'

        if (_consume({'%', '{'}) == '%') {
          co_yield _punct(Token::Punctuation::DelayedMacroOpen);
        } else {
          co_yield _punct(
              Token::Punctuation::DelayedEmittingMacroOpen);
        }
      }

      // Any other freestanding escape sequence is illegal.
      else {
        throw Error(Error::UnknownEscapeSequence);
      }
    }

    // Time for immediate Onyx macros. Otherwise, a single opening
    // curly bracket is deemed to be a simple punctuation token.
    //

    else if (_is('{')) {
      _advance(); // Consume '{'

      if (_is('%')) {
        _advance(); // Consume '%'
        co_yield _punct((Token::Punctuation::MacroOpen));
      } else if (_is('{')) {
        _advance(); // Consume '{'
        co_yield _punct((Token::Punctuation::EmittingMacroOpen));
      } else {
        co_yield _punct(Token::Punctuation::CurlyOpen);
      }
    }

    // Time to lex "invisible" punctuation.
    //

    else if (_is_eol()) {
      co_yield _lex_eol();
    } else if (_is_space()) {
      co_yield _lex_space();
    }

    // Next go literals.
    //
    // NOTE: An optional numeric literal sign is considered a
    // freestanding unary operator.
    //

    else if (_is('\'')) {
      YIELD_ALL(_lex_char_literal(), AnyToken);
    } else if (_is('"')) {
      YIELD_ALL(_lex_string_literal(), AnyToken);
    } else if (_is_decimal()) {
      YIELD_ALL(_lex_numeric_literal(), AnyToken);
    }

    else if (_is('%')) {
      _advance(); // Consume '%'

      // `%~` or `%q` begin a quoted string literal.
      if (_is('~') || _is('q')) {
        YIELD_ALL(_lex_quoted_string(_is('~')), AnyToken);
      }

      // `%c` begins a char container.
      else if (_is('c')) {
        YIELD_ALL(_lex_char_container(), AnyToken);
      }

      // An alphanumeric data or bracket begin a numeric container.
      else if (_is_latin_alpha() || _is({'[', '(', '|'})) {
        YIELD_ALL(_lex_numeric_container(), AnyToken);
      }

      else if (_is_op()) {
        co_yield _lex_op(); // Continue lexing an operator token
      }

      else {
        // Yield a single `%` operator token
        co_yield Token::Operator(_reloc());
      }
    }

    else if (_is(':')) {
      _advance(); // Consume ':'

      if (_is('`') || _is_id_start()) {
        co_yield _lex_id_no_keyword(); // Lex a symbolic id
      } else if (_is(':')) {
        _advance(); // Consume the second ':'
        co_yield _punct(Token::Punctuation::DoubleColon); // `::`
      } else {
        co_yield _punct(Token::Punctuation::Colon); // `:`
      }
    }

    // Then go freestanding punctuation.
    //
    // NOTE: We've already lexed '{' above.
    //
    // NOTE: Arrows have higher precedence than operators.
    //

    else if (_is('@')) {
      _advance();
      co_yield _punct(Token::Punctuation::At);
    } else if (_is('$')) {
      _advance();
      co_yield _punct(Token::Punctuation::C);
    } else if (_is('.')) {
      _advance();
      co_yield _punct(Token::Punctuation::Dot);
    } else if (_is(',')) {
      _advance();
      co_yield _punct(Token::Punctuation::Comma);
    } else if (_is(';')) {
      _advance();
      co_yield _punct(Token::Punctuation::Semicolon);
    } else if (_is('?')) {
      _advance();
      co_yield _punct(Token::Punctuation::Ternary);
    } else if (_is('(')) {
      _advance();
      co_yield _punct(Token::Punctuation::ParenOpen);
    } else if (_is(')')) {
      _advance();
      co_yield _punct(Token::Punctuation::ParenClose);
    } else if (_is('[')) {
      _advance();
      co_yield _punct(Token::Punctuation::SquareOpen);
    } else if (_is(']')) {
      _advance();
      co_yield _punct(Token::Punctuation::SquareClose);
    } else if (_is('}')) {
      _advance(); // EOF is allowed
      co_yield _punct(Token::Punctuation::CurlyClose);
    } else if (_is('~') || _is('-') || _is('=')) {
      char32_t cp = _code_point;
      _advance();

      if (_is('>')) {
        _advance();

        switch (cp) {
        case '~':
          co_yield _punct(Token::Punctuation::CurlyArrow); // `~>`
          break;
        case '-':
          co_yield _punct(Token::Punctuation::ThinArrow); // `->`
          break;
        case '=':
          co_yield _punct(Token::Punctuation::ThickArrow); // `=>`
          break;
        default:
          throw ICE();
        }
      } else if (_is_op()) {
        co_yield _lex_op();
      } else {
        // Yield `~`, `-` or `=` as an operator
        co_yield _op();
      }
    }

    // Next go operators.
    //

    else if (_is_op()) {
      co_yield _lex_op();
    }

    // And finally, identifiers and keywords.
    //
    // NOTE: A symbol may have its colon omitted if it is within a
    // numeric container literal, e.g. `%u8(min max)`.
    //

    else if (_is('`')) {
      co_yield _lex_id_no_keyword();
    } else if (_is_id_start()) {
      YIELD_V(_lex_id_or_keyword(), AnyToken);
    }

    // Otherwise, error.

    else {
      throw Error();
    }
  }
} // namespace FNX::Onyx

#pragma region Lexing subroutines

Token::Comment Lexer::_lex_comment() {
  _consume('#');

  while (!(_is_eof() || _is_eol())) {
    _advance();
  }

  return Token::Comment(_reloc());
}

Token::Punctuation Lexer::_lex_eol() {
  while (_is_eol())
    _advance();

  return _punct(Token::Punctuation::Eol);
}

Token::Punctuation Lexer::_lex_space() {
  while (_is_space())
    _advance();

  return _punct(Token::Punctuation::Space);
}

#define __IMPL(ESC, HEX)                                            \
  case ESC:                                                         \
    value = HEX;                                                    \
    break;

std::variant<Token::Codepoint, Token::Punctuation>
Lexer::_lex_codepoint() {
  if (_is('\\')) {
    _advance(); // Consume the backslash

    if (_is_eol()) {
      _advance(); // Consume a single (!) EOL
      return _punct(Token::Punctuation::EEol);
    }

    std::optional<Token::Codepoint::Format> format;
    char32_t value;

    if (_is('o')) {
      format = Token::Codepoint::Octal;
    } else if (_is('d')) {
      format = Token::Codepoint::DecimalExplicit;
    } else if (_is_decimal()) {
      format = Token::Codepoint::DecimalImplicit;
    } else if (_is('x')) {
      format = Token::Codepoint::Hexadecimal;
    } else {
      switch (_code_point) {
        __IMPL('a', 0x07)
        __IMPL('b', 0x08)
        __IMPL('e', 0x1B)
        __IMPL('f', 0x0C)
        __IMPL('n', 0x0A)
        __IMPL('r', 0x0D)
        __IMPL('t', 0x09)
        __IMPL('v', 0x0B)
        __IMPL('\\', 0x5C)
        __IMPL('\'', 0x27)
        __IMPL('"', 0x22)
        __IMPL('(', 0x28)
        __IMPL(')', 0x29)
        __IMPL('{', 0x7B)
        __IMPL('}', 0x7D)
        __IMPL('[', 0x5B)
        __IMPL(']', 0x5D)
        __IMPL('<', 0x3C)
        __IMPL('>', 0x3E)
        __IMPL('|', 0x7C)
      }

      if (value)
        format = Token::Codepoint::Escaped;
    }

    if (!format.has_value())
      throw Error(Error::CodepointUnknownEscapeSequence);

    switch (format.value()) {
    case Token::Codepoint::Escaped: {
      _advance(); // Consume the escaped code point

      return Token::Codepoint(
          _reloc(), format.value(), value, false);
    }

    case Token::Codepoint::Octal:
    case Token::Codepoint::DecimalImplicit:
    case Token::Codepoint::DecimalExplicit:
    case Token::Codepoint::Hexadecimal: {
      Radix radix;

      switch (format.value()) {
      case Token::Codepoint::Octal:
        radix = Radix::Octal;
        break;
      case Token::Codepoint::DecimalImplicit:
      case Token::Codepoint::DecimalExplicit:
        radix = Radix::Decimal;
        break;
      case Token::Codepoint::Hexadecimal:
        radix = Radix::Hexadecimal;
        break;
      default:
        throw ICE();
      }

      bool is_wrapped;

      if (_is('{')) {
        if (format.value() != Token::Codepoint::DecimalImplicit) {
          _advance(); // Consume '{'
          is_wrapped = true;
        } else {
          throw ICE(); // Impossible
        }
      }

      u32stream ss;

      if (is_wrapped) {
        // Wrapped format allows underscores in the numeric value,
        // e.g. `\d{12_34}`. Akin to common numeric literals, an
        // underscore can not be neither preceding nor succeeding.
        //

        bool last_underscore;

        while (_is_num(radix) || _is('_')) {
          if (_is('_')) {
            if (!ss.gcount())
              throw Error(Error::UnderscorePreceding);

            _advance(); // Consume the underscore
            last_underscore = true;
          } else {
            last_underscore = false;
            ss << _advance();
          }
        }

        if (!_is('}'))
          throw Error(Error::CodepointUnexpected);
        else
          _advance(); // Consume closing '}'

        if (last_underscore)
          throw Error(Error::UnderscoreSucceding);
      } else {
        while (_is_num(radix))
          ss << _advance();
      }

      if (!ss.gcount())
        throw Error(Error::CodepointEmpty);

      uint32_t value = parse<int>(ss.str(), radix_to_base(radix));

      return Token::Codepoint(
          _reloc(), format.value(), value, is_wrapped);
    }
    default:
      throw ICE();
    }
  } else {
    return Token::Codepoint(
        _reloc(), Token::Codepoint::Exact, _code_point, false);
  }
}

#undef __IMPL

Utils::generator<std::variant<Token::Codepoint, Token::Punctuation>>
Lexer::_lex_codepoints(char32_t terminator) {
  while (!_input->eof()) {
    auto res = _lex_codepoint();

    if (res.index() == 0) {
      auto cp = std::get<Token::Codepoint>(res);

      if (cp.format == Token::Codepoint::Exact &&
          cp.value == terminator) {
        co_return;
      } else {
        co_yield cp;
      }
    } else {
      co_yield std::get<Token::Punctuation>(res);
    }
  }

  // The codepoints sequence has not been terminated correctly.
  throw EOFError();
}

Utils::generator<std::variant<Token::Codepoint, Token::Punctuation>>
Lexer::_lex_codepoints(std::u32string heredoc_terminator) {
  const unsigned terminator_size = heredoc_terminator.size();
  std::vector<Token::Codepoint> codepoints_buffer;
  unsigned at;

  while (_input->eof()) {
    auto res = _lex_codepoint();

    if (res.index() == 0) {
      auto cp = std::get<Token::Codepoint>(res);

      // Check if the CP value is the next heredoc character.
      if (cp.format == Token::Codepoint::Exact) {
        if (cp.value == heredoc_terminator.at(at)) {
          // Match! Put the CP into the buffer.
          //

          at++;
          codepoints_buffer.push_back(cp);
        } else {
          // The CP breaks the sequence.
          //

          at = 0;

          // Yield the buffered codepoints.
          for (auto e : codepoints_buffer)
            co_yield e;

          // Yield the CP itself.
          co_yield cp;
        }

        if (at == terminator_size) {
          // Buffered codepoints will discard automatically, but we
          // need proper location for the heredoc terminator token.
          //
          // Note that current cursor is after the latest read
          // codepoint.
          //

          Location loc(
              codepoints_buffer.end()->location.begin,
              codepoints_buffer.front().location.end);

          co_yield Token::Punctuation(
              loc, Token::Punctuation::Heredoc);

          co_return;
        }
      }

      // Non-exact codepoints can not compose a heredoc terminator.
      else {
        // Shall break the sequence.
        //

        at = 0;

        for (auto e : codepoints_buffer)
          co_yield e;

        co_yield cp;
      }
    } else {
      // Duh.
      //

      at = 0;

      for (auto e : codepoints_buffer)
        co_yield e;

      co_yield std::get<Token::Punctuation>(res);
    }
  }

  // The codepoints sequence has not been terminated correctly.
  throw EOFError();
}

std::variant<Token::ID, Token::Keyword> Lexer::_lex_id_or_keyword() {
  u32stream buf;

  if (_is_id_start())
    buf << _advance();
  else
    throw ICE();

  while (_is_id_cont())
    buf << _advance();

  auto kind = Token::Keyword::from_string(buf.str());

  if (kind.has_value())
    return Token::Keyword(_reloc(), kind.value());
  else
    return _id(false);
}

Token::ID Lexer::_lex_id_no_keyword() {
  if (_is('`')) {
    _advance(); // Consume the opening '`'

    while (!_is('`'))
      _advance();

    _advance(); // Consume the closing '`'
    return _id(true);
  } else {
    if (_is_id_start())
      _advance();
    else
      throw ICE();

    while (_is_id_cont())
      _advance();

    return _id(false);
  }
}

Token::Operator Lexer::_lex_op() {
  while (_is_op())
    _advance();

  return _op();
}

#define __IMPL(BRACKET, KIND)                                       \
  case BRACKET:                                                     \
    return _punct(Token::Punctuation::KIND);                        \
    break;

Token::Punctuation Lexer::_lex_bracket() {
  switch (_consume({'{', '[', '(', '<', '|', '}', ']', ')', '>'})) {
    __IMPL('{', CurlyOpen)
    __IMPL('[', SquareOpen)
    __IMPL('(', ParenOpen)
    __IMPL('<', AngleOpen)
    __IMPL('|', PipeBracket)
    __IMPL('}', CurlyClose)
    __IMPL(']', SquareClose)
    __IMPL(')', ParenClose)
    __IMPL('>', AngleClose)
  default:
    throw ICE();
  }
}

#undef __IMPL

Utils::generator<
    std::variant<Token::Punctuation, Token::Codepoint, Token::Data>>
Lexer::_lex_char_literal() {
  const auto kind = Token::Punctuation::SingleQuote;

  _consume('\'');
  co_yield _punct(kind);

  auto coro = _lex_codepoints(kind);
  unsigned counter;

  while (!coro.done()) {
    if (++counter > 1)
      throw Error(Error::CharTooMuch);

    auto result = coro.next();

    if (result.index() != 0)
      throw Error(Error::CharEEol);

    co_yield std::get<Token::Codepoint>(result);
  }

  if (!counter)
    throw Error(Error::CharEmpty);

  _consume('\'');
  co_yield _punct(kind);

  if (_is_latin_alpha() || _is_decimal()) {
    while (_is_latin_alpha() || _is_decimal())
      _advance();

    co_yield _data();
  }
}

Utils::generator<
    std::variant<Token::Punctuation, Token::Codepoint, Token::Data>>
Lexer::_lex_string_literal() {
  using _Ret = decltype(_lex_string_literal().current());
  const auto kind = Token::Punctuation::DoubleQuote;

  _consume('"');
  co_yield _punct(kind);

  YIELD_ALL(_lex_codepoints(kind), _Ret);

  _consume('"');
  co_yield _punct(kind);

  if (_is_latin_alpha() || _is_decimal()) {
    while (_is_latin_alpha() || _is_decimal())
      _advance();

    co_yield _data();
  }
}

/// A helper macro.
#define __IMPL(LETTER, SI, IEC)                                     \
  case LETTER: {                                                    \
    _advance();                                                     \
                                                                    \
    if (_is('i')) {                                                 \
      _advance();                                                   \
      return Token::NumericLiteral::PrefixIEC::IEC;                 \
    } else {                                                        \
      return Token::NumericLiteral::PrefixSI::SI;                   \
    }                                                               \
  }

std::optional<std::variant<
    Token::NumericLiteral::PrefixSI,
    Token::NumericLiteral::PrefixIEC>>
Lexer::_lex_prefix() {
  if (_is('k')) {
    _advance();
    return Token::NumericLiteral::PrefixSI::Kilo;
  }

  else if (_is('K')) {
    _advance();

    if (_is('i')) {
      _advance();
      return Token::NumericLiteral::PrefixIEC::Kibi;
    } else {
      return std::nullopt; // `K` is not a prefix
    }
  }

  else {
    switch (_code_point) {
      __IMPL('M', Mega, Mebi)
      __IMPL('G', Giga, Gibi)
      __IMPL('T', Tera, Tebi)
      __IMPL('P', Peta, Pebi)
      __IMPL('E', Exa, Exbi)
      __IMPL('Z', Zetta, Zebi)
      __IMPL('Y', Yotta, Yobi)
    default:
      return std::nullopt;
    }
  }
}

#undef __IMPL

std::vector<Token::NumericLiteral::Element>
Lexer::_lex_numeric_elements(Radix radix) {
  std::vector<Token::NumericLiteral::Element> elements;

  bool has_digits = false;
  bool latest_is_underscore;

  std::optional<std::variant<
      std::type_identity_t<Token::NumericLiteral::PrefixSI>,
      std::type_identity_t<Token::NumericLiteral::PrefixIEC>>>
      prefix_family;

  while (true) {
    if (_is_num(radix)) {
      uint8_t value = parse<int>(_code_point, radix_to_base(radix));

      _advance(); // Consume the numeric character
      elements.push_back(value);

      has_digits = true;
      latest_is_underscore = false;
    } else if (_is('_')) {
      if (!has_digits)
        throw Error(Error::UnderscorePreceding);

      elements.push_back(Token::NumericLiteral::Underscore());
      latest_is_underscore = true;
    } else {
      auto prefix = _lex_prefix();

      if (!prefix.has_value())
        break; // It's not a prefix, something else.

      auto p = prefix.value();

      if (radix != Radix::Decimal)
        throw Error(Error::NumericLiteralPrefixNonDecimal);

      if (!has_digits || latest_is_underscore)
        throw Error(Error::NumericLiteralInvalidPrefixPlacement);

      std::variant<
          std::type_identity_t<Token::NumericLiteral::PrefixSI>,
          std::type_identity_t<Token::NumericLiteral::PrefixIEC>>
          f;

      if (p.index() == 0)
        f = std::type_identity_t<Token::NumericLiteral::PrefixSI>();
      else
        f = std::type_identity_t<Token::NumericLiteral::PrefixIEC>();

      if (!prefix_family.has_value())
        prefix_family = f;
      else if (prefix_family.value() != f)
        throw Error(Error::NumericLiteralPrefixFamilyIncompatible);

      if (p.index() == 0)
        elements.push_back(
            std::get<Token::NumericLiteral::PrefixSI>(p));
      else
        elements.push_back(
            std::get<Token::NumericLiteral::PrefixSI>(p));

      latest_is_underscore = false;
    }
  }

  if (latest_is_underscore)
    throw Error(Error::UnderscoreSucceding);

  return elements;
}

int Lexer::_lex_exponent() {
  u32stream ss;
  bool has_sign;

  if (_is('-') || _is('+')) {
    has_sign = true;
    ss << _code_point;
    _advance();
  }

  while (_is_decimal())
    ss << _advance();

  auto result = ss.str();
  auto size = result.size();

  if ((!has_sign && !size) || (has_sign && size == 1))
    throw Error(Error::NumericLiteralEmptyExponent);

  // NOTE: Exponent is always decimal.
  return parse<int>(result, 10);
}

Utils::generator<std::variant<
    Token::NumericLiteral,
    Token::Punctuation,
    Token::Data>>
Lexer::_lex_numeric_literal() {
  if (_is('0')) {
    _advance(); // Consume the zero

    auto radix = Radix::Decimal;
    unsigned base = 10;

    switch (_code_point) {
    case 'b':
      radix = Radix::Binary;
      break;
    case 'o':
      radix = Radix::Octal;
      break;
    case 'x':
      radix = Radix::Hexadecimal;
      break;
    }

    if (radix == Radix::Decimal) {
      //  `0_` or `01` are illegal.
      if (_is('_') || _is_decimal())
        throw Error(Error::NumericLiteralDecimalBeginsFromZero);
    } else {
      _advance(); // Consume the radix identifier
    }

    std::optional<std::vector<Token::NumericLiteral::Element>> whole;

    if (_is_num(radix)) {
      whole = _lex_numeric_elements(radix);
    }

    // Could be a fractional part (`0.1`) or a call (`0x42.foo()`).
    if (_is('.')) {
      // For non-decimal radices, the whole
      // part is required before a dot.
      if (!whole.has_value() && radix != Radix::Decimal)
        throw Error(Error::NumericLiteralDotTooEarly);

      _advance(); // Consume the dot

      if (_is_num(radix)) {
        // Lex the fractional value elements.
        auto frac = _lex_numeric_elements(radix);

        // Lex the exponent, if any.
        // Note that exponent can not follow the dot.
        // An exponent always has precedence over data.
        //

        std::optional<int> exponent;

        if (_is(Token::NumericLiteral::exponent_char(radix))) {
          _advance(); // Consume the exponent char
          exponent = _lex_exponent();
        }

        // Yield the numeric literal token.
        co_yield Token::NumericLiteral(
            _reloc(), radix, whole, frac, exponent);

        // Lex data, if any.
        //

        auto data = _lex_data();

        if (data.has_value())
          co_yield data.value();
      } else {
        // A non-decimal following a dot is either a call or an
        // error, checked by a parser.
        //
        // We've checked what's AFTER the dot, thus special logic
        // is required for cursors. After the dot is read, the
        // cursor is at the position after the dot, and previous
        // cursor is before the dot.
        //

        // Yield the exact zero literal (`0`).
        co_yield Token::NumericLiteral(
            Location(_previous_cursor, _latest_yielded_cursor));

        _latest_yielded_cursor = _previous_cursor;

        // Yield the dot.
        co_yield _punct(Token::Punctuation::Dot);
      }
    }

    // An exponent, e.g. `0e3`.
    else if (_is(Token::NumericLiteral::exponent_char(radix))) {
      // An exponent without the whole value is illegal for
      // non-decimal radices, e.g. `0xp3`.
      if (!whole.has_value() && radix != Radix::Decimal)
        throw Error(Error::NumericLiteralExponentTooEarly);

      _advance(); // Consume the exponent char
      int exponent = _lex_exponent();

      // Yield the numeric literal.
      co_yield Token::NumericLiteral(
          _reloc(), radix, whole, std::nullopt, exponent);

      // Lex data, if any.
      //

      auto data = _lex_data();

      if (data.has_value())
        co_yield data.value();
    }

    // There is neither fractional part nor exponent.
    else {
      // A non-decimal numeric literal requires its whole part.
      if (!whole.has_value() && radix != Radix::Decimal)
        throw Error(Error::NumericLiteralEmptyWholePart);

      co_yield Token::NumericLiteral(_reloc(), radix, whole);

      // Lex data, if any.
      //

      auto data = _lex_data();

      if (data.has_value())
        co_yield data.value();
    }
  } else {
    auto whole = _lex_numeric_elements(Radix::Decimal);
    // TODO: Handle dot and exponent
  }
}

Utils::generator<std::variant<Token::Punctuation, Token::Codepoint>>
Lexer::_lex_heredoc() {
  _consume('~');
  co_yield _punct(Token::Punctuation::HeredocSymbol);

  u32stream buf;

  if (_is_heredoc_start())
    buf << _advance();
  else
    throw Error();

  while (_is_heredoc_cont())
    buf << _advance();

  co_yield _punct(Token::Punctuation::Heredoc);

  auto coro = _lex_codepoints(buf.str());

  while (!coro.done()) {
    auto res = coro.next();

    if (res.index() == 0) {
      co_yield std::get<Token::Codepoint>(res);
    } else {
      auto punct = std::get<Token::Punctuation>(res);
      co_yield punct;

      if (punct.kind == Token::Punctuation::Heredoc)
        co_return;
    }
  }

  throw Error(Error::HeredocNonTerminated);
}

std::optional<Token::Data> Lexer::_lex_data() {
  if (!_is_data_start())
    return std::nullopt;

  _advance();

  while (_is_data_cont())
    _advance();

  return _data();
}

Utils::generator<std::variant<
    Token::MagicLiteral,
    Token::Data,
    Token::Punctuation,
    Token::Codepoint>>
Lexer::_lex_quoted_string(bool is_heredoc) {
  using _Ret = decltype(_lex_quoted_string(true).current());

  co_yield Token::MagicLiteral(
      _reloc(), Token::MagicLiteral::QuotedString, !is_heredoc);

  if (is_heredoc) {
    YIELD_ALL(_lex_heredoc(), _Ret);
  } else {
    _consume('q');

    auto data = _lex_data();
    if (data.has_value())
      co_yield data.value();

    if (_is('~')) {
      YIELD_ALL(_lex_heredoc(), _Ret);
    } else if (_is({'[', '<', '(', '{', '|'})) {
      char32_t closing = _get_closing_bracket_pair(_code_point);
      co_yield _lex_bracket();
      YIELD_ALL(_lex_codepoints(closing), _Ret);
      co_yield _lex_bracket();
    }
  }
}

Utils::generator<std::variant<
    Token::MagicLiteral,
    Token::Data,
    Token::Punctuation,
    Token::Codepoint>>
Lexer::_lex_char_container() {
  using _Ret = decltype(_lex_char_container().current());

  _consume('c');
  co_yield Token::MagicLiteral(
      _reloc(), Token::MagicLiteral::CharContainer, true);

  auto data = _lex_data();
  if (data.has_value())
    co_yield data.value();

  if (_is('~')) {
    YIELD_ALL(_lex_heredoc(), _Ret);
  } else if (_is({'[', '<', '('})) {
    char32_t closing = _get_closing_bracket_pair(_code_point);
    co_yield _lex_bracket(); // Yield opening bracket
    YIELD_ALL(_lex_codepoints(closing), _Ret);
    co_yield _lex_bracket(); // Yield closing bracket
  } else if (_is('|')) {
    YIELD_ALL(_lex_char_tensor_literal(), _Ret);
  }
}

#pragma endregion

#pragma region Shortcut subroutines

Token::Punctuation Lexer::_punct(Token::Punctuation::Kind kind) {
  return Token::Punctuation(_reloc(), kind);
}

Token::Data Lexer::_data() { return Token::Data(_reloc()); }

#pragma endregion

#pragma region Match subroutines

#pragma endregion

// Lexer::Ret Lexer::_lex_quoted_string() {}

// /**
//  * @details
//  *
//  * A lexed C constexpr source code is terminated by `}`.
//  However,
//  * it may contain a compound literals or a string or character
//  * constant containing the `}` character. Therefore, the
//  * terminating bracket shall be properly paired and not persist
//  * within neither a string nor character literal.
//  *
//  * The first symbol must be `{`.
//  *
//  * Neither opening nor closing brackets are included into the
//  * yielded token source.
//  */
// Utils::generator<Token::Constexpr> Lexer::_lex_c_constexpr() {
//   if (!_is('{')) {
//     throw _err_expect({'{'});
//   }

//   _advance(); // Consume '{'

//   // The logic is pretty simple: only break the loop once
//   // an unpaired '}' is met, and it's within neither
//   // a character nor a string constant.
//   //

//   std::stringstream buff;

//   unsigned int bracket_pairs_count;
//   bool is_within_char_literal;
//   bool is_within_string_literal;

//   while (true) {
//     while (!_is('}')) {
//       if (_is('{')) {
//         bracket_pairs_count++;
//       } else if (_is('"')) {
//         is_within_string_literal = !is_within_string_literal;
//       } else if (_is('\'')) {
//         is_within_char_literal = !is_within_char_literal;
//       }

//       buff << _code_unit;
//       _advance();
//     }

//     if (bracket_pairs_count == 0 && !is_within_string_literal &&
//         !is_within_char_literal) {
//       break;
//     } else {
//       bracket_pairs_count--;
//     }
//   }

//   _advance(false); // Consume '}', allow EOF

//   co_yield Token::Constexpr(_placement(), buff.str());
// };

// /// @details Begins at Lua source code.
// Utils::generator<Token::Macro::Open>
// Lexer::_lex_macro(Token::Macro::Open::Timing timing, bool
// emitting) {
//   switch (timing) {
//   case Token::Macro::Timing::Immediate: {
//     if (emitting) {
//       co_yield(Token::Control(Token::Control::EmittingMacroOpen));
//     } else {
//       co_yield(Token::Control(Token::Control::MacroOpen));
//     }
//   }
//   }

//   std::stringstream buff;
//   bool escaped;

//   while (true) {
//     while (!((emitting ? _is('}') : _is('%')) && !escaped)) {
//       escaped = _is('\\');

//       trace(__func__) << "Putting '" << _code_unit
//                       << "' into the macro token buffer";

//       buff << _code_unit;
//       _advance();
//     }

//     _advance(); // Consume '}' or '%'

//     if (_is('}')) {
//       _advance(true); // Consume final '}', allow EOF
//       break;          // The macro has ended
//     } else {
//       buff << (emitting ? '}' : '%'); // Nope, just '}' or '%'
//     }
//   }

//   // TODO: Yield `Token::Control::MacroOpen`, then
//   // `Token::MacroSource`, and then
//   `Token::Control::MacroClose`?
//   co_yield(Token::Macro(_placement(), emitting, timing,
//   buff.str()));
// }

#undef YIELD_ALL
#undef __YIELD_ALL_IMPL
#undef YIELD_V
#undef CONCAT
#undef CONCAT_

} // namespace FNX::Onyx
