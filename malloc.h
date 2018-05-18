/**
 * @file   malloc.h
 * @author Jens Munk Hansen <jmh@jmhlaptop>
 * @date   Thu Aug  3 20:11:01 2017
 *
 * @brief  wrapper for malloc.h
 *
 *
 */

#pragma once

#include <sps/cenv.h>
#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>

/**
 * Return size of allocated memory
 *
 * @param data
 *
 * @return
 */
STATIC_INLINE_BEGIN size_t msize(void* data) {
#ifdef _WIN32
  return _msize(data);
#elif __APPLE__
  return malloc_size(data);
#elif defined(__GNUC__)
  return malloc_usable_size(data);
#endif
}

#define SPS_MALLOC(theSize) sps_malloc(__FILE__, __LINE__, theSize)

static inline void *sps_malloc(const char *file,int line,size_t size) {
  void *ptr = malloc(size);

  if(!ptr) {
    fprintf(stderr, "Could not allocate: %zu bytes (%s:%d)\n", size, file, line);
    exit(EXIT_FAILURE);
  }
  return(ptr);
}
