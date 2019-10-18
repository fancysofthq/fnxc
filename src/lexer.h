#include "./tokenizer.h"

union LExpression {
  struct TIdentifier id;
  struct TNumeric number;
};

struct LCall {
  struct TIdentifier caller;
  struct TIdentifier callee;
  union LExpression *arguments;
};

union LAssignment {
  union LExpression *left;
  union LExpression *right;
};
