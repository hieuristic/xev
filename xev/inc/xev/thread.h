#pragma once
#include <atomic>
#include <cstdint>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <semaphore>
#include <thread>
#include <vector>

namespace xev {

struct ThreadPool {
  ThreadPool(uint32_t numThreads);
  ~ThreadPool();

  std::queue<std::function<void()>> tasks;
  std::counting_semaphore<> taskSem{0};
  std::mutex queueMutex;
  std::atomic<bool> shouldDie{false};
  std::vector<std::thread> workers;

  void run(std::function<void()> task);

  template <typename F, typename... Args>
  auto submit(F&& f, Args&&... args) {
    using return_type = std::invoke_result_t<F, Args...>;
    auto task = std::make_shared<std::packaged_task<return_type()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...));
    auto fut = task->get_future();
    this->run([task]() { (*task)(); });
    return fut;
  }
};

}  // namespace xev
