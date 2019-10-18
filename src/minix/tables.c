#include "./tables.h"

struct MinixFunction minix_functions_push(struct MinixFunctionTable *table,
                                          struct MinixFunction func) {
  if (table->size == table->capacity) {
    const size_t esize = sizeof(struct MinixFunction);

    if (table->capacity) {
      size_t size = esize * (table->capacity *= 2);
      table->pointer = realloc(table->pointer, size);
    } else {
      size_t size = esize * (table->capacity = 1);
      table->pointer = malloc(size);
    }
  }

  func.address = table->pointer + table->size;
  table->pointer[table->size] = func;

  table->size += 1;

  return func;
}

struct MinixVariable minix_variables_push(struct MinixVariableTable *table,
                                          struct MinixVariable var) {
  if (table->size == table->capacity) {
    const size_t esize = sizeof(struct MinixVariable);

    if (table->capacity) {
      size_t size = esize * (table->capacity *= 2);
      table->pointer = realloc(table->pointer, size);
    } else {
      size_t size = esize * (table->capacity = 1);
      table->pointer = malloc(size);
    }
  }

  var.address = table->pointer + table->size;
  table->pointer[table->size] = var;

  table->size += 1;
  return var;
}
