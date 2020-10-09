#pragma once

#include <istream>
#include <memory>

#include "./lexer.hpp"

namespace FNX {
namespace Onyx {

/// Parses a stream of tokens into concrete source tree (CST),
/// closely resembling the source code.
///
/// It does not evaluate neither Onyx macros nor C constant
/// expressions.
class Parser {
public:
  Parser(std::basic_istream<char8_t> *input);
  CST parse();

private:
  /// The implementation-defined set of available character sets.
  enum class Charset {
    /// E.g.\ @c 'f'ucs . The implicit default one.
    UCS,

    /// E.g.\ @c 'f'ascii .
    ASCII,
  };

  /// The implementation-defined set of available string encodings.
  ///
  /// @note An encoding is considered a literal prefix, thus shall
  /// not be preceded by an underscore.
  enum class Encoding {
    /// E.g.\ @c "foo"utf8 . The implicit default one.
    UTF8,

    /// E.g.\ @c "foo"utf16le .
    UTF16LE,

    /// E.g.\ @c "foo"utf16be .
    UTF16BE,

    /// E.g.\ @c "foo"utf32le .
    UTF32LE,

    /// E.g.\ @c "foo"utf32le .
    UTF32BE,

    /// E.g.\ @c "foo"ucs2 .
    UCS2,

    /// E.g.\ @c "foo"ucs4 .
    UCS4,
  };

  /// A calculated fixed-point numeric literal restriction.
  struct RestrictionFixed {
    /// Can only be binary or decimal.
    Radix radix;

    std::optional<bool> signedness;

    /// Fixed-point total size, e.g.\ 32 in `i32` or 8 in `Qe7`.
    std::optional<uint16_t> size;

    /// Fixed-point fraction size, e.g.\ 0 in `i32` or 7 in `Qe7`.
    std::optional<uint16_t> fraction_size;
  };

  /// A calculated floating-point numeric literal restriction.
  struct RestrictionFloating {
    /// Can only be binary or decimal.
    Radix radix;

    /// Floating-point precision,
    /// e.g.\ 24 in `f24e8` or 7 in `d7e8`.
    std::optional<uint16_t> precision;

    /// Floating-point exponent size,
    /// e.g.\ 8 in `f24e8` or 8 in `d7e8`.
    std::optional<uint16_t> exponent_size;
  };

  struct Error : std::logic_error {
    Error(std::string);
  };

  struct UnexpectedEOF : Error {
    UnexpectedEOF();
  };

  std::shared_ptr<File> _unit;
};

} // namespace Onyx
} // namespace FNX
