#pragma once

template <class T, size_t Alignment = 64>
struct aligned_ptr
{

  static constexpr size_t alignment = (Alignment < alignof(T)) ? alignof(T) : Alignment;
  static constexpr size_t buffer_size = alignment + sizeof(T) - 1;

  template <class... Args>
  aligned_ptr(Args&&... args)
    : _memory{ new uint8_t[buffer_size] }
    , _object{ make_object_pointer(_memory.get(), std::forward<Args>(args)...), &deleter }
  {
  }

  T* get() const noexcept { return reinterpret_cast<T*>(_object.get()); }

private:
  static void deleter(T* p) noexcept(noexcept(std::declval<T>().~T()))
  {
    p->~T();
    // note: not freed
  }

  template <class... Args>
  T* make_object_pointer(uint8_t* buffer, Args&&... args)
  {
    auto addr_v = reinterpret_cast<void*>(buffer);
    auto space = buffer_size;
    // nBytes = sizeof(T)
    // addr_v is input (is modified)
    // space is decreased by amount needed for alignment
    std::align(alignment, sizeof(T), addr_v, space);
    auto address = reinterpret_cast<T*>(addr_v);
    new (address) T(std::forward<Args>(args)...);
    return address;
  }

private:
  // NOTE: order matters. memory before object
  std::unique_ptr<uint8_t[]> _memory;
  std::unique_ptr<T, void (*)(T*)> _object;
};
