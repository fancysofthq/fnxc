#pragma once

#include <istream>

#include "../lexer.hpp"
#include "../utils/coroutines.hpp"
#include "./token.hpp"

namespace FNX {
namespace C {

/**
 * @brief The C lexer.
 *
 * It only yields C tokens.
 */
class Lexer : protected FNX::Lexer {
public:
  enum class Mode {
    /// An inline constant expression lexing mode.
    InlineConstExpr,
  };

  Lexer(std::istream);
  Utils::generator<C::AnyToken> lex(Mode mode);

protected:
  Utils::generator<C::AnyToken> _lex_routine();

  Utils::generator<C::Token::Operator> _lex_ascii_op();
  Utils::generator<C::Token::Operator> _lex_ascii_op(char);
  Utils::generator<C::Token::Operator> _lex_ascii_op(char, char);

  bool _is_ascii_op();
};

} // namespace C
} // namespace FNX
