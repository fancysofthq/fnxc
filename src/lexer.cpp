#include <fstream>

#include "./lexer.hpp"

namespace Onyx {
Lexer::Cursor::Cursor(uint row, uint col) : row(row), col(col) {}

Lexer::Error::Error(const Location loc, const wstring msg) :
    location(loc),
    message(msg) {}

Lexer::Lexer(wifstream *input) : _input(input) {
  read_codepoint();

  if (!input->eof())
    cursor.row += 1;
}

shared_ptr<Token::Base> Lexer::lex() {
  while (!_input->eof()) {
    _location.begin_row = cursor.row;
    _location.begin_column = cursor.col;

    if (_codepoint == L' ') {
      read_codepoint(); // Skip whitespaces
      continue;
    } else if (_codepoint == L'\n') {
      read_codepoint();
      return make_shared<Token::Newline>(_location);
    } else if (_codepoint == L'\r') {
      read_codepoint();

      if (_codepoint == L'\n') {
        read_codepoint();
        return make_shared<Token::Newline>(_location);
      } else {
        read_codepoint();
        return make_shared<Token::Other>(_location, L"\r");
      }
    } else if (is_lowercase_latin() || is_underscore()) {
      wstring buffer;

      while (is_lowercase_latin() || is_digit() || is_underscore()) {
        buffer.append(wstring(1, _codepoint));
        read_codepoint();
      }

      return make_shared<Token::ID>(_location, buffer);
    } else if (_codepoint == L'"') {
      read_codepoint(); // Consume the opening quotes

      wstring buffer;
      bool _is_backslash = false;

      while (!(_codepoint == L'"' && !_is_backslash)) {
        if (_codepoint == L'\\') {
          _is_backslash = true;
          read_codepoint();
          continue;
        } else {
          if (_is_backslash) {
            buffer.append(L"\\");
            _is_backslash = false;
          }

          buffer.append(wstring(1, _codepoint));
          read_codepoint();
        }
      }

      read_codepoint(); // Consume the closing quotes
      return make_shared<Token::StringLiteral>(_location, buffer);
    }

    // Nothing matched, just yield the token as-is then.
    // Note that we don't know the exact codepoint size
    //

    wstring returned(1, _codepoint);
    read_codepoint();

    return make_shared<Token::Other>(_location, returned);
  }

  return make_shared<Token::Eof>(_location);
}

wchar_t Lexer::read_codepoint() {
  _location.end_row = cursor.row;
  _location.end_column = cursor.col;

  _codepoint = _input->get();

  if (_codepoint == L'\n') {
    cursor.col = 0;
    cursor.row += 1;
  } else if (!_input->eof())
    cursor.col += 1;

  return _codepoint;
}

bool Lexer::is_underscore() { return _codepoint == L'_'; }
bool Lexer::is_digit() { return _codepoint >= 0x30 && _codepoint <= 0x39; }
bool Lexer::is_lowercase_latin() {
  return _codepoint >= 0x61 && _codepoint <= 0x7A;
}
}; // namespace Onyx

// #include <fstream>
// #include <memory>

// #include "./token.hpp"

// using namespace std;

// namespace Onyx {
// // Lexer takes an input file and lexes it into series of tokens,
// // which are then feed to the Parser. The lexer is also responsible
// // for evaluating immediate macros.
// class Lexer {
//   wifstream *_input;
//   wchar_t _codepoint;
//   Location _location;

// public:
//   struct Cursor {
//     uint row;
//     uint col;

//     Cursor(uint row, uint col) : row(row), col(col) {}
//   };

//   Cursor cursor = Cursor(0, 0);

//   struct Error {
//     const Location location;
//     const wstring message;

//     Error(const Location loc, const wstring msg) : location(loc),
//     message(msg) {}
//   };

//   Lexer(wifstream *input) : _input(input) {
//     read_codepoint();

//     if (!input->eof())
//       cursor.row += 1;
//   }

//   shared_ptr<Token::Base> lex() {
//     while (!_input->eof()) {
//       _location.begin_row = cursor.row;
//       _location.begin_column = cursor.col;

//       if (_codepoint == L' ') {
//         read_codepoint();
//         continue;
//       } else if (_codepoint == L'\n') {
//         read_codepoint();
//         return make_shared<Token::Newline>(_location);
//       } else if (_codepoint == L'\r') {
//         read_codepoint();

//         if (_codepoint == L'\n') {
//           read_codepoint();
//           return make_shared<Token::Newline>(_location);
//         } else {
//           read_codepoint();
//           return make_shared<Token::Other>(_location, L"\r");
//         }
//       } else if (is_lowercase_latin() || is_underscore()) {
//         wstring buffer;

//         while (is_lowercase_latin() || is_digit() || is_underscore()) {
//           buffer.append(wstring(1, _codepoint));
//           read_codepoint();
//         }

//         return make_shared<Token::ID>(_location, buffer);
//       } else if (_codepoint == L'"') {
//         read_codepoint(); // Consume the opening quotes

//         wstring buffer;
//         bool _is_backslash = false;

//         while (!(_codepoint == L'"' && !_is_backslash)) {
//           if (_codepoint == L'\\') {
//             _is_backslash = true;
//             read_codepoint();
//             continue;
//           } else {
//             if (_is_backslash) {
//               buffer.append(L"\\");
//               _is_backslash = false;
//             }

//             buffer.append(wstring(1, _codepoint));
//             read_codepoint();
//           }
//         }

//         read_codepoint(); // Consume the closing quotes
//         return make_shared<Token::StringLiteral>(_location, buffer);
//       }

//       // Nothing matched, just yield the token as-is then.
//       // Note that we don't know the exact codepoint size
//       //

//       wstring returned(1, _codepoint);
//       read_codepoint();

//       return make_shared<Token::Other>(_location, returned);
//     }

//     return make_shared<Token::Eof>(_location);
//   }

// private:
//   // Read the next codepoint from the input.
//   wchar_t read_codepoint() {
//     _location.end_row = cursor.row;
//     _location.end_column = cursor.col;

//     _codepoint = _input->get();

//     if (_codepoint == L'\n') {
//       cursor.col = 0;
//       cursor.row += 1;
//     } else if (!_input->eof())
//       cursor.col += 1;

//     return _codepoint;
//   }

//   bool is_underscore() { return _codepoint == L'_'; }
//   bool is_digit() { return _codepoint >= 0x30 && _codepoint <= 0x39; }
//   bool is_lowercase_latin() { return _codepoint >= 0x61 && _codepoint <=
//   0x7A; }
// };
// } // namespace Onyx
