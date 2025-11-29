/**
 * @file   context.cpp
 * @author Jens Munk Hansen <jmh@debian9laptop.parknet.dk>
 * @date   Mon Mar  5 21:03:35 2018
 *
 * @brief
 *
 * Copyright 2017 Jens Munk Hansen
 *
 */

#include <cstdio>
#include <mutex>

#include <sps/context.hpp>
#include <sps/resource.hpp>
#include <sps/threadpool.hpp>

#if 0 // defined(__GNUC__)
#if !defined(__CYGWIN__)
#include <sps/strace.hpp>
#endif
#endif

#if defined(_WIN32)
BOOL APIENTRY DllMain(HANDLE hModule, DWORD ulReasonForCall, LPVOID lpReserved)
{
  SPS_UNREFERENCED_PARAMETERS(hModule, ulReasonForCall, lpReserved);

  switch (ulReasonForCall)
  {
    case DLL_PROCESS_ATTACH:
      break;
    case DLL_THREAD_ATTACH:
      break;
    case DLL_THREAD_DETACH:
      break;
    case DLL_PROCESS_DETACH:
      sps::Context::ThreadPoolDestroy();
      break;
  }
  return TRUE;
}
#endif

namespace sps
{

std::mutex g_mutex;

std::atomic<ThreadPool*> Context::g_threadpool{ nullptr };

void Context::ThreadPoolInit()
{
#if 0 // defined(__GNUC__) && !defined(__CYGWIN__)
#if !defined(NDEBUG)
  sps::STrace::Instance().Enable();
#endif
#endif
}

void Context::ThreadPoolDestroy()
{
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4099)
#endif
  if (sps::Context::g_threadpool)
  {
    delete sps::Context::g_threadpool;
  }
#ifdef _MSC_VER
#pragma warning(pop)
#endif
}

ThreadPool* Context::DefaultThreadPoolGet()
{
  ThreadPool* pThreadPool = g_threadpool.load(std::memory_order_acquire);
  if (!pThreadPool)
  {
    std::lock_guard<std::mutex> guard(g_mutex);
    pThreadPool = g_threadpool.load(std::memory_order_relaxed);
    if (!pThreadPool)
    {
      pThreadPool = new ThreadPool(1);
      g_threadpool.store(pThreadPool, std::memory_order_release);
    }
  }
  return pThreadPool;
}

ThreadPool* Context::ThreadPoolGet()
{
  return m_threadpool;
}

int Context::Create(ContextIF** ppContext)
{
  *ppContext = new Context();
  return 0;
}

int Context::Destroy(ContextIF* pContext)
{
  if (pContext)
  {
    delete pContext;
  }
  return 0;
}

Context::Context()
{
  m_threadpool = Context::DefaultThreadPoolGet();
  m_id = Resource::UIDCreate();
}
} // namespace sps
