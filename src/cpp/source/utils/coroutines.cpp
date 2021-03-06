#include "../../header/utils/coroutines.hpp"

template <class T>
std::experimental::suspend_always
co::generator<T>::promise_type::initial_suspend() {
  return std::experimental::suspend_always{};
}

template <class T>
co::generator<T> co::generator<T>::promise_type::get_return_object() {
  return generator{handle_type::from_promise(*this)};
}

template <class T>
std::experimental::suspend_always
co::generator<T>::promise_type::yield_value(T value) {
  current_value = value;
  return std::experimental::suspend_always{};
}

template <class T>
std::experimental::suspend_never
co::generator<T>::promise_type::return_void() {
  return std::experimental::suspend_never{};
}

template <class T>
std::experimental::suspend_always
co::generator<T>::promise_type::final_suspend() {
  return std::experimental::suspend_always{};
}

template <class T>
void co::generator<T>::promise_type::unhandled_exception() {
  std::exit(1);
}

template <class T>
co::generator<T>::iterator::iterator(generator &o, bool d) :
    owner(o), done(d) {
  if (not done)
    advance();
}

template <class T> void co::generator<T>::iterator::advance() {
  owner.coro.resume();
  done = owner.coro.done();
}

template <class T>
bool co::generator<T>::iterator::operator!=(const iterator &r) const {
  return done != r.done;
}

template <class T>
typename co::generator<T>::iterator &
co::generator<T>::iterator::operator++() {
  advance();
  return *this;
}

template <class T> T co::generator<T>::iterator::operator*() const {
  return owner.coro.promise().current_value;
}

template <class T>
co::generator<T>::generator(generator &&g) : coro(g.coro) {
  g.coro = nullptr;
};

template <class T> co::generator<T>::~generator() {
  if (coro)
    coro.destroy();
}

template <class T> T co::generator<T>::current() {
  return coro.promise().current_value;
}

template <class T> T co::generator<T>::next() {
  coro.resume();
  return current();
}

template <class T> bool co::generator<T>::done() { return coro.done(); }

template <class T>
typename co::generator<T>::iterator co::generator<T>::begin() {
  return iterator{*this, false};
}

template <class T>
typename co::generator<T>::iterator co::generator<T>::end() {
  return iterator{*this, true};
}

template <class T>
co::coreturn<T>::coreturn(coreturn &&s) : coro(s.coro) {
  s.coro = nullptr;
}

template <class T> co::coreturn<T>::~coreturn() {
  if (coro)
    coro.destroy();
}

template <class T> co::coreturn<T> &co::coreturn<T>::operator=(coreturn &&s) {
  coro = s.coro;
  s.coro = nullptr;
  return *this;
}

template <class T>
std::experimental::suspend_never
co::coreturn<T>::promise::return_value(T v) {
  value = v;
  return std::experimental::suspend_never{};
}

template <class T>
std::experimental::suspend_always
co::coreturn<T>::promise::final_suspend() {
  return std::experimental::suspend_always{};
}

template <class T> void co::coreturn<T>::promise::unhandled_exception() {
  std::exit(1);
}

template <class T> T co::coreturn<T>::get() {
  return coro.promise().value;
}

template <class T>
co::sync<T> co::sync<T>::promise_type::get_return_object() {
  return co::sync<T>{handle_type::from_promise(*this)};
}

template <class T>
std::experimental::suspend_never
co::sync<T>::promise_type::initial_suspend() {
  return std::experimental::suspend_never{};
}

template <class T> T co::sync<T>::get() { return coreturn<T>::get(); }

template <class T>
co::async<T> co::async<T>::promise_type::get_return_object() {
  return async<T>{handle_type::from_promise(*this)};
}

template <class T>
std::experimental::suspend_always
co::async<T>::promise_type::initial_suspend() {
  return std::experimental::suspend_always{};
}

template <class T> T co::async<T>::get() {
  if (!this->coro.done())
    this->coro.resume();

  return coreturn<T>::get();
}

template <class T> bool co::async<T>::awaitable_type::await_ready() {
  return this->coro.done();
}

template <class T>
std::experimental::coroutine_handle<>
co::async<T>::awaitable_type::await_suspend(
    std::experimental::coroutine_handle<> awaiting) {
  this->coro.resume();
  return awaiting;
}

template <class T> T co::async<T>::awaitable_type::await_resume() {
  return this->coro.promise().value;
}

template <class T>
typename co::async<T>::awaitable_type co::async<T>::operator co_await() {
  return awaitable_type{this->coro};
}
