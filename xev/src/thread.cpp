#include <xev/thread.h>

namespace xev {

ThreadPool::ThreadPool(uint32_t numThreads) {
  shouldDie.store(false);
  for (uint32_t i = 0; i < numThreads; ++i) {
    workers.emplace_back([this] {
      while (true) {
        taskSem.acquire();
        if (shouldDie.load()) {
          return;
        }

        std::function<void()> task;
        {
          std::lock_guard<std::mutex> lock(queueMutex);
          if (tasks.empty()) {
            if (shouldDie.load()) {
              return;
            }
            continue;
          }
          task = std::move(tasks.front());
          tasks.pop();
        }
        if (task) {
          task();
        }
      }
    });
  }
}

ThreadPool::~ThreadPool() {
  shouldDie.store(true);
  taskSem.release(workers.size());
  for (auto& worker : workers) {
    if (worker.joinable()) {
      worker.join();
    }
  }
}

void ThreadPool::run(std::function<void()> task) {
  {
    std::lock_guard<std::mutex> lock(queueMutex);
    if (shouldDie.load()) {
      return;
    }
    tasks.push(std::move(task));
  }

  taskSem.release();
}

}  // namespace xev
