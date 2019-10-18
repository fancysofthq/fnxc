#ifndef MINIX_TOKENS_H
#define MINIX_TOKENS_H

#include <stdbool.h>
#include <stdint.h>
#include <wchar.h>

// Tokens are produced by tokenizer and then used by parser
//

typedef enum {
  MinixTokenKindEOF,
  MinixTokenKindNewline,
  MinixTokenKindID,
  MinixTokenKindOp,
  MinixTokenKindInt,
  MinixTokenKindFloat,
  MinixTokenKindBool,
  MinixTokenKindString,
} minix_token_kind;

struct MinixTokenID {
  bool is_keyword;
  char *value;
};

struct MinixTokenOp {
  bool is_binary;
  char value[2]; // Maximum of two chars allowed in a Minix operator
};

struct MinixTokenInt {
  intptr_t value;
};

struct MinixTokenFloat {
  double value;
};

struct MinixTokenBool {
  bool value;
};

struct MinixTokenString {
  bool is_regex; // Is it expilicitly regular expression?
  bool is_ascii_only;
  wchar_t *value;
};

struct MinixToken {
  minix_token_kind kind;

  int line_start;
  int line_end;
  int column_start;
  int column_end;

  union {
    struct MinixTokenID id;
    struct MinixTokenOp op;
    struct MinixTokenInt _int;
    struct MinixTokenFloat _float;
    struct MinixTokenBool _bool;
    struct MinixTokenString string;
  };
};

#endif
