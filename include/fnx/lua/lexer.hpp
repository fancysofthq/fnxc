#include "../lexer.hpp"
#include "../utils/coro.hpp"
#include "./token.hpp"

namespace FNX {
namespace Lua {

/// The Lua macro lexer.
class Lexer : public FNX::Lexer {
public:
  enum class Terminator {
    NonEmitting, ///< I.e.\ @c %} .
    Emitting,    ///< I.e.\ @c }} .
  };

  Utils::Coro::generator<Token::Any> lex(Terminator);
};

} // namespace Lua
} // namespace FNX
