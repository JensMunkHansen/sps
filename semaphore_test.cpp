/**
 * @file   semaphore_test.cpp
 * @author Jens Munk Hansen <jens.munk.hansen@gmail.com>
 * @date   Thu Nov 23 19:19:18 2017
 *
 * @brief
 *
 * Copyright 2017 Jens Munk Hansen
 */

#include <gtest/gtest.h>
#include <thread>

#include <sps/semaphore.hpp>


TEST(semaphore_test, test_nothing) {
  EXPECT_EQ(0, 0);
}

sps::Semaphore mySemaphore(0);

static void thread0() {
  mySemaphore.Wait();
}

static void thread1() {
  mySemaphore.Signal();
}

static void thread2() {
  mySemaphore.Wait(std::chrono::milliseconds(100));
}

TEST(semaphore_test, synchronize) {
  std::thread first(thread0);
  std::thread second(thread1);
  first.join();  // pauses until second signals
  second.join();
  EXPECT_EQ(1, 1);
}

TEST(semaphore_test, synchronize_wait) {
  std::thread first(thread2);
  std::thread second(thread1);
  first.join();  // pauses until second signals
  second.join();
  EXPECT_EQ(1, 1);
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
