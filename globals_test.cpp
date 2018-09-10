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

// My stuff

void fun() {
  Test* test = Test::InstanceGet();
  SPS_UNREFERENCED_PARAMETER(test);
}

TEST(globals_test, singleton_test) {
  // We cannot instantiate templates here
  fun();
}

void fun0() {
  const TTest<float>* test = TTest<float>::InstanceGet();
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
