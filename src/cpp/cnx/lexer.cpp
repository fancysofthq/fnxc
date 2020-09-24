#include <optional>

#include "../../../include/fnx/cnx/lexer.hpp"

namespace FNX {
namespace CNX {

Utils::generator<std::variant<C::AnyToken, Lexer::Signal>>
Lexer::lex() {
  // DESIGN: A CNX lexer expects either a single C entity or a block
  // to be exported, which implies that it must be terminated either
  // with `}` or `;`.
  //

  if (_input.eof()) {
    throw Error("Unexpected EOF");
  }

  // DESIGN: An exported block must have the opening curly bracket on
  // the same line as the `export` keyword as the first
  // non-whitespace symbol.
  //

  if (_is_space()) {
    _lex_space();
  }

  if (_is('{')) {
    _advance();
    _is_exporting_block.emplace(true);
    co_yield C::Token::Punctuation('{');
  }

  while (!_input.eof()) {
    // Begin with the `@` symbol, which may only either denote an
    // Onyx annotation application or macro call.
    //

    if (_is('@')) {
      co_yield Signal::At;
    }

    // In C, a freestanding backslash acts as a linebreak.
    // It may also denote a delayed or final Onyx macro.
    //

    else if (_is('\\')) {
      _advance();

      if (_is('!')) {
        _advance();

        if (_is('{')) {
          co_yield Signal::FinalMacro;
        } else if (_is('@')) {
          co_yield Signal::FinalAt;
        } else {
          throw Lexer::Error(); // '\!' does not make sense in C
        }
      } else if (_is('{')) {
        co_yield Signal::DelayedMacro;
      } else if (_is('@')) {
        co_yield Signal::DelayedAt;
      } else {
        co_yield C::Token::Punctuation('\\');
      }
    }

    // `{` may be an Onyx macro start, othwerwise it is a C
    // punctuation token.
    //

    else if (_is('{')) {
      _advance();

      if (_is('%')) {
        _advance();
        co_yield Signal::Macro;
      } else if (_is('{')) {
        _advance();
        co_yield Signal::EmittingMacro;
      } else {
        co_yield C::Token::Punctuation('{');
        _block_pairs++;
      }
    }

    else if (_is('}')) {
      _advance();

      if (--_block_pairs < 0) {
        throw Error("Unmatched bracket");
      } else {
        co_yield C::Token::Punctuation('}');

        if (_block_pairs == 0 && _is_exporting_block.value()) {
          co_return; // The end of the exported block
        }
      }
    }

    else if (_is(';')) {
      _advance();
      co_yield C::Token::Punctuation(';');

      if (_block_pairs == 0 && !_is_exporting_block.value()) {
        co_return; // The end of the single exported entity
      }
    }

    // Now what's left is pure C.
    //

    // `/` may begin a comment, or be a part of an operator.
    //

    else if (_is('/')) {
      _advance();

      if (_is('/')) {
        _advance();
        _lex_comment();
      } else if (_is_ascii_op()) {
        _lex_ascii_op('/');
      } else {
        co_yield C::Token::Operator((char[1]){'/'});
      }
    }

    else if (_is({'.', ',', '#', '?', '(', ')', '[', ']'})) {
      co_yield C::Token::Punctuation(_code_unit);
      _advance();
    }
  }

  if (_is_exporting_block.value()) {
    throw Error("TODO: Unterminated exported block");
  } else {
    throw Error("TODO: Unterminated exported entity");
  }
}

} // namespace CNX
} // namespace FNX
