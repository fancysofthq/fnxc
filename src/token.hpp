#pragma once

#include "./shared.hpp"

using namespace std;

namespace Onyx {
namespace Token {

struct Base {
  Location location;

  Base(Location loc) : location(loc) {}

  virtual ~Base(){};
};

struct Eof : public Base {
  Eof(Location loc) : Base(loc) {}
};

struct Newline : public Base {
  Newline(Location loc) : Base(loc) {}
};

// An identifier which may refer to a variable, constant, function or type.
// It allows Latin and Greek letters, digits and underscores.
struct ID : public Base {
  wstring value;
  ID(Location loc, const wstring _value) : Base(loc), value(_value) {}
};

// An operator token of predefined Unicode range.
struct Op : public Base {
  wstring value;
  Op(Location loc, const wstring value) : Base(loc), value(value) {}
};

struct StringLiteral : public Base {
  wstring value;

  StringLiteral(Location loc, const wstring _value) :
      Base(loc),
      value(_value) {}
};

struct DecimalIntLiteral : public Base {
  unsigned long int value;
  unsigned int bytesize;      // 0 if unset
  unsigned explicit_type : 2; // 1 for `i`, 2 for `u`
  unsigned explicit_sign : 2; // 1 for `-`, 2 for `+`
};

struct Other : public Base {
  wstring value;
  Other(Location loc, const wstring _value) : Base(loc), value(_value) {}
};
} // namespace Token
} // namespace Onyx
