#pragma once

#include <optional>
#include <variant>

#include "../c/lexer.hpp"
#include "../c/token.hpp"
#include "../onyx/token.hpp"
#include "../utils/coroutines.hpp"

namespace FNX {
namespace CNX {

/**
 * @brief The CNX lexer, which may yield both C and Onyx tokens.
 *
 * It is invoked after the `export` keyword. It may lex a single C
 * entity or a block of CNX code.
 */
class Lexer : protected C::Lexer {
public:
  /// @brief A signal to parent `Onyx::Lexer`.
  enum class Signal {
    At,        // '@', stop at '@'
    DelayedAt, // '\@', stop at '@'
    FinalAt,   // '\!@', stop at '@'

    Macro,         // '{%', stop at ''
    EmittingMacro, // '{{', stop at ''
    DelayedMacro,  // '\{', stop at '{'
    FinalMacro,    // '\!{', stop at '{'
  };

  Utils::generator<std::variant<C::Token::Any, Signal>> lex();

protected:
  bool _is_onyx_id();
  Utils::generator<C::Token::Comment> _lex_comment();

private:
  std::optional<bool> _is_exporting_block;
  int _block_pairs;
  Utils::generator<C::Token::Punctuation> _lex_space();
};

} // namespace CNX
} // namespace FNX
