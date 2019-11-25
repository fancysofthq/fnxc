#include <fstream>

#include "./lexer.hpp"

namespace Onyx {
Lexer::Cursor::Cursor(uint row, uint col) : row(row), col(col) {}

Lexer::Error::Error(const Location loc, const wstring msg) :
    location(loc),
    message(msg) {}

Lexer::Lexer(wistream *input) : _input(input) {
  read();

  if (!_input->eof())
    cursor.row += 1;
}

// Function names begin with latin lowercase letters and allow math symbols,
// but can't mix them:
//
// def ∫ (a, b, c)
// end
//
// Math::∫(x, 1, 2)
//
// def unop ∑
//   return self.sum
// end
//
// √(x + 3)
// ∑{1, 2, 3}
//
// Variables can have latin and greek letters (both lower- and uppercase):
//
// var α = 42
// const Π = 3.14
//
// Types begin with uppercase latin and greek letters, but can't mix them:
//
// class ΦΔΣ
// end
//
shared_ptr<Token::Base> Lexer::lex() {
  // TODO:
  //
  //   * [ ] Blocks and their arguments
  //   * [ ] Pipe operator
  //   * [ ] Comments
  //   * [ ] Immediate macros
  //   * [ ] Delayed macros
  //   * [ ] Annotations
  //   * [ ] Interpolation
  //
  while (!_input->eof()) {
    _location.begin_row = cursor.row;
    _location.begin_column = cursor.col;

    if (is(' ')) {
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

        string buffer;

        while (is_digit())
          buffer += (char)read();

        return make_shared<Token::AnonArg>(_location, stoi(buffer));
      } else
        // That's either an operator or an anonymous argument
        return yield_id(Token::Id::Op, L"&");
    } else if (is('"')) {
      read(); // Consume the opening quotes

      wstring buffer;
      bool _is_backslash = false;

      while (!(is('"') && !_is_backslash)) {
        if (is('\\')) {
          _is_backslash = true;
          read();
          continue;
        } else {
          if (_is_backslash) {
            buffer += L'\\';
            _is_backslash = false;
          }

          buffer += read();
        }
      }

      read(); // Consume the closing quotes
      return make_shared<Token::String>(_location, buffer);
    } else if (is_digit()) {
      if (is('0')) {
        read(); // We don't need this zero

        if (is('.')) {
          read(); // Consume the dot

          if (is_digit())
            // That's a decimal float literal beginning with zero
            return lex_decimal("0", true);
          else
            // That's a zero decimal literal followed by a call
            return make_shared<Token::DecimalInt>(_location, 0);
        } else if (is_digit())
          raise(L"Numbers can't begin with zero");
        else if (is('b') || is('o') || is('x')) {
          // That's a non-decimal numeric literal
          lex_nondecimal();
        } else
          // That's a decimal zero literal
          return lex_decimal("0");
      } else {
        string significand;

        while (is_digit())
          significand += (char)read();

        lex_decimal(significand);
      }
    } else if (is_alpha() || is('_') || is('@') || is('`') || is_op()) {
      Token::Id::Kind kind;

      if (is_lowercase_latin() || is_lowercase_greek() || is('_'))
        kind = Token::Id::Var;
      else if (is_uppercase_latin() || is_uppercase_greek())
        kind = Token::Id::Type;
      else if (is_op())
        kind = Token::Id::Op;
      else if (is('@'))
        kind = Token::Id::Intrinsic;
      else if (is('`'))
        kind = Token::Id::C;
      else
        raise(L"BUG: Unhandled case");

      wstring buffer;

      switch (kind) {
      case Token::Id::Var:
      case Token::Id::Type:
      case Token::Id::Intrinsic:
        while (is_alphanum() || is('_'))
          buffer += read();

        break;
      case Token::Id::Op:
        while (is_op())
          buffer += read();

        break;
      case Token::Id::C:
        while (is_latin() || is_digit() || is('_'))
          buffer += read();

        break;
      }

      return yield_id(kind, buffer);
    } else
      raise();
  }

  return yield_control(Token::Control::Eof);
}

shared_ptr<Token::Base>
Lexer::lex_decimal(string significand, bool explicitly_float) {
  bool is_float;
  bool is_exponent_negative;
  string exponent;

  if (is('.') || explicitly_float) {
    read();

    if (!is_digit()) {
      if (explicitly_float)
        raise(L"Expected number");
      else
        return make_shared<Token::DecimalInt>(_location, stoi(significand));
    }

    is_float = true;

    while (is_digit())
      significand += (char)read();

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
        _location, stoi(significand), exponent_value, float_bitsize);
  } else
    return make_shared<Token::DecimalInt>(
        _location, stoi(significand), int_type, int_bitsize);
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
    raise(L"BUG");
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

