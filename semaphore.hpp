/**
 * @file   semaphore.hpp
 * @author  <jens.munk.hansen@gmail.com>
 * @date   Thu Nov 23 13:55:42 2017
 *
 * @brief  Old-style counting and binary semaphores using C++11
 *
 * Copyright 2016 Jens Munk Hansen
 */

#pragma once
#include <sps/cenv.h>

#include <cstdint>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <chrono>

// TODO(JEM): Add reset to interrupt waiting

namespace sps {

class ISemaphore {
 public:
  virtual ~ISemaphore() = default;
  ISemaphore(ISemaphore&& other) = default;
  ISemaphore& operator=(ISemaphore&& other) = default;

  /**
   * Decrements (locks) the semaphore.  If the semaphore's value is
   * greater than zero, then the decrement proceeds, and the function
   * returns, immediately.  If the semaphore currently has the value
   * zero, then the call blocks until it becomes possible to perform
   * the decrement (i.e., the semaphore value rises above zero) or the
   * object is deleted in which case the return value is false.
   *
   * @return true if success, false otherwise
   */
  virtual inline bool Wait() = 0;

  /**
   * Signal the semaphore, increasing the value by one.
   *
   */
  virtual inline void Signal() = 0;

 protected:
  ISemaphore() = default;

 private:
  // Prevent copying
  ISemaphore(const ISemaphore& rhs) = delete;
  ISemaphore& operator=(const ISemaphore& rhs) = delete;
};

class IEvent {
 public:
  virtual ~IEvent() = default;
  IEvent(IEvent&& other) = default;
  IEvent& operator=(IEvent&& other) = default;

  virtual inline bool Wait() const = 0;
  virtual inline bool Signal() = 0;
  virtual inline bool Reset() = 0;
  virtual inline bool IsSet() const = 0;

 protected:
  IEvent() = default;
 private:
  // Prevent copying
  IEvent(const IEvent& rhs) = delete;
  IEvent& operator=(const IEvent& rhs) = delete;
};

class Semaphore : public ISemaphore {
 private:
  std::atomic<int32_t> m_nCount;
  std::mutex m_mutex;
  std::condition_variable m_condition;

 public:
  /**
   * Constructor.
   *
   * @param nCount number of resources available
   *
   * @return
   */
  explicit inline Semaphore(unsigned int nCount)
    : m_nCount(nCount) { }

  inline bool Wait() SPS_OVERRIDE {
    std::unique_lock<std::mutex> lock(m_mutex);
    m_condition.wait(lock, [&]()->bool{ return m_nCount > 0;});
    --m_nCount;
    // If object is deleted, m_nCount is negative and reside on stack
    return !(m_nCount < 0);
  }

  template<typename R, typename P>
  bool Wait(const std::chrono::duration<R, P>& duration) {
    std::unique_lock<std::mutex> lock(m_mutex);
    if (!m_condition.wait_for(lock, duration,
                              [&]()->bool{return m_nCount > 0;})) {
      return false;
    }
    --m_nCount;
    return !(m_nCount < 0);
  }

  inline void Signal() SPS_OVERRIDE {
    // Scoped lock
    std::lock_guard<std::mutex> lock(m_mutex);
    ++m_nCount;
    // Signal one thread
    m_condition.notify_one();
  }

  ~Semaphore() SPS_OVERRIDE {
    std::lock_guard<std::mutex> guard{m_mutex};
    m_nCount = -1;
    // Signal all threads
    m_condition.notify_all();
  }
};

class Event : public IEvent {
 private:
  bool m_bSignaled;
  bool m_bValid;
  mutable std::mutex m_mutex;
  mutable std::condition_variable m_condition;

 public:
  inline Event() : m_bSignaled(false), m_bValid(true) { }

  Event(Event&& other) = default;
  Event& operator=(Event&& other) = default;
  ~Event() SPS_OVERRIDE {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_bValid = false;
  }

  /**
   * Wait()
   *
   */
  inline bool Wait() const SPS_OVERRIDE {
    std::unique_lock<std::mutex> lock(m_mutex);
    m_condition.wait(lock, [&]()->bool{ return m_bSignaled  || !m_bValid;});
    return m_bSignaled && m_bValid;
  }

  template<typename R, typename P>
  bool Wait(const std::chrono::duration<R, P>& duration) const {
    std::unique_lock<std::mutex> lock(m_mutex);
    if (!m_condition.wait_for(lock, duration,
                              [&]()->bool{ return m_bSignaled; })) {
      return false;
    }
    return m_bValid;
  }

  inline bool Signal() SPS_OVERRIDE {
    bool bWasSignalled;
    m_mutex.lock();
    bWasSignalled = m_bSignaled;
    m_bSignaled = true;
    m_mutex.unlock();
    m_condition.notify_all();
    return bWasSignalled == false;
  }

  inline bool Reset() SPS_OVERRIDE {
    bool bWasSignalled;
    m_mutex.lock();
    bWasSignalled = m_bSignaled;
    m_bSignaled = false;
    m_mutex.unlock();
    return bWasSignalled;
  }

  inline bool IsSet() const SPS_OVERRIDE {
    return m_bSignaled;
  }
};
}  // namespace sps

/* Local variables: */
/* indent-tabs-mode: nil */
/* tab-width: 2 */
/* c-basic-offset: 2 */
/* End: */
