#include <functional>
#include <iostream>
#include <mutex>
#include <stdexcept>
#include <type_traits>

#include "fnx/utils/thread_pool.hpp"

namespace FNX::Utils {

ThreadPool::TaskEntry::TaskEntry(int priority, TaskType task) :
    priority(priority), task(std::move(task)) {}

ThreadPool::ThreadPool(size_t threads) {
  _threads.reserve(threads);

  for (size_t i = 0; i < threads; i++) {
    _threads.push_back(std::thread([this] {
      while (true) {
        TaskType task;

        {
          // Lock to obtain a new task.
          std::unique_lock lock(this->_mutex);

          this->_condvar.wait(lock, [this] {
            // Do not wait if the pool has stopped.
            return this->_stopped || !this->_tasks.empty();
          });

          // The pool could've been stopped during the wait.
          if (this->_stopped || this->_tasks.empty())
            return;

          // Pop the first task from the queue.
          task = std::move(this->_tasks.top().task);
          this->_tasks.pop();
        }

        if (!task.valid())
          throw std::runtime_error("Invalid task");

        // Execute the task.
        task();
      }
    }));
  }
}

ThreadPool::~ThreadPool() {
  {
    std::unique_lock lock(_mutex);
    _stopped = true;
  }

  _condvar.notify_all();

  for (auto &thread : _threads)
    thread.join();
}

} // namespace FNX::Utils
