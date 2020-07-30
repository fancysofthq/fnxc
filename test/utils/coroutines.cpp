#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "../../lib/doctest/doctest/doctest.h"

#include "../../src/cpp/source/utils/coroutines.cpp"

generator<int> _generator() {
  co_yield(42);
  co_yield(43);
}

TEST_CASE("straightforward generator") {
  int output = 0;
  auto gen = _generator();

  output = gen.next();
  CHECK(output == 42);
  CHECK(gen.current() == 42);
  CHECK(!gen.done());

  output = gen.next();
  CHECK(output == 43);
  CHECK(gen.current() == 43);
  CHECK(!gen.done());

  output = gen.next();
  CHECK(output == 43);
  CHECK(gen.current() == 43);
  CHECK(gen.done());

  int i = 0;
  for (auto y : _generator()) {
    switch (i++) {
    case 0:
      CHECK(y == 42);
      break;
    case 1:
      CHECK(y == 43);
      break;
    default:
      FAIL("Iterator must have ended");
    }
  }
}

int sync_switch = 0;

co::sync<int> _sync() {
  co_return(42);
  sync_switch = 1;
}

TEST_CASE("straightforward sync") {
  auto s = _sync();
  CHECK(s.get() == 42);
  CHECK(s.get() == 42);
  CHECK(sync_switch == 0);
}

int async_switch = 0;

async<int> _async() {
  co_return(42);
  async_switch = 1;
}

TEST_CASE("straightforward async") {
  auto as = _async();
  CHECK(as.get() == 42);
  CHECK(as.get() == 42);
  CHECK(async_switch == 0);
}

generator<int> _recursive_generator() {
  co_yield(100);
  co_yield(101);
}

generator<int> _complex_generator() {
  auto rec = _recursive_generator();
  co_yield(1);
  co_yield co_await(_async());
  co_yield rec.next();
  co_yield(2);
  co_yield rec.next();
  co_yield co_await(_async());
  co_yield(3);
  co_yield rec.next();
}

TEST_CASE("complex generator") {
  auto gen = _complex_generator();
  CHECK(gen.next() == 1);
  CHECK(gen.next() == 42);
  CHECK(gen.next() == 100);
  CHECK(gen.next() == 2);
  CHECK(gen.next() == 101);
  CHECK(gen.next() == 42);
  CHECK(gen.next() == 3);
  CHECK(gen.next() == 101);
  CHECK(!gen.done());
  CHECK(gen.next() == 101);
  CHECK(gen.done());
}
