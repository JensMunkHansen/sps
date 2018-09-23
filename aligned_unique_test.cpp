#include <cstdio>

struct A {
  A() {}
  ~A() {
    //    printf(".\n");
  }
  int k = 2;
  void operator delete[](void* p, size_t count) {
    for (size_t i = 0 ; i < count ; i++) {
      ((A*)p)->~A();
      p++;
    }
  }
};
#include <sps/aligned.hpp>
#include <sps/align.hpp>

template <class T> class Float : public sps::dynaligned<Float<T>, 16> {
 public:
  T value;
};


struct Foo {
  Foo(int x, int y) : x(x), y(y){};
  int x;
  int y;
};


int main() {

  Float<double> f;
  // Single object
#if 0
  auto x = aligned::make_unique<double,16>(16.0);

  printf("0x%x\n", &x);
  printf("%d\n", (uintptr_t)x.get() % 16);

  // Forbidden thanks to "{}" --- did the user want to write
  // aligned::make_unique<double[]>(16)?
  // auto x = aligned::make_unique<double>(16);

  auto foo = aligned::make_unique<Foo>(3, 4);

  // Array
#endif

  //  auto y = aligned::make_unique<double[]>(16);

  auto z = aligned::make_unique<A[]>(9);

  A* pA = new A[7];
  uintptr_t* q = reinterpret_cast<uintptr_t*>(pA);
  q--;
  //

  std::cout << *(q) << std::endl;

  for (size_t i = 0 ; i < 16 ; i++) {
    std::cout << (*q >> 4*i) << std::endl;
  }

  delete[] pA;


  // Disabled for arrays of known bounds:
  // auto = aligned::make_unique<double[16]>(16);

  // Forbidden --- there is no default constructor:
  // auto foo = aligned::make_unique<Foo[]>(16);

  // Forbidden --- calling constructor & destructors on each array element is
  // not implemented:
  // auto s = aligned::make_unique<std::string[]>(16);
}
