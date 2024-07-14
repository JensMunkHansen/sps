#include <gtest/gtest.h>
#include <sps/memory>
#include <sps/functional.hpp>


int f(int a, int b, int c, int d) {
  return a + b + c - d;
}

int g(int a) {
  return 1 + a;
}

int h() {
  return 1;
}

int i(int a, int b, int c, int d) {
  return 1 + 2*a + b + c + d;
}


TEST(functional_test, prebinder) {
  using sps::trailing_bind;
  using sps::prebind;

  auto a1 = trailing_bind(f, 1);

  int result = a1(3, 8, 13);
  ASSERT_EQ(result, -1);

  //  completely bound
  auto a = prebind(g, 1);
  ASSERT_EQ(a(),2);

  //  nested bind by reference
  auto b = prebind(g, a);
  ASSERT_EQ(b(),3);

  std::get<1>(a.closure) = 2;
  //  bind to prebinder
  auto c = prebind(b);
  ASSERT_EQ(c(),4);

  //  nested bind of temp to temp
  auto d = prebind(prebind(g), prebind(h));
  ASSERT_EQ(d(), 2);

  //  and the one you wanted orginally
  auto e = prebind(i, 2, 1, 1);
  ASSERT_EQ(e(1), 8);
}

// Stack-allocated
void test(void* ptr) {
  size_t a[40]{};
  a[0]= sizeof(a);
  std::cout << (reinterpret_cast<char const *>(a) - reinterpret_cast<char *>(ptr)) << " " << a[0] << "\n";
}


TEST(abe, test) {
  sps::static_function<void(void*)> h(test);
  h(&h);
}

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

