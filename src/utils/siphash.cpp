/*
© [Isaac
Whitfield](https://github.com/whitfin/siphash-cpp/blob/c41497ccda8bf4cc8750bee4fa5719612a8d7c93/LICENSE)
*/

#include "./siphash.hpp"

SipHash::SipHash(const char key[16], const int c, const int d) {
  this->_c = c;
  this->_d = d;

  uint64_t k0 = _u8to64_le(key);
  uint64_t k1 = _u8to64_le(key + 8);

  this->_v0 = (0x736f6d6570736575 ^ k0);
  this->_v1 = (0x646f72616e646f6d ^ k1);
  this->_v2 = (0x6c7967656e657261 ^ k0);
  this->_v3 = (0x7465646279746573 ^ k1);

  this->_m_idx = 0;
  this->_input_len = 0;
  this->_m = 0;
}

SipHash *SipHash::update(const char b) {
  _input_len++;
  _m |= (((long)b & 0xff) << (_m_idx++ * 8));

  if (_m_idx >= 8) {
    _digest();
    _m_idx = 0;
    _m = 0;
  }

  return this;
}

SipHash *SipHash::update(const char data[]) {
  const char *pszChar = data;

  while (pszChar != NULL && *pszChar != '\0') {
    update(*pszChar);
    pszChar++;
  }

  return this;
}

uint64_t SipHash::digest() {
  while (_m_idx < 7) {
    _m |= 0 << (_m_idx++ * 8);
  }

  _m |= ((uint64_t)_input_len) << (_m_idx * 8);

  _digest();

  _v2 ^= 0xff;

  for (int i = 0; i < _d; i++) {
    _compress();
  }

  return ((uint64_t)_v0 ^ _v1 ^ _v2 ^ _v3);
}

uint64_t SipHash::_u8to64_le(const char p[8]) {
  return (
      ((uint64_t)((p)[0])) | ((uint64_t)((p)[1]) << 8) |
      ((uint64_t)((p)[2]) << 16) | ((uint64_t)((p)[3]) << 24) |
      ((uint64_t)((p)[4]) << 32) | ((uint64_t)((p)[5]) << 40) |
      ((uint64_t)((p)[6]) << 48) | ((uint64_t)((p)[7]) << 56));
}

unsigned long SipHash::_rotate_left(const uint64_t x, const uint64_t b) {
  return (((x) << (b)) | ((x) >> (64 - (b))));
}

void SipHash::_compress() {
  _v0 += _v1;
  _v2 += _v3;
  _v1 = _rotate_left(_v1, 13);
  _v3 = _rotate_left(_v3, 16);
  _v1 ^= _v0;
  _v3 ^= _v2;
  _v0 = _rotate_left(_v0, 32);
  _v2 += _v1;
  _v0 += _v3;
  _v1 = _rotate_left(_v1, 17);
  _v3 = _rotate_left(_v3, 21);
  _v1 ^= _v2;
  _v3 ^= _v0;
  _v2 = _rotate_left(_v2, 32);
}

void SipHash::_digest() {
  _v3 ^= _m;
  do {
    int i;
    for (i = 0; i < _c; i++) {
      _compress();
    }
  } while (0);
  _v0 ^= _m;
}
