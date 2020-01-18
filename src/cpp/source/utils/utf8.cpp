#include "../../header/utils/utf8.hpp"

// The implemenation is adapted from
// https://rosettacode.org/wiki/UTF-8_encode_and_decode#C,
// licensed under GNU (FDL) 1.2.
namespace UTF8 {
typedef struct {
  // This mask is applied to a byte to
  // determine the size of a codepoint.
  uint8_t mask;

  // Leading byte mask.
  uint8_t lead;

  // The beginning of the codepoint range.
  uint32_t range_begin;

  // The end of the codepoint range.
  uint32_t range_end;

  // The number of significant bits in an encoded byte.
  uint8_t significant_bits;
} utf_t;

utf_t *utf[] = {
    new utf_t({0b00111111, 0b10000000, 0, 0, 6}),
    new utf_t({0b01111111, 0b00000000, 0000, 0177, 7}),
    new utf_t({0b00011111, 0b11000000, 0200, 03777, 5}),
    new utf_t({0b00001111, 0b11100000, 04000, 0177777, 4}),
    new utf_t({0b00000111, 0b11110000, 0200000, 04177777, 3}),
    0,
};

size_t codepoint_byte_size(uint32_t codepoint) {
  size_t len = 0;

  for (utf_t **u = utf; *u; ++u) {
    if ((codepoint >= (*u)->range_begin) &&
        (codepoint <= (*u)->range_end))
      break;

    ++len;
  }

  if (len > 4)
    throw Error("UTF-8 codepoint length out of boundaries");

  if (len == 0)
    return 1; // \0 still takes one byte
  else
    return len;
}

size_t size_from_leading_byte(char ch) {
  size_t len = 0;

  for (utf_t **u = utf; *u; ++u) {
    uint8_t result = (ch & ~(*u)->mask);

    if (result == ((*u)->lead))
      break;

    ++len;
  }

  if (len > 4)
    throw Error("Malformed leading byte");

  return len;
}

char *to_codeunits(uint32_t codepoint) {
  static thread_local char ret[5];
  const int bytes = codepoint_byte_size(codepoint);

  int shift = utf[0]->significant_bits * (bytes - 1);
  ret[0] = (codepoint >> shift & utf[bytes]->mask) | utf[bytes]->lead;
  shift -= utf[0]->significant_bits;

  for (int i = 1; i < bytes; ++i) {
    ret[i] = (codepoint >> shift & utf[0]->mask) | utf[0]->lead;
    shift -= utf[0]->significant_bits;
  }

  ret[bytes] = '\0';
  return ret;
}

uint32_t to_codepoint(const char *codeunits) {
  int bytes = size_from_leading_byte(*codeunits);
  int shift = utf[0]->significant_bits * (bytes - 1);
  uint32_t codepoint = (*codeunits++ & utf[bytes]->mask) << shift;

  for (int i = 1; i < bytes; ++i, ++codeunits) {
    shift -= utf[0]->significant_bits;
    codepoint |= ((char)*codeunits & utf[0]->mask) << shift;
  }

  return codepoint;
}
} // namespace UTF8
