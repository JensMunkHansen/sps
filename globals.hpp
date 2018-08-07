/**
 * @file   globals.hpp
 * @author Jens Munk Hansen <jens.munk.hansen@gmail.com>
 * @date   Fri Mar 31 20:45:27 2017
 *
 * @brief  Templated singletons
 *
 * Copyright 2017 Jens Munk Hansen
 *
 */
#pragma once

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
#include <sps/cenv.h>  // SPS_ATTR_DESTRUCTOR

#ifdef _MSC_VER
# include <cstdlib> // atexit
#endif

#include <atomic>
#include <mutex>
#include <iostream>

namespace sps {
/*
  Template <struct V> is not allowed so this gives a warning, when V is a struct
*/
#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable : 4099)
#elif __clang__
# pragma clang diagnostic push
# pragma clang diagnostic ignored "-Wmismatched-tags"
#endif

template <typename T, template <typename> class V> class globalstruct {
 public:
  //  template <typename U> using MyTemplate = V<U>;
  static std::atomic<class V<T>*> pVar;
 private:
  globalstruct() = delete;
  globalstruct(const globalstruct& rhs) = delete;
  void operator=(const globalstruct& rhs) = delete;
};

#ifdef _MSC_VER
# pragma warning(pop)
#elif __clang__
# pragma clang diagnostic pop
#endif

template <typename T> class global {
 public:
  static std::atomic<T*> pVar;
 private:
  global() = delete;
  global(const global& rhs) = delete;
  void operator=(const global& rhs) = delete;
};

// TODO(JMH): Consider requiring descendant to be non-copyconstructible
template<bool B, typename T = void> using disable_if = std::enable_if<!B, T>;


template <class T>
class Singleton {
 public:
  /**
   * InstanceGet
   *
   * Acquire the (non-static) singleton instance
   *
   * @return Pointer to singleton instance
   */
  static const T& InstanceGet() {
    T* pInstance = g_instance.load(std::memory_order_acquire);
    if (!pInstance) {
      std::lock_guard<std::mutex> guard(g_mutex);
      pInstance = g_instance.load(std::memory_order_relaxed);
      if (!pInstance) {
        pInstance = new T;
        g_instance.store(pInstance, std::memory_order_release);
      }
    }
    return *pInstance;
  }

  static int InstanceDestroy() SPS_ATTR_DESTRUCTOR;

 protected:
  Singleton() {
#ifdef _MSC_VER
    // If we never include this in a DLL, which is loaded
    // dynamically, we can use the standard atexit handler.
    //
    // TODO(JMH): Figure out to add function pointer
    // to a list called on DLL_PROCESS_DETACH
    std::atexit([]()->void { Singleton<T>::InstanceDestroy();});
#endif
  }
  /**
   * Copy-ctor is deleted. We do not allow copying the singleton
   *
   */
  Singleton(Singleton const &) = delete;
  Singleton& operator=(Singleton const &) = delete;

 private:
  static std::atomic<T*> g_instance;
  static std::mutex g_mutex;
#if 0
  // Ugly hack to enforce generation of InstanceDestroy
  const int atexit = Singleton<T>::InstanceDestroy();
#endif
};

template <class T>
int Singleton<T>::InstanceDestroy() {
  int retval = -1;
  std::lock_guard<std::mutex> guard(g_mutex);
  if (g_instance) {
    delete g_instance;
    g_instance = nullptr;
    retval = 0;
  }
  return retval;
}

template <class T>
std::atomic<T*> Singleton<T>::g_instance{nullptr};

template <class T>
std::mutex Singleton<T>::g_mutex;

}  // namespace sps

/* Local variables: */
/* indent-tabs-mode: nil */
/* tab-width: 2 */
/* c-basic-offset: 2 */
/* End: */
