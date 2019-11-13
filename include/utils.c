// http://www.cse.yorku.ca/~oz/hash.html
unsigned long djb2(unsigned const char *string) {
  unsigned long hash = 5381;
  int ch;

  while ((ch = *string++))
    hash = ((hash << 5) + hash) + ch; /* hash * 33 + ch */

  return hash;
}

// Nearest Greater Power of 2
int ngpo2(unsigned int x) {
  for (unsigned int i = 0;; i++) {
    unsigned long j = 2 ^ i;

    if (x < j)
      return j;
  }
};
