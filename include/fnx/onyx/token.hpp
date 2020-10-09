#pragma once

#include <optional>
#include <stdint.h>
#include <unordered_map>
#include <variant>
#include <vector>

#include "../radix.hpp"
#include "../token.hpp"

namespace FNX {
namespace Onyx {

/// An Onyx source token, excluding macro and i14y tokens.
///
/// General tokenization rules:
///
///   1. Given only the tokens, it shall be possible to reproduce the
///   source code *exactly*. A `location` field of a token in
///   conjuction with the translation unit source stream allow to
///   build an immutable `std::string_view` for the token.
///
///   2. A token stream shall be sequential. In other words, a token
///   itself shall be atomic. For example, there is the magic
///   container token which denotes the beginning of such a
///   container, instead of fully wrapping its contents.
///
///   3. A token may convey semantic value rather than strictly
///   syntactic. For example, in `%(min max)u8`, `min` is deemed to
///   be a symbol token rather then a simple identifier, because it
///   resides within a magic numeric container literal.
///
struct Token : FNX::Token {
  struct Comment;
  struct Punctuation;
  struct Keyword;
  struct ID;
  struct Data;
  struct Operator;
  struct Codepoint;
  struct NumericLiteral;
  struct MagicLiteral;

protected:
  struct Name;
  Token(Location);
};

/// A commentary token.
///
/// It begins with `#` and is terminated by @link Punctuation::Eol
/// @endlink or @link Punctuation::Eof @endlink. A comment may not
/// contain any escape sequences, including @link
/// Punctuation::EscapedEol @endlink.
///
/// @note Adjacent commentary tokens are not merged by a lexer.
struct Token::Comment : Token {
  Comment(Location);
};

/// A token which may be used as an unary or binary operator.
///
/// TODO: It consists of Unicode characters of category `Sm` (Math
/// Symbol), and also `&` (U+0026), `-` (U+002D), `*` (U+002A) and
/// `^` (U+005E) characters.
struct Token::Operator : Token {
  Operator(Location);
};

/// An arbitrary data token to be processed elsewhere.
/// An example would be `ucs` in `'a'ucs`.
struct Token::Data : Token {
  Data(Location);
};

/// A punctuation token.
struct Token::Punctuation : Token {
  enum Kind {
    Eof,

    /// A End-Of-Line (EOL) token.
    ///
    /// @note The source code of this token is platform-dependent.
    Eol,

    /// An Escaped End-Of-Line (EOL) token.
    ///
    /// @code{.nx}
    ///   # `\` is yielded in the middle of
    ///   # the codepoint tokens stream.
    ///   @assert("foo\
    ///     bar" == "foo  bar")
    ///
    ///   # Ditto, two literals are
    ///   # to be concatenated.
    ///   @assert("foo"\
    ///     "bar" == "foobar")
    /// @endcode
    EEol,

    /// A sequence of white space characters, including any of:
    ///
    /// Unicode | Name
    /// ---     | ---
    /// U+0009  | Horizontal Tab
    /// U+000B  | Vertical Tab
    /// U+0020  | Space
    Space,

    /// A @c # token begins a comment.
    Hashtag,

    /// A @c <dot> token used for field and method accesses.
    Dot,

    /// A @c , token used to separate elements.
    Comma,

    /// A @c ; token used to terminate lines.
    Semicolon,

    /// A @c : token used in path lookups (e.g.\ `T:foo`) and
    /// as a type restriction operator (e.g.\ `x : T` ).
    Colon,

    /// A @c :: token used in path lookups, e.g.\ `A::B`.
    DoubleColon,

    /// A ternary operator token, the question mark part ( @c ? ).
    Ternary,

    /// A @c @ token denotes the beginning of either an annotations
    /// application ( @c @[ ) or an intrinsic ( @c @foo ).
    At,

    /// A @c $ followed by an identifier denotes a C identifier.
    /// A @c ${ sequence denotes a C constant expression.
    C,

    /// A @c ' token denotes either the beginning
    /// or the end of a char literal
    SingleQuote,

    /// A @c " token denotes either the beginning
    /// or the end of a string literal
    DoubleQuote,

    /// A @c ~ char in a magic literal means a heredoc.
    HeredocSymbol,

    /// A heredoc terminator token, e.g.\ @c EOF .
    Heredoc,

    /// An opening parenthesis token, @c ( .
    ParenOpen,

    /// A closing parenthesis token, @c ) .
    ParenClose,

    /// An opening curly bracket token, @c { .
    CurlyOpen,

    /// A closing curly bracket token, @c } .
    CurlyClose,

    /// An opening square bracket token, @c [ .
    SquareOpen,

    /// A closing square bracket token, @c ] .
    SquareClose,

    /// An opening angle bracket token, @c < .
    AngleOpen,

