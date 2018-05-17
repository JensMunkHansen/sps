
#include <gtest/gtest.h>

#include <sps/weak_binder.hpp>
#include <memory>

#include <iostream>

// Use std::optional

TEST(bindWeakTest, testBasics) {
  struct Y {
    void bar() {};
  };

  struct X : std::enable_shared_from_this<X> {
    int increment(int by) {
      count += by;
      return count;
    }

    void foo() {
    }

    Y y;

    int count = 0;
  };

  auto px = std::make_shared<X>();

  auto wf = sps::bind_weak(&X::increment, px, std::placeholders::_1);
  auto weak_call_bar = sps::bind_weak(&Y::bar, std::shared_ptr<Y>(px, &px->y));

  auto ret1 = wf(4);
  EXPECT_TRUE(bool(ret1));
  EXPECT_EQ(4, ret1.value());

  auto wfoo1 = sps::bind_weak(&X::foo, px);
  auto retfoo1 = wfoo1();
  EXPECT_TRUE(bool(retfoo1));

  auto retbar1 = weak_call_bar();
  EXPECT_TRUE(bool(retbar1));

  px.reset();
  auto ret2 = wf(4);

  // none_t when optional<T> is uninitialized
  EXPECT_FALSE(bool(ret2));

  auto retfoo2 = wfoo1();
  EXPECT_FALSE(bool(retfoo2));

  auto retbar2 = weak_call_bar();
  EXPECT_FALSE(bool(retbar2));
}

struct Resource {
};

std::string something() {
  return "foo";
}

#if 0
TEST(bindWeakTest, unrelatedResourceTest) {
  auto pr = std::make_shared<Resource>();

  size_t touched = 0;
  auto f = [&touched](size_t times) {
    touched += times;
  };

  auto weak_touch_n = sps::bind_weak(f, pr, std::placeholders::_1);
  auto weak_touch_4 = sps::bind_weak(f, pr, 4);
  auto weak_something = sps::bind_weak(something, pr);

  auto touched_n = weak_touch_n(1);
  EXPECT_TRUE(bool(touched_n));
  EXPECT_EQ(1, touched);

  auto touched_4 = weak_touch_4();
  EXPECT_TRUE(bool(touched_4));
  EXPECT_EQ(5, touched);

  auto str = weak_something();
  EXPECT_TRUE(bool(str));
  EXPECT_EQ(std::string("foo"), str.value());

  pr.reset();

  touched_n = weak_touch_n(1);
  EXPECT_FALSE(bool(touched_n));
  EXPECT_EQ(5, touched);

  touched_4 = weak_touch_4();
  EXPECT_FALSE(bool(touched_4));
  EXPECT_EQ(5, touched);

  str = weak_something();
  EXPECT_FALSE(bool(str));
}


TEST(invokeTest, invoke1) {
  struct X {
    int foo(int& x) const {
      return x++;
    };
  };
  X x;

  auto z = 7;
  auto y = sps::invoke(&X::foo, std::cref(x), z);
  EXPECT_EQ(7, y);
  EXPECT_EQ(8, z);
}

#endif

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
