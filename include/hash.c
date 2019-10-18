unsigned long hash(unsigned char *string) {
  unsigned long hash = 5381;
  int ch;

  while ((ch = *string++))
    hash = ((hash << 5) + hash) + ch; /* hash * 33 + ch */

  return hash;
}
