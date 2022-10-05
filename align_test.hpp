#pragma once
#include <sps/cenv.h>

#ifndef CXX11
# error This software requires C++11
#endif

#include <cstdint>
#include <stdexcept>

#define SPS_ALIGNAS(x) alignas(x)

#ifndef SPS_ALIGN_THROW
# define SPS_ALIGN_THROW 1
#endif

#if SPS_ALIGN_THROW
# define SPS_ALIGN_TEST(pointer, byteAlignment)   \
  do {              \
    if ((uintptr_t) pointer % byteAlignment != 0) \
      throw std::runtime_error("Invalid alignment");  \
  } while (0)
#else
# define SPS_ALIGN_TEST(pointer, byteAlignment)
#endif

