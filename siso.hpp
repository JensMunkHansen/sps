/**
 * @file   siso.hpp
 * @author Jens Munk Hansen <jmh@debian9laptop.parknet.dk>
 * @date   Sun Apr  8 13:35:29 2018
 *
 * @brief
 *
 *
 */

#include <sps/cenv.h>
#include <cstddef>  // std::size_t
#include <cstdint>  // std::int32_t
#include <type_traits>  // std::is_copy_constructible
#include <atomic>  // std::atomic
#include <utility>  // std::move

namespace sps {
template <typename T, size_t Size, bool = std::is_copy_constructible<T>::value>
class ISRSWRingBuffer {
 public:
  /**
   * Destructor.
   *
   */
  virtual ~ISRSWRingBuffer() = default;

  /**
   * All descendants are move-constructible
   *
   * @param other
   */
  ISRSWRingBuffer(ISRSWRingBuffer&& other) = default;

  /**
   * All descendants are move-assignable
   *
   * @param other
   *
   * @return
   */
  ISRSWRingBuffer& operator=(ISRSWRingBuffer&& other) = default;

  /**
   * Pop first element in the ring buffer.
   *
   * @param destination
   *
   * @return True if a value is written to destination, false otherwise
   */
  virtual bool try_pop(T& destination) = 0;

  /**
   * Pop first element in the ring buffer.
   *
   * @param destination
   *
   * @return True if a value is written to destination, false otherwise
   */
  virtual T pop() = 0;

#ifdef CXX11
  /**
   * Push element onto ring buffer.
   *
   * @param source
   */
  virtual bool try_push(T&& source) = 0;

  virtual void push(T&& source) = 0;
#else
  /**
   * Push element onto ring buffer.
   *
   * @param source
   */
  virtual bool try_push(T source) = 0;

  virtual void push(T source) = 0;
#endif

  //virtual void invalidate() = 0;
  /**
   * Is the ring buffer valid
   *
   * @return
   */
  //  virtual bool valid() const = 0;
  /**
   * Clear the ring buffer. All readers are notified
   *
   */
  //  virtual void clear() = 0;
  /**
   * Is the ring buffer empty
   *
   * @return
   */
  //  virtual bool empty() const = 0;

 protected:
  ISRSWRingBuffer() = default;

 private:
  /**
     Inner interface. Prevent descendants from implementing
     copy ctor or assignment
   */
  ISRSWRingBuffer(const ISRSWRingBuffer&) = delete;
  ISRSWRingBuffer& operator=(const ISRSWRingBuffer&) = delete;
};

template <typename T, size_t Size>
class ISRSWRingBuffer<T, Size, true> : ISRSWRingBuffer<T, Size, false> {
 public:
  virtual bool try_push(const T& souce) = 0;
  virtual void push(const T& souce) = 0;
};




template <typename T, size_t Size, bool = std::is_copy_constructible<T>::value >
class SRSWRingBuffer :
  public ISRSWRingBuffer<T, Size, std::is_copy_constructible<T>::value> {
 protected:
  std::atomic<std::uint32_t> m_iWrite;
  std::atomic<std::uint32_t> m_iRead;
  T m_Buffer[Size];

  std::int32_t increment(int n) {
    return (n + 1) % Size;
  }

 public:
  /**
   * Ctor
   *
   *
   * @return
   */
  SRSWRingBuffer() {
    m_iWrite.store(0);
    m_iRead.store(0);
  }

  size_t size() const {
    return Size;
  }
#ifdef CXX11
  /**
   * Push element onto queue (screws up if copy-constructible)
   *
   * @param source
   */
  bool try_push(T&& source) SPS_OVERRIDE {
    const auto current_tail = m_iWrite.load();
    const auto next_tail = increment(current_tail);
    if (next_tail != m_iRead.load()) {
      m_Buffer[current_tail] = std::move(source);
      m_iWrite.store(next_tail);
      return true;
    }
    return false;
  }

  void push(T&& source) SPS_OVERRIDE {
    while (!try_push(std::move(source))) {
    }
  }
#else
  bool try_push(T source) SPS_OVERRIDE {
    const auto current_tail = m_iWrite.load();
    const auto next_tail = increment(current_tail);
    if (next_tail != m_iRead.load()) {
      m_Buffer[current_tail] = std::move(source);
      m_iWrite.store(next_tail);
      return true;
    }
    return false;
  }

  void push(T source) SPS_OVERRIDE {
    while (!try_push(source)) {}
  }
#endif

  bool try_pop(T& destination) {
    auto currentHead = m_iRead.load();
    if (currentHead == m_iWrite.load()) {
      return false;
    }
    destination = std::move(m_Buffer[currentHead]);
    m_iRead.store(increment(currentHead));
    return true;
  }

  T pop() {
    T ret;
    while (!try_pop(ret)) {}
    return ret;
  }
};

template <typename T, size_t Size>
class SRSWRingBuffer<T, Size, true> : public SRSWRingBuffer<T, Size, false> {
 public:
  /**
   * Push element onto queue
   *
   * @param source
   */
  bool try_push(const T& source) {
    const auto current_tail = this->m_iWrite.load();
    const auto next_tail = this->increment(current_tail);
    if (next_tail != this->m_iRead.load()) {
      this->m_Buffer[current_tail] = source;
      this->m_iWrite.store(next_tail);
      return true;
    }
    return false;
  }

  void push(const T& source) {
    while (!try_push(source)) {}
  }
};
}  // namespace sps

