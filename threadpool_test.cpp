#include <sps/cenv.h>
#include <gtest/gtest.h>

#include <iostream>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <atomic>
#include <random>

#include <sps/threadpool.hpp>

#include <sps/if_threadpool.hpp>

TEST(threadpool_test, test_nothing) {
  EXPECT_EQ(1, 1);
}

class SomeObject {
 public:
  float value;
  int someInteger;
  explicit SomeObject(int i = 0) : someInteger(i) {}
  int MemberFunction(float a) {
    someInteger = 2;
    SPS_UNREFERENCED_PARAMETER(a);
    return 0;
  }
};

void FunctionReadingObject(const SomeObject& object) {
  SPS_UNREFERENCED_PARAMETER(object);
}

int FunctionReadingAndReturningVariables(int a, float b) {
  return a + static_cast<int>(b);
}

typedef struct MyUserData {
  float fValue;
  char msg[12];
} MyUserData;

typedef void (*CallbackPtr)(void* pUserData);

void MyCallbackFunction(void* pUserData) {
  MyUserData* pMyUserData = static_cast<MyUserData*>(pUserData);
  std::cout << pMyUserData->msg << std::endl;
}

/**
 * Test queuing tasks with various signatures
 *
 */
TEST(threadpool_test, test_signatures) {
  // Test object with an identifier 2
  SomeObject obj(1);

  // Function using object (preferred usage - nothing is captured except arguments to submitJob)
  auto taskFuture0 =
  sps::thread::defaultpool::submitJob([](const SomeObject& object) {
    FunctionReadingObject(object);
  }, std::cref(obj));

  // Detach future (call is deferred)
  taskFuture0.Detach();

  auto taskFuture1 =
  sps::thread::defaultpool::submitJob([](int a, float b)->int {
    return FunctionReadingAndReturningVariables(a, b);
  }, 5, 10.0f);

  // Wait for job to finish
  auto bla = taskFuture1.Get();

  EXPECT_EQ(bla, 15);

  auto taskFuture2 =
  sps::thread::defaultpool::submitJob([&](float a)->int {
    return obj.MemberFunction(a);}, 2.0f);
  taskFuture2.Get();

  EXPECT_EQ(obj.someInteger, 2);

  MyUserData myData;
  strncpy(myData.msg, "Hello World\0", 12);

  auto taskFuture3 =
  sps::thread::defaultpool::submitJob([](CallbackPtr pCallback, void* pUserData) {
    (*pCallback)(pUserData);
  }, &MyCallbackFunction, &myData);
}

/**
 * Short task executes in 10 milliseconds
 *
 * @return
 */
int shortTask() {
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  return 1;
}

/**
 * Long task executes in 100 milliseconds
 *
 *
 * @return
 */
int longTask() {
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  return 1;
}

/**
 * Test that adding a task, while a long task is executing works. No starvation
 *
 */
TEST(threadpool_test, test_no_starvation) {
  // No tasks are executed
  int shortTaskExecuted = 0;
  int longTaskExecuted = 0;

  // Pool with single thread
  sps::ThreadPool myPool(1);

  // Pointer to show if task has been executed
  int* pShortTaskExecuted = &shortTaskExecuted;
  int* pLongTaskExecuted = &longTaskExecuted;

  auto taskFuture0 =
    myPool.submit([=]()->void {*pLongTaskExecuted = longTask();});
  auto taskFuture1 =
    myPool.submit([=]()->void {*pShortTaskExecuted = shortTask();});

  taskFuture0.Get();
  taskFuture1.Get();

  EXPECT_EQ(longTaskExecuted, 1);
  EXPECT_EQ(shortTaskExecuted, 1);
}

/**
 * Test that adding a task, while a long task is executing works,
 * when tasks are detached (deferred). Both task must complete if
 * we wait long enough before tearing down threadpool.
 *
 */
