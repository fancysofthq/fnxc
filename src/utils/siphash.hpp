/*
© [Isaac
Whitfield](https://github.com/whitfin/siphash-cpp/blob/c41497ccda8bf4cc8750bee4fa5719612a8d7c93/LICENSE)
*/

#pragma once

#include <cstdint>

class SipHash {
  int _c, _d, _m_idx;
  uint64_t _v0, _v1, _v2, _v3, _m;
  unsigned char _input_len;

public:
  // Create a new SipHash instance.
  SipHash(const char key[16], const int c = 2, const int d = 4);

  // Update the input with a single character.
  SipHash *update(const char);

  // Update the input with some characters
  // terminated with `NULL` or `\0`.
  SipHash *update(const char[]);

  // Calculate the resulting digest.
  uint64_t digest();

private:
  uint64_t _u8to64_le(const char[8]);
  void _digest();
  void _compress();
  unsigned long _rotate_left(const uint64_t, const uint64_t);
};
