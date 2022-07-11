/*===================================================================
*   Copyright (c) Vadim Karkhin. All rights reserved.
*   Use, modification, and distribution is subject to license terms.
*   You are welcome to contact the author at: vdksoft@gmail.com
===================================================================*/

#include <map>
#include <atomic>
#include <thread>
#include <memory>
#include <utility>
#include <type_traits>

#include <gtest/gtest.h>
#include <signals.h>

namespace {
// Class to keep track of slot invocations
template<typename T>
struct slot_tracker : public std::multimap<
  std::remove_reference_t<T>, std::thread::id> {
  template<typename U>
  void operator()(U && arg) {
    this->emplace(std::forward<U>(arg),
                  std::this_thread::get_id());
  }
};

slot_tracker<int> global_indicator;

// ========================= Functions ========================= //

void function(int arg) {
  global_indicator(arg);
}

int function_unique(int arg) noexcept {
  global_indicator(arg);
  return arg;
}

std::string function_non_void_no_args() {
  global_indicator(100);
  return "100";
}

void function_multi_args(int arg1, int arg2) {
  global_indicator(arg1 + arg2);
}

// ===================== Function objects ===================== //

struct functor {
  explicit functor(int arg) noexcept
    : value_ {
    arg
  }
  {}
  std::pair<int, int> operator()() {
    global_indicator(100);
    return { 100, 100 };
  }
  void operator()(int arg) {
    global_indicator(arg);
  }
  void operator()(int arg1, int arg2) {
    global_indicator(arg1 + arg2);
  }
  bool operator==(const functor & other) const noexcept {
    return value_ == other.value_;
  }
  int value_;
};

// ========================= Classes ========================= //

template<typename BaseClass>
class test_class : public BaseClass {
 public:

  void method(int arg) {
    global_indicator(arg);
  }
  int method_unique(int arg) {
    global_indicator(arg);
    return arg;
  }
  std::string method_non_void_no_args() {
    global_indicator(100);
    return "100";
  }
  void method_multi_args(int arg1, int arg2) {
    global_indicator(arg1 + arg2);
  }
};

struct DummyType {};

// ====================== Test utilities ====================== //

class test_thread {
 public:

  template<typename Fn>
  explicit test_thread(Fn start_thread) {
    test_thread::set_flag(false);
    thread_ = std::thread{ std::move(start_thread) };
    while (!test_thread::get_flag()) std::this_thread::yield();
  }
  void wait_thread() {
    if (thread_.joinable()) thread_.join();
  }
  std::thread::id get_id() const noexcept {
    return thread_.get_id();
  }
  static bool set_flag(bool flag) noexcept {
    return flag_.exchange(flag);
  }
  static bool get_flag() noexcept {
    return flag_.load();
  }
 private:

  std::thread thread_;
  static std::atomic_bool flag_;
};
std::atomic_bool test_thread::flag_{ false };

struct copy_counter {
  copy_counter() noexcept = default;
  ~copy_counter() noexcept = default;
  copy_counter(const copy_counter & other) noexcept
    : count_ {
    other.count_ + 1
  }
  {}
  copy_counter & operator=(const copy_counter & other) noexcept {
    count_ = other.count_ + 1;
    return *this;
  }
  unsigned int get() const noexcept {
    return count_;
  }
  unsigned int count_ = 0;
};

} // namespace

template<typename T>
struct SignalsTest : public testing::Test {
  using signal_int_t      = typename T::signal_int;
  using signal_no_args_t  = typename T::signal_no_args;
  using signal_int_int_t  = typename T::signal_int_int;
  using signal_cc_t       = typename T::signal_cc;
  using signal_cc_ref_t   = typename T::signal_cc_ref;
  using cxt_class_t       = typename T::cxt_class;
  using ord_class_t       = typename T::ord_class;
};
struct TypeSet {
  using signal_int     = vdk::signal<void(int)>;
  using signal_no_args = vdk::signal<void()>;
  using signal_int_int = vdk::signal<void(int, int)>;
  using signal_cc      = vdk::signal<void(copy_counter)>;
  using signal_cc_ref  = vdk::signal<void(copy_counter&)>;
  using cxt_class      = test_class<vdk::context>;
  using ord_class      = test_class<DummyType>;
};
struct TypeSetLite {
  using signal_int     = vdk::lite::signal<void(int)>;
  using signal_no_args = vdk::lite::signal<void()>;
  using signal_int_int = vdk::lite::signal<void(int, int)>;
  using signal_cc      = vdk::lite::signal<void(copy_counter)>;
  using signal_cc_ref  = vdk::lite::signal<void(copy_counter&)>;
  using cxt_class      = test_class<vdk::lite::context>;
  using ord_class      = test_class<DummyType>;
};

