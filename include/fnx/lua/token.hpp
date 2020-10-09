#pragma once

#include <variant>

#include "../token.hpp"

namespace FNX {
namespace Lua {

struct Token : FNX::Token {
  struct Comment {};

  struct Identifier {};

  struct Punctuation {
    enum Kind {
      Semicolon, // ;
      Colon,     // :
      Comma,     // ,
      Dot,       // .
      Varg,      // ...

      OpenParen,   // (
      CloseParen,  // )
      OpenCurly,   // {
      CloseCurly,  // }
      OpenSquare,  // [
      CloseSquare, // ]
    };
  };

  struct Operator {
    enum Kind {
      Add, // +
      Sub, // -
      Mul, // *
      Div, // /
      Mod, // %
      Exp, // ^
      Neg, // -

      Eq,  // ==
      Neq, // ~=
      Gt,  // >
      Ll,  // <
      Gte, // >=
      Lte, // <=

      And, // and
      Or,  // or
      Not, // not

      Concat, // ..
      Length, // #
    };
  };

  struct Keyword {
    enum Kind {
      Local,

      Do,
      End,

      Function,
      Return,

      For,
      In,
      Repeat,
      Break,

      Until,
      While,

      If,
      Then,
      Else,
      Elseif,

      False,
      True,
      Nil
    };
  };

  struct Literal {
    struct Number;
    struct String;
  };

  using Any = std::variant<
      Comment,
      Identifier,
      Punctuation,
      Operator,
      Keyword,
      Literal>;
};

} // namespace Lua
} // namespace FNX
