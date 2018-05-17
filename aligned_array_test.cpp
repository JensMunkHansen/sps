#include <sps/cenv.h>
#include <sps/aligned_array.hpp>

#include <gtest/gtest.h>

using namespace std;

TEST(aligned_array_test, ctor) {
  aligned_array<float, 12> arr1;
  aligned_array<double, 3> arr2;
  SPS_UNREFERENCED_PARAMETERS(arr1, arr2);
}

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
