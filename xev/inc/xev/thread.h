#pragma once

namespace xev {

struct ThreadPool {
  ThreadPool(uint32_t numThreads);
  ~ThreadPool();

  std::vector<std::thread> workers;
  std::queue<std::function<void()>> tasks;
  std::counting_semaphore<> taskSemaphore{0};
  std::mutex queueMutex;
  std::atomic<bool> shouldDie;

  void run(std::function<void()> task);
};

}  // namespace xev
