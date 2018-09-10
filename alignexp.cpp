#include <malloc.h>
#include <mm_malloc.h>
#include <new>
#include <memory>
#include <iostream>

constexpr int nElements = 3;

struct A {
  int k;
  A() : k(0xFFFFFFFF) {
    std::cout << "c" << std::endl;
  }
  ~A() {
    std::cout << "d" << std::endl;
  }
  void operator delete[](void* p, size_t count) {
    for (size_t i = 0 ; i < count ; i++) {
      ((A*)p)->~A();
      p++;
    }
  }

};

template <typename T>
using unique_aligned_ptr = std::unique_ptr<T[], std::function<void(T*)> >;

template<typename T, std::size_t Alignment = 4*sizeof(T)>
unique_aligned_ptr<T> create(size_t n) {
  return unique_aligned_ptr<T>(
      static_cast<T*>(_mm_malloc(n*sizeof(T), Alignment)),
      [](T* f)->void { _mm_free(f);});
}


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

    uint32_t* q = reinterpret_cast<uint32_t*>(data);
    std::cout << "useable: " << malloc_usable_size(data) << std::endl;
    std::cout << "data: " << nElements*sizeof(T) << std::endl;

    q--;
    q--;
    for (size_t i = 0 ; i < 16 ; i++) {
      std::cout << (*q >> 2*i) << std::endl;
    }

    printf("Free address: 0x%x\n", (uintptr_t) data);

    // NOT GOOD - WE NEED TO KEEP THE COUNT
    T::operator delete[](data, nElements);

    _mm_free(data);
  }
};


template<typename T, std::size_t Alignment = 4*sizeof(T)>
unique_aligned_ptr<T> create0(size_t n) {
  // Use portable alignment + store size in memory
  T* pBuffer = (T*) _mm_malloc(n*sizeof(T), Alignment);

  T* pFirst = new(pBuffer) T[n];
  printf("0x%x\n", (uintptr_t) pFirst);
  return unique_aligned_ptr<T>(
      pFirst,
      Deleter<T[]>()); // [](T* f)->void { _mm_free(f);});
}

int main() {
  alignas(double) char buffer[100];
  double* p = new(buffer) double[3];

  auto arr = create<int>(10);

  auto arr0 = create0<A>(nElements);





  return 0;
}
