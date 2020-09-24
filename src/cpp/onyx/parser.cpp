#include <memory>

#include "../../../include/fnx/onyx/file.hpp"
#include "../../../include/fnx/onyx/lexer.hpp"

namespace FNX {
namespace Onyx {

class Parser {
  /// The implementation-defined set of available character sets.
  enum class Charset {
    UCS,   // E.g. `'f'ucs`. The implicit default one.
    ASCII, // E.g. `'f'ascii`
  };

  /// The implementation-defined set of available string encodings.
  ///
  /// @note An encoding is considered a literal prefix, thus shall
  /// not be preceded by an underscore.
  enum class Encoding {
    UTF8,    // E.g. `"foo"utf8`. The implicit default one.
    UTF16LE, // E.g. `"foo"utf16le`
    UTF16BE, // E.g. `"foo"utf16be`
    UTF32LE, // E.g. `"foo"utf32le`
    UTF32BE, // E.g. `"foo"utf32le`
    UCS2,    // E.g. `"foo"ucs2`
    UCS4,    // E.g. `"foo"ucs4`
  };

  struct Restriction {
    /// A calculated literal restriction.
    struct Fixed {
      enum Base {
        Binary,
        Decimal,
      };

      /// Can only be binary or decimal.
      Base base;

      std::optional<bool> signedness;

      /// Fixed-point total size, e.g. 32 in `i32` or 8 in `Qe7`.
      std::optional<uint16_t> size;

      /// Fixed-point fraction size, e.g. 0 in `i32` or 7 in `Qe7`.
      std::optional<uint16_t> fraction_size;
    };

    struct Floating {
      enum Base {
        Binary,
        Decimal,
      };

      /// Can only be binary or decimal.
      Base base;

      /// Floating-point precision,
      /// e.g. 24 in `f24e8` or 7 in `d7e8`.
      std::optional<uint16_t> precision;

      /// Floating-point exponent size,
      /// e.g. 8 in `f24e8` or 8 in `d7e8`.
      std::optional<uint16_t> exponent_size;
    };
  };

  void parse(std::shared_ptr<File> unit) {}
};

} // namespace Onyx
} // namespace FNX
