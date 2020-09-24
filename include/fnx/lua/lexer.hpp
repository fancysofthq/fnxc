#include "../../header/utils/coroutines.hpp"
#include "../lexer.hpp"
#include "./token.hpp"

namespace FNX {
namespace Lua {

/// The Lua macro lexer.
class Lexer : FNX::Lexer {
public:
  enum class Terminator {
    NonEmitting,
    Emitting,
  };

  Utils::generator<AnyToken> lex(Terminator);
};

} // namespace Lua
} // namespace FNX
