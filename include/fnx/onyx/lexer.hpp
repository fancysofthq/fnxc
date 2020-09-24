#pragma once

#include <optional>
#include <set>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <variant>

#include "../c/token.hpp"
#include "../lexer.hpp"
#include "../utils/coroutines.hpp"
#include "../utils/flatten_variant.hpp"
#include "./macro.hpp"
#include "./token.hpp"

namespace FNX {
namespace Onyx {

/// Lexes a translation unit, yielding tokens.
///
/// A Lexer tries to be context-free as much as possible. However, in
/// the following cases context awareness is required:
///
///   * Tokenizing Onyx literals.
///   * Tokenizing Onyx macros.
///   * Tokenizing C constant expressions.
///   * Tokenizing `export` statements.
///
/// Technically, `lex`-family functions could've implemented as
/// static generators, and context to be stored within their
/// coroutine bodies. But it would require more cumbersome calls,
/// e.g. `_advance(unit, cursor, offset)`. Therefore, there shall be
/// separate a Lexer instance for every translation unit.
///
/// An Onyx lexer yields Lua and C tokens along with Onyx tokens.
///
/// TODO: Always allow EOF, it's lexer who'd panic.
class Lexer : FNX::Lexer {
public:
  // I'd honestly like to put this struct into a separate
  // `./lexer/error.hpp` file, but I fucking can't, because `_err`
  // makes use of `Error::Kind`, and  C++ prohibits declaration of
  // nested members of an incomplete type.
  struct Error : std::logic_error {
    enum Kind {
      // General errors
      //

      /// The lexer did not expect THAT.
      Unexpected,

      /// Something's wrong with the unit.
      UnitError,

      /// Invalid escape sequence met directly in the input.
      UnknownEscapeSequence,

      // Numeric literal errors
      //

      NumericLiteralInvalidPrefixPlacement,
      NumericLiteralPrefixFamilyIncompatible,

      /// Can not use a prefix in a non-decimal numeric literal.
      NumericLiteralPrefixNonDecimal,

      /// E.g. `0v1` is unknown.
      NumericLiteralUnknownBase,

      /// A decimal numeric literal can not begin from zero.
      NumericLiteralDecimalBeginsFromZero,

      /// E.g. `0x.1` is invalid.
      NumericLiteralDotTooEarly,
      NumericLiteralExponentTooEarly,

      NumericLiteralEmptyWholePart,

      /// E.g. `0x1.p` is invalid.
      NumericLiteralEmptyFractionPart,

      NumericLiteralEmptyExponent,

      CodepointUnknownEscapeSequence, /// E.g. `'\\h'`: what's that?
      CodepointMalformed,             /// A UTF-8 error

      /// E.g.\ a character out of base ( @c '\\x{Z}' ).
      CodepointUnexpected,

      ///< E.g.\ @c '\\x{}' .
      CodepointEmpty,

      UnderscorePreceding,
      UnderscoreSucceding,

      CharEEol,    /// EEOLs aren't allowed in chars
      CharEmpty,   /// Empty chars aren't allowed
      CharTooMuch, /// A character literal must not contain more than
                   /// one codepoint.

      HeredocEmpty,
      HeredocNonTerminated,

      SymbolAndLabel, ///< E.g.\ `:foo:` is errornous.
      CLabel,         ///< Can not have a C label, e.g.\ `$foo:`.
    };

    Kind kind;

    Error(Kind kind = Kind::Unexpected, std::string msg = NULL);

    /// Create a error denoting a set of expected characters.
    Error(std::set<char>);
  };

  Utils::generator<Onyx::AnyToken> lex();

private:
  char32_t _get_closing_bracket_pair(char32_t);

#pragma region Lexing subroutines

  /// Lex a comment, starting from @c # .
  [[nodiscard]] Token::Comment _lex_comment();

  /// Lex a EOL characters sequence.
  [[nodiscard]] Token::Punctuation _lex_eol();

  /// Lex a space characters sequence.
  [[nodiscard]] Token::Punctuation _lex_space();

  /// Lex a single codepoint or an escaped newline.
  /// @note Returned tokens call reloc().
  [[nodiscard]] std::variant<Token::Codepoint, Token::Punctuation>
  _lex_codepoint();

  /// Lex multiple codepoints, terminated with an unescaped
  /// terminator. Can also yield an escaped EOL.
  [[nodiscard]] Utils::generator<
      std::variant<Token::Codepoint, Token::Punctuation>>
  _lex_codepoints(char32_t);

  /// Lex multiple codepoints, terminated with an exact sequence of
  /// UTF-8 codepoints. Can also yield an escaped EOL.
  [[nodiscard]] Utils::generator<
      std::variant<Token::Codepoint, Token::Punctuation>>
      _lex_codepoints(std::u32string);

  /// Tokenize a non-explicit identifer
  /// or keyword, starting at alpha.
  [[nodiscard]] std::variant<Token::ID, Token::Keyword>
  _lex_id_or_keyword();

  /// Lex an identifer, starting at @c ` or alpha.
  [[nodiscard]] Token::ID _lex_id_no_keyword();

  /// Lex an operator.
  [[nodiscard]] Token::Operator _lex_op();

  /// Lex an either opening or closing bracket.
  [[nodiscard]] Token::Punctuation _lex_bracket();

  /// Lex a character literal, e.g.\ @c 'f'ucs , starting from @c ' .
  [[nodiscard]] Utils::generator<
      std::
          variant<Token::Punctuation, Token::Codepoint, Token::Data>>
  _lex_char_literal();

