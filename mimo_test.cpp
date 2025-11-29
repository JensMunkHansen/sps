/**
 * @file   mimo_test.cpp
 * @author Jens Munk Hansen <jmh@debian9laptop.parknet.dk>
 * @date   Sun Apr  8 13:37:40 2018
 *
 * @brief
 *
 *
 */

#include <gtest/gtest.h>
#include <sps/cenv.h>

#include <iostream>

#include <chrono>
#include <cstdio>
#include <mutex>
#include <random>
#include <thread>

#include <sps/mimo.hpp>

#if 0 // defined(__GNUC__) && !defined(__CYGWIN__)
#include <sps/strace.hpp>
void Init()     __attribute__((constructor(101)));

void Init() {
#if !defined(NDEBUG)
  sps::STrace::Instance().Enable();
#endif
}
#endif

static void thread_push(void* arg)
{
  auto pQueue = static_cast<sps::MRMWQueue<float>*>(arg);
  for (size_t i = 0; i < 100; i++)
  {
    float value = float(i);
    pQueue->push(value);
  }
}

static void thread_try_pop(void* arg)
{
  auto pQueue = static_cast<sps::MRMWQueue<float>*>(arg);
  float value = 0.0f;
  for (size_t i = 0; i < 100; i++)
  {
    pQueue->try_pop(value);
  }
}

TEST(mimo_test, siso_queue_copyable)
{
  sps::MRMWQueue<float> queue;
  for (size_t i = 0; i < 10; i++)
  {
    queue.push(std::move(float(i)));
  }
  for (size_t i = 0; i < 10; i++)
  {
    float val = 0.0f;
    queue.try_pop(val);
  }
  EXPECT_EQ(1, 1);
}

TEST(mimo_test, siso_queue)
{
  sps::MRMWQueue<float> queue;
  std::thread first(thread_push, static_cast<void*>(&queue));
  std::this_thread::sleep_for(std::chrono::milliseconds{ 10 });
  first.join(); // pauses until first finishes

  std::thread second(thread_try_pop, static_cast<void*>(&queue));
  second.join(); // pauses until second finishes
  EXPECT_EQ(1, 1);
}

TEST(mimo_test, circular_buffer)
{
  sps::MRMWCircularBuffer<int, 10> circularBuffer;
  circularBuffer.push(1);
  circularBuffer.push(2);
  int val = 0;
  circularBuffer.pop(val);
  EXPECT_EQ(val, 1);
  circularBuffer.pop(val);
  EXPECT_EQ(val, 2);
}

// Add two threads
static void thread_push_circular(void* arg, size_t count)
{
  auto pQueue = static_cast<sps::MRMWCircularBuffer<int, 10>*>(arg);
  for (size_t i = 0; i < count; i++)
  {
    int value = int(i);
    pQueue->push(value);
  }
}

static void thread_try_pop_circular(void* arg, size_t count)
{
  auto pQueue = static_cast<sps::MRMWCircularBuffer<int, 10>*>(arg);
  int value = 0;
  for (size_t i = 0; i < count; i++)
  {
    pQueue->try_pop(value);
    // Important - if not popping, the pusher will hang and wait
    std::this_thread::sleep_for(std::chrono::milliseconds{ 10 });
  }
}

TEST(mimo_test, circular_buffer2)
{
  sps::MRMWCircularBuffer<int, 10> queue;
  std::thread first(thread_push_circular, static_cast<void*>(&queue), 100);
  std::this_thread::sleep_for(std::chrono::milliseconds{ 10 });
  std::thread second(thread_try_pop_circular, static_cast<void*>(&queue), 100);
  first.join();
  second.join();
  EXPECT_EQ(1, 1);
}

TEST(mimo_test, circular_buffer3)
{
  sps::MRMWCircularBuffer<int, 10> queue;
  std::thread first(thread_push_circular, static_cast<void*>(&queue), 100);
  queue.invalidate();
  first.join();
  EXPECT_EQ(1, 1);
}

#if 0
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
  // TODO: Fix this using sps::thread
  delete pQueue;

  std::this_thread::sleep_for(std::chrono::milliseconds{500});

  second.join();
  EXPECT_EQ(1, 1);
}
#endif

int main(int argc, char* argv[])
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
