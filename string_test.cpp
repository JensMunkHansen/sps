#include <gtest/gtest.h>

#include <sps/string>

TEST(string_test, format_test)
{
  std::string s = "a";
  const std::string cs = "b";
  const char* cc = "c";

  int i = 1;
  const int i2 = 2;

  using namespace std::literals;

  std::string result =
    sps::string_format("%s %s %s %s %s %d %d %d \n", s, cs, cc, "d", "e"s, i, i2, 3);
  EXPECT_EQ(1, 1);
}

int main(int argc, char* argv[])
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
