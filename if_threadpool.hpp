/**
 * @file   if_threadpool.hpp
 * @author Jens Munk Hansen <jmh@debian9laptop.parknet.dk>
 * @date   Sun Jul  1 23:40:35 2018
 *
 * @brief
 *
 * Copyright 2018 Jens Munk Hansen
 */

#pragma once

#include <sps/sps_export.h>
#include <cstddef>

namespace sps {
typedef void (*SimpleCallback)(void*);

class SPS_EXPORT IThreadPool {
 public:
  virtual int Initialize() = 0;
  virtual int SubmitJob(SimpleCallback cb, void* pUserData) = 0;
  virtual ~IThreadPool() = default;

  // If you start to default functions - no longer a pure virtual interface
  // and you need to provide a default ctor
  // IThreadPool(IThreadPool&& other) = default;
  // IThreadPool& operator=(IThreadPool&& other) = default;
 protected:
  // If you add ctors, you must define em
  // IThreadPool() = default;
  // IThreadPool(const std::size_t nThreads);
 private:
  // If you start to delete functions - no longer a pure virtual interface
  // and you need to provide a default ctor
  // IThreadPool(const IThreadPool& other) = delete;
  // IThreadPool& operator=(const IThreadPool& other) = delete;
};

SPS_EXPORT int ThreadPoolCreate(IThreadPool** ppObj);
SPS_EXPORT int ThreadPoolDestroy(IThreadPool* pObj);

}  // namespace sps
