// Coroutines implementation inspired by
// https://kirit.com/How%20C%2B%2B%20coroutines%20work,
// © 2002-2019 Kirit & Tai Sælensminde

#pragma once

// TODO: Only valid header for CLang-CL driver.
#ifdef _WIN32
#include "./windows-coroutines.hpp"
#elif __linux__
#include <experimental/coroutine>
#endif

namespace FNX {
namespace Utils {

template <class T> struct generator {
  struct promise_type {
    T current_value;

    promise_type(){};
    ~promise_type(){};

    std::experimental::suspend_always initial_suspend();
    generator get_return_object();
    std::experimental::suspend_always yield_value(T);
    std::experimental::suspend_never return_void();
    std::experimental::suspend_always final_suspend();
    void unhandled_exception();
  };

  struct iterator {
    generator &owner;
    bool done;

    iterator(generator &, bool done);
    void advance();

    bool operator!=(const iterator &) const;
    iterator &operator++();
    T operator*() const;
  };

  using handle_type =
      std::experimental::coroutine_handle<promise_type>;

  generator(const generator &) = delete;
  generator(generator &&);
  ~generator();

  T current();
  T next();
  bool done();

  iterator begin();
  iterator end();

private:
  handle_type coro;
  generator(handle_type h) : coro(h){};
};

template <class T> struct coreturn {
  struct promise;
  friend struct promise;

  using handle_type = std::experimental::coroutine_handle<promise>;

  coreturn(const coreturn &) = delete;
  coreturn(coreturn &&);
  ~coreturn();

  coreturn &operator=(const coreturn &) = delete;
  coreturn &operator=(coreturn &&);

  struct promise {
    T value;

    friend struct coreturn;

    promise() {}
    ~promise() {}

    std::experimental::suspend_never return_value(T);
    std::experimental::suspend_always final_suspend();
    void unhandled_exception();
  };

protected:
  handle_type coro;
  coreturn(handle_type h) : coro(h) {}
  T get();
};

template <class T> struct sync : public coreturn<T> {
  using coreturn<T>::coreturn;
  using handle_type = typename coreturn<T>::handle_type;

  struct promise_type : public coreturn<T>::promise {
    sync<T> get_return_object();
    std::experimental::suspend_never initial_suspend();
  };

  T get();
};

template <typename T> struct async : public coreturn<T> {
  using coreturn<T>::coreturn;
  using handle_type = typename coreturn<T>::handle_type;

  struct promise_type : public coreturn<T>::promise {
    async<T> get_return_object();
    std::experimental::suspend_always initial_suspend();
  };

  struct awaitable_type {
    handle_type coro;

    // Return a boolean to describe whether or not a suspend is
    // needed, `true` for "don't suspend" and `false` for "suspend".
    bool await_ready();

    // Called when a suspend is needed because the value isn't ready
    // yet.
    std::experimental::coroutine_handle<>
    await_suspend(std::experimental::coroutine_handle<> awaiting);

    // Return the value that is being awaited on.
    // This becomes the result of the `co_await` expression.
    T await_resume();
  };

  T get();

  awaitable_type operator co_await();
};

} // namespace Utils
} // namespace FNX
