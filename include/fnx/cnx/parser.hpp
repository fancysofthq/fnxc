#pragma once

#include "./block.hpp"
#include "./lexer.hpp"
#include <memory>

namespace FNX {
namespace CNX {

/// The CNX parser.
class Parser {
public:
  Parser(std::shared_ptr<Block>);
  void parse();

private:
  std::shared_ptr<Block> block;
};

} // namespace CNX
} // namespace FNX
