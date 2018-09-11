#include <gtest/gtest.h>
#include <sps/memory>
#include <sps/functional.hpp>
#include <sps/multi_malloc.hpp>
class A {
 public:
  A() {
    m_data = sps::deleted_aligned_array_create<float>(20);
  }
 private:
  sps::deleted_aligned_array<float> m_data;
};

struct B {
  B() : pData(nullptr)  {
    pData = new int[10];
  }
  ~B() {
    delete pData;
  }
  int* pData;
};

TEST(memory_test, aligned_unique_array) {
  auto a = sps::deleted_aligned_array_create<float>(0);

  auto b = sps::deleted_aligned_array_create<float>(35);

  auto c = sps::deleted_aligned_multi_array_create<double, 2>(1, 1);

  auto d = sps::deleted_aligned_multi_array_create<double, 2>(0, 0);

  auto f = sps::deleted_aligned_multi_array_create<double, 2>(1, 0);

  auto g = sps::deleted_aligned_multi_array_create<double, 2>(0, 1);

#ifdef __GNUG__
  //  sps::unique_multi_array<double, 2> h =
  //    sps::nix::deleted_aligned_multi_array_create<double, 2>(7, 8);
  //  size_t m = 7;
  //  size_t n = 8;
  //  const size_t nDims = 2;
  //  sps::unique_multi_array<double, nDims> i =
  //    sps::unique_multi_array_create<double, nDims>(m, n);
#endif

  // Must fail
  // auto j = sps::deleted_aligned_multi_array<B,2>(1,1);
  // auto k = sps::deleted_aligned_multi_array_create<B, 2>(1,1);

  auto l = sps::deleted_aligned_multi_array_create<double, 1>(1);

  ASSERT_TRUE(true);
}

TEST(memory_test, unique_member) {
  A a;
}

TEST(memory_test, multi_malloc) {
  float* pFloat0 = static_cast<float*>(_mm_multi_malloc<float>(3, 2, 2, 4));
  _mm_multi_free<float>(pFloat0, 3);
#ifndef _MSC_VER
  float* pFloat1 = static_cast<float*>(_mm_multi_malloc_nc<float>(2, 10, 10));
  _mm_multi_free_nc<float>(pFloat1, 2);
#endif
}

TEST(memory_test, reset_array) {
  auto b = sps::deleted_aligned_array_create<float>(35);
  b.reset(static_cast<float*>(_mm_malloc(45*sizeof(float), 16)));
  b.get_deleter() = sps::make::function([](float* f)->void { _mm_free(f);});
}

template <class T>
struct AlignedData : sps::aligned<4*sizeof(T)> {
  T a;
};

TEST(memory_test, inherit_alignment) {
  float f;
  SPS_UNREFERENCED_PARAMETER(f);
  AlignedData<double> c[2];
  ASSERT_EQ(((uintptr_t)&c[1] & 0x1F), 0UL);
}

int main(int argc, char* argv[]) {
#ifdef __GNUG__
  static_assert(sps::ptr_depth<sps::depth_ptr<float, 2>::value_type>::level == 2,
                "The level of depth_ptr<T,2>::value_type must equal 2");
#endif
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

// GNU extension
//
