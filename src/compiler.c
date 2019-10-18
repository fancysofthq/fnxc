#include "./tokenizer.h"
#include <lauxlib.h>
#include <locale.h>
#include <lua.h>
#include <lualib.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <threads.h>
#include <wchar.h>

// A function to compile an Onyx file.
// Compilation of each file happens in a separate thread.
// There is a queue of files to be compiled.
void compile_file(const char *filename) {
  FILE *file = fopen(filename, "r");

  if (!file) {
    // Oops, the file can't be opened!
    printf("Can't open \"%s\"\n", filename);
    exit(EXIT_FAILURE);
  }

  int row = 0;
  int col = 0;
  wchar_t codepoint;

  // Let's tokenize the file token by token...
  //

  while ((codepoint = fgetwc(file)) != EOF) {
    // wprintf(L"%lc\n", codepoint);
  }

  fclose(file);
}

#define FILE_QUEUE_LIMIT 128

static int jobs_limit;
static atomic_int active_jobs;

static char *file_queue[FILE_QUEUE_LIMIT];
static int file_queue_size = 0;

int queue_file(char *filename) {
  if (file_queue_size == FILE_QUEUE_LIMIT) {
    return 0; // You're out of luck!
  } else {
    file_queue[file_queue_size] = filename;
    file_queue_size++;
    return 1; // Successfully queued the file
  }
}

int main(int argc, char *argv[]) {
  const char *progname = argv[0];
  char *filename;

  int is_jobs;

  for (int i = 1; i <= argc; i++) {
    if (i == 1) {
      // The first argument is expected to be an Onyx file to compile
      filename = argv[1];
    }

    if (is_jobs) {
      jobs_limit = atoi(argv[i]);

      if (!jobs_limit) {
        puts("Expected jobs count to be > 0");
        exit(EXIT_FAILURE);
      }

      is_jobs = 0;
    }

    if (strcmp(argv[i], "-j"))
      is_jobs = 1;
  }

  if (!filename) {
    puts("Expected a file name to be the first argument!");
    exit(EXIT_FAILURE);
  }

  // Found a file, let's open it...
  //

  queue_file(filename);

  exit(EXIT_SUCCESS);

  char buff[256];
  int error;

  // Create a new Lua context
  lua_State *L = luaL_newstate();
  if (L == NULL) {
    fprintf(stderr, "Fatal! Cannot create Lua state: not enough memory");
  }

  // Require standard Lua libs
  luaL_openlibs(L);

  printf("> ");
  while (fgets(buff, sizeof(buff), stdin)) {
    error = luaL_loadbufferx(L, buff, strlen(buff), "line", 0) ||
            lua_pcall(L, 0, 0, 0);

    if (error) {
      fprintf(stderr, "%s\n", lua_tostring(L, -1));
      lua_close(L);
      exit(EXIT_FAILURE);
    }

    printf("> ");
  }

  lua_close(L);
  return 0;
}
