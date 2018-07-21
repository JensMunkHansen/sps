#include <sps/globals.hpp>

class Test : public sps::Singleton<Test> {
 public:
  //  Test* InstanceGet() {}
 private:
  Test() {
    m_float = 2.0f;
  }
  float m_float;

  friend class sps::Singleton<Test>;
};

// Explicit instantiate destructor (otherwise not called)
template void sps::Singleton<Test>::InstanceDestroy();

int main(int argc, char* argv[]) {
  const Test& pTest = Test::InstanceGet();
  Test::InstanceDestroy();
  return 0;
}

