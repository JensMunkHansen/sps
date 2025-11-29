#define RINGBUFFER_DO_NOT_ALIGN 1

#include "RingBuffer_v1.hpp"

#include <chrono>
#include <cstdlib>
#include <thread>

const int INTEGERS_PER_CALL = 1;
// const int INTEGERS_TO_WRITE = (1 << 30);
const int INTEGERS_TO_WRITE = (1 << 30) / 4;

const int BUFFER_SIZE = (1 << 30);

static void thread_push(void* arg)
{
  auto pQueue = (RingBuffer*)arg;
  for (size_t i = 0; i < INTEGERS_TO_WRITE / INTEGERS_PER_CALL; i++)
  {
    for (size_t j = 0; j < INTEGERS_PER_CALL; j++)
    {
      int value = int(i * INTEGERS_PER_CALL + j);
      pQueue->Write(value);
    }
    pQueue->FinishWrite();
  }
}

static void thread_pop(void* arg)
{
  auto pQueue = (RingBuffer*)arg;
  for (size_t i = 0; i < INTEGERS_TO_WRITE / INTEGERS_PER_CALL; i++)
  {
    for (size_t j = 0; j < INTEGERS_PER_CALL; j++)
    {
      int value = pQueue->Read<int>();
    }
    pQueue->FinishRead();
  }
}

int main()
{
  size_t size = BUFFER_SIZE;
  int* pBuffer = static_cast<int*>(std::aligned_alloc(alignof(int), size));

  RingBuffer queue;
  queue.Initialize((void*)pBuffer, size);

  std::thread first(thread_push, (void*)&queue);
  std::this_thread::sleep_for(std::chrono::milliseconds{ 10 });
  first.join(); // pauses until first finishes

  std::thread second(thread_pop, (void*)&queue);
  second.join(); // pauses until second finishes

  std::free(pBuffer);
  return 0;
}
