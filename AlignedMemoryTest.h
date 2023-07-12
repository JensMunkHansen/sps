#pragma once
#if defined(__cplusplus)
# if (__cplusplus >= 201103L)
#  define CXX11 11
# endif
# if defined(_MSC_VER) && (_MSC_VER >= 1900)
#  define CXX11 11
# endif
#endif

#ifndef CXX11
# error This software requires C++11
#endif

#include <cstdint>
#include <stdexcept>

#define SPS_ALIGNAS(x) alignas(x)

#ifndef SPS_ALIGN_THROW
# define SPS_ALIGN_THROW 0
#endif

#if SPS_ALIGN_THROW
# define SPS_ALIGN_TEST(pointer, byteAlignment)	  \
  do {						  \
    if ((uintptr_t) pointer % byteAlignment != 0) \
      throw std::exception("Invalid alignment");  \
  } while (0)
#else
# define SPS_ALIGN_TEST(pointer, byteAlignment)
#endif

