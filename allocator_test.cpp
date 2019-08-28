#define SPS_DEBUG 1

#include <sps/debug.h>
#include <sps/dynallocators.hpp>

#include <iostream>
#include <memory>

#include <gtest/gtest.h>

class alignas(64) A : public sps::DynAllocators<A> {
 public:
   //static void* operator new(std::size_t count, void* pUser) {debug_print("\n");}
  float m_f;
  A() : m_f(1.0f) {
    debug_print("\n");
  }
  ~A() {
    debug_print("\n");
  }
  // void* operator new(std::size_t size, std::align_val_t al) = delete;
};

class B : public sps::DynAllocators<B> {
public:
  float m_f;
  B() : m_f(1.0f) {
    debug_print("\n");
  }
  ~B() {
    debug_print("\n");
  }
};

int main(int argc, char** argv) {
  SPS_UNREFERENCED_PARAMETERS(argc, argv);
  A* pAs = new A[2];
  delete[] pAs;
#if 0
  A* pA = new A(); // without :: it is the one defined in A or parent
  delete pA;

  B* pB = new B();
  delete pB;

  B* pBs = new B[2];
  delete[] pBs;

  std::unique_ptr<B[]> puBs = std::make_unique<B[]>(4);
  std::unique_ptr<A[]> puAs = std::make_unique<A[]>(3);
#endif
  //  testing::InitGoogleTest(&argc, argv);
  //  return RUN_ALL_TESTS();
  return 0;
}
