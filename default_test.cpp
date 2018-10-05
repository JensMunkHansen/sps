#include <gtest/gtest.h>
#include <sps/default.hpp>

int f1(int , fluent::Defaulted<int, 42> y, int ) {
    return y.get_or_default();
}

TEST(Default_value, [Defaulted]) {
  REQUIRE(f1(0, fluent::defaultValue, 1) == 42);
}

TEST(Value_provided, [Defaulted]) {
    REQUIRE(f1(0, 43, 1) == 43);
}

std::string f2(int , fluent::Defaulted<std::string> y, int ) {
    return y.get_or_default();
}

TEST(Default_constructor, [Defaulted]) {
  REQUIRE(f2(0, fluent::defaultValue, 1) == std::string());
}

TEST(Default_constructor_value_provided, [Defaulted]) {
  REQUIRE(f2(0, std::string("hello"), 1) == "hello");
}

struct GetDefaultAmount{ static double get(){ return 45.6; } };

double f3(int x, fluent::DefaultedF<double, GetDefaultAmount> y, int z) {
  return y.get_or_default();
}

TEST(DefaultedF_Default_value, [DefaultedF]) {
  REQUIRE(f3(0, fluent::defaultValue, 1) == 45.6);
}

TEST(DefaultedF_Value_provided, [DefaultedF]) {
  REQUIRE(f3(0, 13.4, 1) == 13.4);
}

class CopyLogger {
 public:
  CopyLogger(int& copyCount) : copyCount_(copyCount) {
  }
  CopyLogger(CopyLogger const& other) : copyCount_(other.copyCount_) {
    ++copyCount_;
  }
 private:
    int& copyCount_;
};

void f4(fluent::Defaulted<CopyLogger>){}

#if 0
TEST(Defaulted makes a copy", "[Defaulted]") {
    int copyCount = 0;
    f4(CopyLogger(copyCount));
    REQUIRE(copyCount == 1);
}

void f5(fluent::Defaulted<CopyLogger const&>){}

TEST("Defaulted const ref makes no copy", "[Defaulted]") {
    int copyCount = 0;
    f5(CopyLogger(copyCount));
    REQUIRE(copyCount == 0);
}

struct GetDefaultZ {
    static int get(int x, int y) { return x + y; }
};

int f6(int x, int y, fluent::DefaultedF<int, GetDefaultZ> z) {
    return z.get_or_default(x, y);
}

TEST(DefaultedF on dependent parameter", "[Defaulted]") {
    REQUIRE(f6(1, 5, fluent::defaultValue) == 6);
}
#endif

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

/* Local variables: */
/* indent-tabs-mode: nil */
/* tab-width: 2 */
/* c-basic-offset: 2 */
/* End: */
