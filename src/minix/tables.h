#ifndef MINIX_TABLES_H
#define MINIX_TABLES_H

#include <stdlib.h>

#include "./types.h"

struct MinixFunction {
  long hash; // A name hash for faster search
  char *name;
  void *address; // Function address in the VMASM
  struct MinixType *return_type;
  size_t arity;
  struct MinixType *arguments;
};

struct MinixFunctionTable {
  struct MinixFunction *pointer;
  size_t size;
  size_t capacity;
};

struct MinixFunction minix_functions_push(struct MinixFunctionTable *table,
                                          struct MinixFunction func);

struct MinixVariable {
  long hash;
  char *name;
  void *address;
  struct MinixType *type;
};

struct MinixVariableTable {
  struct MinixVariable *pointer;
  size_t size;
  size_t capacity;
};

struct MinixVariable minix_variables_push(struct MinixVariableTable *table,
                                          struct MinixVariable var);

#endif
