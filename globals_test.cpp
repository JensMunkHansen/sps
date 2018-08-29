#include <gtest/gtest.h>

#include <sps/globals.hpp>

#include <array>

class Test : public sps::Singleton<Test> {
 public:
 private:
  Test() {
    m_float = 2.0f;
  }
  float m_float;
  Test& operator=(const Test& rhs) = delete;

  friend class sps::Singleton<Test>;
};

// Explicit instantiate destructor (otherwise not called)
template int sps::Singleton<Test>::InstanceDestroy();

template <class T>
class TTest : public sps::Singleton<TTest<T> > {
 public:
  T m_value;
  TTest() {
    m_value = T(1.0);
  }
 private:
  friend class sps::Singleton<TTest<T> >;
};

// Explicit instantiate destructor (otherwise not called)
template int sps::Singleton<TTest<float> >::InstanceDestroy();

// Some templated gtest stuff
template <typename T, size_t SIZE>
size_t foo1(std::array<T, SIZE>& a, size_t start) {
  return !(a[start] == 1);
}

template <typename T, size_t SIZE>
size_t foo2(std::array<T,SIZE>& a, size_t start) {
  return !(a[start] == 1);
}

template <typename T, size_t SIZE>
size_t foo3(std::array<T,SIZE>& a, size_t start) {
  return !(a[start] == 1);
}

template<typename T, size_t SIZE>
using fooFp = size_t(*)(std::array<T, SIZE>&, size_t);

template <typename T, size_t SIZE>
class fooTemplateClass: public ::testing::TestWithParam<fooFp<T, SIZE>> {};

class Tc1: public fooTemplateClass<int, 1> {};

TEST_P(Tc1, Fnc) {
  fooFp<int, 1> fnc = GetParam();
  std::array<int, 1> arr ={1};
  EXPECT_EQ(1, arr[fnc(arr, 0)]);
}

INSTANTIATE_TEST_CASE_P(
    Name, Tc1,
    ::testing::Values(foo1<int, 1>, foo2<int, 1>, foo3<int, 1>),);

// My stuff

void fun() {
  const Test& test = Test::InstanceGet();
  SPS_UNREFERENCED_PARAMETER(test);
}

TEST(globals_test, singleton_test) {
  // We cannot instantiate templates here
  fun();
}

void fun0() {
  const TTest<float>& test = TTest<float>::InstanceGet();
  // If you make ctor public
  TTest<float> object();
  SPS_UNREFERENCED_PARAMETER(test);
}

TEST(globals_test, singleton_ttest) {
  // We cannot instantiate templates here
  fun0();
}

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
