#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <threads.h>
#include <wchar.h>
#include <wctype.h>

thread_local wchar_t *minix_emitbuff; // The emitted UTF-8 Onyx code buffer

struct MinixError {
  char *message;
  int line_start;
  int line_end;
  int column_start;
  int column_end;
};

thread_local struct MinixError minix_error;
thread_local wchar_t *minix_included_filepath;

typedef enum { MinixFailureOOM = 1 } minix_failure;

// Initialize the global state. It must be called exactly once!
int minix_initialize();

// Prepare a fresh new context, i.e. local stack and virtual tables.
// Should be called even after `initialize`.
int minix_reset(size_t vasm_size, size_t stack_size);

// In case of successfull parsing it returns zero.
// The caller then should check `minix_emitbuff` then.
//
// If the source is not enough to finish an expression,
// -1 is returned. In that case, all freestanding Onyx code should
// be treated as `emit("<code>")` call made to Minix.
// This is repeated until the expression is finished.
//
// In case of error, 1 is returned. Should check `minix_error` then.
int minix_parse(wchar_t *input);

// A function to execute instructions stored in the Minix VASM buffer.
int minix_execute();
