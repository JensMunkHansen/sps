#include <memory>

template<typename T, typename Sfinae = void>
struct Foo {
  // static assert here
};

template<typename T>
struct Foo<T, typename std::enable_if<std::is_copy_constructible<T>::value>::type> {
  Foo();
};


template<typename T>
Foo<T, typename std::enable_if<std::is_copy_constructible<T>::value>::type>::Foo()
{
  /* Phew, we're there */
}

int main() {
  Foo<float> a;
  // A<std::unique_ptr<float> > b;
  return 0;
}
