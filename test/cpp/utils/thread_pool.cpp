#include <atomic>
#include <chrono>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"

#include "../../../src/cpp/utils/thread_pool.cpp"

using namespace FNX::Utils;

// TODO: Test the priority.

TEST_CASE("ThreadPool") {
  ThreadPool pool(std::thread::hardware_concurrency());

  std::atomic<int> sum = 0; // C++14§29.6.5¶4
  std::future<int> future;

  for (int i = 1; i <= 1024; i++) {
    future = pool.enqueue([&sum, i] {
      sum.fetch_add(i);
      return i;
    });
  }

  std::this_thread::sleep_for(std::chrono::milliseconds(100));

#ifdef _MSC_VER
  CHECK_MESSAGE(future.get() == 1024, "Futures are broken on MSVC");
#else
  CHECK(future.get() == 1024);
#endif

  CHECK(sum == 524800);
}
