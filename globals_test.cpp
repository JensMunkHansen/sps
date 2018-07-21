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

int main(int argc, char* argv[]) {
  Test* pTest = Test::InstanceGet();
  Test::InstanceDestroy();
  return 0;
}