using TestTypes = testing::Types<TypeSet, TypeSetLite>;
TYPED_TEST_CASE(SignalsTest, TestTypes);

//==== Tests for common functionality (normal and lite versions) ====//

TYPED_TEST(SignalsTest, Empty) {
  using signal_t = typename TestFixture::signal_int_t;
  signal_t sig;
  sig.emit(5);
  sig(5);
  sig.disconnect();
  SUCCEED();
}

TYPED_TEST(SignalsTest, Function) {
  using signal_t = typename TestFixture::signal_int_t;
  signal_t sig;

  global_indicator.clear();
  EXPECT_NE(sig.connect(function), 0U);
  sig.emit(5);
  EXPECT_EQ(global_indicator.count(5), 1U);
  EXPECT_EQ(global_indicator.size(), 1U);

  global_indicator.clear();
  EXPECT_TRUE(sig.disconnect(function));
  sig.emit(5);
  EXPECT_EQ(global_indicator.size(), 0U);
  EXPECT_FALSE(sig.disconnect(function));

  global_indicator.clear();
  void(*func_null)(int) = nullptr;
  EXPECT_EQ(sig.connect(func_null), 0U);
  sig.emit(5);
  EXPECT_EQ(global_indicator.size(), 0U);
  EXPECT_FALSE(sig.disconnect(func_null));
}

TYPED_TEST(SignalsTest, Functor) {
  using signal_t = typename TestFixture::signal_int_t;
  signal_t sig;

  global_indicator.clear();
  EXPECT_NE(sig.connect(functor{ 1 }), 0U);
  sig.emit(5);
  EXPECT_EQ(global_indicator.count(5), 1U);
  EXPECT_EQ(global_indicator.size(), 1U);

  global_indicator.clear();
  EXPECT_TRUE(sig.disconnect(functor{ 1 }));
  sig.emit(5);
  EXPECT_EQ(global_indicator.size(), 0U);
  EXPECT_FALSE(sig.disconnect(functor{ 1 }));
}

TYPED_TEST(SignalsTest, Lambda) {
  using signal_t = typename TestFixture::signal_int_t;
  signal_t sig;
  double ballast = 2.0; // Prevent lambda from turning into function
  auto lambda = [ballast](int arg) {
    global_indicator(arg);
  };

  global_indicator.clear();
  EXPECT_NE(sig.connect(lambda), 0U);
  sig.emit(5);
  EXPECT_EQ(global_indicator.count(5), 1U);
  EXPECT_EQ(global_indicator.size(), 1U);

  global_indicator.clear();
  // Lambdas are not equality comparable, so this must return false
  EXPECT_FALSE(sig.disconnect(lambda));
  // Lambda has not been disconnected
  sig.emit(5);
  EXPECT_EQ(global_indicator.count(5), 1U);
  EXPECT_EQ(global_indicator.size(), 1U);
}

TYPED_TEST(SignalsTest, Method) {
  using signal_t = typename TestFixture::signal_int_t;
  using class_t = typename TestFixture::ord_class_t;
  class_t object;
  signal_t sig;

  global_indicator.clear();
  EXPECT_NE(sig.connect(&object, &class_t::method), 0U);
  sig.emit(5);
  EXPECT_EQ(global_indicator.count(5), 1U);
  EXPECT_EQ(global_indicator.size(), 1U);

  global_indicator.clear();
  EXPECT_TRUE(sig.disconnect(&object, &class_t::method));
  sig.emit(5);
  EXPECT_EQ(global_indicator.size(), 0U);
  EXPECT_FALSE(sig.disconnect(&object, &class_t::method));

  global_indicator.clear();
  void(class_t::*method)(int) = nullptr;
  EXPECT_EQ(sig.connect(&object, method), 0U);
  sig.emit(5);
  EXPECT_EQ(global_indicator.size(), 0U);

  global_indicator.clear();
  class_t * obj = nullptr;
  EXPECT_EQ(sig.connect(obj, &class_t::method), 0U);
  sig.emit(5);
  EXPECT_EQ(global_indicator.size(), 0U);

  global_indicator.clear();
  EXPECT_EQ(sig.connect(obj, method), 0U);
  sig.emit(5);
  EXPECT_EQ(global_indicator.size(), 0U);

  EXPECT_FALSE(sig.disconnect(&object, method));
  EXPECT_FALSE(sig.disconnect(obj, &class_t::method));
  EXPECT_FALSE(sig.disconnect(obj, method));
}