TEST(threadpool_test, test_no_starvation_detached) {
  // No tasks are executed
  int shortTaskExecuted = 0;
  int longTaskExecuted = 0;

  // Pool with single thread
  sps::ThreadPool myPool(1);

  // Pointer to show if task has been executed
  int* pShortTaskExecuted = &shortTaskExecuted;
  int* pLongTaskExecuted = &longTaskExecuted;

  auto taskFuture0 =
    myPool.submit([=]()->void {*pLongTaskExecuted = longTask();});
  taskFuture0.Detach();

  auto taskFuture1 =
    myPool.submit([=]()->void {*pShortTaskExecuted = shortTask();});
  taskFuture1.Detach();

  // Wait enough time
  std::this_thread::sleep_for(std::chrono::milliseconds(200));

  EXPECT_EQ(longTaskExecuted, 1);
  EXPECT_EQ(shortTaskExecuted, 1);
}

/**
 * Test that pending jobs are skipped correctly when tearing down a threadpool
 * before a task is executed. It is expected that only the first task is executed.
 *
 */
TEST(threadpool_test, test_teardown_pending_jobs) {
  // No tasks are executed
  int longTask0Executed = 0;
  int longTask1Executed = 0;

  // Thread pool is destroyed when exiting this scope. Exit is made
  // before task1 is started (on purpose)
  {
    // Pool with single thread
    sps::ThreadPool myPool(1);

    // Pointer to show if task has been executed
    int* pLongTask0Executed = &longTask0Executed;
    int* pLongTask1Executed = &longTask1Executed;

    auto taskFuture0 =
      myPool.submit([=]()->void {*pLongTask0Executed = longTask();});
    auto taskFuture1 =
      myPool.submit([=]()->void {*pLongTask1Executed = longTask();});
    taskFuture0.Detach();
    taskFuture1.Detach();

    // Wait enough time for first task to start,
    // but not enough for second task to start
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
  }

  EXPECT_EQ(longTask0Executed, 1);
  // Task 1 should never complete. The threadpool is destroyed before task 1 is started
  EXPECT_EQ(longTask1Executed, 0);
}

//{
std::condition_variable condition;
std::mutex mutex;
std::atomic<bool> start{false};

static void thread0() {
  std::unique_lock<std::mutex> lock {mutex};
  condition.wait(lock, [&]() {
    return start.load();
  });

  std::random_device rd;   // non-deterministic generator
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> dist(1, 6);

  for (size_t i = 0 ; i < 100 ; i++) {
    auto taskFuture2 =
    sps::thread::defaultpool::submitJob([&](int a, float b) {
      FunctionReadingAndReturningVariables(a, b);
      std::this_thread::sleep_for(std::chrono::milliseconds{dist(gen)});
    }, 5, 10.0f);
  }
}

static void thread1() {
  std::unique_lock<std::mutex> lock {mutex};
  condition.wait(lock, [&]() {
    return start.load();
  });

  std::random_device rd;   // non-deterministic generator
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> dist(1, 6);

  for (size_t i = 0 ; i < 100 ; i++) {
    auto taskFuture2 =
    sps::thread::defaultpool::submitJob([&](int a, float b) {
      FunctionReadingAndReturningVariables(a, b);
      std::this_thread::sleep_for(std::chrono::milliseconds{dist(gen)});
    }, 5, 10.0f);
  }
}

TEST(threadpool_test, two_threads_pushing_tasks) {
  std::thread first(thread0);
  std::thread second(thread1);

  start = true;
  condition.notify_all();

  first.join();                // pauses until first finishes
  second.join();               // pauses until second finishes
  EXPECT_EQ(1, 1);
}
//}

TEST(threadpool_test, interface_test) {
  MyUserData myData;
  strncpy(myData.msg, "Hello World\0", 12);

  sps::IThreadPool* pThreadPool;
  sps::ThreadPoolCreate(&pThreadPool);
  pThreadPool->SubmitJob(&MyCallbackFunction, &myData);
  sps::ThreadPoolDestroy(pThreadPool);
}


int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
