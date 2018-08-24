/**
 * @file   sps_threads.hpp
 * @author Jens Munk Hansen <jens.munk.hansen@gmail.com>
 * @date   Thu Jun 16 00:05:13 2016
 *
 * @brief
 *
 * Copyright 2016 Jens Munk Hansen
 */
/*
 *  This file is part of SOFUS.
 *
 *  SOFUS is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  SOFUS is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with SOFUS.  If not, see <http://www.gnu.org/licenses/>.
 */
#pragma once

#include <sps/cenv.h>
#include <sps/cerr.h>
#include <sps/config.h>
#include <stdint.h>

#ifdef _WIN32
# ifndef NOMINMAX
#  define NOMINMAX
# endif
# include <windows.h>
# include <process.h>
# include <io.h>
# if defined(HAVE_PTHREAD_H)
#  include <pthread.h>
# endif
#else
# if defined(HAVE_SIGNAL_H)
#  include <csignal>
# endif
# if defined(HAVE_PTHREAD_H)
#  include <pthread.h>
# endif
# include <unistd.h>
# ifndef __CYGWIN__
#  include <sys/syscall.h>
# endif
#endif

#include <float.h>

#include <thread>

#ifdef __linux__
STATIC_INLINE_BEGIN pid_t gettid(void) {
  pid_t pid;
  CallErr(pid = syscall, (__NR_gettid));
  return pid;
}
#endif

STATIC_INLINE_BEGIN int setcpuid(int cpu_id) {
#ifdef __GNUG__
# ifndef __CYGWIN__
  cpu_set_t set;
  CPU_ZERO(&set);
  CPU_SET(cpu_id, &set);
  CallErrReturn(sched_setaffinity,
                (gettid(), sizeof(cpu_set_t), &set), -1);
# endif
#elif defined(_WIN32)
  HANDLE hThread = GetCurrentThread();
  SetThreadIdealProcessor(hThread, cpu_id);
#endif
  return 0;
}

// TODO(JEM): Move this crap to windows crap

#ifndef EM_INEXACT
# ifdef _MSC_VER
#  define EM_INEXACT _EM_INEXACT
# else
#  define EM_INEXACT 0
# endif
#endif

#ifndef MCW_EM
# ifdef _MSC_VER
#  define MCW_EM _MCW_EM
# else
#  define MCW_EM 0
# endif
#endif

/**
 * Floating point control. It is masking everything except
 *
 * _MCW_EM - exception mask
 *
 * _EM_INVALID
 * _EM_DENORMAL
 * _EM_ZERODIVIDE
 * _EM_OVERFLOW
 * _EM_UNDERFLOW
 * _EM_INEXACT
 *
 * @param newCtrlWordBits New control-word bit values.
 * @param mask Mask for new control-word bits to set.
 *
 * @return
 */
STATIC_INLINE_BEGIN unsigned int controlfp(unsigned int newCtrlWordBits = EM_INEXACT,
    unsigned int mask = MCW_EM) {
#ifdef _MSC_VER
  return _controlfp(newCtrlWordBits, mask);
#else
  return -1;
#endif
  // Using MSVC you need to:
  //   C/C++ -> Code Generation -> Modify the Enable C++ Exceptions to "Yes With SEH Exceptions"
}

STATIC_INLINE_BEGIN uint64_t sps_rdtsc() {
#ifdef _MSC_VER
  return __rdtsc();
#elif defined(__GNUC__)
  uint32_t lo, hi;
  /*
    We cannot use "=A", since this would use %rax on x86_64 and return
    only the lower 32bits of the TSC
  */
  __asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
  return (uint64_t)hi << 32 | lo;
#endif
}

STATIC_INLINE_BEGIN int getncpus() {
  int nproc = 0;
#if defined(__linux__)
  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
  CallErr(sched_getaffinity,
          (gettid(), sizeof( cpu_set_t ), &cpuset));

  nproc = CPU_COUNT(&cpuset);
#elif defined(_WIN32)
  SYSTEM_INFO info;
  GetSystemInfo(&info);
  nproc = info.dwNumberOfProcessors;
#endif
  return nproc;
}

namespace sps {

#if defined(HAVE_PTHREAD_H)
template<class T, void*(T::*thread_func)(void*)>
#elif defined(_WIN32)
template<class T, unsigned int(__stdcall T::*thread_func)(void*)>
#endif

class pthread_launcher {
 public:
  explicit pthread_launcher(T* obj = NULL, void* arg = NULL) :
    _obj(obj), _arg(arg) {}
#if defined(HAVE_PTHREAD_H)
  void *launch() {
    return (_obj->*thread_func)(_arg);
  }
#elif defined(_WIN32)
  unsigned int launch() {
    return (_obj->*thread_func)(_arg);
  }
#endif
 private:
  /// Object pointer
  T* _obj;
  /// Command argument for member function
  void *_arg;
};

template<class T>
#if defined(HAVE_PTHREAD_H)
void *launch_member_function(void *obj)
#elif defined(_WIN32)
unsigned int __stdcall launch_member_function(void *obj)
#endif
{
  T* launcher = reinterpret_cast<T*>(obj);
  return launcher->launch();
}

// Naming threads - consider upgrading to using SetThreadDescription
#ifdef _WIN32
const DWORD MS_VC_EXCEPTION = 0x406D1388;

#pragma pack(push, 8)
typedef struct tagTHREADNAME_INFO {
  DWORD dwType;  // Must be 0x1000.
  LPCSTR szName;  // Pointer to name (in user addr space).
  DWORD dwThreadID;  // Thread ID (-1=caller thread).
  DWORD dwFlags;  // Reserved for future use, must be zero.
} THREADNAME_INFO;
#pragma pack(pop)

STATIC_INLINE_BEGIN void SetThreadName(uint32_t dwThreadID,
                                       const char* threadName) {
  // DWORD dwThreadID =
  // ::GetThreadId( static_cast<HANDLE>( t.native_handle() ) );

  // Only works when inside the debugger... TODO: Use SetThreadDescription (Windows 10)

  THREADNAME_INFO info;
  info.dwType = 0x1000;
  info.szName = threadName;
  info.dwThreadID = dwThreadID;
  info.dwFlags = 0;

  __try {
    RaiseException(MS_VC_EXCEPTION,
                   0,
                   sizeof(info)/sizeof(ULONG_PTR),
                   reinterpret_cast<ULONG_PTR*>(&info));
  } __except(EXCEPTION_EXECUTE_HANDLER) {
  }
}

STATIC_INLINE_BEGIN void SetThreadName(const char* threadName) {
  SetThreadName(GetCurrentThreadId(), threadName);
}

STATIC_INLINE_BEGIN void SetThreadName(std::thread* thread,
                                       const char* threadName) {
  DWORD threadId =
    ::GetThreadId(static_cast<HANDLE>(thread->native_handle()));
  SetThreadName(threadId, threadName);
}

#else
STATIC_INLINE_BEGIN void SetThreadName(std::thread* thread,
                                       const char* threadName) {
  auto handle = thread->native_handle();
  pthread_setname_np(handle, threadName);
}

#include <sys/prctl.h>
STATIC_INLINE_BEGIN void SetThreadName(const char* threadName) {
  prctl(PR_SET_NAME, threadName, 0, 0, 0);
}

#endif
}  // namespace sps





















/* Local variables: */
/* indent-tabs-mode: nil */
/* tab-width: 2 */
/* c-basic-offset: 2 */
/* End: */