TYPED_TEST(SignalsTest, ContextMethod) {
  using signal_t = typename TestFixture::signal_int_t;
  using class_t = typename TestFixture::cxt_class_t;
  class_t object;
  signal_t sig;

  global_indicator.clear();
  EXPECT_NE(sig.connect(&object, &class_t::method), 0U);
  sig.emit(5);
  EXPECT_EQ(global_indicator.count(5), 1U);
  EXPECT_EQ(global_indicator.size(), 1U);

  global_indicator.clear();
  EXPECT_TRUE(sig.disconnect(&object, &class_t::method));
  sig.emit(5);
  EXPECT_EQ(global_indicator.size(), 0U);
  EXPECT_FALSE(sig.disconnect(&object, &class_t::method));

  global_indicator.clear();
  void(class_t::*method)(int) = nullptr;
  EXPECT_EQ(sig.connect(&object, method), 0U);
  sig.emit(5);
  EXPECT_EQ(global_indicator.size(), 0U);

  global_indicator.clear();
  class_t * obj = nullptr;
  EXPECT_EQ(sig.connect(obj, &class_t::method), 0U);
  sig.emit(5);
  EXPECT_EQ(global_indicator.size(), 0U);

  global_indicator.clear();
  EXPECT_EQ(sig.connect(obj, method), 0U);
  sig.emit(5);
  EXPECT_EQ(global_indicator.size(), 0U);

  EXPECT_FALSE(sig.disconnect(&object, method));
  EXPECT_FALSE(sig.disconnect(obj, &class_t::method));
  EXPECT_FALSE(sig.disconnect(obj, method));
}

TYPED_TEST(SignalsTest, DisconnectById) {
  using signal_t = typename TestFixture::signal_int_t;
  using class_t = typename TestFixture::cxt_class_t;
  double ballast = 2.0; // Prevent lambda from turning into function
  auto lambda = [ballast](int arg) {
    global_indicator(arg);
  };
  class_t object;
  signal_t sig;

  auto id1 = sig.connect(function);
  auto id2 = sig.connect(functor{ 1 });
  auto id3 = sig.connect(lambda);
  auto id4 = sig.connect(&object, &class_t::method);
  EXPECT_NE(id1, 0U);
  EXPECT_NE(id2, 0U);
  EXPECT_NE(id3, 0U);
  EXPECT_NE(id4, 0U);

  // id value 0 disconnects nothing
  EXPECT_FALSE(sig.disconnect(unsigned(0)));

  EXPECT_TRUE(sig.disconnect(id1));
  EXPECT_TRUE(sig.disconnect(id2));
  EXPECT_TRUE(sig.disconnect(id3));
  EXPECT_TRUE(sig.disconnect(id4));
  EXPECT_FALSE(sig.disconnect(id1));
  EXPECT_FALSE(sig.disconnect(id2));
  EXPECT_FALSE(sig.disconnect(id3));
  EXPECT_FALSE(sig.disconnect(id4));

  global_indicator.clear();
  sig.emit(5);
  EXPECT_EQ(global_indicator.size(), 0U);
}

