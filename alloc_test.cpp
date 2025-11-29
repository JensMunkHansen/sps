#define SPS_DEBUG 1

#include <sps/cenv.h>
#include <sps/debug.h>

#include <memory>
#include <new>

#include <mm_malloc.h>

class alignas(64) A
{
private:
  char arr[1024];

public:
  A() { debug_print("\n"); }
  ~A() { debug_print("\n"); }
  void* operator new(std::size_t count) throw()
  {
    debug_print("unaligned\n");
    return ::operator new(count); // May throw std::bad_alloc, call T::T()
  }
  void operator delete(void* p)
  {
    debug_print("unaligned\n");
    A* pObject = reinterpret_cast<A*>(p);
    ::operator delete(pObject); // Calls destructor, T::~T(), free
  }

  void operator delete(void* p, size_t sz)
  {
    debug_print("called instead - if user-replacement\n");
    A* pObject = reinterpret_cast<A*>(p);
    ::operator delete(pObject);
  }

  void* operator new[](std::size_t count)
  {
    debug_print("mm_align\n");
    // return ::operator new[](count);
    return _mm_malloc(count, 16);
  }
  void operator delete[](void* p, size_t count)
  {
    debug_print("mm_align\n");
    //::operator delete[](p); // Calls many dtors, free
    _mm_free(p);
  }

  // Placement
  static void* operator new(std::size_t count, void* pUser) throw()
  {
    debug_print("Placement\n");
    // count is size of object, pUser is address
    // TEST
    // SHOULD CALL CTOR HERE
    return pUser;
  }
  static void* operator new[](std::size_t count, void* pUser) throw()
  {
    // count - alignment is size of objects, pUser address of first element
    debug_print("Placement\n");
    // SHOULD CALL CTORS HERE
    return pUser;
  }

  // User-defined placement deallocation (must be called explicit)
  static void operator delete(void* ptr, int first, int second) { debug_print("Placement\n"); }
  static void operator delete[](void* ptr, int first, int second) { debug_print("Placement\n"); }

#ifdef CXX17
  //  /Zc:alignedNew
  // /std:c++17
  // __cplusplus
  // _HAS_CXX17

  void* operator new(std::size_t size, std::align_val_t al)
  {
    debug_print("aligned\n");
#ifdef _MSC_VER
    A* ptr = (A*)_aligned_malloc(size, static_cast<std::size_t>(al));
    new (ptr) A(); // Calls placement new
    return ptr ? ptr : throw std::bad_alloc{};
#else
    A* ptr = (A*)_mm_malloc(size, static_cast<std::size_t>(al));
    return ptr ? ptr : throw std::bad_alloc{};
#endif
  }
  void operator delete(void* ptr, std::size_t size, std::align_val_t align)
  {
    debug_print("aligned\n");
    A* pObject = reinterpret_cast<A*>(ptr);
    pObject->~A();
#ifdef _MSC_VER
    _aligned_free(ptr);
#else
    _mm_free(ptr);
#endif
  }
  void operator delete(void* ptr, std::align_val_t align)
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
#if 0
    // Why is this wrong
    A* ptr = (A*)_aligned_malloc(count, static_cast<std::size_t>(al));
    size_t nObjects = (count - static_cast<std::size_t>(al)) / sizeof(A);
    new (ptr) A[nObjects]; // Calls placement new
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
#if 0
    // Why is this wrong
    size_t nObjects = (count - static_cast<std::size_t>(al)) / sizeof(A);
    A* pObject = reinterpret_cast<A*>(reinterpret_cast<uintptr_t>(ptr) + static_cast<std::size_t>(al));
    for (size_t i = 0; i < nObjects; ++i) {
      // pObject->~T(); // Calls dtors twice???
      pObject++;
    }
    _aligned_free(ptr);
#else
    // Default
    return ::operator delete[](ptr, count, al); // Calls many dtors, free
#endif
  }

#endif
};

int main()
{

  printf("%d\n", __cplusplus);
  A* pAs = new A[2];
  delete[] pAs;

  A* pA = new A(); // without :: it is the one defined in A or parent
  delete pA;

  A* pB = new A();
  delete pB;

  A* pBs = new A[2];
  delete[] pBs;

  std::unique_ptr<A[]> puBs = std::make_unique<A[]>(4);
  std::unique_ptr<A[]> puAs = std::make_unique<A[]>(3);

  return 0;
}