bool Lexer::is(wchar_t cmp) { return _codepoint == cmp; }

bool Lexer::is_underscore() { return is('_'); }
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

// shared_ptr<Token> Lexer::lex() {
//   while (!_input->eof()) {
//     _location.begin_row = cursor.row;
//     _location.begin_column = cursor.col;

//     if (is(' ')) {
//       switch (_state.top()) {
//       case State::Access:
//         raise(L"Unexpected whitespace");
//       default:
//         break;
//       }

//       postread(); // Consume unused whitespaces
//       continue;

//     } else if (is('\n')) {
//       switch (_state.top()) {
//       case State::Access:
//       case State::AccessMod:
//         raise(L"Unexpected newline"); // Callee must be on the same line
//       default:
//         break;
//       }

//       postread(); // Consume the newline
//       return yield(Token::Newline);

//     } else if (is('\r')) {
//       switch (_state.top()) {
//       case State::Access:
//       case State::AccessMod:
//         raise(L"Unexpected carriage return");
//       default:
//         break;
//       }

//       buffread(); // Buff and consume the CR

//       if (is('\n')) {
//         postread(); // Consume the newline
//         return yield(Token::Newline);
//       } else
//         continue;

//     } else if (
//         is_lowercase_latin() || is_lowercase_greek() || is_underscore()) {
//       // Identifiers begin with a lowercase letter or an underscore.
//       // Then they may contain letters, digits and underscores.
//       //

//       while (is_latin() || is_greek() || is_digit() || is_underscore())
//         buffread();

//       switch (_state.top()) {
//       case State::Space:
//         _state.pop();

//         switch (_state.top()) {
//         case State::Expression:
//           // `(foo) bar # Panic!`
//           //        ^
//           raise(L"Unexpected identifier");
//         case State::Empty:
//           _state.push(State::Reference);
//         case State::Reference:
//           // Treated as an argument.
//           //
//           // `foo bar`
//           //      ^
//           //
//           return yield(Token::Id);
//         case State::AccessMod:
//           // Treated as a successive modifier.
//           //
//           // `foo.(bar baz)`
//           //           ^
//           //
//           return yield(Token::Id);
//         default:
//           raise(L"BUG: Unhandled case");
//         }

//         break;

//       case State::Expression:
//         // `(foo)bar # Panic!`
//         //       ^
//         raise(L"Unexpected identifier");
//       case State::Access:
//         // `foo.bar`
//         //      ^
//         _state.pop(); // Access state is satisified
//       case State::AccessMod:
//         // Within access modifiers, the state does not change
//         //
//         // `foo.(bar)`
//         //       ^
//         //
//         return yield(Token::Id);
//       case State::Unop:
//         // `!foo`
//         //   ^
//       case State::Binop:
//         // `foo + bar`
//         //        ^
//         _state.pop(); // No need for the op state anymore
//       case State::Empty:
//         _state.push(State::Reference);
//         return yield(Token::Id);
//       case State::Reference:
//         raise(L"BUG: Unhandled case");
//       }

//     } else if (is('.')) {
//       switch (_state.top()) {
//       case State::Expression:
//         _state.push(State::Access);
//         return yield(Token::Access);
//       default:
//         raise();
//       }

//     } else if (is('(')) {
//       postread();

//       switch (_state.top()) {
//       case State::Empty:
//       case State::Binop:
//       case State::Unop:
//       case State::Call:
//       case State::Expression:
//         _state.push(State::Expression);
//         break;
//       case State::Access:
//         _state.push(State::AccessMod);
//         break;
//       case State::AccessMod:
//         raise(); // No expressionas are allowed within a modifiers list
//       }

//       _terminators.push(Terminator(_location, Token::CloseParen));
//       return yield(Token::OpenParen);

//     } else if (_codepoint == L'"') {
//       postread(); // Consume the opening quotes

//       wstring buffer;
//       bool _is_backslash = false;

//       while (!(_codepoint == L'"' && !_is_backslash)) {
//         if (_codepoint == L'\\') {
//           _is_backslash = true;
//           postread();
//           continue;
//         } else {
//           if (_is_backslash) {
//             buffer.append(L"\\");
//             _is_backslash = false;
//           }

//           buffer.append(wstring(1, _codepoint));
//           postread();
//         }
//       }

//       postread(); // Consume the closing quotes
//       return make_shared<Token::StringLiteral>(_location, buffer);
//     }

//     // Nothing matched, just yield the token as-is then.
//     // Note that we don't know the exact codepoint size
//     //

//     wstring returned(1, _codepoint);
//     postread();

//     return make_shared<Token::Other>(_location, returned);
//   }

//   return make_shared<Token::Eof>(_location);
// } // namespace Onyx
