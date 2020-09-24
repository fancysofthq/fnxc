#include "../../../include/fnx/c/lexer.hpp"

namespace FNX {
namespace C {

Utils::generator<C::AnyToken> Lexer::lex(Mode mode) {
  switch (mode) {
  case Mode::InlineConstExpr: {
    /*
      In inline constant expression lexing mode, the expression
      begins with C code and is terminated with `}`, which is not
      yielded. It may not contain source code other than C.

      DESIGN: Note that it may contain a compound literals or a
      string or character constant containing the `}` character.
      Therefore, the terminating bracket shall be properly paired and
      not persist within neither a string nor character literal.
     */
  }
  }
}

} // namespace C
} // namespace FNX
