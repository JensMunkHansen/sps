#include <cstdio>

#include <sps/aligned.hpp>
#include <sps/align.hpp>
#include <sps/aligned_allocator.hpp>

struct A : public sps::dynaligned<A> {
  A() {}
  ~A() {
    //    printf(".\n");
  }
  int k = 2;

};

template <class T> class Float : public sps::dynaligned<Float<T>, 16> {
 public:
  T value;
};


struct Foo : public sps::aligned<32> {
  Foo(int x, int y) : x(x), y(y) {
    printf(".\n");
  };
  int x;
  int y;
  ~Foo() {
    printf("x\n");
  }
};


int main() {

  Float<double> f;
  // Single object
  auto x = aligned::make_unique<double,16>(16.0);
  printf("0x%lx\n", reinterpret_cast<uintptr_t>(x.get()));
  printf("%ld\n", (uintptr_t)x.get() % 16);

  // Okay
  auto foo = aligned::make_unique<Foo>(3, 4);

  // Not working
  // auto z0 = aligned::make_unique<A>();


  // Works
  auto z = make_unique_array(std::allocator<Foo>(), 2, 3, 4);

  printf("0x%lx\n", (uintptr_t) &(z.get()[0]));
  printf("%ld\n", (uintptr_t) &(z.get()[1]) % 16);


  auto z0 = make_unique_aligned_array<Foo, 32, int, int>(aligned_allocator<Foo, 32>(), 2, 3, 4);

  printf("0x%x\n", (uintptr_t) &(z0.get()[1]));
  printf("%d\n", (uintptr_t) &(z0.get()[1]) % 32);


  // Disabled for arrays of known bounds:
  // auto = aligned::make_unique<double[16]>(16);

  // Forbidden --- there is no default constructor:
  // auto foo = aligned::make_unique<Foo[]>(16);

  // Forbidden --- calling constructor & destructors on each array element is
  // not implemented:
  // auto s = aligned::make_unique<std::string[]>(16);
}
