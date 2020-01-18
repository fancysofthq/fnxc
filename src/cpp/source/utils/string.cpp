#include <cstdarg>
#include <sstream>

#include "../../header/utils/string.hpp"

std::string format(const char *fmt...) {
  va_list args;
  va_start(args, fmt);

  std::stringstream buff;

  while (*fmt != '\0') {
    if (*fmt++ == '%') {
      if (*fmt == 'd') {
        int i = va_arg(args, int);
        buff << i << '\n';
      } else if (*fmt == 'c') {
        // note automatic conversion to integral type
        int c = va_arg(args, int);
        buff << static_cast<char>(c) << '\n';
      } else if (*fmt == 'f') {
        double d = va_arg(args, double);
        buff << d << '\n';
      } else if (*fmt == '\0')
        break;
    }
  }

  va_end(args);
  return buff.str();
}
