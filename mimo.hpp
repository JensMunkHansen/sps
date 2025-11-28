/**
 * @file   mimo.hpp
 * @author Jens Munk Hansen <jens.munk.hansen@gmail.com>
 * @date   Thu Oct 19 01:34:56 2017
 *
 * @brief  Multi-reader-multi-writer (MRMW) thread-safe queue
 *         and circular buffers.
 *
 * Copyright 2017 Jens Munk Hansen
 *
 */
/*
 *  This file is part of SOFUS.
 *
 *  SOFUS is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  SOFUS is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with SOFUS.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  TODO: make version using std::shared_lock - one writer, multiple readers
 */

#pragma once

// Suppress warning about overloaded virtual functions being hidden.
// The interface classes use intentional design with different push() overloads.
#ifdef __GNUC__
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Woverloaded-virtual"
# pragma GCC diagnostic ignored "-Wsuggest-override"
#endif

#include <sps/cenv.h>
#include <sps/sps_export.h>

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <utility>
#include <queue>
#include <memory>

// TODO(JMH): Create instead an interface for a threadpool to be used
// with a fixed signature. Crazy difficult to instantiate all well-formed functions needed

/*
  An explicit instantiation that names a class template specialization
  is also an explicit instantiation of the same kind (declaration or
  definition) of each of its members (not including members inherited
  from base classes) that has not been previously explicitly
  specialized in the translation unit containing the explicit
  instantiation, except as described below.

  Since some of the members of std::deque<foo> require a copyable type
  foo - at the very least, the copy constructor - instantiating them
  is ill-formed. This is the cause of the errors you observe.

  The workaround for this is to explicitly instantiate only the
  well-formed members that your program uses, something like:

  typedef void (*DummyCallback)(void*);
  template std::deque<DummyCallback>::deque();
  template class SPS_EXPORT std::queue<DummyCallback, std::deque<DummyCallback> >;

*/

namespace sps {

#ifdef CXX11
using std::is_copy_constructible;
#else
# include <sps/c98.hpp>
using sps::is_copy_constructible;
#endif

/*! \brief Multi-Reader-Multi-Writer Queue interface
 *
 * @tparam T type of objects administered
 *
 * An interface for a thread-safe multi-reader-multi-writer
 * queue. It supports administration of object, which are
 * move-only assignable.
 *
 * There is no need for the variable \p m_valid to be atomic
 *
 */
template <typename T, bool = is_copy_constructible<T>::value>
class IMRMWQueue {
 public:
  /**
   * Destructor.
   *
   */
  virtual ~IMRMWQueue() = default;

  /**
   * All descendants are move-constructible
   *
   * @param other
   */
  IMRMWQueue(IMRMWQueue&& other) = default;

  /**
   * All descendants are move-assignable
   *
   * @param other
   *
   * @return
   */
  IMRMWQueue& operator=(IMRMWQueue&& other) = default;

  /**
   * Pop first element in the queue.
   *
   * @param destination
   *
   * @return True if a value is written to destination, false otherwise
   */
  virtual bool try_pop(T& destination) = 0;

  /**
   * Pop first element in the queue.
   *
   * @param destination
   *
   * @return True if a value is written to destination, false otherwise
   */
  virtual bool pop(T& destination) = 0;

#ifdef CXX11
  /**
   * Push element onto queue.
   *
   * @param source
   */
  virtual bool push(T&& source) = 0;
#else
  /**
   * Push element onto queue.
   *
   * @param source
   */
  virtual bool push(T source) = 0;
#endif

  /**
   * Invalidate the queue
   *
   */
  virtual void invalidate() = 0;

  /**
   * Is the queue valid
   *
   * @return
   */
  virtual bool valid() const = 0;
  /**
   * Clear the queue. All readers are notified
   *
   */
  virtual void clear() = 0;
  /**
   * Is the queue empty
   *
   * @return
   */
  virtual bool empty() const = 0;

