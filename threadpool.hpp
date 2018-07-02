/**
 * @file   threadpool.hpp
 * @author Jens Munk Hansen <jens.munk.hansen@gmail.com>
 * @date   Sun Oct 29 20:38:37 2017
 *
 * @brief
 *
 * Copyright 2017 Jens Munk Hansen
 */

#pragma once

#include <sps/cenv.h>

#include <algorithm>
#include <atomic>
#include <cstdint>
#include <functional>
#include <future>
#include <memory>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

#include <sps/mimo.hpp>

// TESTING
// #include <iostream>

// std::result_of can be implemented using decltype

namespace sps {

class ThreadPool {
 private:
  template <typename T>
#ifdef SPS_USE_INTEL_TBB_QUEUE
  // If Intel's TBB is available, we can use their queue primitive
  using QueueImpl = tbb::concurrent_queue<T>;
#else
  using QueueImpl = sps::MRMWQueue<T>;
#endif

  //! Thread task interface
  /*!
    Abstract interface for thread task.
   */
  class IThreadTask {
   public:
    virtual ~IThreadTask() = default;
    IThreadTask(IThreadTask&& other) = default;
    IThreadTask& operator=(IThreadTask&& other) = default;

    /**
     * Function executed by thread
     *
     */
    virtual void Execute() = 0;

   protected:
    IThreadTask() = default;

   private:
    /**
       Inner interface - how am I implemented. This ensures that
       descendants must be move-only copy- and assignable.
     */
    IThreadTask(const IThreadTask& rhs) = delete;
    IThreadTask& operator=(const IThreadTask& rhs) = delete;
  };

  //! Thread task
  /*!
    Thread task implementation.
  */
  template <typename Func>
  class ThreadTask : public IThreadTask {
   public:
    /**
     * Construct thread task (we don't allow implicit conversion of func)
     *
     * @param func
     *
     * @return
     */
    explicit ThreadTask(Func&& func)
      : m_func{std::move(func)} {
    }

    ThreadTask(ThreadTask&& other) = default;
    ThreadTask& operator=(ThreadTask&& other) = default;
    ~ThreadTask() SPS_OVERRIDE = default;

    /**
     * Function executed by thread
     *
     */
    void Execute() SPS_OVERRIDE {
      m_func();
    }

    // Using std::packaged_task::make_ready_at_thread_exit will only
    // make the future available, when the thread has exited

   private:
    ThreadTask(const ThreadTask& rhs) = delete;
    ThreadTask& operator=(const ThreadTask& rhs) = delete;
    Func m_func;
  };

 public:
  //! Task future
  /*!
    Simple wrapper around std::future. The behavoir of futures
    returned from std::async is added. The object will block and wait
    for completion unless, detach is called.
  */
  template <typename T>
  class TaskFuture {
   public:
    explicit TaskFuture(std::future<T>&& future)
      : m_future{std::move(future)}, m_detach(false) {
    }

    TaskFuture(TaskFuture&& other) = default;
    TaskFuture& operator=(TaskFuture&& other) = default;

    ~TaskFuture() {
      if ( m_future.valid() ) {
        if ( !m_detach ) {
          // Block if not detached
          m_future.get();
        }
      }
    }

    /**
     * Get the std::future to wait for. Note, deduced return types
     * only available using c++14
     *
     * @return
     */
    auto Get() {
      return m_future.get();
    }

    /**
     * Detach future from from processing
     *
     */
    void Detach() {
      m_detach = true;
    }

   private:
    TaskFuture(const TaskFuture& rhs) = delete;
    TaskFuture& operator=(const TaskFuture& rhs) = delete;

    std::future<T> m_future;  ///< Future
    bool m_detach;            ///< Detach future
  };

 public:
  /**
   * Constructor.
   */
  ThreadPool()
    : ThreadPool{std::max<unsigned int>(std::thread::hardware_concurrency(),
                                        2u) - 1u} {
    /*
     * Always create at least one thread.  If hardware_concurrency() returns 0,
     * subtracting one would turn it to UINT_MAX, so get the maximum of
     * hardware_concurrency() and 2 before subtracting 1.
     */
  }

