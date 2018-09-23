﻿/**
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
#include <sps/debug.h>

#ifdef _MSC_VER
# include <cstdlib>  // atexit
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
  static T* InstanceGet() {
    T* pInstance = g_instance.load(std::memory_order_acquire);
    if (!pInstance) {
      std::lock_guard<std::mutex> guard(g_mutex);
      pInstance = g_instance.load(std::memory_order_relaxed);
      if (!pInstance) {
        pInstance = new T;
        g_instance.store(pInstance, std::memory_order_release);
      }
    }
    return pInstance;
  }

  static int InstanceDestroy() SPS_ATTR_DESTRUCTOR;

 protected:
  Singleton() {
    // If we never include this in a DLL, which is loaded
    // dynamically, we can use the standard atexit handler.
    // std::atexit([]()->void { Singleton<T>::InstanceDestroy();});
  }
  /**
   * Copy-ctor and assignment could be deleted such that we do not allow copying the singleton
   *
   */
  Singleton(Singleton const &) = delete;
  Singleton& operator=(Singleton const &) = delete;

 private:
  static std::atomic<T*> g_instance;
  static std::mutex g_mutex;

  // Ugly hack to enforce generation of InstanceDestroy - deadlocks
  //const int atexit = Singleton<T>::InstanceDestroy();
};

template <class T>
int Singleton<T>::InstanceDestroy() {
  int retval = -1;
  std::lock_guard<std::mutex> guard(g_mutex);
  if (g_instance) {
    debug_print("Singleton destroyed");
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


template <class T>
class Default {
 public:
  /**
   * InstanceGet
   *
   * Acquire the (non-static) singleton instance
   *
   * @return Pointer to singleton instance
   */
  static T* InstanceGet() {
    T* pInstance = g_instance.load(std::memory_order_acquire);
    if (!pInstance) {
      std::lock_guard<std::mutex> guard(g_mutex);
      pInstance = g_instance.load(std::memory_order_relaxed);
      if (!pInstance) {
        pInstance = new T;
        g_instance.store(pInstance, std::memory_order_release);
      }
    }
    return pInstance;
  }

  static int InstanceDestroy() SPS_ATTR_DESTRUCTOR;

  ~Default() {
    debug_print("\n");
  }

 protected:
  Default() {
    debug_print("\n");
  }

 private:
  static std::atomic<T*> g_instance;
  static std::mutex g_mutex;
};

template <class T>
int Default<T>::InstanceDestroy() {
  int retval = -1;
  std::lock_guard<std::mutex> guard(g_mutex);
  if (g_instance) {
    debug_print("Default destroyed");
    delete g_instance;
    g_instance = nullptr;
    retval = 0;
  }
  return retval;
}

template <class T>
std::atomic<T*> Default<T>::g_instance{nullptr};

template <class T>
std::mutex Default<T>::g_mutex;

}  // namespace sps

/* Local variables: */
/* indent-tabs-mode: nil */
/* tab-width: 2 */
/* c-basic-offset: 2 */
/* End: */
