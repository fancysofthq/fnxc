#ifndef MINIX_H
#define MINIX_H

// Minix is an interpreted programming language
// used exclusively in Onyx macros

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <threads.h>
#include <wchar.h>
#include <wctype.h>

#include "../include/hash.c"
#include "./minix.h"
#include "./minix/tables.h"
#include "./minix/tokens.h"
#include "./minix/types.h"

// Local registers, stack and heap are expected to be cleaned
// up upon exiting from compiling a file.
//

static thread_local void *RNext;     // The Program Counter register
static thread_local void *RStackPtr; // The Stack Pointer register
static thread_local void *RBasePtr;  // The Base Pointer register
static thread_local intptr_t RArg;   // General-purpose argument register

static thread_local char *vasm;   // The virtual assembly buffer
static thread_local char *vstack; // The virtual call stack

// Here go the virtual tables.
// The threadlocal ones are for local definitions,
// shared are for global definitions.
//

static thread_local struct MinixFunctionTable local_functions;
static struct MinixFunctionTable global_functions;

static thread_local struct MinixVariableTable local_variables;
static struct MinixVariableTable global_variables;

// Variables below are for parsing.
//

// The latest tokenized token.
static thread_local struct MinixToken token;

// Some internal variables...
static thread_local wchar_t *input;
static thread_local int input_line;
static thread_local int input_column;

// Function definitions.
//

int minix_reset(size_t vasm_size, size_t stack_size) {
  free(vasm);
  vasm = malloc(vasm_size);

  free(vstack);
  vstack = malloc(stack_size);

  return 0;

  free(local_functions.pointer);
  local_functions.pointer = NULL;

  free(local_variables.pointer);
  local_variables.pointer = NULL;

  input_line = 1;
  input_column = 1;

  return 0;
}

// Compare current codepoint with *codepoint* arguments and then
// return the next codepoint in input.
// Returns -1 in case of codepoint mismatch.
static wchar_t consume(wchar_t codepoint) {
  if (*input++ == codepoint) {
    return *input;
  } else
    return -1;
}

// A private function to tokenize another token.
// 0 means successfull tokenization, should check the `token` then.
// Otherwise check `minix_error`.
static int tokenize() {
  wchar_t cp; // Current Code Point

  while ((cp = *input)) {
    if (cp == '\n' || cp == ';') {
      // Newlines are counted as separate tokens
      token = (struct MinixToken){.kind = MinixTokenKindNewline,
                                  .line_start = input_line,
                                  .column_start = input_column};

      if (cp == '\n') {
        input_line++;
        input_column = 0;
      }

      consume(cp);

      return 0;
    } else if (cp == '#') {
      // This is a Minix comment.
      // Skip input until newline or EOF is met
      while (*input != WEOF && *input != '\n')
        ++input; // No need to increment column here
    } else if (iswalpha(cp) || cp == '_' || cp == '$' || cp == '&') {
      token = (struct MinixToken){.kind = MinixTokenKindID,
                                  .line_start = input_line,
                                  .column_start = input_column,
                                  .id = {.value = malloc(sizeof(char) + 1)}};

      token.id.value[0] = (char)cp;
      token.id.value[1] = '\0';

      if (cp == '$' || cp == '&') {
        cp = consume(cp);
        input_column++;

        if (!(iswalnum(cp) || cp == '_')) {
          minix_error = (struct MinixError){
              .message = "Expected identifier",
              .line_start = token.line_start,
              .column_start = token.column_start,
              .line_end = input_line,
              .column_end = input_column,
          };

          return 1;
        }
      }

      while (iswalnum(cp) || cp == '_') {
        token.id.value = strcat(token.id.value, (char *)&cp);
        cp = consume(cp);
        input_column++;
      }

      token.line_end = input_line;
      token.column_end = input_column;

      return 0;
    } else if (iswdigit(cp)) {
      // Minix supports decimal integer and floating point numbers only
    }
  }

  token = (struct MinixToken){.kind = MinixTokenKindEOF};

  return 0;
}

// Point the *minix_error* to the *token* with given *message*.
static int err_token(struct MinixToken *token, char *message) {
  minix_error = (struct MinixError){
      message,           token->line_start,
      token->line_end,   token->column_start,
      token->column_end,
  };

  return 1;
}

int minix_parse(wchar_t *input) {
  bool is_completed = true;
  bool is_global = false;
  bool is_function_prototype = false;

  while (true) {
    int err = tokenize();
    if (!err)
      return err;

    if (token.kind == MinixTokenKindID) {
      if (strcmp(token.id.value, "global")) {
        is_global = true;
        continue;
      }

      if (strcmp(token.id.value, "function")) {
        is_function_prototype = true;
        continue;
      }

      if (strcmp(token.id.value, "include")) {
        int err = tokenize();
        if (!err)
          return err;

        if (token.kind != MinixTokenKindString)
          return err_token(&token, "Expected file path");

        if (token.string.is_regex)
          return err_token(&token, "Unexpected regex");

        if (!token.string.is_ascii_only)
          return err_token(&token,
                           "File path must include ASCII characters only");

        minix_included_filepath = token.string.value;
        return -2;
      }

      if (is_function_prototype) {
        if (!(token.kind == MinixTokenKindID))
          return err_token(&token, "Expected function name");

        struct MinixFunction fun = {
            .hash = hash((unsigned char *)token.id.value),
            .name = token.id.value,
        };

        // ...

        if (is_global)
          minix_functions_push(&global_functions, fun);
        else
          minix_functions_push(&local_functions, fun);
      }
    }
  }
}

#endif
