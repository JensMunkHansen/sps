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

template<template<typename> typename... Skills>
class X : public Skills<X<Skills...>>...
{
public:
  void basicMethod() {}
};

template<typename Derived>
class ExtraFeature1 {
 public:
  void extraMethod1() {
    auto derived = static_cast<Derived&>(*this);
    derived.basicMethod();
    derived.basicMethod();
    derived.basicMethod();
  }
};

template<typename Derived>
class ExtraFeature2 {
 public:
  void extraMethod2() {
    auto derived = static_cast<Derived&>(*this);
  }
};

template<typename Derived>
class ExtraFeature3 {
 public:
  void extraMethod3() {
    auto derived = static_cast<Derived&>(*this);
  }
};

template<typename Derived>
class ExtraFeature4 {
 public:
  void extraMethod4() {
    auto derived = static_cast<Derived&>(*this);
  }
};

template<typename Derived>
struct FeaturePack1 : ExtraFeature1<Derived>, ExtraFeature2<Derived> {};

template<typename Derived>
struct FeaturePack2 : ExtraFeature3<Derived>, ExtraFeature4<Derived> {};

int main() {
  Foo<float> a;
  // A<std::unique_ptr<float> > b;

  using X12 = X<ExtraFeature1, ExtraFeature2>;
  X12 x;
  x.extraMethod1();
  x.extraMethod2();

  using X1234 = X<FeaturePack1, FeaturePack2>;
  X1234 y;
  y.extraMethod1();
  y.extraMethod2();
  y.extraMethod3();
  y.extraMethod4();


  return 0;
}
