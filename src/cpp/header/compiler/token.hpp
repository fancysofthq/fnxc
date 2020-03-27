#pragma once

#include "./location.hpp"
#include <optional>
#include <set>
#include <unordered_map>
#include <variant>
#include <vector>

namespace Onyx {
namespace Compiler {
namespace Token {
struct Base {
  const Location location;

  Base(Location loc) : location(loc) {}
  virtual ~Base() {}

  // Return the token as it was in the source file.
  virtual string source();

  // Return a distinct identifer, e.g. `Control`.
  virtual string type_id();
};

struct Control : Base {
  enum Kind {
    Eof,     //
    Newline, // \n
    Space,   //
    Comment, // #

    Dot,         // .
    Comma,       // ,
    Semicolon,   // ;
    Colon,       // :
    DoubleColon, // ::
    DotColon,    // :.
    Splat,       // ..
    // Skwat,       // ** (foo **bar <- maybe it's a power?)
    Assignment, // =
    Ternary,    // ?
    Elvis,      // ?:

    SingleQuote,  // '
    DoubleQuotes, // "
    Tick,         // `

    OpenParen,   // (
    CloseParen,  // )
    OpenCurly,   // {
    CloseCurly,  // }
    OpenSquare,  // [
    CloseSquare, // ]
    BlockPipe,   // |

    Annotation,       // @[ (requires closing bracket)
    DelayedMacro,     // \{%
    DelayedEmitMacro, // \{{
    MacroClose,       // %}
    EmitMacroClose,   // }}

    CurlyArrow, // ~>
    ThinArrow,  // ->
    ThickArrow, // =>
    PipeArrow,  // |>
  };

  const Kind kind;

  Control(Location loc, Kind kind) : Base(loc), kind(kind) {}

  // Used for outputting special symbols, e.g. `"<newline>"`.
  char *debug_source();
};

struct Keyword : Base {
  enum Kind {
    Var,
    Const,

    Do,
    End,

    Def,
    Macro,

    Return,
    Convey,
    Yield,
    Break,
    Continue,

    Begin,
    Catch,
    Rescue,
    Raise,

    If,
    Unless,
    While,
    Until,

    Namespace,
    Module,
    Primitive,
    Struct,
    Class,
    Enum,
    Flag,
    Annotation,

    Protected,
    Private,

    Static,
    Threadlocal,

    Threadsafe,
    Volatile,
    Unsafe,
    Unordered,
  };

  const Kind kind;

  static optional<Kind> from_string(string cmp);
  Keyword(Location loc, Kind kind);
  string to_string();

private:
  static const unordered_map<Kind, string> _map;
  static const unordered_map<string, Kind> _invmap;
};

// A value token of certain kind.
struct Value : Base {
  enum Kind {
    ID,               // foo (can be variable or keyword)
    CID,              // `Foo or `unsigned int`
    Type,             // Foo
    Op,               // && (ASCII only)
    Kwarg,            // foo:, Foo:
    Intrinsic,        // @foo
    CommentIntrinsic, // :foo:
    Symbol,           // :foo
    StringSymbol,     // :"foo" (ASCII only)
    Text, // An arbitrary text used in comments and macros
  };

  const Kind kind;
  const string value;

  static bool is_comment_intrinsic(string);

  Value(Location loc, Kind kind, string value) :
      Base(loc), kind(kind), value(value) {}

  string debug_kind();
  static char *pretty_kind(Kind kind);

private:
  static const set<string> _comment_intrinsics;
};

struct Codepoint : Base {
  const string _source;

  // Chars support up to 4 Unicode bytes.
  const uint32_t value;

  enum Kind {
    Exact,       // a
    Binary,      // \b1100001
    Octal,       // \o141
    Decimal,     // \97, \d97
    Hexadecimal, // \x61
  };

  const Kind kind;

  Codepoint(Location loc, Kind kind, uint32_t val, string source) :
      Base(loc), value(val), _source(source), kind(kind) {}
};

struct Linebreak : Base {
  Linebreak(Location loc) : Base(loc) {}
};

// A single codepoint char literal limited to ASCII.
struct CharLiteral : Base {
  const Codepoint codepoint;
  CharLiteral(Location, Codepoint);
  string debug_source();
};

// A string literal enabling up to four byte
// codepoints and also linebreaks.
struct StringLiteral : Base {
  const vector<variant<Linebreak, Codepoint>> codepoints;

  StringLiteral(
      Location loc,
      vector<variant<Linebreak, Codepoint>> codepoints) :
      Base(loc), codepoints(codepoints) {}

  string debug_source();
};

struct NumericLiteral : Base {
  enum Radix {
    Deci, // 42
    Bina, // 0b101010
    Octa, // 0o052
    Hexa, // 0x2A
  };

  const Radix radix;

  // `10.2_345e6` -> `"10"`
  const vector<char> whole;

  // `10.2_345e6` -> `"2_345"`
  const optional<vector<char>> fraction;

  // `10.2_345e6` -> `6`
  const optional<int> exponent;

  enum Type {
    TypeUndef,
    Int,   // i
    UInt,  // u
    Float, // f
  };

  const Type type;

  // `42_i16` -> `16`; `0` if not set.
  // Explicit `0` is error.
  const uint32_t bitsize;

  NumericLiteral(
      Location loc,
      Radix radix,
      vector<char> whole,
      optional<vector<char>> fraction = nullopt,
      optional<int> exponent = nullopt,
      Type type = TypeUndef,
      uint32_t bitsize = 0) :
      Base(loc),
      radix(radix),
      whole(whole),
      fraction(fraction),
      exponent(exponent),
      type(type),
      bitsize(bitsize) {}
};

struct PercentLiteral : Base {
  enum Type {
    String,  // %q("foo") == %("foo") == "\"foo\""
    Words,   // %w(foo bar) == ("foo", "bar")
    Symbols, // %y(foo bar) == (:foo, :bar)
    Chars,   // %c(foo) == ('f', 'o', 'o')
    Numbers, // %u8(1 2 3) == (1u8, 2u8, 3u8), includes non-decimal
  };

  const Type type;

  enum Bracket {
    Paren,  // (
    Square, // [
    Curly,  // {
    Angle,  // <
  };

  const Bracket bracket;

  enum NumericRadix {
    NumericRadixUndef,
    Octa, // o
    Deci, // d
    Hexa, // x
  };

  const NumericRadix numeric_radix;

  enum NumericType {
    NumericTypeUndef,
    Int,   // i
    UInt,  // u
    Float, // f
  };

  const NumericType numeric_type;
  const uint32_t numeric_bitsize;

  // static const unordered_map<Bracket, char> _brackets_map;
  // static const unordered_map<Bracket, char> _reverse_brackets_map;

  // static const unordered_map<NumericRadix, char> _radices_map;
  // static const unordered_map<NumericRadix, char>
  // _reverse_radices_map;

  // static const unordered_map<NumericType, char> _types_map;
  // static const unordered_map<NumericType, char>
  // _reverse_types_map;

  PercentLiteral(
      Location loc,
      Type type,
      Bracket bracket,
      NumericRadix numeric_radix = NumericRadixUndef,
      NumericType numeric_type = NumericTypeUndef,
      uint32_t numeric_bitsize = 0) :
      Base(loc),
      type(type),
      bracket(bracket),
      numeric_radix(numeric_radix),
      numeric_type(numeric_type),
      numeric_bitsize(numeric_bitsize) {}
};
} // namespace Token
} // namespace Compiler
} // namespace Onyx
