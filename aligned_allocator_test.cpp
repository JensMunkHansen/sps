#include <gtest/gtest.h>
#include <sps/cenv.h>

#include <sps/align.hpp>
#include <sps/aligned_allocator.hpp>

struct Foo : public sps::aligned<32>
{
  Foo(int x_, int y_)
    : x(x_)
    , y(y_)
  {
  }
  int x;
  int y;
  ~Foo() {}
};

TEST(aligned_allocator, unique_array_test)
{
  auto z = sps::make_unique_array<Foo>(2, 3, 4);
  ASSERT_EQ(1, 1);
}

TEST(aligned_allocator, unique_aligned_array_test)
{
  auto z = sps::make_unique_aligned_array<Foo, 16>(2, 3, 4);
  ASSERT_TRUE(reinterpret_cast<uintptr_t>(&(z.get()[1])) % 16 == 0);
}

TEST(aligned_allocator, unique_aligned_array_test_max_align)
{
  auto z = sps::make_unique_aligned_array<Foo, 32>(2, 3, 4);
  ASSERT_TRUE(reinterpret_cast<uintptr_t>(&(z.get()[1])) % 32 == 0);
}

int main(int argc, char* argv[])
{
  float* p = static_cast<float*>(alloca(4 * sizeof(float)));
  SPS_UNREFERENCED_PARAMETER(p);
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