    /// A closing angle bracket token, @c > .
    AngleClose,

    /// A pipe ( @c | ) acting as a bracket.
    PipeBracket,

    /// An immediate non-emitting macro opening, @c {% .
    MacroOpen,

    /// A non-emitting macro closing, @c %} .
    MacroClose,

    /// An immediate emitting macro opening, @c {{ .
    EmittingMacroOpen,

    /// An emitting macro closing, @c }} .
    EmittingMacroClose,

    /// A delayed non-emitting macro opening, @c \{% .
    DelayedMacroOpen,

    /// A delayed emitting macro opening, @c \{{ .
    DelayedEmittingMacroOpen,

    /// A final non-emitting macro opening, @c \!{% .
    FinalMacroOpen,

    /// A final emitting macro opening, @c \!{{ .
    FinalEmittingMacroOpen,

    /// A chaining operator token, @c | .
    /// It shall be *immediately* followed by one of @link
    /// Punctuation::DoubleColon @endlink (`::`) or @link
    /// Punctuation::Colon @endlink (`:`) or @link
    /// Punctuation::Dot @endlink (`.`) tokens.
    ///
    /// A pipe performs the operation on the operand and returns the
    /// operand itself. For example, `x|.foo()` ⇔ `(x.foo(); x)`.
    Pipe,

    /// A returning block pipe token, @c <|> .
    /// Calls the right operand code block with the left operand as
    /// the first anonymous argument, and returns the left operand.
    /// For example, `x <|> &.foo()` is equivalent to `x.foo(); x`.
    ReturningBlockPipe,

    /// A block pipe token, @c |> .
    /// Calls the right operand code block with the left operand as
    /// the first anonymous argument, and returns the result.
    /// For example, `x |> @.foo()` is equivalent to `x.foo()`.
    BlockPipe,

    /// A curly arrow token, @c ~> used in lambdas, e.g.\ `foo ~>
    /// @.bar`.
    CurlyArrow,

    /// A thin arrow token, @c -> used for fast pointer dereferencing
    /// (e.g.\ `ptr->.foo`) and in blocks (e.g.\ `foo -> @.bar`).
    ThinArrow,

    /// A thick arrow token, @c => a.k.a.\ the _rocket operator_.
    /// Used to create pairs of values, e.g. `A => B` ⇔ `(A, B)`.
    ThickArrow,
  };

  const Kind kind;

  Punctuation(Location, Kind);

public:
  /// Return a single-byte codepoint associated with the kind.
  /// @throw If it can not be expressed in a single byte.
  static char8_t to_char(Kind);

  /// Used to debug special symbols, e.g.\ `"<newline>"`.
  std::string debug();
};

/// A keyword token.
struct Token::Keyword : Token {
  enum Kind {
    Let,
    Final,

    Const,
    Mut,

    Do,
    End,

    Decl,
    Impl,
    Def,
    Reimpl,
    Undecl,

    Reopen,
    Alias,

    As,
    To,

    Proc,
    Gen,
    Macro,
    Hook,
    Export,

    Return,
    Break,
    Continue,

    Try,
    With,
    Catch,
    Ensure,
    Throw,

    If,
    Else,
    Unless,
    While,
    Until,

    Namespace,
    Trait,
    Struct,
    Class,
    Enum,
    Flag,
    Annotation,

    Public,
    Protected,
    Private,

    Instance,
    Static,
    Native,

    Compl,
    Incompl,
    Abstract,

    ThreadsafeModifier,
    FragileModifier,
    UnsafeModifier,

    ThreadsafeStatement,
    FragileStatement,
    UnsafeStatement,

    Nothrow,
    Throws,

    Asm,
    Template,
    In,
    Out,

    True,
    False,

    And,
    Or,
    Xor,
    Not,

    Ghost,
  };

  const Kind kind;

  Keyword(Location, Kind);

public:
  static std::optional<Kind> from_string(std::u32string cmp);
  std::string to_string();

private:
  static const std::unordered_map<Kind, std::string> _map;
  static const std::unordered_map<std::string, Kind> _invmap;
};

struct Token::ID : Token {
  /// Whether is it wrapped in backticks (`` ` ``, U+0060).
  ///
  /// If so, any characters are allowed until the closing
  /// backtick is met.
  ///
  /// @note The backticks are included into the token itself.
  bool is_explicit;

  ID(Location, bool is_explicit);
};

/// A codepoint token used in string and char literals.
/// @todo Describe the algorithm.
struct Token::Codepoint : Token {
  // IDEA: Allow underscores within a non-exact codepoint?
  // IDEA: Uppercase for little-endian, e.g. `\xABCD == \XDCBA`.
  //

  enum Format {
    /// E.g.\ @c a .
    Exact,

