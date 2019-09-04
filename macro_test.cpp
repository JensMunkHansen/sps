#include <gtest/gtest.h>

#include <sps/cenv.h>

template <typename T>
T tplFuncImpl0(const T& input, int k) {
  T output = T(0.0);
  for (int i = 0 ; i < k ; i++) {
    output += input;
  }
  return output;
}

SPS_ALIAS_TEMPLATE_FUNCTION(tplFunc, tplFuncImpl0)

TEST(macro_test, tpl_alias) {
  float input = 2.0f;
  float output = tplFunc(input, 2);
  SPS_UNREFERENCED_PARAMETERS(input, output);
}

float funcImpl0(const float& input, int k) {
  float output = 0.0f;
  for (int i = 0 ; i < k ; i++) {
    output += input;
  }
  return output;
}

SPS_ALIAS_FUNCTION(func, funcImpl0)

TEST(macro_test, func_alias) {
  float input = 2.0f;
  float output = func(input, 2);
  SPS_UNREFERENCED_PARAMETERS(input, output);
}

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

/* Local variables: */
/* indent-tabs-mode: nil */
/* tab-width: 2 */
/* c-basic-offset: 2 */
/* End: */

