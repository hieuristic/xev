#include <xev/thread.h>

ThreadPool::ThreadPool(uint32_t numThreads) {
  for (uint32 i = 0; i < numThreads; ++i) {
    workers.emplace_back([this] {
      while (true) {
        taskSem.acquire();

        std::function<void()> task;
        {
          std::lock_guard<std::mutex> lock(queueMutex);
          if (tasks.empty()) {
            if (shouldDie.load())
              return;
            continue;
          }
          task = std::move(tasks.front());
          tasks.pop();
        }
        task();
      }
    });
  }
}

ThreadPool::~ThreadPool() {
  shouldDie.store(true);
  taskSemaphore.release(workers.size());
  for (auto& worker : workers) {
    if (worker.joinable())
      worker.join();
  }
}

void ThreadPool::run(std::function<void()> task) {
  {
    std::lock_guard<std::mutex> lock(queueMutex);
    if (shouldDie.load())
      return;
    tasks.push(std::move(task));
  }

  taskSemaphore.release();
}

template <typename F, typename... Args>
auto ThreadPool::submit(F&& f, Args&&... args) {
  using return_type = std::invoke_result_t<F, Args...>;
  auto task = std::make_shared<std::packaged_task<return_type()>>(
      std::bind(std::forward<F>(f), std::forward<Args>(args)...));
  auto fut = task->get_future();
  this->run([task]() { (*task)(); });
  return fut;
}