TYPED_TEST(SignalsTest, ConnectDisconnectEqualSlotsByOne) {
  using signal_t = typename TestFixture::signal_int_t;
  using class_t = typename TestFixture::cxt_class_t;
  class_t object;
  signal_t sig;

  EXPECT_NE(sig.connect(function), 0U);
  EXPECT_NE(sig.connect(function), 0U);
  EXPECT_NE(sig.connect(function), 0U);
  EXPECT_TRUE(sig.disconnect(function));
  EXPECT_TRUE(sig.disconnect(function));
  EXPECT_TRUE(sig.disconnect(function));
  EXPECT_FALSE(sig.disconnect(function));

  EXPECT_NE(sig.connect(functor{ 1 }), 0U);
  EXPECT_NE(sig.connect(functor{ 1 }), 0U);
  EXPECT_NE(sig.connect(functor{ 1 }), 0U);
  EXPECT_TRUE(sig.disconnect(functor{ 1 }));
  EXPECT_TRUE(sig.disconnect(functor{ 1 }));
  EXPECT_TRUE(sig.disconnect(functor{ 1 }));
  EXPECT_FALSE(sig.disconnect(functor{ 1 }));

  EXPECT_NE(sig.connect(&object, &class_t::method), 0U);
  EXPECT_NE(sig.connect(&object, &class_t::method), 0U);
  EXPECT_NE(sig.connect(&object, &class_t::method), 0U);
  EXPECT_TRUE(sig.disconnect(&object, &class_t::method));
  EXPECT_TRUE(sig.disconnect(&object, &class_t::method));
  EXPECT_TRUE(sig.disconnect(&object, &class_t::method));
  EXPECT_FALSE(sig.disconnect(&object, &class_t::method));

  global_indicator.clear();
  sig.emit(5);
  EXPECT_EQ(global_indicator.size(), 0U);
}

TYPED_TEST(SignalsTest, CompareFunctions) {
  using signal_t = typename TestFixture::signal_int_t;
  signal_t sig;

  EXPECT_NE(sig.connect(function), 0U);
  EXPECT_NE(sig.connect(function_unique), 0U);
  EXPECT_TRUE(sig.disconnect(function));
  EXPECT_FALSE(sig.disconnect(function));
  EXPECT_TRUE(sig.disconnect(function_unique));
  EXPECT_FALSE(sig.disconnect(function_unique));

  global_indicator.clear();
  sig.emit(5);
  EXPECT_EQ(global_indicator.size(), 0U);
}

TYPED_TEST(SignalsTest, CompareFunctors) {
  using signal_t = typename TestFixture::signal_int_t;
  signal_t sig;

  EXPECT_NE(sig.connect(functor{ 1 }), 0U);
  EXPECT_NE(sig.connect(functor{ 2 }), 0U);
  EXPECT_NE(sig.connect(functor{ 3 }), 0U);
  EXPECT_TRUE(sig.disconnect(functor{ 1 }));
  EXPECT_FALSE(sig.disconnect(functor{ 1 }));
  EXPECT_TRUE(sig.disconnect(functor{ 2 }));
  EXPECT_FALSE(sig.disconnect(functor{ 2 }));
  EXPECT_TRUE(sig.disconnect(functor{ 3 }));
  EXPECT_FALSE(sig.disconnect(functor{ 3 }));

  global_indicator.clear();
  sig.emit(5);
  EXPECT_EQ(global_indicator.size(), 0U);
}

TYPED_TEST(SignalsTest, CompareMethods) {
  using signal_t = typename TestFixture::signal_int_t;
  using class_t = typename TestFixture::ord_class_t;
  class_t object1;
  class_t object2;
  class_t object3;
  signal_t sig;

  // The same object but different methods
  EXPECT_NE(sig.connect(&object1, &class_t::method), 0U);
  EXPECT_NE(sig.connect(&object1, &class_t::method_unique), 0U);
  // Different objects but the same method
  EXPECT_NE(sig.connect(&object2, &class_t::method), 0U);
  EXPECT_NE(sig.connect(&object3, &class_t::method), 0U);

  EXPECT_TRUE(sig.disconnect(&object1, &class_t::method));
  EXPECT_FALSE(sig.disconnect(&object1, &class_t::method));
  EXPECT_TRUE(sig.disconnect(&object1, &class_t::method_unique));
  EXPECT_FALSE(sig.disconnect(&object1, &class_t::method_unique));
  EXPECT_TRUE(sig.disconnect(&object2, &class_t::method));
  EXPECT_FALSE(sig.disconnect(&object2, &class_t::method));
  EXPECT_TRUE(sig.disconnect(&object3, &class_t::method));
  EXPECT_FALSE(sig.disconnect(&object3, &class_t::method));

  global_indicator.clear();
  sig.emit(5);
  EXPECT_EQ(global_indicator.size(), 0U);
}