    /// E.g.\ @c \\n (see @link EscapeSequence @endlink).
    Escaped,

    /// E.g.\ @c \\o141 or @c \\o{141} .
    Octal,

    /// E.g.\ @c \\97 .
    /// @note @c \\{97} results in @c { followed by @c 9 and @c 7 .
    DecimalImplicit,

    /// E.g.\ @c \\d97 or @c \\d{97} .
    DecimalExplicit,

    /// E.g.\ @c \\x61 or @c \\x{61} .
    Hexadecimal
  };

  /// Foo bar.
  const Format format;

  /// The codepoint value in decimal base.
  ///
  /// @note It will be a UCS value for the @link Exact @endlink and
  /// @link Escaped @endlink formats.
  const char32_t value;

  /// Is the numerical codepoint value wrapped, e.g.\ @c \\x{61} ?
  ///
  /// Only set if @link format @endlink is neither @link Exact
  /// @endlink, nor @link Escaped @endlink.
  const std::optional<bool> is_wrapped;

  Codepoint(Location, Format, char32_t val, bool is_wrapped);
};

/// A numeric literal token.
struct Token::NumericLiteral : Token {
  /// An underscore contained in a numeric literal.
  struct Underscore {};

  // IDEA: `2.5M == 2_500_000`?
  //

  /// International System of Units (a.k.a. SI)
  enum class PrefixSI {
    Kilo,  //!< @c k, 10³
    Mega,  //!< @c M, 10⁶
    Giga,  //!< @c G, 10⁹
    Tera,  //!< @c T, 10¹²
    Peta,  //!< @c P, 10¹⁵
    Exa,   //!< @c E, 10¹⁸
    Zetta, //!< @c Z, 10²¹
    Yotta, //!< @c Y, 10²⁴
  };

  // ISO/IEC 80000 (a.k.a. IEC)
  //
  // NOTE: Prefixes have higher precedence, therefore `1Mi32` is
  // invalid. Valid options would be `1M_i32` or `1Mii32`.
  //
  enum class PrefixIEC {
    Kibi, //!< @c Ki, 1024
    Mebi, //!< @c Mi, 1024²
    Gibi, //!< @c Gi, 1024³
    Tebi, //!< @c Ti, 1024⁴
    Pebi, //!< @c Pi, 1024⁵
    Exbi, //!< @c Ei, 1024⁶
    Zebi, //!< @c Zi, 1024⁷
    Yobi, //!< @c Yi, 1024⁸ ( ͡° ͜ʖ ͡°)
  };

  /// Either a digit value, prefix or an underscore.
  ///
  /// A digit value is in given base, e.g. `11` for `b` in
  /// hexadecimal.
  ///
  /// @note An underscore shall not precede a numeric prefix.
  using Element =
      std::variant<uint8_t, PrefixSI, PrefixIEC, Underscore>;

  /// The literal value radix.
  const Radix radix;

  /// Parsed whole value part, e.g.\ `42` in `42.17e2`.
  /// Nullopt value means zero.
  /// @note Can neither begin nor end with an underscore.
  const std::optional<std::vector<Element>> whole;

  /// Parsed fractional value part, e.g.\ `17` in `42.17e2`.
  /// @note Can neither begin nor end with an underscore.
  const std::optional<std::vector<Element>> fractional;

  /// Parsed exponent, e.g.\ `2` in `42.17e2`, or `2` in `0xAp2`.
  /// @note Can not be preceded by an underscore.
  const std::optional<int> exponent;

  NumericLiteral(
      Location,
      Radix = Radix::Decimal,
      std::optional<std::vector<Element>> whole = std::nullopt,
      std::optional<std::vector<Element>> fractional = std::nullopt,
      std::optional<int> exponent = std::nullopt);

public:
  /// Return `e` for `Radix::Decimal`, `p` otherwise.
  static char exponent_char(Radix);
};

struct Token::MagicLiteral : Token {
  enum Kind {
    /// E.g.\ @c %[] , @c %u8() .
    NumericContainer,

    /// E.g.\ @c %c[] , @c %c~EOF .
    CharContainer,

    /// E.g.\ @c %q<> , @c %q~EOF .
    QuotedString,
  };

  const Kind kind;

  /// Only applicable to `CharContainer` and `QuotedString`.
  const std::optional<bool> is_explicit;

  MagicLiteral(Location, Kind, bool is_explicit);
};

// CPP: Unfortunately, we can not have `Token::Any` here.
using AnyToken = std::variant<
    Token::Comment,
    Token::Punctuation,
    Token::Operator,
    Token::Keyword,
    Token::ID,
    Token::Data,
    Token::Codepoint,
    Token::NumericLiteral,
    Token::MagicLiteral>;

} // namespace Onyx
} // namespace FNX
