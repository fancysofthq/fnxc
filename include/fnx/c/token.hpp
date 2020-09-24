#pragma once

#include <variant>

#include "../token.hpp"

namespace FNX {
namespace C {

/// @brief A C token.
struct Token : FNX::Token {
  struct Punctuation {};

  /**
   * @brief An ASCII-only operator token.
   * @note `sizeof` and `_Alignof` are considered keyword tokens.
   */
  struct Operator {};

  struct Keyword {};
  struct Identifier {};
  struct Literal {};
  struct Comment {};
};

using AnyToken = std::variant<
    Token::Punctuation,
    Token::Operator,
    Token::Keyword,
    Token::Identifier,
    Token::Literal,
    Token::Comment>;

} // namespace C
} // namespace FNX
