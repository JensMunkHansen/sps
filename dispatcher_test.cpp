#include <sps/cenv.h>
#include <gtest/gtest.h>

#include <sps/dispatcher.hpp>

TEST(dispatcher_test, test_nothing) {
  EXPECT_EQ(1, 1);
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

TEST(dispatcher_test, dispatcher_add_calback) {
  sps::Context context = sps::Context();
  sps::DispatcherEvent dispatcherEvent(&context);
  sps::EventListener evListener(&context);
  MyUserData myData;
  strncpy(myData.msg, "Hello World\0", 12);

  evListener.RegisterCallback(
  [](CallbackPtr pCallback, void* pUserData) {
    (*pCallback)(pUserData);
  }, &MyCallbackFunction, &myData);

  evListener.m_callbacks.begin()->get()->Execute();
  EXPECT_EQ(1, 1);
}

TEST(dispatcher_test, dispatcher_exec_calback) {
  sps::Context context = sps::Context();
  sps::DispatcherEvent dispatcherEvent(&context);
  sps::EventListener evListener(&context);
  dispatcherEvent.ListenerAdd(&evListener);

  MyUserData myData;
  strncpy(myData.msg, "Hello World\0", 12);
  evListener.RegisterCallback(
  [](CallbackPtr pCallback, void* pUserData) {
    (*pCallback)(pUserData);
  }, &MyCallbackFunction, &myData);

  dispatcherEvent.Signal();
  EXPECT_EQ(1, 1);
}



int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
