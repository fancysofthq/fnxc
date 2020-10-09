#pragma once

#include <variant>

#include "../token.hpp"

namespace FNX {
namespace C {

/// @brief A C token.
struct Token : FNX::Token {
  struct Punctuation {};

  /// An ASCII-only operator token.
  /// @note `sizeof` and `_Alignof` are considered keywords.
  struct Operator {};

  struct Keyword {};
  struct Identifier {};
  struct Literal {};
  struct Comment {};

  using Any = std::variant<
      Punctuation,
      Operator,
      Keyword,
      Identifier,
      Literal,
      Comment>;
};

} // namespace C
} // namespace FNX
