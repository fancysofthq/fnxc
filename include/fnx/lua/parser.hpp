#pragma once

#include <memory>
#include <optional>

#include "./block.hpp"
#include "./lexer.hpp"

namespace FNX {
namespace Lua {

class Parser {
  const std::shared_ptr<Block> _block;

public:
  enum class Terminator {
    NonEmitting, ///< I.e.\ @c %} .
    Emitting,    ///< I.e.\ @c }} .
  };

  Parser(std::shared_ptr<Block>);

  void parse(std::optional<Lexer::Terminator>);
};

} // namespace Lua
} // namespace FNX
