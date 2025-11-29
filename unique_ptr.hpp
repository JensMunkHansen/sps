namespace ThorsAnvil
{
template <typename T>
class UP
{
  T* data;

public:
  UP()
    : data(nullptr)
  {
  }
  // Explicit constructor
  explicit UP(T* data)
    : data(data)
  {
  }
  ~UP() { delete data; }

  // Constructor/Assignment that binds to nullptr
  // This makes usage with nullptr cleaner
  UP(std::nullptr_t)
    : data(nullptr)
  {
  }
  UP& operator=(std::nullptr_t)
  {
    reset();
    return *this;
  }

  // Constructor/Assignment that allows move semantics
  UP(UP&& moving) noexcept { moving.swap(*this); }
  UP& operator=(UP&& moving) noexcept
  {
    moving.swap(*this);
    return *this;
  }

  // Constructor/Assignment for use with types derived from T
  template <typename U>
  UP(UP<U>&& moving)
  {
    UP<T> tmp(moving.release());
    tmp.swap(*this);
  }
  template <typename U>
  UP& operator=(UP<U>&& moving)
  {
    UP<T> tmp(moving.release());
    tmp.swap(*this);
    return *this;
  }

  // Remove compiler generated copy semantics.
  UP(UP const&) = delete;
  UP& operator=(UP const&) = delete;

  // Const correct access owned object
  T* operator->() const { return data; }
  T& operator*() const { return *data; }

  // Access to smart pointer state
  T* get() const { return data; }
  explicit operator bool() const { return data; }

  // Modify object state
  T* release() noexcept
  {
    T* result = nullptr;
    std::swap(result, data);
    return result;
  }
  void swap(UP& src) noexcept { std::swap(data, src.data); }
  void reset()
  {
    T* tmp = releae();
    delete tmp;
  }
};
template <typename T>
void swap(UP<T>& lhs, UP<T>& rhs)
{
  lhs.swap(rhs);
}
}
