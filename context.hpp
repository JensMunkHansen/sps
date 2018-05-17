/**
 * @file   context.hpp
 * @author Jens Munk Hansen <jmh@debian9laptop.parknet.dk>
 * @date   Tue Feb 27 00:46:20 2018
 *
 * @brief
 *
 * Copyright 2017 Jens Munk Hansen
 */
#pragma once

#include <sps/cenv.h>
#include <atomic>
#include <sps/contextif.hpp>

#ifdef _WIN32
# include <windows.h>
#endif
namespace sps {

class ThreadPool;

class Context : public ContextIF {
 public:
  static int Create(ContextIF** ppContext);
  static int Destroy(ContextIF* pContext);

  Context(Context&& other) = default;
  Context& operator=(Context&& other) = default;
  ~Context() SPS_OVERRIDE = default;
  Context();

  /** @name Used by implementation of Event
   *
   */
  ///@{

  static ThreadPool* DefaultThreadPoolGet();

  ThreadPool* ThreadPoolGet();
  ///@}

 private:
  static std::atomic<ThreadPool*> g_threadpool;

#if defined(__GNUC__)
  static void ThreadPoolInit()     __attribute__((constructor(101)));
  static void ThreadPoolDestroy()  __attribute__((constructor(101)));
#elif defined(_WIN32)
  static void ThreadPoolInit();
 public:
  static void ThreadPoolDestroy();
 private:
  friend BOOL APIENTRY DllMain(HANDLE hModule, DWORD  ulReasonForCall, LPVOID lpReserved);
#endif

  Context(const Context& rhs) = delete;
  Context& operator=(const Context& rhs) = delete;

  ThreadPool* m_threadpool;
  uint32_t m_id;
};
}  // namespace sps

/* Local variables: */
/* indent-tabs-mode: nil */
/* tab-width: 2 */
/* c-basic-offset: 2 */
/* End: */