  /// Lex a string literal, e.g.\ @c "foo"utf8 , starting from @c " .
  [[nodiscard]] Utils::generator<
      std::
          variant<Token::Punctuation, Token::Codepoint, Token::Data>>
  _lex_string_literal();

  /// Lex a numeric literal prefix, either SI or IEC, starting from
  /// its letter. May lex a non-prefix, returning nullopt.
  [[nodiscard]] std::optional<std::variant<
      Token::NumericLiteral::PrefixSI,
      Token::NumericLiteral::PrefixIEC>>
  _lex_prefix();

  /// Lex an exponent, starting at @b after @c e or @c p.
  ///
  /// An exponent may be preceded by a sign, consist of decimal
  /// digits only, and be a single zero.
  [[nodiscard]] int _lex_exponent();

  /// Lex elements of a numeric literal value.
  [[nodiscard]] std::vector<Token::NumericLiteral::Element>
  _lex_numeric_elements(Radix radix);

  /// Lex a numeric literal, starting at a @b decimal number.
  /// Can also yield a dot as a punctuation token.
  [[nodiscard]] Utils::generator<std::variant<
      Token::NumericLiteral,
      Token::Punctuation,
      Token::Data>>
  _lex_numeric_literal();

  /// Lex a heredoc, starting from @c ~ .
  [[nodiscard]] Utils::generator<
      std::variant<Token::Punctuation, Token::Codepoint>>
  _lex_heredoc();

  /// Lex some context-specific data.
  [[nodiscard]] std::optional<Token::Data> _lex_data();

  /// Lex a char tensor literal, e.g.\ `%c|[ab] [cd]|r`,
  /// starting from the opening @c | bracket.
  [[nodiscard]] Utils::generator<
      std::
          variant<Token::Punctuation, Token::Codepoint, Token::Data>>
  _lex_char_tensor_literal();

  /// Lex a numeric tensor literal, e.g.\ `%|[1 2][3 4]|c`,
  /// starting from the opening @c | bracket.
  [[nodiscard]] Utils::generator<
      std::
          variant<Token::Punctuation, Token::Codepoint, Token::Data>>
  _lex_numeric_tensor_literal();

  /// Lex a quoted string, starting from @c ~ or @c q .
  /// @param is_heredoc True if begins with @c ~ .
  [[nodiscard]] Utils::generator<std::variant<
      Token::MagicLiteral,
      Token::Data,
      Token::Punctuation,
      Token::Codepoint>>
  _lex_quoted_string(bool is_heredoc);

  /// Lex a character container, starting from @c c .
  [[nodiscard]] Utils::generator<std::variant<
      Token::MagicLiteral,
      Token::Data,
      Token::Punctuation,
      Token::Codepoint>>
  _lex_char_container();

  /// Lex a numeric container, starting
  /// from whatever is following @c % .
  [[nodiscard]] Utils::generator<std::variant<
      Token::MagicLiteral,
      Token::Data,
      Token::Punctuation,
      Token::NumericLiteral>>
  _lex_numeric_container();

#pragma endregion

#pragma region Shortcut subroutines

  /// Decorate a punctuation token, calling reloc().
  Token::Punctuation _punct(Token::Punctuation::Kind);

  /// Decorate a data token, calling reloc().
  Token::Data _data();

  /// Decorate an identifier token, calling reloc().
  Token::ID _id(bool is_explicit);

  /// Decorate an operator token, calling reloc().
  Token::Operator _op();

#pragma endregion

#pragma region Match subroutines

  /// Match an Onyx identifier start, roughly
  /// @c /[a-zA-Zα-ωΑ-Ω_]/ .
  bool _is_id_start();

  /// Match an Onyx identifier continuation, roughly
  /// @c /[a-zA-Zα-ωΑ-Ω_0-9]/ .
  bool _is_id_cont();

  /// Match a C identifier start, roughly @c /[a-zA-Z_]/ .
  bool _is_c_id_start();

  /// Match a C identifier continuation, roughly @c /[a-zA-Z_0-9]/ .
  bool _is_c_id_cont();

  /// Match an operator.
  ///
  /// UCS code point | Glyph
  /// --- | ---
  /// U+0021 | @c !
  /// U+0025 | @c %
  /// U+0026 | @c &
  /// U+002A | @c *
  /// U+002B | @c +
  /// U+002D | @c -
  /// U+002F | @c /
  /// U+003C | @c <
  /// U+003D | @c =
  /// U+003E | @c >
  /// U+005E | @c ^
  /// U+007C | @c |
  /// U+007E | @c ~
  /// U+2200–U+22FF | *Unicode Mathematical Operators*
  ///
  /// @note U+2211 ( @c ∑ ) is normalized to U+03A3 ( @c Σ ). That
  /// said, a capital sigma acts both as an identifier and an
  /// operator.
  bool _is_op();

  /// Match data start.
  bool _is_data_start();

  /// Match data continuation.
  bool _is_data_cont();

  /// Match the start of a numerical prefix.
  /// @see Token::NumericLiteral::Prefix.
  bool _is_numerical_prefix();

  /// Match a heredoc first char, similar to _is_c().
  bool _is_heredoc_start();

  /// Match a heredoc continuation char, similar to _is_c().
  bool _is_heredoc_cont();

#pragma endregion

}; // namespace Onyx
}; // namespace Onyx
} // namespace FNX