TYPED_TEST(SignalsTest, CompareContextMethods) {
  using signal_t = typename TestFixture::signal_int_t;
  using class_t = typename TestFixture::cxt_class_t;
  class_t object1;
  class_t object2;
  class_t object3;
  signal_t sig;

  // The same object but different methods
  EXPECT_NE(sig.connect(&object1, &class_t::method), 0U);
  EXPECT_NE(sig.connect(&object1, &class_t::method_unique), 0U);
  // Different objects but the same method
  EXPECT_NE(sig.connect(&object2, &class_t::method), 0U);
  EXPECT_NE(sig.connect(&object3, &class_t::method), 0U);

  EXPECT_TRUE(sig.disconnect(&object1, &class_t::method));
  EXPECT_FALSE(sig.disconnect(&object1, &class_t::method));
  EXPECT_TRUE(sig.disconnect(&object1, &class_t::method_unique));
  EXPECT_FALSE(sig.disconnect(&object1, &class_t::method_unique));
  EXPECT_TRUE(sig.disconnect(&object2, &class_t::method));
  EXPECT_FALSE(sig.disconnect(&object2, &class_t::method));
  EXPECT_TRUE(sig.disconnect(&object3, &class_t::method));
  EXPECT_FALSE(sig.disconnect(&object3, &class_t::method));

  global_indicator.clear();
  sig.emit(5);
  EXPECT_EQ(global_indicator.size(), 0U);
}

TYPED_TEST(SignalsTest, MultipleSlots) {
  using signal_t = typename TestFixture::signal_int_t;
  using class_t = typename TestFixture::cxt_class_t;
  double ballast = 2.0; // Prevent lambda from turning into function
  auto lambda = [ballast](int arg) {
    global_indicator(arg);
  };
  class_t object;
  signal_t sig;

  global_indicator.clear();
  auto id1 = sig.connect(function);
  EXPECT_NE(id1, 0U);
  sig.emit(5);
  auto id2 = sig.connect(functor{ 1 });
  EXPECT_NE(id2, 0U);
  sig.emit(10);
  auto id3 = sig.connect(lambda);
  EXPECT_NE(id3, 0U);
  sig.emit(15);
  auto id4 = sig.connect(&object, &class_t::method);
  EXPECT_NE(id4, 0U);
  sig.emit(20);

  EXPECT_EQ(global_indicator.size(), 10U);
  EXPECT_EQ(global_indicator.count(5), 1U);
  EXPECT_EQ(global_indicator.count(10), 2U);
  EXPECT_EQ(global_indicator.count(15), 3U);
  EXPECT_EQ(global_indicator.count(20), 4U);

  global_indicator.clear();
  EXPECT_TRUE(sig.disconnect(id1));
  sig.emit(5);
  EXPECT_EQ(global_indicator.count(5), 3U);
  EXPECT_EQ(global_indicator.size(), 3U);

  global_indicator.clear();
  EXPECT_TRUE(sig.disconnect(id2));
  sig.emit(5);
  EXPECT_EQ(global_indicator.count(5), 2U);
  EXPECT_EQ(global_indicator.size(), 2U);

  global_indicator.clear();
  EXPECT_TRUE(sig.disconnect(id3));
  sig.emit(5);
  EXPECT_EQ(global_indicator.count(5), 1U);
  EXPECT_EQ(global_indicator.size(), 1U);

  global_indicator.clear();
  EXPECT_TRUE(sig.disconnect(id4));
  sig.emit(5);
  EXPECT_EQ(global_indicator.size(), 0U);
}

TYPED_TEST(SignalsTest, TrackObject) {
  using signal_t = typename TestFixture::signal_int_t;
  using class_t = typename TestFixture::cxt_class_t;
  double ballast = 2.0; // Prevent lambda from turning into function
  auto lambda = [ballast](int arg) {
    global_indicator(arg);
  };
  signal_t sig;

  {
    global_indicator.clear();
    class_t object; // associated object
    EXPECT_NE(sig.connect(&object, function), 0U);
    EXPECT_NE(sig.connect(&object, functor{ 1 }), 0U);
    EXPECT_NE(sig.connect(&object, lambda), 0U);
    EXPECT_NE(sig.connect(&object, &class_t::method), 0U);
    sig.emit(5);
    EXPECT_EQ(global_indicator.count(5), 4U);
    EXPECT_EQ(global_indicator.size(), 4U);
  }

  global_indicator.clear();
  sig.emit(10);
  EXPECT_EQ(global_indicator.size(), 0U);

  // Test disconnection without associated object
  global_indicator.clear();
  class_t object;
  EXPECT_NE(sig.connect(&object, function), 0U);
  EXPECT_NE(sig.connect(&object, functor{ 2 }), 0U);
  EXPECT_TRUE(sig.disconnect(function));
  EXPECT_TRUE(sig.disconnect(functor{ 2 }));
  sig.emit(10);
  EXPECT_EQ(global_indicator.size(), 0U);
}

