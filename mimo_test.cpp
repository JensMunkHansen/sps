/**
 * @file   mimo_test.cpp
 * @author Jens Munk Hansen <jmh@debian9laptop.parknet.dk>
 * @date   Sun Apr  8 13:37:40 2018
 *
 * @brief
 *
 *
 */

#include <sps/cenv.h>
#include <gtest/gtest.h>

#include <iostream>

#include <chrono>
#include <random>
#include <thread>
#include <mutex>
#include <cstdio>

#include <sps/mimo.hpp>

#if defined(__GNUC__) && !defined(__CYGWIN__)
#include <sps/strace.hpp>
void Init()     __attribute__((constructor(101)));

void Init() {
# if !defined(NDEBUG)
  sps::STrace::Instance().Enable();
# endif
}
#endif

static void thread_push(void* arg) {
  auto pQueue = (sps::MRMWQueue<float>*) arg;
  for (size_t i = 0 ; i < 100 ; i++) {
    float value = float(i);
    pQueue->push(value);
  }
}

static void thread_try_pop(void* arg) {
  auto pQueue = (sps::MRMWQueue<float>*) arg;
  float value = 0.0f;
  for (size_t i = 0 ; i < 100 ; i++) {
    pQueue->try_pop(value);
  }
}

TEST(mimo_test, siso_queue_copyable) {
  sps::MRMWQueue<float> queue;
  for (size_t i = 0 ; i < 10 ; i++) {
    queue.push(std::move(float(i)));
  }
  for  (size_t i = 0 ; i < 10 ; i++) {
    float val = 0.0f;
    queue.try_pop(val);
  }
  EXPECT_EQ(1, 1);
}

TEST(mimo_test, siso_queue) {
  sps::MRMWQueue<float> queue;
  std::thread first (thread_push, (void*) &queue);
  std::this_thread::sleep_for(std::chrono::milliseconds{10});
  first.join();                // pauses until first finishes

  std::thread second (thread_try_pop, (void*) &queue);
  second.join();               // pauses until second finishes
  EXPECT_EQ(1, 1);
}

TEST(mimo_test, circular_buffer) {
  sps::MRMWCircularBuffer<int, 10> circularBuffer;
  circularBuffer.push(1);
  circularBuffer.push(2);
  int val = 0;
  circularBuffer.pop(val);
  EXPECT_EQ(val, 1);
  circularBuffer.pop(val);
  EXPECT_EQ(val, 2);
}

#if 1
static void thread_pop(void* arg) {
  auto pQueue = (sps::MRMWQueue<float>*) arg;
  float value = 0.0f;
  for (size_t i = 0 ; i < 100 ; i++) {
    pQueue->pop(value);
  }
}

TEST(mimo_test, dtor) {
  sps::MRMWQueue<float>* pQueue = new sps::MRMWQueue<float>();
  std::thread second = std::thread(thread_pop, (void*) pQueue);
  delete pQueue;

  std::this_thread::sleep_for(std::chrono::milliseconds{100});

  second.join();
  EXPECT_EQ(1, 1);
}
#endif

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
