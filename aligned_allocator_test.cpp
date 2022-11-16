#include <gtest/gtest.h>

#include <sps/align.hpp>
#include <sps/aligned_allocator.hpp>

struct Foo : public sps::aligned<32> {
  Foo(int x, int y) : x(x), y(y) {}
  int x;
  int y;
  ~Foo() {}
};

TEST(aligned_allocator, unique_array_test) {
  auto z = sps::make_unique_array<Foo>(2, 3, 4);
  ASSERT_EQ(1, 1);
}

TEST(aligned_allocator, unique_aligned_array_test) {
  auto z =
    sps::make_unique_aligned_array<Foo, 16>(2, 3, 4);
  ASSERT_TRUE((uintptr_t) &(z.get()[1]) % 16 == 0);
}

TEST(aligned_allocator, unique_aligned_array_test_max_align) {
  auto z =
    sps::make_unique_aligned_array<Foo, 32>(2, 3, 4);
  ASSERT_TRUE((uintptr_t) &(z.get()[1]) % 32 == 0);
}

int main(int argc, char* argv[]) {
  float* p = (float*) alloca(4*sizeof(float));
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