TYPED_TEST(SignalsTest, BlockSignal) {
  using signal_t = typename TestFixture::signal_int_t;
  using class_t = typename TestFixture::cxt_class_t;
  double ballast = 2.0; // Prevent lambda from turning into function
  auto lambda = [ballast](int arg) {
    global_indicator(arg);
  };
  class_t object;
  signal_t sig;

  global_indicator.clear();
  EXPECT_NE(sig.connect(function), 0U);
  EXPECT_NE(sig.connect(functor{ 1 }), 0U);
  EXPECT_NE(sig.connect(lambda), 0U);
  EXPECT_NE(sig.connect(&object, &class_t::method), 0U);
  sig.emit(5);
  EXPECT_EQ(global_indicator.count(5), 4U);
  EXPECT_EQ(global_indicator.size(), 4U);

  global_indicator.clear();
  EXPECT_FALSE(sig.block()); // was not blocked prior to the call
  EXPECT_TRUE(sig.blocked());
  sig.emit(10);
  EXPECT_EQ(global_indicator.size(), 0U);

  global_indicator.clear();
  EXPECT_TRUE(sig.block(false)); // was blocked prior to the call
  EXPECT_FALSE(sig.blocked());
  sig.emit(10);
  EXPECT_EQ(global_indicator.count(10), 4U);
  EXPECT_EQ(global_indicator.size(), 4U);
}

TYPED_TEST(SignalsTest, DisconnectAllSlots) {
  using signal_t = typename TestFixture::signal_int_t;
  using class_t = typename TestFixture::cxt_class_t;
  double ballast = 2.0; // Prevent lambda from turning into function
  auto lambda = [ballast](int arg) {
    global_indicator(arg);
  };
  class_t object;
  signal_t sig;

  // Total disconnection for 1 slot
  global_indicator.clear();
  EXPECT_NE(sig.connect(function), 0U);
  sig.disconnect();
  sig.emit(5);
  EXPECT_EQ(global_indicator.size(), 0U);

  // Total disconnection for multiple slots
  global_indicator.clear();
  EXPECT_NE(sig.connect(function), 0U);
  EXPECT_NE(sig.connect(functor{ 1 }), 0U);
  EXPECT_NE(sig.connect(lambda), 0U);
  EXPECT_NE(sig.connect(&object, &class_t::method), 0U);
  sig.disconnect();
  sig.emit(5);
  EXPECT_EQ(global_indicator.size(), 0U);
}

TYPED_TEST(SignalsTest, SlotSelfDisconnection) {
  using signal_t = typename TestFixture::signal_int_t;
  signal_t sig;

  struct self_disconnect {
    self_disconnect(signal_t & s)
      : sig_{ s }
    {}
    void operator()(int arg) {
      global_indicator(arg);
      sig_.disconnect(*this);
    }
    bool operator==(const self_disconnect &) const noexcept {
      return true;
    }
    signal_t & sig_;
  };

  global_indicator.clear();
  EXPECT_NE(sig.connect(self_disconnect{ sig }), 0U);
  EXPECT_NE(sig.connect([](int arg) {
    global_indicator(arg);
  }), 0U);
  sig.emit(5);
  EXPECT_EQ(global_indicator.count(5), 2U);
  EXPECT_EQ(global_indicator.size(), 2U);

  global_indicator.clear();
  sig.emit(5);
  EXPECT_EQ(global_indicator.count(5), 1U);
  EXPECT_EQ(global_indicator.size(), 1U);
}

