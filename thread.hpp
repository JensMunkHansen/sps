/**
 * @file   thread.hpp
 * @author Jens Munk Hansen <jmh@debian9laptop.parknet.dk>
 * @date   Thu Feb 22 19:02:38 2018
 *
 * @brief  Wrapper for std::thread with added functionality
 *
 * Copyright 2017 Jens Munk Hansen
 */

#pragma once

#include <sps/cerr.h>

#ifdef __GNUG__
# include <sys/prctl.h>
#elif defined(_MSC_VER)
# include <windows.h>
const DWORD MS_VC_EXCEPTION = 0x406D1388;

# pragma pack(push, 8)

typedef struct tagTHREADNAME_INFO {
  DWORD dwType;  // Must be 0x1000.
  LPCSTR szName;  // Pointer to name (in user addr space).
  DWORD dwThreadID;  // Thread ID (-1=caller thread).
  DWORD dwFlags;  // Reserved for future use, must be zero.
} THREADNAME_INFO;

# pragma pack(pop)
#endif

#include <atomic>
#include <thread>
#include <utility>

// Max length for names on *NIX
#ifndef TASK_COMM_LEN
# define TASK_COMM_LEN 16
#endif

namespace sps {
class thread {
 private:
  std::atomic<bool> m_active{true};
  std::thread m_thread;

 public:
#if 0
  // TODO(JEM): Handle naming on Windows 10 using

  HRESULT hr =
    ::SetThreadDescription(::GetCurrentThread(), L"simulation_thread");
  if (FAILED(hr)) {
    // Call failed.
  }

  HRESULT hr = ::GetThreadDescription(ThreadHandle, &data);
  if (SUCCEEDED(hr)) {
    wprintf(“%ls\m”, data);
    LocalFree(data);
  }
#endif

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:6320)
  static bool set_thread_name(uint32_t dwThreadID, const char* psName) {
    /*
      Threads don't actually have names in Win32. The process via RaiseException
      is just a "Secret Handshake" with the VS Debugger, who actually stores the
      TID => Name mapping. Windows itself has no notion of a thread "Name".
    */
    int retval = 0;

    THREADNAME_INFO info;
    info.dwType = 0x1000;
    info.szName = psName;
    info.dwThreadID = dwThreadID;
    info.dwFlags = 0;

    __try {
      RaiseException(MS_VC_EXCEPTION, 0, sizeof(info)/sizeof(ULONG_PTR),
                     reinterpret_cast<ULONG_PTR*>(&info));
    } __except(EXCEPTION_EXECUTE_HANDLER) {
      retval = -1;
    }
    return retval == 0;
  }
# pragma warning(pop)
#else
  static bool set_thread_name(const char* psName) {
    int retval = 0;
    CallErr(retval = prctl, (PR_SET_NAME, psName, 0, 0, 0));
    return (retval != ENOENT) &&  (retval != ERANGE);
  }

  static bool set_thread_name(pthread_t handle, const char* psName) {
    int retval = 0;
    CallErr(retval = pthread_setname_np, (handle, psName));
    return (retval != ENOENT) &&  (retval != ERANGE);
  }

  static bool get_thread_name(pthread_t handle, char *psName, size_t len) {
    int retval = 0;
    CallErr(retval = pthread_getname_np, (handle, psName, len));
    return (retval != ENOENT) &&  (retval != ERANGE);
  }
#endif

  template <class F, class ... Args>
  explicit thread(F&& f, Args&& ... args)
#ifdef _MSC_VER
    : m_active {false}, m_thread([&]() {  // Using C++14, we can skip '()'
    m_active = true;
    std::forward<F>(f)(std::forward<Args>(args)...);
    m_active = false;
  }) {}
#else
    :
    m_thread(
     [func = std::forward<F>(f), flag = &m_active](Args && ... args)
     // Compiles using _MSCVER = 2000 if f is replaced with func
     noexcept(noexcept(f(std::forward<Args>(args)...))) {
    func(std::forward<Args>(args)...);
    flag->store(false, std::memory_order_release);
  },
  std::forward<Args>(args)...) {}
#endif

  bool isActive() const noexcept {
    return m_active.load(std::memory_order_acquire);
  }
  void join() {
    m_thread.join();
  }

  bool joinable() const noexcept {
    return m_thread.joinable();
  }

  void detach() {
    m_thread.detach();
  }

  /**
   * Capable of cancelling, while waiting for a condition variable
   *
   * You need to execute the following in the thread for this to work
   *
   * int oldstate;
   * pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &oldstate)
   *
   * @return
   */
  int cancel() {
    int retval = -1;
#ifdef __GNUG__
    // Will only work for GCC 8 (and onwards)
    pthread_t handle = m_thread.native_handle();
    retval = pthread_cancel(handle);
#else
    // Not supported on Winblows
#endif
    return retval;
  }

  std::thread::id get_id() const noexcept {
    return m_thread.get_id();
  }

#ifdef _WIN32
  bool set_name(const char* psName) {
    int retval = 0;
    DWORD dwThreadID =
      ::GetThreadId(static_cast<HANDLE>(m_thread.native_handle()));
    retval = set_thread_name(dwThreadID, psName);
    return retval;
  }

  bool get_name(char *psName, size_t len) noexcept {
    if (len > 0) {
      psName[len-1] = '\0';
    }
    return false;
  }
#else
  // Handles errors: buffer to short to hold name and thread no longer alive
  bool set_name(const char* psName) {
    pthread_t handle = m_thread.native_handle();
    int retval = sps::thread::set_thread_name(handle, psName);
    return (retval != ENOENT) &&  (retval != ERANGE);
  }

  bool get_name(char *psName, size_t len) noexcept {
    pthread_t handle = m_thread.native_handle();
    return sps::thread::get_thread_name(handle, psName, len);
  }
#endif
};
}  // namespace sps

/* Local variables: */
/* indent-tabs-mode: nil */
/* tab-width: 2 */
/* c-basic-offset: 2 */
/* End: */
