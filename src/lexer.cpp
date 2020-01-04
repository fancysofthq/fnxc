#include <istream>

#include "./lexer.hpp"

namespace Onyx {
Lexer::Cursor::Cursor(uint row, uint col) : row(row), col(col) {}

Lexer::Error::Error(const Location loc, const wstring msg) :
    location(loc),
    message(msg) {}

Lexer::Lexer(wistream *input, Macro *macro) : _input(input), _macro(macro) {
  read();

  if (!_input->eof())
    cursor.row += 1;
}

shared_ptr<Token::Base> Lexer::lex() {
  // TODO:
  //
  //   * [ ] Blocks and their arguments
  //   * [ ] Pipe operator
  //   * [x] Comments
  //   * [x] Immediate macros
  //   * [x] Delayed macros
  //   * [ ] Annotations
  //   * [ ] Interpolation
  //   * [ ] Percent literals
  //   * [ ] Brackets
  //   * [ ] Key-value
  //
  while (!_input->eof()) {
    _location.begin_row = cursor.row;
    _location.begin_column = cursor.col;

    if (is('{')) {
      read();

      if (is('%')) {
        read();

        if (_macro->is_incomplete) {
          // New macro means the emitted expression is likely to end.
          //
          // ```
          // {% for i ... do %}
          //   `print({{ i }})
          // {% end %}
          // ^ New macro, must evaluate `` emit("`print(" .. i .. ")\n") ``
          // ```
          //

          _macro->end_emit();
          _macro->eval();

          if (!_macro->error.empty())
            raise(L"Error executing macro: " + _macro->error);
        }

        bool backslash = false;

        while (true) {
          if (is(EOF))
            raise(L"Expected macro completion");
          else if (is('\\'))
            backslash = true;
          else if (is('%') && !backslash) {
            read();

            if (is('}')) {
              read();
              break;
            }

            _macro->input << '%';
          } else {
            backslash = false;
            _macro->input << _codepoint;
          }

          read();
        }

        // Evaluate the accumulated macro code
        _macro->eval();

        if (_macro->is_incomplete)
          _macro->begin_emit();
        else if (!_macro->error.empty())
          raise(L"Error while executing macro: " + _macro->error);
      } else if (is('{')) {
        // That's an emitting macro, e.g. `{{ "foo" }}`.
        //

        if (_macro->is_incomplete)
          _macro->begin_emitting_expression();

        bool backslash = false;

        while (true) {
          if (is(EOF))
            raise(L"Expected macro completion");
          else if (is('\\'))
            backslash = true;
          else if (is('}') && !backslash) {
            read();

            if (is('}')) {
              read();
              break;
            }

            _macro->input << '}';
          } else {
            backslash = false;
            _macro->input << _codepoint;
          }

          read();
        }

        if (_macro->is_incomplete) {
          _macro->end_emitting_expression();
          continue;
        }

        _macro->eval();

        if (_macro->is_incomplete)
          raise(L"Emitting macros must be complete");

        if (!_macro->error.empty())
          raise(L"Error while executing macro: " + _macro->error);
      } else if (_macro->is_incomplete) {
        _macro->ensure_begin_emitting_onyx_code();
        _macro->input << '{';
      } else
        return yield_control(Token::Control::OpenCurly);
    } else if (is('\\')) {
      read();

      if (is('{')) {
        read();

        if (is('%') || is('{')) {
          bool is_emit = is('{');
          bool backslash = false;
          wstring buff;

          while (true) {
            if (is(EOF))
              raise(L"Expected macro completion");
            else if (is('\\'))
              backslash = true;
            else if ((is_emit ? is('}') : is('%')) && !backslash) {
              wchar_t cp = _codepoint;
              read();

              if (is('}')) {
                read();
                break;
              }

              buff += cp;
            } else {
              backslash = false;
              buff += _codepoint;
            }

            read();
          }

          yield_value(Token::Value::Macro, buff);
        } else
          raise(L"Expected `{%` or `{{`");
      } else
        raise();
    } else if (_macro->is_incomplete) {
      _macro->ensure_begin_emitting_onyx_code();

      if (_macro->onyx_code_needs_escape(_codepoint))
        _macro->input << '\\' << _codepoint;
      else
        _macro->input << _codepoint;

      read();
    } else if (is(' ')) {
      // A sequence of whitespaces is treated as one
      //

      while (is(' '))
        read();

      return yield_control(Token::Control::Space);
    } else if (is('\n')) {
      read();
      return yield_control(Token::Control::Newline);
    } else if (is('\r')) {
      read();

      if (is('\n')) {
        read();
        return yield_control(Token::Control::Newline);
      } else
        raise();
    } else if (is('&')) {
      read();

      if (is_digit()) {
        // That's an indexed anonymous argument, e.g. `&1`
        //

        string buff;

        while (is_digit())
          buff += (char)read();

        return make_shared<Token::AnonArg>(_location, stoi(buff));
      } else if (is_op()) {
        // That's an operator
        //

        wstring buff;
        buff += '&';

        while (is_op())
          buff += read();

        return yield_value(Token::Value::Op, buff);
      } else
        // That's either an operator or an anonymous argument
        return yield_value(Token::Value::Op, L"&");
    } else if (is('"')) {
      read(); // Consume the opening quotes

      wstring buff;
      bool _is_backslash = false;

      while (!(is('"') && !_is_backslash)) {
        if (is('\\')) {
          _is_backslash = true;
          read();
          continue;
        } else {
          if (_is_backslash) {
            buff += L'\\';
            _is_backslash = false;
          }

          buff += read();
        }
      }

      read(); // Consume the closing quotes
      return yield_value(Token::Value::String, buff);
    } else if (is_digit()) {
      if (is('0')) {
        read(); // We don't need this zero

        if (is('.')) {
          read(); // Consume the dot

          vector<char> digits;
          digits.push_back('0');

          if (is_digit())
            // That's a decimal float literal beginning with zero
            return lex_decimal(digits, true);
          else
            // That's a zero decimal literal followed by a call
            return make_shared<Token::DecimalInt>(_location, digits);
        } else if (is_digit())
          raise(L"Numbers can't begin with zero");
        else if (is('b') || is('o') || is('x')) {
          // That's a non-decimal numeric literal
          lex_nondecimal();
        } else {
          // That's a decimal zero literal
          //

          vector<char> digits;
          digits.push_back('0');

          return lex_decimal(digits);
        }
      } else {
        vector<char> significand;

        while (is_digit())
          significand.push_back((char)read());

        lex_decimal(significand);
      }
    } else if (
        is_alpha() || is('_') || is_op() || is('@') || is('`') || is('#')) {
      Token::Value::Kind kind;

      if (is_lowercase_latin() || is_lowercase_greek() || is('_'))
        kind = Token::Value::Var;
      else if (is_uppercase_latin() || is_uppercase_greek())
        kind = Token::Value::Type;
      else if (is_op())
        kind = Token::Value::Op;
      else if (is('@'))
        kind = Token::Value::Intrinsic;
      else if (is('`'))
        kind = Token::Value::C;
      else if (is('#'))
        kind = Token::Value::Comment;
      else
        throw "BUG";

      wstring buff;

      switch (kind) {
      case Token::Value::Var:
      case Token::Value::Type:
      case Token::Value::Intrinsic:
        while (is_alphanum() || is('_'))
          buff += read();

        break;
      case Token::Value::Op:
        while (is_op())
          buff += read();

        break;
      case Token::Value::C:
        while (is_latin() || is_digit() || is('_'))
          buff += read();

        break;
      case Token::Value::Comment:
        while (!(is('\n') || is('\r')))
          buff += read();

        break;
      case Token::Value::String:
      case Token::Value::Macro:
        throw "BUG"; // Already handled
      }

      return yield_value(kind, buff);
    } else
      raise();
  }

  return yield_control(Token::Control::Eof);
}

shared_ptr<Token::Base>
Lexer::lex_decimal(vector<char> significand, bool explicitly_float) {
  bool is_float;
  bool is_exponent_negative;
  string exponent;

  if (is('.') || explicitly_float) {
    read();

    if (!is_digit()) {
      if (explicitly_float)
        raise(L"Expected digits after the floating point");
      else
        return make_shared<Token::DecimalInt>(_location, significand);
    }

    is_float = true;

    while (is_digit())
      significand.push_back((char)read());

    if (is('e')) {
      read();

      if (is('-')) {
        is_exponent_negative = true;
        read();
      }

      while (is_digit())
        exponent += (char)read();

      if (exponent.empty())
        raise(L"Exponent can't be empty");
    }
  }

  bool expect_type;

  if (is('_')) {
    expect_type = true;
    read();
  }

  Token::DecimalInt::Type int_type;
  bool expect_bitsize;
  string int_bitsize;
  Token::DecimalFloat::Bitsize float_bitsize;

  switch (_codepoint) {
  case 'i':
    if (is_float)
      raise(L"Expected `f`");

    int_type = Token::DecimalInt::Type::Signed;
    expect_bitsize = true;
    read();

    break;
  case 'u':
    if (is_float)
      raise(L"Expected `f`");

    int_type = Token::DecimalInt::Type::Unsigned;
    expect_bitsize = true;
    read();

    break;
  case 'f':
    if (!is_float)
      raise(L"Expected `i` or `u`");

    expect_bitsize = true;
    read();

    break;
  default:
    if (expect_type) {
      if (is_float)
        raise(L"Expected `f`");
      else
        raise(L"Expected `i` or `u`");
    }
  }

  if (expect_bitsize) {
    while (is_digit())
      int_bitsize += (char)read();

    if (is_float) {
      switch (stoi(int_bitsize)) {
      case 16:
        float_bitsize = Token::DecimalFloat::Float16;
        break;
      case 32:
        float_bitsize = Token::DecimalFloat::Float32;
        break;
      case 64:
        float_bitsize = Token::DecimalFloat::Float64;
        break;
      default:
        raise(L"Expected `16`, `32` or `64`");
      }
    }
  }

  if (is_float) {
    int exponent_value = stoi(exponent);

    if (is_exponent_negative)
      exponent_value *= -1;

    return make_shared<Token::DecimalFloat>(
        _location, significand, exponent_value, float_bitsize);
  } else
    return make_shared<Token::DecimalInt>(
        _location, significand, int_type, stoi(int_bitsize));
}

shared_ptr<Token::NonDecimalNumber> Lexer::lex_nondecimal() {
  Token::NonDecimalNumber::Kind kind;

  vector<char> buff;

  switch (_codepoint) {
  case 'b':
    kind = Token::NonDecimalNumber::Bina;

    while (is_binary())
      buff.push_back(read());

    break;
  case 'o':
    kind = Token::NonDecimalNumber::Octa;

    while (is_octadecimal())
      buff.push_back(read());

    break;
  case 'x':
    kind = Token::NonDecimalNumber::Hexa;

    while (is_hexadecimal())
      buff.push_back(read());

    break;
  default:
    throw "BUG";
  };

  Token::NonDecimalNumber::Type type;

  switch (_codepoint) {
  case 'i':
    type = Token::NonDecimalNumber::SignedInt;
    read();
    break;
  case 'u':
    type = Token::NonDecimalNumber::UnsignedInt;
    read();
    break;
  case 'f':
    type = Token::NonDecimalNumber::Float;
    read();
    break;
  }

  uint bitsize;

  if (is_digit()) {
    string buff;

    while (is_digit())
      buff += (char)read();

    bitsize = stoi(buff);
  }

  return make_shared<Token::NonDecimalNumber>(
      _location, kind, buff, type, bitsize);
};

wchar_t Lexer::read() {
  wchar_t previous = _codepoint;

  if (_is_reading_from_macro) {
    auto stream = &_macro->output;

    if (stream->eof())
      _is_reading_from_macro = false;
    else {
      _codepoint = stream->get();
      return previous;
    }
  }

  // Reading from the `_input` would not happen
  // if already read from the macro input.
  //

  // Move the end location one codepoint forward
  _location.end_row = cursor.row;
  _location.end_column = cursor.col;

  _codepoint = _input->get();

  if (is('\n')) {
    cursor.col = 0;
    cursor.row += 1;
  } else if (!_input->eof())
    cursor.col += 1;

  return previous;
}

void Lexer::raise(wstring message) { throw Error(_location, message); }

shared_ptr<Token::Control> Lexer::yield_control(Token::Control::Kind kind) {
  return make_shared<Token::Control>(_location, kind);
}

shared_ptr<Token::Value>
Lexer::yield_value(Token::Value::Kind kind, wstring value) {
  return make_shared<Token::Value>(_location, kind, value);
}

bool Lexer::is(wchar_t cmp) { return _codepoint == cmp; }
bool Lexer::is_underscore() { return is('_'); }
bool Lexer::is_binary() { return _codepoint == 0x30 || _codepoint == 0x31; }

bool Lexer::is_octadecimal() {
  return _codepoint >= 0x30 && _codepoint <= 0x37;
}

bool Lexer::is_hexadecimal() {
  return (_codepoint >= 0x30 && _codepoint <= 0x39) ||
         (_codepoint >= 0x41 && _codepoint <= 0x46) ||
         (_codepoint >= 0x61 && _codepoint <= 0x66);
}

bool Lexer::is_digit() { return _codepoint >= 0x30 && _codepoint <= 0x39; }

bool Lexer::is_lowercase_latin() {
  return _codepoint >= 0x61 && _codepoint <= 0x7A;
}

bool Lexer::is_uppercase_latin() {
  return _codepoint >= 0x41 && _codepoint <= 0x5A;
}

bool Lexer::is_latin() { return is_lowercase_latin() || is_uppercase_latin(); }

bool Lexer::is_lowercase_greek() {
  return _codepoint >= 0x03B1 && _codepoint <= 0x03C9;
}

bool Lexer::is_uppercase_greek() {
  return (_codepoint >= 0x0391 && _codepoint <= 0x03A1) ||
         (_codepoint >= 0x03A3 && _codepoint <= 0x03A9);
}
bool Lexer::is_greek() { return is_lowercase_greek() || is_uppercase_greek(); }
bool Lexer::is_alpha() { return is_latin() || is_greek(); }
bool Lexer::is_alphanum() { return is_alpha() || is_digit(); }

bool Lexer::is_op() {
  switch (_codepoint) {
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
  }

  // Mathematical Operators, see
  // https://en.wikipedia.org/wiki/Mathematical_operators_and_symbols_in_Unicode#Mathematical_Operators_block
  if (_codepoint >= 0x2200 && _codepoint <= 0x22FF)
    return true;

  // Supplemental Mathematical Operators, see
  // https://en.wikipedia.org/wiki/Mathematical_operators_and_symbols_in_Unicode#Supplemental_Mathematical_Operators_block
  if (_codepoint >= 0x2A00 && _codepoint <= 0x2AFF)
    return true;

  return false;
}
}; // namespace Onyx