TYPED_TEST(SignalsTest, SignalSelfDestruction) {
  using signal_t = typename TestFixture::signal_int_t;
  auto sig = std::make_unique<signal_t>();

  global_indicator.clear();
  ASSERT_TRUE(sig);
  EXPECT_NE(sig->connect([&sig](int arg) {
    global_indicator(arg);
    sig.reset();
  }), 0U);

  sig->emit(5);
  EXPECT_EQ(global_indicator.count(5), 1U);
  EXPECT_EQ(global_indicator.size(), 1U);
  ASSERT_FALSE(sig);
}

TYPED_TEST(SignalsTest, NonVoidNoArgsSlots) {
  using signal_t = typename TestFixture::signal_no_args_t;
  using class_t = typename TestFixture::cxt_class_t;
  double ballast = 2.0; // Prevent lambda from turning into function
  auto lambda = [ballast]()->double { global_indicator(100); return 2.0; };
  class_t object;
  signal_t sig;

  global_indicator.clear();
  EXPECT_NE(sig.connect(function_non_void_no_args), 0U);
  EXPECT_NE(sig.connect(functor{ 1 }), 0U);
  auto id = sig.connect(lambda);
  EXPECT_NE(id, 0U);
  EXPECT_NE(sig.connect(&object, &class_t::method_non_void_no_args), 0U);
  sig.emit();
  EXPECT_EQ(global_indicator.count(100), 4U);
  EXPECT_EQ(global_indicator.size(), 4U);

  global_indicator.clear();
  EXPECT_TRUE(sig.disconnect(function_non_void_no_args));
  EXPECT_TRUE(sig.disconnect(functor{ 1 }));
  EXPECT_TRUE(sig.disconnect(id));
  EXPECT_TRUE(sig.disconnect(&object, &class_t::method_non_void_no_args));
  sig.emit();
  EXPECT_EQ(global_indicator.size(), 0U);
}

TYPED_TEST(SignalsTest, MultiArgsSlots) {
  using signal_t = typename TestFixture::signal_int_int_t;
  using class_t = typename TestFixture::cxt_class_t;
  double ballast = 2.0; // Prevent lambda from turning into function
  auto lambda = [ballast](int arg1, int arg2) {
    global_indicator(arg1 + arg2);
  };
  class_t object;
  signal_t sig;

  global_indicator.clear();
  EXPECT_NE(sig.connect(function_multi_args), 0U);
  EXPECT_NE(sig.connect(functor{ 1 }), 0U);
  auto id = sig.connect(lambda);
  EXPECT_NE(id, 0U);
  EXPECT_NE(sig.connect(&object, &class_t::method_multi_args), 0U);
  sig.emit(5, 10);
  EXPECT_EQ(global_indicator.count(15), 4U);
  EXPECT_EQ(global_indicator.size(), 4U);

  global_indicator.clear();
  EXPECT_TRUE(sig.disconnect(function_multi_args));
  EXPECT_TRUE(sig.disconnect(functor{ 1 }));
  EXPECT_TRUE(sig.disconnect(id));
  EXPECT_TRUE(sig.disconnect(&object, &class_t::method_multi_args));
  sig.emit(5, 10);
  EXPECT_EQ(global_indicator.size(), 0U);
}

TYPED_TEST(SignalsTest, ArgCopyCount) {
  using signal_cc_t = typename TestFixture::signal_cc_t;
  using signal_cc_ref_t = typename TestFixture::signal_cc_ref_t;
  unsigned int count = 0;
  copy_counter arg{};

  signal_cc_t sig1;
  sig1.connect([&count](copy_counter arg) {
    count = arg.get();
  });
  sig1.emit(arg);
  EXPECT_LE(count, 2U);

  signal_cc_ref_t sig2;
  sig2.connect([&count](copy_counter arg) {
    count = arg.get();
  });
  sig2.emit(arg);
  EXPECT_LE(count, 1U);

  signal_cc_ref_t sig3;
  sig3.connect([&count](copy_counter & arg) {
    count = arg.get();
  });
  sig3.emit(arg);
  EXPECT_EQ(count, 0U);
}

//================ Tests for multithreaded version ================//

