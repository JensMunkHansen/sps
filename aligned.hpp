#ifndef ALIGNED_H
#define ALIGNED_H

// TODO(JMH): use std::align + placement new
// TODO(JMH): Dtors not called on elements of arrays

// TODO(JMH):
/*
  new(2,x) T[5] --> operator new[](sizeof(T)*5+y,x)

  alignas(double) char buffer[100];
  double* p = new(buffer, sizeof(buffer)) double[3];


 */

#include <malloc.h>  // malloc_usable_size
#include <mm_malloc.h>
#include <stdio.h>

#include <cstring>
#include <algorithm>
#include <memory>

#include <iostream>
#include <new>

// The "make_unique" parts are copied from GCC (/usr/include/c++/4.9/bits), and
// were adapted for alignment.

namespace aligned {

/// For internal use only!
namespace details {

/// Default alignment is set to 64 Byte, i.e., the most common cache-line size.
/// This alignment is sufficient at the least for AVX-512.
constexpr std::size_t default_alignment = 64;

/// Deleter for single object in aligned memory, used by aligned::unique_ptr
template <class T> struct Deleter {
  void operator()(T *data) const {
    // Single object, was created by placement-new => destruct explicitly
    data->~T();
    // Data allocated by "posix_memalign", so we must "free" it.
    free(data);
  }
};

/// Specialization of Deleter for array of objects, used by aligned::unique_ptr
template <class T> struct Deleter<T[]> {
  void operator()(T *data) const {
    // Data allocated by "posix_memalign", so we must "free" it.

    // NOT GOOD - WE NEED TO KEEP THE COUNT
    T::operator delete[](data, 9);

    free(data);
  }
};

/// Allocation function for aligned memory, used by aligned::make_unique
template <typename T, std::size_t alignment>
inline typename std::remove_extent<T>::type *alloc(std::size_t num) {
  // Ensure minimum alignment for given type
  std::size_t align = std::max(std::alignment_of<T>::value, alignment); // alignof(T)
  // If T is an array type, we remove the "[]"
  using TYPE = typename std::remove_extent<T>::type;
  void *mem = nullptr;
#if 0
  mem = _mm_malloc(sizeof(TYPE) * num, align);
#else
  int error = posix_memalign(&mem, align, sizeof(TYPE) * num);

  memset(mem,0,sizeof(TYPE) * num);

  if (error == EINVAL)
    throw std::logic_error("Error: Alignment must be a power of two "
                           "(posix_memalign returned EINVAL)");
  else if (error != 0)
    throw std::bad_alloc();
#endif
  return static_cast<TYPE*>(mem);
}

}  // namespace details

/// Typedef providing aligned::unique_ptr
template <class T> using unique_ptr = std::unique_ptr<T, details::Deleter<T>>;

/// For internal use only!
namespace details {

template <typename T> struct MakeUniq { typedef unique_ptr<T> single_object; };

template <typename T> struct MakeUniq<T[]> { typedef unique_ptr<T[]> array; };

template <typename T, std::size_t Bound> struct MakeUniq<T[Bound]> {
  struct invalid_type {};
};

}  // namespace details

/// aligned::make_unique for single objects
template <typename T,
          std::size_t alignment = details::default_alignment,
          typename... Args>
inline typename details::MakeUniq<T>::single_object
make_unique(Args &&... args) {
  // Placement-new into aligned memory
  // We use constructor with "{}" to prevent narrowing
  return unique_ptr<T>(new (details::alloc<T, alignment>(1))
                           T{std::forward<Args>(args)...});
}

// TODO(JMH): new () T[nums];

/// aligned::make_unique for arrays of unknown bound
template <typename T,
          std::size_t alignment = details::default_alignment>
inline typename details::MakeUniq<T>::array make_unique(std::size_t num) {
  // We are not using "new", which would prevent allocation of
  // non-default-constructible types, so we need to verify explicitly
  static_assert(std::is_default_constructible<
                    typename std::remove_extent<T>::type>::value,
                "Error: aligned::make_unique<T[]> supports only "
                "default-constructible types");

#if 0
  static_assert(std::is_pod<
                    typename std::remove_extent<T>::type>::value,
                "Error: aligned::make_unique<T[]> supports only "
                "pod types");
#endif
  //T* pArray = new (details::alloc<T, alignment>(num)) T[num];
  //return unique_ptr<T>(pArray);


  return unique_ptr<T>(details::alloc<T, alignment>(num));
}

/// Disable aligned::make_unique for arrays of known bound
template <typename T, typename... Args>
inline typename details::MakeUniq<T>::invalid_type
make_unique(Args &&...) = delete;

} // namespace aligned

#endif // ALIGNED_H