 protected:
  IMRMWQueue() = default;

 private:
  /**
     Inner interface. Prevent descendants from implementing
     copy ctor or assignment
   */
  IMRMWQueue(const IMRMWQueue&) = delete;
  IMRMWQueue& operator=(const IMRMWQueue&) = delete;
};

template <typename T>
class IMRMWQueue<T, true> : IMRMWQueue<T, false> {
 public:
  virtual bool push(const T& souce) = 0;
};


template <typename T, bool = is_copy_constructible<T>::value >
class MRMWQueue : public IMRMWQueue<T, is_copy_constructible<T>::value> {
 public:
  /**
   * Ctor
   *
   *
   * @return
   */
  MRMWQueue() = default;

  /**
   * Destructor. Invalidate and empty queue
   *
   */
  ~MRMWQueue() override {
    invalidate();
    clear();
  }

  /**
   * Pop first element in the queue.
   *
   * @param destination
   *
   * @return True if a value is written to destination, false otherwise
   */
  bool try_pop(T& destination) SPS_OVERRIDE {
    std::lock_guard<std::mutex> guard(m_mutex);
    if (m_queue.empty()) {
      return false;
    } else {
      destination = std::move(m_queue.front());
      m_queue.pop();
      return true;
    }
  }

  /**
   * Pop first element in queue. Will block until a value is
   * available unless clear is called or the instance is destructed.
   *
   * Note that a continouous check is made for invalidity or !empty
   *
   * @param destination
   *
   * @return True if a value is written to destination, false
   * otherwise
   */
  bool pop(T& destination) SPS_OVERRIDE {
    std::unique_lock<std::mutex> lock{m_mutex}; // m_mutex.lock()
    // Equivalent to:
    // while ( !(!m_queue.empty() || !m_valid) ) {m_condition.wait(lock);}
    m_condition.wait(lock, [this]() {
      return !m_queue.empty() || !m_valid;
    });
    // Lock is reacquired

    if (!m_valid) {
      return false;
    }
    destination = std::move(m_queue.front());
    m_queue.pop();
    return true;
  }

#ifdef CXX11
  /**
   * Push element onto queue
   *
   * @param source
   */
  bool push(T&& source) SPS_OVERRIDE {
    std::lock_guard<std::mutex> guard(m_mutex);
    m_queue.push(std::move(source));
    m_condition.notify_one();
    return true;
  }
#else
  bool push(T source) SPS_OVERRIDE {
    // Scoped lock
    std::lock_guard<std::mutex> guard{m_mutex};
    m_queue.push(std::move(source));
    m_condition.notify_one();
    return true;
  }
#endif

  /**
   * Invalidate the queue. This is used to ensure no conditions are
   * being waited on in @ref try_pop when a thread or the
   * application is trying to exit.  The queue is invalid after
   * calling this method and it is an error to continue using a
   * queue after this method has been called.
   *
   */
  void invalidate() SPS_OVERRIDE {
    std::lock_guard<std::mutex> guard{m_mutex};
    m_valid = false;
    m_condition.notify_all();
  }

  /**
   * Is the queue valid
   *
   *
   * @return
   */
  bool valid() const SPS_OVERRIDE {
    std::lock_guard<std::mutex> guard{m_mutex};
    return m_valid;
  }

  /**
   * Clear the queue. All readers are notified
   *
   */
  void clear() SPS_OVERRIDE {
    std::lock_guard<std::mutex> guard{m_mutex};
    while (!m_queue.empty()) {
      m_queue.pop();
    }
    m_condition.notify_all();
  }

  /**
   * Is the queue empty
   *
   *
   * @return
   */
  bool empty() const SPS_OVERRIDE {
    // Scoped lock
    std::lock_guard<std::mutex> guard{m_mutex};
    return m_queue.empty();
  }

