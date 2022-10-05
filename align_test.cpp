#include <gtest/gtest.h>

#include <sps/align_test.hpp>
#include <cstring>
#include <iostream>

TEST(align_test, wrong_alignment_throw) {
  try {
    SPS_ALIGNAS(32) float arr[32];
    memset(&arr[0], 0, 32*sizeof(float));
    SPS_ALIGN_TEST(&arr[1], 32);
    ASSERT_TRUE(false);
  } catch (std::exception& e) {
    std::cout << e.what() << std::endl;
    ASSERT_TRUE(true);
  }
}

TEST(align_test, right_alignment_accept) {
  SPS_ALIGNAS(32) float arr[32];
  try {
    SPS_ALIGN_TEST(&arr[0], 32);
  } catch(std::exception& e) {
    ASSERT_TRUE(false);
  }
}

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
