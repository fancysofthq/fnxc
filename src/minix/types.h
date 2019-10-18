#ifndef MINIX_TYPES_H
#define MINIX_TYPES_H

#include <stdbool.h>
#include <stdlib.h>

// Here goes the Minix type system.
// The thing is that we need to validate arguments before call,
// so no segmentation fault happens. For that, we have to store
// function arguments and variable types in virtual tables.
//

typedef int MinixTypeBasic; // One of the predefined basic types

enum {
  MinixTypeVoid,
  MinixTypeInt,
  MinixTypeFloat,
  MinixTypeBool,
  MinixTypeString,
  MinixTypeRegex,
  MinixTypeFile,
  MinixTypeNode
};

// A user-defined struct type, e.g. `type &foo(string bar)`.
struct MinixTypeStruct {
  size_t size;
  struct MinixType *types;
  char *names;
};

// A type may be compound — an array or a hash.
// For that, we would need a complex wrapper
// which would tell us the exact type.
//
// Minix type     | `MinixType` value
// ---            | ---
// `void`         | `{0, 1, 0, 0, 0}`
// `int`          | `{0, 1, 0, TInt, 0}`
// `float[]`      | `{1, 1, 0, TFloat, 0}`
// `int[string]`  | `{1, 1, 1, TInt, TString}`
// `&foo`         | `{0, 0, 0, &foo, 0}`
// `&foo[]`       | `{1, 0, 0, &foo, 0}`
// `&foo[string]` | `{1, 0, 1, &foo, TString}`
//
struct MinixType {
  bool is_compound;
  bool is_value_basic;
  bool is_key_basic;

  union {
    MinixTypeBasic basic_value;
    struct MinixTypeStruct *struct_value;
  };

  union {
    MinixTypeBasic basic_key;
    struct MinixTypeStruct *struct_key;
  };
};

#endif
