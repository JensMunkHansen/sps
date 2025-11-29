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

#include <sps/siso.hpp>

#if 0 // defined(__GNUC__) && !defined(__CYGWIN__)
#include <sps/strace.hpp>
void Init()     __attribute__((constructor(101)));

void Init() {

#if !defined(NDEBUG)
  sps::STrace::Instance().Enable();
#endif
}
#endif

// Must be minimum 2
#define RING_BUFFER_SZ 2

#define NUM_ELEMENTS 100

static void thread_push(void* arg)
{
  auto pQueue = static_cast<sps::SRSWRingBuffer<float, RING_BUFFER_SZ>*>(arg);
  size_t i = 0;
  while (i < NUM_ELEMENTS)
  {
    float value = float(i);
    pQueue->push(value);
    i++;
  }
}

static void thread_pop(void* arg)
{
  auto pQueue = static_cast<sps::SRSWRingBuffer<float, RING_BUFFER_SZ>*>(arg);
  size_t i = 0;
  float value;
  while (i < NUM_ELEMENTS)
  {
    value = pQueue->pop();
    SPS_UNREFERENCED_PARAMETER(value);
    i++;
  }
}

TEST(siso_test, siso_circular_buffer)
{
  sps::SRSWRingBuffer<float, RING_BUFFER_SZ> queue;
  std::thread first(thread_push, static_cast<void*>(&queue));
  std::thread second(thread_pop, static_cast<void*>(&queue));

  first.join();  // pauses until first finishes
  second.join(); // pauses until second finishes
  EXPECT_EQ(1, 1);
}

int main(int argc, char* argv[])
{

  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
