#include <cstdint>
#include <cstdlib>
#include <sps/cenv.h>
#include <sps/crtp_helper.hpp>
#include <sps/debug.h>

#include <new>

//#include <malloc.h>
#include <sps/mm_malloc.h>
//#include <features.h>

namespace sps
{
template <typename T>
class DynAllocators : public sps::CRTP<T, DynAllocators>
{
public:
  DynAllocators() { debug_print("\n"); }
  ~DynAllocators() { debug_print("\n"); }
  void* operator new(std::size_t count) throw()
  {
    debug_print("\n");
    return ::operator new(count); // May throw std::bad_alloc, call T::T()
  }
  void operator delete(void* p)
  {
    debug_print("\n");
    T* pObject = reinterpret_cast<T*>(p);
    ::operator delete(pObject); // Calls destructor, T::~T(), free
  }

  void* operator new[](std::size_t count)
  {
    debug_print("\n");
    return ::operator new[](count);
  }
  void operator delete[](void* p, size_t count)
  {
    SPS_UNREFERENCED_PARAMETER(count);
    debug_print("\n");
    return ::operator delete[](p); // Calls many dtors, free
  }

  // Placement
  static void* operator new(std::size_t /*count*/, void* /*pUser*/) throw()
  {
    debug_print("Placement\n");
    // count is size of object, pUser is address
    // TEST
    // SHOULD CALL CTOR HERE
    return nullptr;
  }
  static void* operator new[](std::size_t /*count*/, void* /*pUser*/) throw()
  {
    // count - alignment is size of objects, pUser address of first element
    debug_print("Placement\n");
    // SHOULD CALL CTORS HERE
    return nullptr;
  }

  // User-defined placement deallocation (must be called explicit)
  static void operator delete(void* /*ptr*/, int /*first*/, int /*second*/)
  {
    debug_print("Placement\n");
  }
  static void operator delete[](void* /*ptr*/, int /*first*/, int /*second*/)
  {
    debug_print("Placement\n");
  }

#ifdef CXX17
  //  /Zc:alignedNew
  // /std:c++17
  // __cplusplus
  // _HAS_CXX17

  void* operator new(std::size_t size, std::align_val_t al)
  {
    debug_print("aligned\n");
#ifdef _MSC_VER
    T* ptr = static_cast<T*>(_aligned_malloc(size, static_cast<std::size_t>(al)));
    new (ptr) T(); // Calls placement new
    return ptr ? ptr : throw std::bad_alloc{};
#else
    T* ptr = static_cast<T*>(_mm_malloc(size, static_cast<std::size_t>(al)));
    return ptr ? ptr : throw std::bad_alloc{};
#endif
  }
  void operator delete(void* ptr, std::size_t /*size*/, std::align_val_t /*align*/)
  {
    debug_print("aligned\n");
    T* pObject = reinterpret_cast<T*>(ptr);
    pObject->~T();
#ifdef _MSC_VER
    _aligned_free(ptr);
#else
    _mm_free(ptr);
#endif
  }
  void operator delete(void* ptr, std::align_val_t /*align*/)
  {
    debug_print("aligned\n");
#ifdef _MSC_VER
    _aligned_free(ptr);
#else
    _mm_free(ptr);
#endif
  }
  void* operator new[](std::size_t count, std::align_val_t al)
  {
    debug_print("aligned\n");
#if 1
    // Why is this wrong
    T* ptr = static_cast<T*>(std::aligned_alloc(static_cast<std::size_t>(al), count));
    size_t nObjects = (count - static_cast<std::size_t>(al)) / sizeof(T);
    new (ptr) T[nObjects]; // Calls placement new
    // Ctors called anyway, since new of descendant is not overloaded
    return ptr ? ptr : throw std::bad_alloc{};
#else
    // Default
    return ::operator new[](count, al);
#endif
  }

  void operator delete[](void* ptr, std::size_t count, std::align_val_t al)
  {
    debug_print("aligned\n");
#if 1
    // Why is this wrong
    size_t nObjects = (count - static_cast<std::size_t>(al)) / sizeof(T);
    T* pObject =
      reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(ptr) + static_cast<std::size_t>(al));
    for (size_t i = 0; i < nObjects; ++i)
    {
      // pObject->~T(); // Calls dtors twice???
      pObject++;
    }
    std::free(ptr);
#else
    // Default
    return ::operator delete[](ptr, count, al); // Calls many dtors, free
#endif
  }

#endif
};
} // namespace sps