TEST(SignalTest, CrossThreadSlotCall) {
  using class_t = test_class<vdk::context>;
  vdk::signal<void(int)> sig;

  global_indicator.clear();
  test_thread thread{ [&sig]() {
    double ballast = 2.0; // Prevent lambda from turning into function
    auto lambda = [ballast](int arg) {
      global_indicator(arg);
    };
    class_t object; // associated object
    sig.connect(&object, function);
    sig.connect(&object, functor{ 1 });
    sig.connect(&object, lambda);
    sig.connect(&object, &class_t::method);
    test_thread::set_flag(true);
    int count = 0;
    while (count != 12) if (vdk::signals_execute()) ++count;
  } };
  auto thread_id = thread.get_id();
  sig.emit(5);
  sig.emit(10);
  sig.emit(15);
  thread.wait_thread();

  EXPECT_EQ(global_indicator.size(), 12U);
  EXPECT_EQ(global_indicator.count(5), 4U);
  EXPECT_EQ(global_indicator.count(10), 4U);
  EXPECT_EQ(global_indicator.count(15), 4U);

  for (auto & e : global_indicator) {
    EXPECT_EQ(e.second, thread_id);
  }
}

TEST(SignalTest, SyncSlotCall) {
  // Test that exec::sync forces to call a slot synchronously,
  // even if the target object lives in another thread
  using class_t = test_class<vdk::context>;
  vdk::signal<void(int)> sig;

  global_indicator.clear();
  test_thread thread{ [&sig]
    {
      class_t object;
      sig.connect(&object, &class_t::method, vdk::exec::sync);
      test_thread::set_flag(true);
      while (test_thread::get_flag()) vdk::signals_execute();
    } };
  sig.emit(5);
  std::this_thread::yield();
  test_thread::set_flag(false);
  thread.wait_thread();

  EXPECT_EQ(global_indicator.size(), 1U);
  EXPECT_EQ(global_indicator.count(5), 1U);
  auto iter = global_indicator.find(5);
  ASSERT_NE(iter, global_indicator.end());
  EXPECT_EQ(iter->second, std::this_thread::get_id());
}

TEST(SignalTest, AsyncSlotCall) {
  // Test that exec::async forces to call a slot asynchronously,
  // even if the target object lives in the same thread
  using class_t = test_class<vdk::context>;
  class_t object;
  vdk::signal<void(int)> sig;

  global_indicator.clear();
  EXPECT_NE(sig.connect(&object, &class_t::method, vdk::exec::async), 0U);
  sig.emit(5);
  // Slot is async, so it has not been executed synchronously
  // Instead, it has been transferred into the current thread's queue
  EXPECT_EQ(global_indicator.size(), 0U);

  // Critical place!
  // Returns true iff slot call has been executed asynchronously
  ASSERT_TRUE(vdk::signals_execute());

  EXPECT_EQ(global_indicator.size(), 1U);
  EXPECT_EQ(global_indicator.count(5), 1U);
  auto iter = global_indicator.find(5);
  ASSERT_NE(iter, global_indicator.end());
  EXPECT_EQ(iter->second, std::this_thread::get_id());
}

TEST(SignalTest, CrossThreadArgCopyCount) {
  using class_t = test_class<vdk::context>;
  unsigned int count = 0;
  copy_counter arg{};

  vdk::signal<void(copy_counter)> sig1;
  test_thread thread1{ [&sig1, &count]
    {
      class_t object;
      sig1.connect(&object,
      [&count](copy_counter arg) {
        count = arg.get();
      });
      test_thread::set_flag(true);
      while (!vdk::signals_execute());
    } };
  sig1.emit(arg);
  thread1.wait_thread();
  EXPECT_LE(count, 3U);

  vdk::signal<void(copy_counter&)> sig2;
  test_thread thread2{ [&sig2, &count]
    {
      class_t object;
      sig2.connect(&object,
      [&count](copy_counter arg) {
        count = arg.get();
      });
      test_thread::set_flag(true);
      while (!vdk::signals_execute());
    } };
  sig2.emit(arg);
  thread2.wait_thread();
  EXPECT_LE(count, 2U);

  vdk::signal<void(copy_counter&)> sig3;
  test_thread thread3{ [&sig3, &count]
    {
      class_t object;
      sig3.connect(&object,
      [&count](copy_counter & arg) {
        count = arg.get();
      });
      test_thread::set_flag(true);
      while (!vdk::signals_execute());
    } };
  sig3.emit(arg);
  thread3.wait_thread();
  EXPECT_LE(count, 1U);
}

int main(int argc, char ** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}