 protected:
  std::queue<T> m_queue;                ///< Queue with callables
  mutable std::mutex m_mutex;           ///< Mutex for locking

  std::atomic<bool> m_valid{true};      ///< State for invalidation
  std::condition_variable m_condition;  ///< Condition for signal not empty
};

template <typename T>
class MRMWQueue <T, true> : public MRMWQueue<T, false> {
 public:
  /**
   * Push element onto queue
   *
   * @param source
   */
  bool push(const T& source) {
    std::lock_guard<std::mutex> guard(this->m_mutex);
    this->m_queue.push(source);
    this->m_condition.notify_one();
    return true;
  }
};

// Specify whether or not overwrite is allowed

// Optimize length to be multipla of page-size (aligned).. Virtual memory

/*! \brief Multi-Reader-Multi-Writer Circular Buffer interface
 *
 * @tparam T type of objects administered
 *
 * An interface for a thread-safe multi-reader-multi-writer
 * ring buffer. It supports administration of object, which are
 * move-only assignable.
 *
 * There is no need for the atomic to be atomic
 *
 * Implements FIFO with an upper bound. If the maximum is exceeded,
 * the oldest argument is discarded and replaced by new one...
 *
 */
template <typename T, size_t Size, bool overwrite = false,
          bool = is_copy_constructible<T>::value>
class IMRMWCircularBuffer {
 public:
  /**
   * Destructor.
   *
   */
  virtual ~IMRMWCircularBuffer() = default;

  /**
   * All descendants are move-constructible
   *
   * @param other
   */
  IMRMWCircularBuffer(IMRMWCircularBuffer&& other) = default;

  /**
   * All descendants are move-assignable
   *
   * @param other
   *
   * @return
   */
  IMRMWCircularBuffer& operator=(IMRMWCircularBuffer&& other) = default;

  /**
   * Pop first element in the ring buffer.
   *
   * @param destination
   *
   * @return True if a value is written to destination, false otherwise
   */
  virtual bool pop(T& destination) = 0;

  virtual bool try_pop(T& destination) = 0;

#ifdef CXX11
  /**
   * Push element onto ring buffer.
   *
   * @param source
   */
  virtual bool push(T&& source) = 0;
#else
  /**
   * Push element onto ring buffer.
   *
   * @param source
   */
  virtual bool push(T source) = 0;
#endif

  /**
   * Invalidate
   *
   */
  virtual void invalidate() = 0;

  /**
   * Is the ring buffer valid
   *
   * @return
   */
  virtual bool valid() const = 0;

  /**
   * Is the ring buffer empty
   *
   * @return
   */
  virtual bool empty() const = 0;

 protected:
  IMRMWCircularBuffer() = default;