  /**
   * Constructor. No implicit conversion allowed
   */
  explicit ThreadPool(const std::size_t numThreads)
    : m_nThreadsOnHold{0},
      m_done{false},
      m_workQueue{},
      m_threads{} {
    try {
      for (std::size_t i = 0u ; i < numThreads ; ++i) {
        m_threads.emplace_back(&ThreadPool::worker, this);
      }
    } catch(...) {
      // Function registered may throw
      destroy();
      throw;
    }
  }

  /**
   * Destructor.
   */
  ~ThreadPool() {
    destroy();
  }

  /**
   * Submit a job to be run by the thread pool. The return true
   * should never be deduced to a reference, so auto is used rather
   * than decltype(auto)
   */
  template <typename Func, typename... Args>
  auto submit(Func&& func, Args&&... args) {
    // If called with an immediate, say 42 an rvalue,
    // Args is deduced to int and std::forward<int> is int&& (rvalue)
    // If called with an lvalue of int, Args is deduced to int&,
    // forward<int&> is int& &&, which collapses to int& (lvalue)
    // Without templates - forward only works with rvalues

    // Perfect forwarding - boundTask declared as r-value reference &&
    auto boundTask =
      std::bind(std::forward<Func>(func), std::forward<Args>(args)...);

    // TODO(JEM): Change to std::invoke_result_t
    //            std::result_of_t is deprecated as of c++17
    using ResultType =
      std::result_of_t<decltype(boundTask)()>;

    // wrapped for async invocation
    using PackagedTask = std::packaged_task<ResultType()>;
    // stored in a shared state, which can be accessed through std::future.
    using TaskType = ThreadTask<PackagedTask>;

    PackagedTask task{std::move(boundTask)};
    TaskFuture<ResultType> result{task.get_future()};
    m_workQueue.push(std::make_unique<TaskType>(std::move(task)));
    return result;
  }

 private:
  /**
   * Non-copyable.
   */
  ThreadPool(const ThreadPool& rhs) = delete;

  /**
   * Non-assignable.
   */
  ThreadPool& operator=(const ThreadPool& rhs) = delete;

  /**
   * Constantly running function each thread uses to acquire work items from the queue.
   */
  void worker() {
    while (!m_done) {
      std::unique_ptr<IThreadTask> pTask{nullptr};
      if (m_workQueue.pop(pTask)) {
        pTask->Execute();
      }
    }
  }

  /**
   * Invalidates the queue and joins all running threads.
   */
  void destroy() {
    m_done = true;
    m_workQueue.invalidate();
    for (auto& thread : m_threads) {
      if (thread.joinable()) {
        thread.join();
      }
    }
  }

 private:
  std::atomic<int> m_nThreadsOnHold;                    ///< Threads on hold
  std::atomic_bool m_done;                              ///< Are we done?
  QueueImpl<std::unique_ptr<IThreadTask>> m_workQueue;  ///< Work queue
  std::vector<std::thread> m_threads;                   ///< Threads in the pool
};

namespace thread {
namespace defaultpool {

/**
 * Get the default thread pool for the application.
 * This pool is created with std::thread::hardware_concurrency() - 1 threads.
 */
static inline ThreadPool& ThreadPoolGet() {
  // Meyer's singleton, okay if magic statics are supported
  static ThreadPool defaultPool;
  return defaultPool;
}

/**
 * Submit a job to the default thread pool.
 */
template <typename Func, typename... Args>
inline auto submitJob(Func&& func, Args&&... args) {
  return ThreadPoolGet().submit(std::forward<Func>(func),
                                std::forward<Args>(args)...);
}
}  // namespace defaultpool
}  // namespace thread
}  // namespace sps

/* Local variables: */
/* indent-tabs-mode: nil */
/* tab-width: 2 */
/* c-basic-offset: 2 */
/* End: */
