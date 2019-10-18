#include <wchar.h>

struct TIdentifier {
  char value[32];
};

// Suits -, hexa- and octadecimal and also bit integers.
struct TNumeric {
  char significand[100];
  char int sign : 2; // 0 — none, 1 — +, 2 — -
  int literal : 2;   // 0 — none, 1 — i, 2 — u, 3 — f
  int bitsize;       // 0 — none, otherwise is bitsize
};

struct tok_context {
  wchar_t buffer[1024];
};
