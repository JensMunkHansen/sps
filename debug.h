﻿/**
 * @file   debug.h
 * @author Jens Munk Hansen <jens.munk.hansen@gmail.com>
 * @date   Sun Jul 12 16:16:54 2015
 *
 * @brief  Debug macros
 *
 * Include this and define SPS_DEBUG to use the debug macros.
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

#include <sps/stdio.h>
#include <sps/cenv.h>

#ifndef SPS_DEBUG
# define SPS_DEBUG 0
#endif

#ifndef __SPS_FUNCTION__
# ifdef _MSC_VER
#  define __SPS_FUNCTION__ __FUNCTION__
# else
#  define __SPS_FUNCTION__ __FUNCTION__
# endif
#endif

#ifndef __SPS_PRETTY_FUNCTION__
# ifdef _MSC_VER
#  define __SPS_PRETTY_FUNCTION__ __FUNCSIG__
# else
#  define __SPS_PRETTY_FUNCTION__ __PRETTY_FUNCTION__
# endif
#endif

#ifndef __SPS_DEBUG_FILE__
# define __SPS_DEBUG_FILE__ 0
#endif

#ifndef __SPS_DEBUG_LINE__
# define __SPS_DEBUG_LINE__ 1
#endif

#ifndef __SPS_DEBUG_FUNCTION__
# define __SPS_DEBUG_FUNCTION__ 1
#endif


/*!
 *  #MULTI_LINE_MACRO_BEGIN() can be used for creating other macros, which
 *  work inside conditional statements, e.g.
 *
 *  if (cond) SOME_MACRO(i++; j--;)
 */
#define MULTI_LINE_MACRO_BEGIN do {

/*!
 *  #MULTI_LINE_MACRO_END() is the closing part for #MULTI_LINE_MACRO_BEGIN()
 */
#define MULTI_LINE_MACRO_END                    \
    __pragma(warning(push)) \
    __pragma(warning(disable: 4127)) \
    } while(0) \
    __pragma(warning(pop))

// Compiler bug in MSVC. Disabling C4127 is not working for a do { } while (0)
#if defined(_WIN32)
# if (__SPS_DEBUG_FUNCTION__) && (__SPS_DEBUG_LINE__) && (__SPS_DEBUG_FILE__)

#  define debug_print(fmt, ...)                 \
__pragma(warning(push)) \
__pragma(warning(disable: 4127)) \
for(;;) { \
  if (SPS_DEBUG) fprintf(stderr, "%s:%s():%d: " fmt, __FILE__, __SPS_FUNCTION__, __LINE__,  __VA_ARGS__); \
} break; } \
__pragma(warning(pop))

# else

#  define debug_print(fmt,...)                  \
   __pragma(warning(push)) \
   __pragma(warning(disable: 4127)) \
   __pragma(warning(disable: 6271)) \
   for(;;) { if (SPS_DEBUG) { fprintf(stderr, "%s(): " fmt, __SPS_FUNCTION__,  __VA_ARGS__);} break; } \
   __pragma(warning(pop))

# endif

# define debug_cond_print(cond, fmt,...)        \
  __pragma(warning(push)) \
  __pragma(warning(disable: 4127)) \
  for(;;) { if (cond) { fprintf(stdout, "%s(): " fmt, __SPS_FUNCTION__, __VA_ARGS__);} break; } \
  __pragma(warning(pop))

#elif defined(C99)

// C99 is more stricht and do not allow ## __VA_ARGS__
# if (__SPS_DEBUG_FUNCTION__) && (__SPS_DEBUG_LINE__) && (__SPS_DEBUG_FILE__)

#  define debug_print(...)                                              \
  do { if (SPS_DEBUG) fprintf(stderr, "%s:%d:%s(): " FIRST(__VA_ARGS__), __FILE__, \
                              __SPS_FUNCTION__, __LINE__  REST(__VA_ARGS__)); } while (0)
# else

#  define debug_print(...)                                              \
   do { if (SPS_DEBUG) fprintf(stderr, "%s(): " FIRST(__VA_ARGS__), __SPS_FUNCTION__ REST(__VA_ARGS__)); } while (0)

# endif

# define debug_cond_print(cond, ...)                                    \
  do { if (cond) fprintf(stdout, "%s(): " FIRST(__VA_ARGS__), __SPS_FUNCTION__  REST(__VA_ARGS__)); } while (0)

#else
# if (__SPS_DEBUG_FUNCTION__) && (__SPS_DEBUG_LINE__) && (__SPS_DEBUG_FILE__)

#  define debug_print(fmt, ...)                                     \
  do { if (SPS_DEBUG) fprintf(stderr, "%s:%s():%d: " fmt, __FILE__, \
                              __SPS_FUNCTION__, __LINE__,  ## __VA_ARGS__); } while (0)

# else

#  define debug_print(fmt, ...)                                         \
  do { if (SPS_DEBUG) fprintf(stderr, "%s(): " fmt, __SPS_FUNCTION__, ## __VA_ARGS__); } while (0)

# endif

#  define debug_cond_print(cond, fmt, ...)                              \
  do { if (cond) fprintf(stdout, "%s:%d " fmt, __FILE__, __LINE__, ## __VA_ARGS__); } while (0)

#endif

// Alternative way
// MULTI_LINE_MACRO_BEGIN if (DEBUG) fprintf(stderr, "%s(): " fmt, __SPS_FUNCTION__, __VA_ARGS__); MULTI_LINE_MACRO_END

/* Local variables: */
/* indent-tabs-mode: nil */
/* tab-width: 2 */
/* c-basic-offset: 2 */
/* End: */
