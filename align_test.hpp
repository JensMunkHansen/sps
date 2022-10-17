#pragma once
#include <sps/cenv.h>

#ifndef CXX11
# error This software requires C++11
#endif

#include <cstdint>
#include <stdexcept>

#define SPS_ALIGNAS(x) alignas(x)

// We do not want to allocate memory in exception-handling parhs
#define S(x) #x
#define S_(x) S(x)
#define S__LINE__ S_(__LINE__)

#ifndef SPS_ALIGN_THROW
# define SPS_ALIGN_THROW 1
#endif

#if SPS_ALIGN_THROW
# define SPS_ALIGN_TEST(pointer, byteAlignment)   \
  do {              \
    if ((uintptr_t) pointer % byteAlignment != 0)                       \
      throw std::runtime_error("Invalid alignment: " __FILE__ ":" S__LINE__);  \
  } while (0)
#else
# define SPS_ALIGN_TEST(pointer, byteAlignment)
#endif