 private:
  /**
     Inner interface. Prevent descendants from implementing
     copy ctor or assignment
   */
  IMRMWCircularBuffer(const IMRMWCircularBuffer&) = delete;
  IMRMWCircularBuffer& operator=(const IMRMWCircularBuffer&) = delete;
};

template <typename T, size_t Size, bool overwrite>
class IMRMWCircularBuffer<T, Size, overwrite, true> :
  IMRMWCircularBuffer<T, Size, overwrite, false> {
 public:
  virtual bool push(const T& souce) = 0;
};


template <typename T, size_t Size,
          bool overwrite = false, bool = is_copy_constructible<T>::value>
class MRMWCircularBuffer :
  public IMRMWCircularBuffer<T, Size, overwrite,
  is_copy_constructible<T>::value> {
 protected:
  typedef std::queue<T> container_type;
  typedef typename container_type::size_type size_type;
  typedef typename container_type::value_type value_type;

  size_type m_unread;
  size_type m_capacity;
  container_type m_container;
  mutable std::mutex m_mutex;
  std::condition_variable m_cond_not_empty;
  std::condition_variable m_cond_not_full;
  std::atomic<bool> m_valid{true};           ///< State for invalidation

  size_type capacity() const {
    return m_capacity;
  }

  bool is_not_empty() const {
    return m_unread > 0;
  }

  bool is_not_full() const {
    return m_unread < m_capacity;
  }

 public:
  /**
   * Ctor
   *
   *
   * @return
   */
  MRMWCircularBuffer() : m_unread(0), m_capacity(Size) {}

#ifdef CXX11
  /**
   * Push element onto ring buffer.
   *
   * @param source
   */
  bool push(value_type&& source) {
    std::unique_lock<std::mutex> lock{m_mutex};
    m_cond_not_full.wait(lock, [&]() {
      return this->is_not_full() || !m_valid;
    });

    if (!m_valid) {
      return false;
    }

    m_container.push(std::move(source));
    m_unread++;
    m_cond_not_empty.notify_one();
    return true;
  }
#else
  /**
   * Push element onto ring buffer.
   *
   * @param source
   */
  bool push(T source) {
    std::unique_lock<std::mutex> lock{m_mutex};
    m_cond_not_full.wait(lock, [&]() {
      return this->is_not_full() || !m_valid;
    });

    if (!m_valid) {
      return false;
    }
    m_container.push(source);
    m_unread++;
    m_cond_not_empty.notify_one();
    return true;
  }
#endif

  /**
   * Pop element from ring buffer
   *
   * @param destination
   *
   * @return
   */
  bool pop(T& destination) SPS_OVERRIDE {
    std::unique_lock<std::mutex> lock{m_mutex};

    m_cond_not_empty.wait(lock, [this]() {
      // Lock is acquired for evaluating predicate
      return this->is_not_empty() || !m_valid;
    });
    // Lock is now reacquired
    if (!m_valid) {
      return false;
    }
    destination = std::move(m_container.front());
    m_container.pop();
    m_unread--;
    m_cond_not_full.notify_one();
    return true;
  }

  bool try_pop(T& destination) SPS_OVERRIDE {
    std::lock_guard<std::mutex> guard(m_mutex);
    // Lock is now reacquired
    if (!m_valid) {
      return false;
    } else if (this->is_not_empty()) {
      destination = std::move(m_container.front());
      m_container.pop();
      m_unread--;
      m_cond_not_full.notify_one();
      return true;
    } else {
      return false;
    }
  }



  /**
   * Destructor. Invalidate and empty queue
   *
   */
  ~MRMWCircularBuffer() {
    invalidate();
  }

  void invalidate() {
    std::lock_guard<std::mutex> guard{m_mutex};
    m_valid.store(false);
    m_cond_not_empty.notify_all();
    m_cond_not_full.notify_all();
  }


  /**
   * Is the ring buffer empty
   *
   * @return
   */
  bool empty() const SPS_OVERRIDE {
    std::lock_guard<std::mutex> guard{m_mutex};
    return m_unread == 0;
  }

  /**
   * Is the buffer valid
   *
   * @return
   */
  bool valid() const SPS_OVERRIDE {
    return m_valid;
  }
};

template <typename T, size_t Size, bool overwrite>
class MRMWCircularBuffer <T, Size, overwrite, true> : public MRMWCircularBuffer<T, Size, overwrite, false> {
 public:
  /**
   * Push element onto queue
   *
   * @param source
   */
  bool push(const T& source) {
    std::unique_lock<std::mutex> lock{this->m_mutex};
    this->m_cond_not_full.wait(lock, [&]() {
      return this->is_not_full() || !this->m_valid;
    });

    if (!this->m_valid) {
      return false;
    }
    this->m_container.push(source);
    this->m_unread++;
    this->m_cond_not_empty.notify_one();
    return true;
  }
};



}  // namespace sps

#ifdef __GNUC__
# pragma GCC diagnostic pop
#endif

/* Local variables: */
/* indent-tabs-mode: nil */
/* tab-width: 2 */
/* c-basic-offset: 2 */
/* End: */
