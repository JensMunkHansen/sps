#include <gtest/gtest.h>
#include <sps/default.hpp>

int f1(int, sps::Defaulted<int, 42> y, int)
{
  return y.get_or_default();
}

TEST(Default_value, Defaulted)
{
  ASSERT_EQ(f1(0, sps::defaultValue, 1), 42);
}

TEST(Value_provided, Defaulted)
{
  ASSERT_EQ(f1(0, 43, 1), 43);
}

std::string f2(int, sps::Defaulted<std::string> y, int)
{
  return y.get_or_default();
}

TEST(Default_constructor, Defaulted)
{
  ASSERT_EQ(f2(0, sps::defaultValue, 1), std::string());
}

TEST(Default_constructor_value_provided, Defaulted)
{
  ASSERT_EQ(f2(0, std::string("hello"), 1), "hello");
}

struct GetDefaultAmount
{
  static double get() { return 45.6; }
};

double f3(int x, sps::DefaultedF<double, GetDefaultAmount> y, int z)
{
  SPS_UNREFERENCED_PARAMETERS(x, z);
  return y.get_or_default();
}

TEST(DefaultedF_Default_value, DefaultedF)
{
  ASSERT_EQ(f3(0, sps::defaultValue, 1), 45.6);
}

TEST(DefaultedF_Value_provided, DefaultedF)
{
  ASSERT_EQ(f3(0, 13.4, 1), 13.4);
}

class CopyLogger
{
public:
  explicit CopyLogger(int& copyCount)
    : copyCount_(copyCount)
  {
  }
  explicit CopyLogger(CopyLogger const& other)
    : copyCount_(other.copyCount_)
  {
    ++copyCount_;
  }

private:
  int& copyCount_;
};

void f4(sps::Defaulted<CopyLogger>) {}

TEST(Defaulted_makes_a_copy, Defaulted)
{
  int copyCount = 0;
  f4(CopyLogger(copyCount));
  ASSERT_EQ(copyCount, 1);
}

void f5(sps::Defaulted<CopyLogger const&>) {}

TEST(Defaulted_const_ref_makes_no_copy, Defaulted)
{
  int copyCount = 0;
  f5(CopyLogger(copyCount));
  ASSERT_EQ(copyCount, 0);
}

struct GetDefaultZ
{
  static int get(int x, int y) { return x + y; }
};

int f6(int x, int y, sps::DefaultedF<int, GetDefaultZ> z)
{
  return z.get_or_default(x, y);
}

TEST(DefaultedF_on_dependent_parameter, Defaulted)
{
  ASSERT_EQ(f6(1, 5, sps::defaultValue), 6);
}

/*
std::string f7(int , sps::Defaulted<std::string, "hello"> y, int ) {
  return y.get_or_default();
}

TEST(Constructor_with_value, Defaulted) {
  ASSERT_EQ(f7(0, sps::defaultValue, 1), std::string("hello"));
}
*/

int main(int argc, char* argv[])
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

/* Local variables: */
/* indent-tabs-mode: nil */
/* tab-width: 2 */
/* c-basic-offset: 2 */
/* End: */
