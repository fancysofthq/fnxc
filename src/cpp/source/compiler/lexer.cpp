#include "../../header/compiler/lexer.hpp"
#include "../../header/utils/log.hpp"
#include "../../header/utils/utf8.hpp"
#include <fstream>
#include <functional>
#include <memory>
#include <set>
#include <stdint.h>

namespace Onyx {
namespace Compiler {
Lexer::Lexer(std::shared_ptr<Unit> unit) :
    _unit(unit), _location(Location(unit)) {
  ldebug() << "[Lexer()] Opening file " << unit->path;

  if (!filesystem::exists(unit->path)) {
    ltrace() << "[Lexer()] File does not exist, panicking";
    throw Error(_cursor, Error::FileError);
  }

  // The file is opened in text mode to make
  // newline-dependent code cross-platform.
  _input = make_unique<ifstream>(ifstream(unit->path));
  ldebug() << "[Lexer()] Opened the file";

  _read(false);
}

char Lexer::_read(bool raise_on_eof) {
  char prev_codeunit = _codeunit;

  if (_is_reading_from_macro) {
    ltrace() << "[Lexer::_read] Accessing macro output";
    auto stream = &_macro->output;

    if (stream->eof()) {
      ltrace() << "[Lexer::_read] Stop reading from macro (EOF)";
      _is_reading_from_macro = false;
    } else {
      ltrace() << "[Lexer::_read] Reading from macro";
      _codeunit = stream->get();
      ltrace() << "[Lexer::_read] Read `" << _codeunit << "` (0x"
               << std::hex << +_codeunit << std::dec
               << ") from macro output";
      return prev_codeunit;
    }
  }

  // Reading from the `_input` would not happen
  // if already read from the macro input.
  //

  ltrace() << "[Lexer::_read] Reading from source";
  _codeunit = _input->get();
  ltrace() << "[Lexer::_read] Read `" << _codeunit << "` (0x"
           << std::hex << +_codeunit << ")" << std::dec;

  if (_is(EOF) && raise_on_eof)
    _err();

  _prev_cursor = _cursor;

  // NOTE: `\n` is platform-specific. It may be bigger
  // than a single byte (e.g. `\r\n` on Windows).
  // https://en.wikipedia.org/wiki/Newline#In_programming_languages
  if (_is('\n')) {
    ltrace() << "[Lexer::_read] Read newline, "
                "incrementing cursor row";
    _cursor.col = 0;
    _cursor.row += 1;
  } else
    _cursor.col += 1;

  return prev_codeunit;
}

void Lexer::_err(Error::Kind kind) { throw Error(_cursor, kind); }

void Lexer::_err_expect(set<char> expected) {
  throw ExpectationError(_cursor, expected);
}

void Lexer::_err_macro(string message) {
  throw MacroError(_cursor, message);
}

shared_ptr<Token::Control>
Lexer::_control(Token::Control::Kind kind) {
  return make_shared<Token::Control>(_reloc(), kind);
}

shared_ptr<Token::Value>
Lexer::_value(Token::Value::Kind kind, string value) {
  return make_shared<Token::Value>(_reloc(), kind, value);
}

Location Lexer::_reloc() {
  ltrace() << "[Lexer::_reloc] Resetting the location to "
           << _prev_cursor.row << ":" << _prev_cursor.col;

  auto copy = _location;
  copy.end.row = _prev_cursor.row;
  copy.end.col = _prev_cursor.col;

  _location.begin.row = _prev_cursor.row;
  _location.begin.col = _prev_cursor.col;

  return copy;
}

optional<variant<Token::Linebreak, Token::Codepoint>>
Lexer::_lex_codepoint(char terminator, bool ascii_only) {
  if (_is('\\')) {
    _read(); // Consume the backslash

    // The linebreak. It ignores the newline within
    // the literal, as if there never was one.
    //
    // ```
    // "foo \
    //   bar" == "foo bar"
    // ```
    if (_is('\n')) {
      _read(); // Consume the '\n' char
      return Token::Linebreak(_reloc());
    }

    // The kind is `Exact` by default.
    Token::Codepoint::Kind kind;

    if (_is_num() || _is('{')) {
      // Escaped digits optionally enclosed in curly brackets
      // are considered a decimal encoding of a codepoint.
      // For example, `'\65' == '\{65}' == '\d65' == 'A'`.
      //
      // NOTE: Won't `_read()` here because of
      // the further `want_closing_curly` logic.
      kind = Token::Codepoint::Decimal;
    } else {
      switch (_codeunit) {
      case 'i':
        _read();
        kind = Token::Codepoint::Binary;
        break;
      case 'o':
        _read();
        kind = Token::Codepoint::Octal;
        break;
      case 'd':
        _read();
        kind = Token::Codepoint::Decimal;
        break;
      case 'x':
        _read();
        kind = Token::Codepoint::Hexadecimal;
        break;
      }
    }

    // An escape sequence codepoint, e.g. `\n`.
    // Numeric sequence is considered a decimal char.
    // Unknown sequences are considered error.
    if (kind == Token::Codepoint::Exact) {
      enum Sequence {
        Backslash,         // `\\`
        SingleQuote,       // `\'`
        DoubleQuotes,      // `\"`
        CloseParen,        // `\)`
        CloseCurly,        // `\}`
        CloseSquare,       // `\]`
        CloseAngle,        // `\>`
        Percent,           // `\%`
        Alert,             // `\a`
        Backspace,         // `\b`
        Escape,            // `\e`
        FormfeedPageBreak, // `\f`
        Newline,           // `\n`
        CarriageReturn,    // `\r`
        HorizontalTab,     // `\t`
        VerticalTab,       // `\v`
      };

      static auto recognize =
          [](char codeunit) -> optional<Sequence> {
        switch (codeunit) {
        case '\\':
          return Sequence::Backslash;
        case '\'':
          return Sequence::SingleQuote;
        case '"':
          return Sequence::DoubleQuotes;
        case ')':
          return Sequence::CloseParen;
        case '}':
          return Sequence::CloseCurly;
        case ']':
          return Sequence::CloseSquare;
        case '>':
          return Sequence::CloseAngle;
        case '%':
          return Sequence::Percent;
        case 'a':
          return Sequence::Alert;
        case 'b':
          return Sequence::Backspace;
        case 'e':
          return Sequence::Escape;
        case 'f':
          return Sequence::FormfeedPageBreak;
        case 'n':
          return Sequence::Newline;
        case 'r':
          return Sequence::CarriageReturn;
        case 't':
          return Sequence::HorizontalTab;
        case 'v':
          return Sequence::VerticalTab;
        default:
          return nullopt;
        }
      };

      static auto to_char = [](Sequence seq) -> optional<char> {
        switch (seq) {
        case Alert:
          return '\a';
        case Backspace:
          return '\b';
        case Escape:
          return '\e';
        case FormfeedPageBreak:
          return '\f';
        case Newline:
          return '\n';
        case CarriageReturn:
          return '\r';
        case HorizontalTab:
          return '\t';
        case VerticalTab:
          return '\v';
        default:
          // Otherwise it's the codeunit itself
          return nullopt;
        }
      };

      static set<char> escapeables = {
          '\\', '\'', '\'', ')', '}', ']', '>', '%', 'a', 'b',
          'e',  'f',  'n',  'r', 't', 'v', 'i', 'o', 'd', 'x',
      };

      auto seq = recognize(_codeunit);
      if (!seq.has_value())
        _err_expect(escapeables);

      auto raw = to_char(seq.value());
      if (!raw.has_value())
        raw = _codeunit;

      _read(); // Consume the escaped character
      return Token::Codepoint(
          _reloc(), kind, raw.value(), string({'\\', _codeunit}));
    }

    bool want_closing_curly;

    if (_is('{')) {
      _read(); // Consume the opening curly bracket
      want_closing_curly = true;
    }

    if (kind == Token::Codepoint::Decimal) {
      // OPTIMIZE:
      string buf;

      bool is_underscore = false;
      while (_is_num() || _is('_')) {
        if (_is('_')) {
          if (is_underscore)
            _err(Error::UnderscoreDuplicate);

          // Won't put the underscore into the buffer
          is_underscore = true;
        }

        is_underscore = false;
        buf += _read();
      }

      long long cp = stoll(buf);

      if (ascii_only) {
        if (cp < 0 || cp > 255)
          _err(Error::CodepointOutOfRangeASCII);
      } else {
        if (cp < 0 || cp > 0xffffffff)
          _err(Error::CodepointOutOfRange);
      }

      if (want_closing_curly) {
        if (_is('}'))
          _read(); // Consume the closing curly bracket
        else
          _err_expect({'}'});
      }

      return Token::Codepoint(
          _reloc(), kind, (uint32_t)cp, "\\" + buf);
    }

    unsigned short max_length;
    function<bool()> check;

    switch (kind) {
    case Token::Codepoint::Binary:
      max_length = ascii_only ? 8 : 32;
      check = [this]() -> bool { return _is_binary(); };
      break;
    case Token::Codepoint::Octal:
      max_length = ascii_only ? 3 : 12;
      check = [this]() -> bool { return _is_octadecimal(); };
      break;
    case Token::Codepoint::Hexadecimal:
      max_length = ascii_only ? 2 : 8;
      check = [this]() -> bool { return _is_hexadecimal(); };
      break;
    default:
      throw "BUG";
    }

    // The source is always little-endian under the hood!
    vector<char> source(max_length);

    int size = 0;
    for (int size = 0; size < max_length; size++) {
      bool is_underscore;

      if (check()) {
        // `'\d6' == '\i011' == '\i01100000'`
        source.push_back(_codeunit);

        is_underscore = false;
      } else if (_is('_')) {
        if (is_underscore)
          _err(Error::UnderscoreDuplicate);

        is_underscore = true;
      } else {
        break;
      }

      _read();
    }

    string source_string(max_length, '0');

    int i = 0;
    for (char c : source)
      source_string[i++] = c;

    uint32_t result;

    switch (kind) {
    case Token::Codepoint::Binary:
      result = stoi(source_string, 0, 2);
      break;
    case Token::Codepoint::Octal:
      result = stoi(source_string, 0, 8);
      break;
    case Token::Codepoint::Hexadecimal:
      result = stoi(source_string, 0, 16);
      break;
    default:
      throw "BUG! Unexpected char kind";
    }

    if (want_closing_curly) {
      if (_is('}'))
        _read(); // Consume the closing curly bracket
      else
        _err_expect({'}'});
    }

    return Token::Codepoint(_reloc(), kind, result, source_string);
  } else {
    if (_is(terminator))
      return std::nullopt;

    try {
      char codeunits[4];
      auto size = UTF8::size_from_leading_byte(_codeunit);

      if (ascii_only && size > 1)
        _err(Error::CodepointOutOfRangeASCII);

      for (int i = 0; i < size; i++)
        codeunits[i] = _read();

      uint32_t codepoint = UTF8::to_codepoint(codeunits);

      return Token::Codepoint(
          _reloc(),
          Token::Codepoint::Exact,
          codepoint,
          string(codeunits));
    } catch (UTF8::Error &e) {
      _err(Error::CodepointMalformed);
    }

    throw "Unreacheable";
  }
}

generator<shared_ptr<Token::Base>> Lexer::_lex_char_literal() {
  if (!_is('\''))
    throw "BUG";

  _read(); // Consume the quote
  co_yield _control(Token::Control::SingleQuote);

  auto codepoint = _lex_codepoint('\'', true);

  if (codepoint.has_value()) {
    if (holds_alternative<Token::Linebreak>(codepoint.value()))
      _err(Error::CharLinebreak);

    if (!_is('\''))
      _err_expect({'\''});

    co_yield make_shared<Token::CharLiteral>(
        _reloc(), get<Token::Codepoint>(codepoint.value()));

    _read(false); // Consume the closing quote, allowing EOF
    co_yield _control(Token::Control::SingleQuote);
  } else
    _err(Error::CharEmpty);
}

generator<shared_ptr<Token::Base>> Lexer::_lex_string_literal() {
  if (!_is('"'))
    throw "BUG";

  _read(); // Consume the quotes
  co_yield _control(Token::Control::DoubleQuotes);

  vector<variant<Token::Linebreak, Token::Codepoint>> codepoints;
  while (true) {
    auto codepoint = _lex_codepoint('"', false);

    if (!codepoint.has_value())
      break;

    codepoints.push_back(codepoint.value());
  }

  if (!_is('"'))
    throw "BUG";

  co_yield make_shared<Token::StringLiteral>(_reloc(), codepoints);

  _read(false); // Consume the closing quote, allowing EOF
  co_yield _control(Token::Control::DoubleQuotes);
}

generator<shared_ptr<Token::Base>> Lexer::_lex_numeric_literal() {
  if (!_is_num())
    throw "BUG";

  std::function<bool()> check = [this]() -> bool {
    return _is_num();
  };

  Token::NumericLiteral::Radix radix;
  vector<char> whole;
  optional<vector<char>> fraction;
  optional<int> exponent;
  Token::NumericLiteral::Type type;
  uint32_t bitsize;

  Position begin = _prev_cursor;

  // Read a number matching the `check` function,
  // into the *container*. Would stop on a non-macthing
  // char, and raise in case of either preceding or
  // trailing underscore.
  const auto parse_number = [this, &check](vector<char> *container) {
    bool first = true;
    bool is_underscore = false;

    while (true) {
      if (check()) {
        // `6 == 0i011 == 0i01100000`
        container->push_back(_codeunit);
        is_underscore = false;
        first = false;
      } else if (_is('_')) {
        if (first)
          _err(Error::UnderscorePreceding);

        if (is_underscore)
          _err(Error::UnderscoreDuplicate);

        is_underscore = true;
        first = false;
      } else
        break;

      _read(false); // It's okay if EOF
    }

    if (is_underscore)
      _err(Error::UnderscoreTrailing);
  };

  std::function<char(void)> foo;
  const auto exponent_symbol = [&radix]() {
    if (radix == Token::NumericLiteral::Deci)
      return 'e';
    else
      return 'p';
  };

  if (_is('0')) {
    Position end = _cursor;
    _read(false);

    switch (_codeunit) {
    case EOF:
      co_yield make_shared<Token::NumericLiteral>(
          Location(_unit, begin, end), radix, whole);

      co_return;
    case '.': {
      whole.push_back('0');

      Position dot_begin = _prev_cursor;
      Position dot_end = _cursor;

      _read(); // The dot must be followed by something

      if (_is_num()) {
        // That's a floating point numeric literal
        // which whole part consists of
        // a single zero, e.g. `0.42`
        //

        fraction = vector<char>();
        break;

      } else {
        // That's a zero literal followed by a dot
        //

        co_yield make_shared<Token::NumericLiteral>(
            Location(_unit, begin, end), radix, whole);

        co_yield make_shared<Token::Control>(
            Location(_unit, dot_begin, dot_end),
            Token::Control::Dot);

        _reloc();
        co_return; // Halt the numerical lexing
      }
    }
    case 'i':
      _read();
      radix = Token::NumericLiteral::Bina;
      check = [this]() -> bool { return _is_binary(); };
      break;
    case 'o':
      _read();
      radix = Token::NumericLiteral::Octa;
      check = [this]() -> bool { return _is_octadecimal(); };
      break;
    case 'x':
      _read();
      radix = Token::NumericLiteral::Hexa;
      check = [this]() -> bool { return _is_hexadecimal(); };
      break;
    default:
      if (_is_num())
        _err(Error::NumericDecimalBeginsFromZero);
      else
        _err(Error::NumericUnknownRadix);
    }
  }

  // At this point, the codeunit is either `a` in `0xab`.
  // Or either the first whole or fraction digit in a decimal
  // literal, depending on whether does fraction has value.
  //

  if (!fraction.has_value()) {
    // We're still within the whole part.
    //

    if (radix != Token::NumericLiteral::Deci && _is('.'))
      // A dot CAN NOT go right after the radix symbol.
      _err(Error::NumericDotTooEarly);

    parse_number(&whole);

    if (whole.empty())
      _err(Error::NumericEmptyWholePart);

    if (_is('.')) {
      Position end = _prev_cursor;
      Position dot_begin = _prev_cursor;
      Position dot_end = _cursor;

      _read(); // Consume the dot

      if (check()) {
        // That's the fraction part,
        // continue lexing the literal
        fraction = vector<char>();
      } else {
        // The literal has ended.
        //

        co_yield make_shared<Token::NumericLiteral>(
            Location(_unit, begin, end), radix, whole);

        co_yield make_shared<Token::Control>(
            Location(_unit, dot_begin, dot_end),
            Token::Control::Dot);

        _reloc();
        co_return; // Halt the numerical lexing
      }
    } else {
      bool is_underscore;

      // Underscore is allowed between the
      // whole part and the exponent,
      // e.g. `42_e3`
      if (_is('_')) {
        _read();
        is_underscore = true;
      }

      if (_is(exponent_symbol())) {
        _read();
        exponent = 0;
      } else {
        if (is_underscore)
          _err(Error::UnderscoreTrailing);

        // Looks like the literal has ended
        //

        co_yield make_shared<Token::NumericLiteral>(
            Location(_unit, begin, _prev_cursor), radix, whole);

        _reloc();
        co_return;
      }
    }
  }

  // The fraction part can be skipped
  // in case of immediate exponent.
  if (fraction.has_value()) {
    parse_number(&fraction.value());

    if (fraction.value().size() == 0)
      _err(Error::NumericEmptyFractionPart);

    bool is_underscore;

    // Underscore is allowed between the
    // fraction part and the exponent,
    // e.g. `42.17_e3`
    if (_is('_')) {
      _read();
      is_underscore = true;
    }

    if (_is(exponent_symbol()))
      exponent = 0;
    else if (is_underscore)
      _err(Error::UnderscoreTrailing);
  }

  if (exponent.has_value()) {
    string buf;
    int sign = 1;

    if (_is('-')) {
      _read();
      sign = -1;
    } else if (_is('+'))
      _read();

    while (_is_num())
      buf += _read(false);

    if (buf.empty())
      _err(Error::NumericEmptyExponent);

    exponent = stoi(buf) * sign;

    if (!exponent.value())
      _err(Error::NumericZeroExponent);
  }

  bool is_underscore;

  if (_is('_')) {
    _read();
    is_underscore = true;
  }

  switch (_codeunit) {
  case 'i':
    type = Token::NumericLiteral::Type::Int;
    break;
  case 'u':
    type = Token::NumericLiteral::Type::UInt;
    break;
  case 'f':
    type = Token::NumericLiteral::Type::Float;
    break;
  case '_':
    if (is_underscore)
      _err(Error::UnderscoreDuplicate);
  }

  if (type != Token::NumericLiteral::Type::TypeUndef) {
    string buf;

    while (_is_num())
      buf += _read(false);

    if (!buf.empty()) {
      bitsize = (uint32_t)stol(buf);

      if (bitsize == 0)
        _err(Error::NumericInvalidBitsize);

      if (type == Token::NumericLiteral::Type::Float) {
        if (!(bitsize == 16 || bitsize == 32 || bitsize == 64))
          _err(Error::NumericInvalidBitsize);
      } else {
        if (bitsize > 8388607)
          _err(Error::NumericInvalidBitsize);
      }
    }
  } else if (is_underscore) {
    _err(Error::UnderscoreTrailing);
  }

  co_yield make_shared<Token::NumericLiteral>(
      Location(_unit, begin, _prev_cursor),
      radix,
      whole,
      fraction,
      exponent,
      type,
      bitsize);
}

generator<shared_ptr<Token::Base>> Lexer::lex() {
  // TODO:
  //
  //   * [ ] Blocks and their arguments
  //   * [ ] Pipe operator
  //   * [ ] Percent literals
  //   * [x] Comments
  //   * [x] Immediate macros
  //   * [x] Delayed macros
  //   * [x] Annotations
  //   * [x] Brackets
  //   * [x] Key-value
  //
  // Stateful? E.g. `.` only after `Callable`.
  // Match brackets?
  //
  while (!_input->eof()) {
    if (_is('{')) /* Macro */ {
      _read();

      if (_is('%')) {
        ltrace() << "[Lexer::lex] Encountered a non-emitting macro";
        _read();

        if (_ensure_macro()->is_incomplete()) {
          // A new macro while current expression is incomplete
          // means that the implicitly emitted expression has
          // ended, thus the time came to evaluate what's buffered.
          //
          // ```
          //   {% for i ... do %}
          //     @pp {{ i }}
          //   {% end %}
          // # ^ New macro, must evaluate
          // # `emit("@pp " .. 1 .. "\n")`
          // ```
          //

          ltrace() << "[Lexer::lex] New macro within an "
                   << "incomplete statement; "
                   << "evaluating what's buffered...";

          _macro->end_implicit_emit();
          _macro->eval();

          if (_macro->error.has_value())
            _err_macro(_macro->error.value());

          ltrace() << "[Lexer::lex] Successfully evaluated "
                   << "the macro buffer. Would not "
                   << "output from macro yet";
        }

        ltrace() << "[Lexer::lex] Reading the macro code...";

        // Within a macro, `%}` would mean macro termination.
        // Escaping with a backslash would prevent that: `\%}`.
        bool is_backslash = false;

        while (true) {
          if (_is('\\'))
            is_backslash = true;
          else if (_is('%') && !is_backslash) {
            _read();

            if (_is('}')) {
              // The macro has ended.
              // Do not read what's next yet,
              // as we'd like to evaluate
              // the macro first.
              break;
            }

            // That's just an escaped char,
            // the macro continues
            _macro->input << '%';
          } else {
            is_backslash = false;
            _macro->input << _codeunit;
          }

          _read();
        }

        // Evaluate the accumulated macro code
        ltrace() << "[Lexer::lex] Evaluating the macro...";
        _macro->eval();

        if (_macro->error.has_value()) {
          _err_macro(_macro->error.value());
        } else if (_macro->is_incomplete()) {
          // From now on, all the code is treated
          // as implicitly emitted by a macro.
          // This is true until a macro expression
          // is terminated (i.e. completed).
          ltrace() << "[Lexer::lex] The macro is incomplete";
          _macro->begin_implicit_emit();
          _read(); // Would raise on EOF if incomplete macro
        } else {
          // The macro's been completely evaluated without
          // any errors; it's time to read its output
          //

          ltrace() << "[Lexer::lex] The macro has been successfully "
                   << "evaluated. Start reading from its output";

          _is_reading_from_macro = true;
          _read(false); // There may be no output at all and also EOF
        }
      } else if (_is('{')) {
        _read();

        // That's an emitting macro, e.g. `{{ "foo" }}`.
        ltrace() << "[Lexer::lex] Encountered an emitting macro";
        _ensure_macro()->begin_explicit_emit();
        ltrace() << "[Lexer::lex] Reading the macro code...";

        // Within a macro, `}}` would mean macro termination.
        // Escaping with a backslash would prevent that: `\}}`.
        bool is_backslash = false;

        while (true) {
          if (_is('\\'))
            is_backslash = true;
          else if (_is('}') && !is_backslash) {
            _read();

            if (_is('}')) {
              // The macro has ended.
              // Do not read what's next yet,
              // as we'd like to evaluate
              // the macro first.
              break;
            }

            _macro->input << '}';
          } else {
            is_backslash = false;
            _macro->input << _codeunit;
          }

          _read();
        }

        // If within an incomplete macro expression,
        // would postpone the evaluation
        //

        if (_macro->is_incomplete()) {
          ltrace() << "[Lexer::lex] Explicit emitting macro "
                   << "is within an incomplete macro expression; "
                   << "would not evaluate";
          _macro->end_explicit_emit();
          _read(); // Would raise on EOF if incomplete macro
          continue;
        }

        // Otherwise, evaluate the emitting macro immediately.
        // It must be complete, as it's wrapped into an `emit` call.
        //

        ltrace()
            << "[Lexer::lex] Evaluating explicit emitting macro... ";
        _macro->eval();

        if (_macro->is_incomplete())
          throw "BUG! An emitting macro must be complete";

        if (_macro->error.has_value()) {
          _err_macro(_macro->error.value());
        } else {
          // The emitting macro's been evaluated
          // without errors; it's time to read its output
          //

          ltrace() << "[Lexer::lex] The macro has been successfully "
                   << "evaluated. Start reading from its output";

          _is_reading_from_macro = true;
          _read(false); // The output may be empty, EOF allowed
        }
      } else if (_macro && _macro->is_incomplete()) {
        // Within an incomplete macro expression block,
        // everything is wrapped into a macro `emit` call.
        ltrace() << "[Lexer::lex] Putting the code unit "
                 << "into the macro buffer";
        // _macro->ensure_implicit_emit();
        _macro->input << '{';
      } else {
        co_yield _control(Token::Control::OpenCurly);
      }
    } else if (_is('\\')) /* Delayed macro */ {
      _read();

      if (_is('{')) {
        _read();

        if (_is('%') || _is('{')) {
          bool is_emit = _is('{');
          _read();

          if (is_emit)
            co_yield _control(Token::Control::DelayedEmitMacro);
          else
            co_yield _control(Token::Control::DelayedMacro);

          bool backslash = false;
          string buff;

          ltrace() << "[Lexer::lex] Reading delayed macro input";
          while (true) {
            if (_is('\\'))
              backslash = true;
            else if ((is_emit ? _is('}') : _is('%')) && !backslash) {
              char c = _codeunit;
              _read();

              if (_is('}')) {
                co_yield _value(Token::Value::Text, buff);

                // Won't eval the macro, and it may be EOF
                _read(false);

                if (is_emit)
                  co_yield _control(Token::Control::EmitMacroClose);
                else
                  co_yield _control(Token::Control::MacroClose);

                break;
              } else {
                buff += c;
              }
            } else {
              backslash = false;
              buff += _codeunit;
            }

            _read();
          }
        } else
          _err_expect({"%", "{"});
      } else
        _err(); // Escaping what?

    } else if (_macro && _macro->is_incomplete()) {
      // Any code within an incomplete macro expression
      // is treated as emitted by the expression upon eval.
      //

      // _macro->ensure_begin_emitting_onyx_code();

      if (Macro::needs_escape(_codeunit))
        _macro->input << '\\' << _codeunit;
      else
        _macro->input << _codeunit;

      _read(); // EOF is not allowed until the expression is complete
      continue;

    } else if (_is(' ')) /* Whitespace(s) */ {
      // A sequence of whitespaces is treated as one
      // XXX: Other space characters?
      //

      while (_is(' '))
        _read(false);

      co_yield _control(Token::Control::Space);

    } else if (_is('\n')) /* Newline(s) */ {
      // According to the C standard, if
      // a file is opened in the text mode,
      // then OS-specific newlines are all
      // encoded using the `\n` char.
      //
      // Hence wouldn't worry about `\r`.
      //

      while (_is('\n'))
        _read(false);

      co_yield _control(Token::Control::Newline);

    } else if (_is('#')) /* Comment line */ {
      // A comment may end at any moment,
      // hence `false` here and below
      _read(false);

      co_yield _control(Token::Control::Comment);
      string buff;

      while (!_is(EOF) && !_is('\n')) {
        // Possible comment intrinsic, e.g. `:ditto:`.
        // Comment intrinsics begin from alpha and
        // contain alphanumeric and `-` characters.
        // An unknown intrinsic is ignored.
        if (_is(':')) {
          // Save the potential intrinsic begin position
          Position int_begin = _prev_cursor;
          _read(false); // Consume the `:`

          if (_is_alpha()) {
            string intrinsic;

            while (_is_alphanum() || _is('-'))
              intrinsic += _read(false);

            if (_is(':')) {
              _read(false);

              if (Token::Value::is_comment_intrinsic(intrinsic)) {
                // First, yield the text token
                //

                _location.end = int_begin;

                auto text_tok = Token::Value(
                    _location, Token::Value::Text, buff);

                co_yield make_shared<Token::Value>(text_tok);
                buff = "";

                // Then yield the intrinsic token
                //

                co_yield make_shared<Token::Value>(
                    Location(_unit, int_begin, _prev_cursor),
                    Token::Value::CommentIntrinsic,
                    intrinsic);

                // Finally, reset the current location
                _reloc();
              } else
                // An unknown intrinsic is ignored
                buff.append(intrinsic);
            } else
              // That is not a complete intrinsic,
              // e.g. `:ditto`. Still a legit comment
              buff.append(intrinsic);
          } else
            buff += ':'; // That's just a colon
        } else
          buff += _read(false);
      }

      if (!buff.empty())
        co_yield _value(Token::Value::Text, buff);

    } else if (_is('"')) /* String literal */ {
      for (auto tok : _lex_string_literal()) {
        co_yield tok;
      }
    } else if (_is('\'')) /* Char literal */ {
      for (auto tok : _lex_char_literal()) {
        co_yield tok;
      }
    } else if (_is('`')) /* C identifer */ {
      _read(); // Consume the tick

      string buff;

      if (_is_alpha() || _is('_'))
        buff += _read(false);
      else
        _err(Error::ValueInvalidCID);

      while (_is_alphanum() || _is('_'))
        buff += _read(false);

      if (_is('`'))
        _read(false); // Consume the optional closing tick

      co_yield _value(Token::Value::CID, buff);

    } else if (_is('%')) /* TODO: Percent literal */ {
      _read();

      static set<char> modifiers = {
          'q', // Quoted string
          'w', // Space-separated Words
          'y', // Space-separated sYmbols
          'c', // Space-separated Chars

          // These are `i` by default, but allow
          // explicit type, e.g. `%xu()`
          'o', // Space-separated octadecimal numeric literals
          'd', // Space-separated decimal numeric literals
          'x', // Space-separated hexadecimal numeric literals
          // 'b', // Space-separate binary numeric literals

          // These allow explicit bitsize (e.g. `%i32()`)
          'i', // Space-separated Int literals
          'u', // Space-separated UInt literals
          'f', // Space-separated Float literals
      };

      static set<char> radix_modifiers = {'x', 'o'};
      static set<char> numeric_modifiers = {'i', 'u', 'f'};
      static set<char> opening_brackets = {'(', '{', '[', '<'};

      Token::PercentLiteral::Type type;
      Token::PercentLiteral::NumericRadix numeric_radix;
      Token::PercentLiteral::NumericType numeric_type;
      Token::PercentLiteral::Bracket bracket;
      uint32_t numeric_bitsize;

      if (_is(modifiers)) {
        // That is an explicit percent literal
        if (_is(radix_modifiers)) {
          type = Token::PercentLiteral::Type::Numbers;

          switch (_codeunit) {
          case 'o':
            numeric_radix =
                Token::PercentLiteral::NumericRadix::Octa;
            break;
          case 'd':
            numeric_radix =
                Token::PercentLiteral::NumericRadix::Deci;
            break;
          case 'x':
            numeric_radix =
                Token::PercentLiteral::NumericRadix::Hexa;
            break;
          default:
            throw "BUG";
          }

          _read(); // Consume the radix
        }

        if (_is(numeric_modifiers)) {
          type = Token::PercentLiteral::Type::Numbers;

          switch (_codeunit) {
          case 'i':
            numeric_type = Token::PercentLiteral::NumericType::Int;
            break;
          case 'u':
            numeric_type = Token::PercentLiteral::NumericType::UInt;
            break;
          case 'f':
            numeric_type = Token::PercentLiteral::NumericType::Float;
            break;
          default:
            throw "BUG";
          }

          _read(); // Consume the type

          if (_is_num()) {
            string buf;

            while (_is_num())
              buf += _read();

            numeric_bitsize = (uint32_t)stol(buf);

            if (numeric_type ==
                Token::PercentLiteral::NumericType::Float) {
              if (!set{16, 32, 64}.count(numeric_bitsize)) {
                _err(Error::NumericInvalidBitsize);
              }
            } else if (numeric_bitsize > 8388607) {
              _err(Error::NumericInvalidBitsize);
            }
          }
        } else if (numeric_radix) {
          _err_expect({'i', 'u', 'f', '(', '{', '[', '<'});
        } else {
          switch (_codeunit) {
          case 'q':
            type = Token::PercentLiteral::Type::String;
            break;
          case 'w':
            type = Token::PercentLiteral::Type::Words;
            break;
          case 'y':
            type = Token::PercentLiteral::Type::Symbols;
            break;
          case 'c':
            type = Token::PercentLiteral::Type::Chars;
            break;
          default:
            throw "BUG";
          }

          _read();
        }
      } else if (_is(opening_brackets)) {
        // That is an implicit quoted string literal
        type = Token::PercentLiteral::Type::String;
      } else {
        // That's just the `%` symbol, may be an operator
        // Continue lexing a possible operator
        //

        string buff = string({'%'});

        while (_is_ascii_op())
          buff += _read();

        co_yield _value(Token::Value::Op, buff);
        continue;
      }

      switch (_codeunit) {
      case '(':
        bracket = Token::PercentLiteral::Bracket::Paren;
        break;
      case '{':
        bracket = Token::PercentLiteral::Bracket::Curly;
        break;
      case '[':
        bracket = Token::PercentLiteral::Bracket::Square;
        break;
      case '<':
        bracket = Token::PercentLiteral::Bracket::Angle;
        break;
      default:
        _err_expect(opening_brackets);
      }

      _read(); // Consume the opening bracket

      co_yield make_shared<Token::PercentLiteral>(
          _reloc(),
          type,
          bracket,
          numeric_radix,
          numeric_type,
          numeric_bitsize);

      switch (type) {
      case Token::PercentLiteral::Type::String:
      // TODO: _parse_string(bracket_terminator)
      case Token::PercentLiteral::Type::Words:
      // TODO: _parse_string(bracket_terminator, space_separator)
      case Token::PercentLiteral::Type::Symbols:
      // TODO: _parse_text(bracket_terminator, space_separator)
      // (ASCII only)
      case Token::PercentLiteral::Type::Chars:
      // TODO: _parse_chars(bracket_terminator, amount = unknown)
      case Token::PercentLiteral::Type::Numbers:
          // TODO: _parse_numeric_literals(bracket_terminator, amount
          // = unknown)
          ;
      }

    } else if (_is('~') || _is('-') || _is('=') || _is('|')) {
      auto prev = _codeunit;
      _read();

      if (_is('>')) {
        Token::Control::Kind kind;

        switch (prev) {
        case '~':
          kind = Token::Control::CurlyArrow;
          break;
        case '-':
          kind = Token::Control::ThinArrow;
          break;
        case '=':
          kind = Token::Control::ThickArrow;
          break;
        case '|':
          kind = Token::Control::PipeArrow;
          break;
        default:
          throw "BUG! Unmatched previous codeunit";
        }

        co_yield _control(kind);
      } else {
        // Continue lexing a possible operator
        //

        string buff = string({prev});

        while (_is_ascii_op())
          buff += _read();

        co_yield _value(Token::Value::Op, buff);
      }
    } else if (_is('@')) /* Annotation or intrinsic */ {
      _read();

      if (_is('[')) {
        _read();
        co_yield _control(Token::Control::Annotation);
      } else {
        string buff;

        if (_is_alpha() || _is('_'))
          buff += _read(false);
        else
          _err(Error::ValueInvalidIntrinsic);

        while (_is_alphanum() || _is('_'))
          buff += _read(false);

        co_yield _value(Token::Value::Intrinsic, buff);
      }
    } else if (_is_num()) /* Numeric literal */ {
      for (auto tok : _lex_numeric_literal()) {
        co_yield tok;
      }
    } else if (_is_alpha() || _is('_') || _is_ascii_op()) {
      Token::Value::Kind kind;

      if (_is_lowercase() || _is('_'))
        kind = Token::Value::ID;
      else if (_is_uppercase())
        kind = Token::Value::Type;
      else if (_is_ascii_op())
        kind = Token::Value::Op;
      else
        throw "BUG";

      string buff;

      switch (kind) {
      case Token::Value::ID:
      case Token::Value::Type:
        while (_is_alphanum() || _is('_'))
          buff += _read(false);

        if (_is(':'))
          kind = Token::Value::Kwarg;

        break;
      case Token::Value::Op:
        while (_is_ascii_op())
          buff += _read(false);

        if (buff == "=") {
          co_yield _control(Token::Control::Assignment);
          continue;
        } else
          break;
      default:
        throw "BUG";
      }

      co_yield _value(kind, buff);
    } else {
      switch (_codeunit) {
      case '.': /* Access or splat */
        _read();

        if (_is('.')) {
          _read();
          co_yield _control(Token::Control::Splat);
        } else {
          co_yield _control(Token::Control::Dot);
        }

        break;
      case ',':
        _read();
        co_yield _control(Token::Control::Comma);
        break;
      case ';':
        _read(false);
        co_yield _control(Token::Control::Semicolon);
        break;
      case ':': /* May be a symbol */ {
        _read();

        if (_is_alpha() || _is('_')) {
          // That's a bare symbol, e.g. `:foo`.
          //

          string buff;

          while (_is_alphanum() || _is('_'))
            buff += _read(false);

          co_yield _value(Token::Value::Symbol, buff);
        } else if (_is('"')) {
          // That's a string symbol, e.g. `:"foo"`.
          //
          // String symbols won't support non-exact
          // chars, because symbols are meant to be
          // readable for a developer.
          //

          string buff;
          bool escaped = false;

          while (!(_is('"') && !escaped)) {
            if (_is('\\')) {
              if (escaped) {
                buff += '\\';
                escaped = false;
              } else
                escaped = true;
            } else {
              escaped = false;
              buff += _read();
            }
          }

          _read(false); // Consume the closing quotes
          co_yield _value(Token::Value::StringSymbol, buff);
        } else {
          _read();

          if (_is('.')) {
            _read();
            co_yield _control(Token::Control::DotColon);
          } else if (_is(':')) {
            _read();
            co_yield _control(Token::Control::DoubleColon);
          } else {
            _read();
            co_yield _control(Token::Control::Colon);
          }
        }

        break;
      }
      case '(':
        _read();
        co_yield _control(Token::Control::OpenParen);
        break;
      case '{':
        _read();
        co_yield _control(Token::Control::OpenCurly);
        break;
      case '[':
        _read();
        co_yield _control(Token::Control::OpenSquare);
        break;
      case ')':
        _read(false);
        co_yield _control(Token::Control::CloseParen);
        break;
      case '}':
        _read(false);
        co_yield _control(Token::Control::CloseCurly);
        break;
      case ']':
        _read(false);
        co_yield _control(Token::Control::CloseSquare);
        break;
      default:
        _err();
      }
    }
  }

  if (_macro && _macro->is_incomplete())
    _err();
}

bool Lexer::_is(char cmp) { return _codeunit == cmp; }

bool Lexer::_is(const set<char> chars) {
  return chars.count(_codeunit);
}

bool Lexer::_is_binary() {
  return _codeunit == 0x30 || _codeunit == 0x31;
}

bool Lexer::_is_octadecimal() {
  return _codeunit >= 0x30 && _codeunit <= 0x37;
}

bool Lexer::_is_hexadecimal() {
  return (_codeunit >= 0x30 && _codeunit <= 0x39) ||
         (_codeunit >= 0x41 && _codeunit <= 0x46) ||
         (_codeunit >= 0x61 && _codeunit <= 0x66);
}

bool Lexer::_is_num() {
  return _codeunit >= 0x30 && _codeunit <= 0x39;
}

bool Lexer::_is_lowercase() {
  return _codeunit >= 0x61 && _codeunit <= 0x7A;
}

bool Lexer::_is_uppercase() {
  return _codeunit >= 0x41 && _codeunit <= 0x5A;
}

bool Lexer::_is_alpha() {
  return _is_lowercase() || _is_uppercase();
}
bool Lexer::_is_alphanum() { return _is_alpha() || _is_num(); }

bool Lexer::_is_ascii_op() {
  switch (_codeunit) {
  case 0x25: // %
  case 0x26: // &
  case 0x2A: // *
  case 0x2B: // +
  case 0x2D: // -
  case 0x2F: // /
  case 0x3C: // <
  case 0x3D: // =
  case 0x3E: // >
  case 0x5E: // ^
  case 0x7C: // |
  case 0x7e: // ~
    return true;
  default:
    return false;
  }
}
}; // namespace Compiler
} // namespace Onyx
