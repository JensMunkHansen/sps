#define SPS_DEBUG 1

#include <cstdio>
#include <cstdint>
#include <memory>

#include <sps/align.hpp>

const size_t Alignment = 64;

class A : public sps::dynaligned<A, Alignment> {
 public:
  A() {
    printf("A()\n");
  }
  ~A() {
    printf("~A()\n");
  }
  char data[4];
};


int main() {
  A a;
  printf("0x%x\n", uintptr_t(&a) % Alignment);

  A* p = new A();
  printf("0x%x\n", uintptr_t(p) % Alignment);
  delete p;

  std::unique_ptr<A> uA = std::unique_ptr<A>(new A());
  printf("0x%x\n", uintptr_t(uA.get()) % Alignment);
  //  std::unique_ptr<A> uA0 = std::make_unique<A>();
  //  printf("0x%x\n", uintptr_t(uA0.get()) % Alignment);

  return 0;
}
