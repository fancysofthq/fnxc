#pragma once

#include "./shared.hpp"

using namespace std;

namespace Onyx {
namespace Token {
struct Base {
  Location loc;
  Base(Location loc) : loc(loc) {}
  virtual ~Base() {}
};

struct Control : Base {
  enum Kind {
    Eof,         //
    Newline,     // \n
    Space,       //
    Dot,         // .
    Semi,        // ;
    Colon,       // :
    Comma,       // ,
    Assign,      // =
    OpenParen,   // (
    CloseParen,  // )
    OpenCurly,   // {
    CloseCurly,  // }
    OpenSquare,  // [
    CloseSquare, // ]
    Block,       // ~>
    BlockParen,  // |
    Pipe,        // |>
    KeyValue,    // =>
    Interpol,    // #{
  };

  Kind kind;

  Control(Location loc, Kind kind) : Base(loc), kind(kind) {}
};

struct AnonArg : Base {
  ushort index;
  AnonArg(Location loc, ushort index) : Base(loc), index(index) {}
};

struct Macro : Base {};

struct Id : Base {
  enum Kind {
    Var,       // `foo` (/[a-zα-ω_][a-zA-Zα-ωΑ-Ω0-9_]*/)
    Type,      // `Foo` (/[A-ZΑ-Ω][a-zA-Zα-ωΑ-Ω0-9_]*/)
    Op,        // `&&` (in predefined ranges of Unicode)
    Intrinsic, // `@foo.bar`
    C          // `` `Foo ``
  };

  Kind kind;
  wstring value;

  Id(Location loc, Kind kind, wstring value) :
      Base(loc),
      kind(kind),
      value(value) {}
};

struct String : Base {
  wstring value;
  String(Location loc, wstring value) : Base(loc), value(value) {}
};

// `1`, `42i16`
struct DecimalInt : Base {
  ulong value; // The numerical value

  enum Type { UndefinedType, Signed, Unsigned };
  Type type;

  uint bitsize; // 0 if undefined

  DecimalInt(
      Location loc, ulong value, Type type = UndefinedType, uint bitsize = 0) :
      Base(loc),
      value(value),
      type(type),
      bitsize(bitsize) {}
};

// `0.5`, `42.10e-3`
struct DecimalFloat : Base {
  ulong significand; // `5` for `0.5`, `4210` for `-42.10e-3`
  int exponent;      // `-1` for `0.5`, `-5 == (-3 - 2)` for `-42.10e-3`

  enum Bitsize { UndefinedBitesize, Float16, Float32, Float64 };
  Bitsize bitsize;

  DecimalFloat(Location loc, ulong significand, int exponent, Bitsize bitsize) :
      Base(loc),
      significand(significand),
      bitsize(bitsize) {}
};

// `0b00000001`, `0b10101010_10010111_u16`
// `0o0317`, `0o4516_1254_f16`
// `0xF8`, `0x4e_3a_i16`
struct NonDecimalNumber : Base {
  enum Kind { Bina, Octa, Hexa };
  Kind kind;

  vector<char> value;

  enum Type { UndefinedType, SignedInt, UnsignedInt, Float };
  Type type;

  uint bitsize;

  NonDecimalNumber(
      Location loc, Kind kind, vector<char> value, Type type, uint bitsize) :
      Base(loc),
      kind(kind),
      value(value),
      type(type),
      bitsize(bitsize) {}
};
}; // namespace Token
} // namespace Onyx
