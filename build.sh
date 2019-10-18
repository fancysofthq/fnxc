musl-gcc src/compiler.c \
  -O0 \
  -Ilib/lua-5.3.5/src \
  -Llib/lua-5.3.5/src \
  -llua \
  -o bin/onyx
