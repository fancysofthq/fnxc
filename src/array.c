#include <stdlib.h>

typedef struct {
  void *pointer;
  size_t element_size;
  size_t size;
  size_t capacity;
} vector;

// Initializes a new array with `capacity * element_size` buffer.
void vector_resize(vector *vec, size_t capacity) {
  size_t size = capacity * vec->element_size;

  if (vec->pointer) {
    vec->pointer = realloc(vec->pointer, size);
  } else {
    vec->pointer = malloc(size);
  }
}

void vector_initialize(vector *vec) { vector_resize(vec, 1); }

void vector_push(vector *vec, void *value) {
  if (vec->size == vec->capacity) {
    vector_resize(vec, vec->capacity * 2);
  }

  vec->pointer[vec->size] = value;
  vec->size++;
}
