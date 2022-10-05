#include <gtest/gtest.h>

#include <sps/align_test.hpp>
#include <cstring>

TEST(aligned_test, test_alignment) {
  try {
    SPS_ALIGNAS(32) float arr[32];
    memset(&arr[0], 0, 32*sizeof(float));
    SPS_ALIGN_TEST(&arr[1], 32);
    ASSERT_TRUE(false);
  }
  catch (std::exception& e)
  {
    ASSERT_TRUE(true);
  }
}

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
