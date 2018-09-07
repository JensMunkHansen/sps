/**
 * @file   bitswitch.hpp
 * @author Jens Munk Hansen <jmh@debian9laptop.parknet.dk>
 * @date   Fri Sep  7 19:52:05 2018
 *
 * @brief
 *
 * Copyright 2016 Jens Munk Hansen
 */

#include <sps/malloc.h>

#include <cstdlib>
#include <cstdarg>
#include <cstdint>
#include <cstring>

namespace sps {

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wparentheses"

constexpr uint32_t basis = 0;

constexpr uint32_t
bin2int_compile_time(char const* str,
                     uint32_t last_value = basis) {
  return *str ?
         bin2int_compile_time(str + 1,
                              (last_value << 1) ^ ((uint8_t)*str) - 48) :
         last_value;
}

constexpr uint32_t operator "" _bin2int(char const* p, size_t) {
  return bin2int_compile_time(p);
}

uint32_t bool2int(const bool* bools, const size_t length) {
  uint32_t mask = bools[0];
  for (size_t i = 1 ; i < length ; i++) {
    mask = mask << 1;
    mask = mask ^ bools[i];
  }
  return mask;
}

uint32_t bool2int(const size_t d, ...) {
  uint32_t mask;
  va_list ap;             /* varargs list traverser */
  va_start(ap, d);

  size_t *d1 = static_cast<size_t*>(SPS_MALLOC(d*sizeof(bool)));
  for (size_t i = 0 ; i < d ; i++) {
    d1[i] = va_arg(ap, int32_t);
  }

  mask = d1[0];
  for (size_t i = 1 ; i < d ; i++) {
    mask = mask << 1;
    mask = mask ^ d1[i];
  }
  free(d1);
  va_end(ap);
  return mask;
}
#pragma GCC diagnostic pop
}  // namespace sps
