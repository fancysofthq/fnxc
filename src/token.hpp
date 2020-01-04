#pragma once

#include "./shared.hpp"

using namespace std;

namespace Onyx {
namespace Token {
struct Base {
  Location location;
  Base(Location loc) : location(loc) {}
  virtual ~Base() {}
};

struct Control : Base {
  enum Kind {
    Eof,       //
    Newline,   // \n
    Space,     //
    Dot,       // .
    Semicolon, // ;
    Colon,     // :
    Comma,     // ,

    OpenParen,   // (
    CloseParen,  // )
    OpenCurly,   // {
    CloseCurly,  // }
    OpenSquare,  // [
    CloseSquare, // ]

    CurlyArrow, // ~>
    ThinArrow,  // ->
    ThickArrow, // =>
    PipeArrow,  // |>
  };

  Kind kind;

  Control(Location loc, Kind kind) : Base(loc), kind(kind) {}

  wstring pretty_source();
  wstring source();
};

// A value token.
struct Value : Base {
  enum Kind {
    Variable,    // Variable, e.g. `foo`
    Type,        // Type, e.g. `Foo`
    Operator,    // Operator, e.g. `&&`
    Intrinsic,   // Intrinsic, e.g. `@foo`
    CIdentifier, // C identifier, e.g. `` `Foo ``
    Comment,
    Macro,
  };

  Kind kind;
  wstring value;

  Value(Location loc, Kind kind, wstring value) :
      Base(loc),
      kind(kind),
      value(value) {}

  wstring pretty_kind();
};

// An indexed anonymous block argument, e.g. `&1`.
struct AnonArg : Base {
  uint8_t index;
  AnonArg(Location loc, uint8_t index) : Base(loc), index(index) {}
  wstring source();
};

struct Literal : Base {};

// A decimal integer literal, e.g. `1`, `42i16`.
struct DecimalInt : Base {
  vector<char> digits;

  enum Type { UndefType, Signed, Unsigned };
  Type type;

  uint32_t bitsize; // 0 if undefined

  DecimalInt(
      Location loc,
      vector<char> digits,
      Type type = UndefType,
      uint32_t bitsize = 0) :
      Base(loc),
      digits(digits),
      type(type),
      bitsize(bitsize) {}

  wstring source();
};

// A decimal floating point literal, e.g. `0.5`, `42.10e-3`.
struct DecimalFloat : Base {
  // Examples of significands: `5` for `0.5`, `4210` for `-42.10e-3`
  vector<char> significand;

  // For example, `-1` for `0.5`, `-5 == (-3 - 2)` for `-42.10e-3`
  int exponent;

  // Floating point literals have predefined list of
  // possible bitsizes dictated by IEEE 754.
  enum Bitsize { UndefBitsize, Float16, Float32, Float64 };
  Bitsize bitsize;

  DecimalFloat(
      Location loc, vector<char> significand, int exponent, Bitsize bitsize) :
      Base(loc),
      significand(significand),
      bitsize(bitsize) {}

  wstring source();
}; // namespace Token

// A non-decimal number literal, for example:
//
// * `0b00000001`, `0b10101010_10010111_u16` (bina-decimal (or just binary))
// * `0o0317`, `0o4516_1254_f16` (octa-decimal (or just octal))
// * `0xF8`, `0x4e_3a_i16` (hexa-decimal)
//
struct NonDecimalNumber : Base {
  enum Kind { Bina, Octa, Hexa };
  Kind kind;

  vector<char> chars; // Only ASCII chars are allowed

  enum Type { UndefType, SignedInt, UnsignedInt, Float };
  Type type;

  uint32_t bitsize; // 0 if undefined

  NonDecimalNumber(
      Location loc,
      Kind kind,
      vector<char> chars,
      Type type,
      uint32_t bitsize) :
      Base(loc),
      kind(kind),
      chars(chars),
      type(type),
      bitsize(bitsize) {}

  wstring source();
};
}; // namespace Token
} // namespace Onyx
