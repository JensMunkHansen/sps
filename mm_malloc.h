/**
 * @file   mm_malloc.h
 * @author Jens Munk Hansen <jens.munk.hansen@gmail.com>
 * @date   Fri May 29 17:27:06 2015
 *
 * @brief
 *
 *
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

#ifdef _MSC_VER
# include <malloc.h>
#else
# include <mm_malloc.h>
#endif

#include <stdlib.h>
#include <stdio.h>

#define SPS_MM_MALLOC(theSize, theAlignment) sps_mm_malloc(__FILE__, __LINE__, theSize, theAlignment)

#define SPS_MM_FREE(ptr) _mm_free(ptr)

static inline void *sps_mm_malloc(const char *file, int line, size_t size, int alignment) {
  void *ptr = _mm_malloc(size, alignment);

  if(!ptr) {
    printf("Could not allocate: %zu bytes (%s:%d)\n", size, file, line);
    exit(EXIT_FAILURE);
  }
  return ptr;
}



/* Local variables: */
/* indent-tab-mode: nil */
/* tab-width: 2 */
/* c-basic-offset: 2 */
/* indent-tabs-mode: nil */
/* End: */
