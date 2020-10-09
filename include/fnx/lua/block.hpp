#pragma once

#include <istream>
#include <memory>
#include <variant>
#include <vector>

#include "../unit.hpp"
#include "./cst.hpp"
#include "./token.hpp"

namespace FNX {
namespace Onyx {

class File;
class Block;

} // namespace Onyx

namespace Lua {

/// A Lua block of code.
class Block : public Unit {
  /// CST root node for the block.
  const std::shared_ptr<CST> _cst_root;

  /// Lexed tokens of the block.
  std::vector<std::shared_ptr<Token::Any>> _tokens;

  /// The Lua source code stream, written during lexing an Onyx
  /// source, and read during lexing the block source.
  std::basic_iostream<char8_t> _source_stream;

  /// The emitted Onyx code stream.
  std::basic_iostream<char8_t> _emit_stream;

public:
  Block(std::variant<
        std::shared_ptr<Onyx::File>,
        std::shared_ptr<Onyx::Block>> parent);

  const std::shared_ptr<CST> get_cst_root();
  const std::basic_ostream<char8_t> *get_emit_stream();
};

} // namespace Lua
} // namespace FNX
