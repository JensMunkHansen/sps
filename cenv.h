﻿/**
 * @file   cenv.h
 * @author Jens Munk Hansen <jens.munk.hansen@gmail.com>
 * @date   Sat Nov 29 17:48:45 2014
 *
 * @brief  Environment macros introduced for portability
 *         This file must be kept C compliant
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
/* Microsoft compiler does not understand __restrict */
# define __restrict
#endif

#ifdef __cplusplus
# include <cstdarg>
#else
# include <stdarg.h>
#endif

// TODO: Add other OS'es
#if defined(_WIN32) || defined(WIN32) || defined(__CYGWIN__) || defined(__MINGW32__) || defined(__BORLANDC__)
# define SPS_OS_WIN
#endif

#ifdef __GNUC__
# include <features.h>
#endif

/*
MSVC++ 15.0 _MSC_VER == 2000 (Visual Studio 2017)
MSVC++ 14.0 _MSC_VER == 1900 (Visual Studio 2015)
MSVC++ 12.0 _MSC_VER == 1800 (Visual Studio 2013)
MSVC++ 11.0 _MSC_VER == 1700 (Visual Studio 2012)
MSVC++ 10.0 _MSC_VER == 1600 (Visual Studio 2010)
MSVC++ 9.0  _MSC_VER == 1500 (Visual Studio 2008)
MSVC++ 8.0  _MSC_VER == 1400 (Visual Studio 2005)
MSVC++ 7.1  _MSC_VER == 1310 (Visual Studio 2003)
MSVC++ 7.0  _MSC_VER == 1300
MSVC++ 6.0  _MSC_VER == 1200
MSVC++ 5.0  _MSC_VER == 1100
*/

#if defined(__STDC__)
# define C89   /* ANSI X3.159-1989 */
# if defined(__STDC_VERSION__)
#  define C90  /* ISO/IEC 9899:1990 */
#  if (__STDC_VERSION__ >= 199409L)
#   define C94 /* ISO/IEC 9899-1:1994 */
#  endif
#  if (__STDC_VERSION__ >= 199901L)
#   define C99 /* ISO/IEC 9899:1999 */
#  endif
#  if (__STDC_VERSION__ >= 201112L)
#   define C11 /* ISO/IEC 9899:2011 */
#  endif
# elif defined(__cplusplus)
#  ifdef __USE_ISOC99
#   define C99
#  endif
# endif
#endif

#ifdef C99
# define SPS_FCOMPLEX float _Complex
#elif defined(_MSC_VER)
# define SPS_FCOMPLEX _Fcomplex
#endif

#if defined(__cplusplus)
# if (__cplusplus >= 201103L)
#  define CXX11 11
# endif
# if defined(_MSC_VER) && (_MSC_VER >= 1900)
#  define CXX11 11
# endif

# if defined(_MSC_VER) && (_MSC_VER <= 1900)
#  define SPS_NOEXCEPT
#  define SPS_ALIGNAS(x) __declspec(align(32))
# else
#  define SPS_NOEXCEPT noexcept
#  define SPS_ALIGNAS(x) alignas(x)
# endif

# if CXX11
// Deprecated in C++11.
#  define SPS_REGISTER
#  define SPS_OVERRIDE override
# else
#  define SPS_REGISTER #register
#  define SPS_OVERRIDE
# endif
# if (__cplusplus >= 201402L)
#  define CXX14 14
# endif
# if (__cplusplus >= 201703L)
#  define CXX17 17
# endif
#endif


/*
C++98 __cplusplus = 199711L ISO/IEC 14882:1998
C++11 __cplusplus = 201103L ISO/IEC 14882:2011
C++14 __cplusplus = 201402L ISO/IEC 14882:2014
C++/CLI __cplusplus_cli = 200406L ECMA-372
DSP-C   ISO/IEC JTC1/SC22 WG14/N854
EC++  __embedded_cplusplus  Embedded C++
 */

// Alignment, TODO: Use alignas if present

/*

 TODO: Consider using a construction like

 template <class T>
 struct __attribute__((aligned(4*sizeof(T)))) Data
 {
   T a;
 };

*/

#if (defined(_MSC_VER) && defined(_WIN32))
# define ALIGN16_BEGIN __declspec(align(16))
# define ALIGN16_END
# define ALIGN32_BEGIN __declspec(align(32))
# define ALIGN32_END
#elif defined(__GNUC__)
# define ALIGN16_BEGIN
# define ALIGN16_END __attribute__((aligned(16)))
# define ALIGN32_BEGIN
# define ALIGN32_END __attribute__((aligned(32)))
#endif

// Static inlines
#if (defined(_MSC_VER) && defined(_WIN32))
// Note when used inside a namespace, the static is superfluous
# define STATIC_INLINE_BEGIN static inline //__forceinline
# define STATIC_INLINE_END
#elif (defined(__GNUC__))
# define STATIC_INLINE_BEGIN static inline
# if defined(__CYGWIN__)
#  define STATIC_INLINE_END
# else
#  define STATIC_INLINE_END /* Work on this */ //__attribute__ ((always_inline))
# endif
#endif

#ifdef __GNUG__
# define SPS_LIKELY(x)         __builtin_expect((x),1)
# define SPS_UNLIKELY(x)       __builtin_expect((x),0)
# define SPS_ASS_ALIGNED(x,y)  __builtin_assume_aligned((x),(y))
#else
# define SPS_LIKELY(x)       (x)
# define SPS_UNLIKELY(x)     (x)
# define SPS_ASS_ALIGNED(x,y)  (x)
#endif


#define __BEGIN__       {
#define __END__         goto exit; exit: ; }

// Thread-local storage
#if defined(_WIN32) && defined(_MSC_VER)
# define __THREAD __declspec(thread)
#elif defined(__clang__)
# define __THREAD thread_local
#elif defined(__GNUC__)
# define __THREAD __thread
#endif

#ifdef __GNUG__
# define GCC_VERSION (__GNUC__ * 10000 \
              + __GNUC_MINOR__ * 100   \
              + __GNUC_PATCHLEVEL__)
#endif

// TODO: Make file with helper macros
#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

/* count arguments */
#define NUMARGS(...) \
  SELECT_5TH(__VA_ARGS__, 4, 3, 2, 1, 0, throwaway, throwaway)
#define SELECT_5TH(a1,a2,a3,a4,a5, ...) a5

/* expands to the first arguments */
#define RETURN_FIRST(...) RETURN_FIRST_HELPER(__VA_ARGS__, throwaway)
#define RETURN_FIRST_HELPER(first, ...) return first;

#define FIRST(...) FIRST_HELPER(__VA_ARGS__, throwaway)
#define FIRST_HELPER(first, ...) first

/*
 * if there's only one argument, expands to nothing.  if there is more
 * than one argument, expands to a comma followed by everything but
 * the first argument.  only supports up to 9 arguments but can be
 * trivially expanded.
 */
#define REST(...) REST_HELPER(NNUM(__VA_ARGS__), __VA_ARGS__)
#define REST_HELPER(qty, ...) REST_HELPER2(qty, __VA_ARGS__)
#define REST_HELPER2(qty, ...) REST_HELPER_##qty(__VA_ARGS__)
#define REST_HELPER_ONE(first)
#define REST_HELPER_TWOORMORE(first, ...) , __VA_ARGS__
#define NNUM(...) \
    SELECT_10TH(__VA_ARGS__, TWOORMORE, TWOORMORE, TWOORMORE, TWOORMORE,\
                TWOORMORE, TWOORMORE, TWOORMORE, TWOORMORE, ONE, throwaway)
#define SELECT_10TH(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, ...) a10

#ifdef _WIN32

// Does not respect C99

# define EXPAND(x) x
# define FOR_EACH_1(what, x, ...) what(x)
# define FOR_EACH_2(what, x, ...)\
  what(x);\
  EXPAND(FOR_EACH_1(what,  __VA_ARGS__))
# define FOR_EACH_3(what, x, ...)\
  what(x);\
  EXPAND(FOR_EACH_2(what, __VA_ARGS__))
# define FOR_EACH_4(what, x, ...)\
  what(x);\
  EXPAND(FOR_EACH_3(what,  __VA_ARGS__))
# define FOR_EACH_5(what, x, ...)\
  what(x);\
  EXPAND(FOR_EACH_4(what,  __VA_ARGS__))
# define FOR_EACH_6(what, x, ...)\
  what(x);\
  EXPAND(FOR_EACH_5(what,  __VA_ARGS__))
# define FOR_EACH_7(what, x, ...)\
  what(x);\
  EXPAND(FOR_EACH_6(what,  __VA_ARGS__))
# define FOR_EACH_8(what, x, ...)\
  what(x);\
  EXPAND(FOR_EACH_7(what,  __VA_ARGS__))
# define FOR_EACH_9(what, x, ...)\
  what(x);\
  EXPAND(FOR_EACH_8(what,  __VA_ARGS__))
# define FOR_EACH_10(what, x, ...)\
  what(x);\
  EXPAND(FOR_EACH_9(what,  __VA_ARGS__))
# define FOR_EACH_11(what, x, ...)\
  what(x);\
  EXPAND(FOR_EACH_10(what,  __VA_ARGS__))
# define FOR_EACH_12(what, x, ...)\
  what(x);\
  EXPAND(FOR_EACH_11(what,  __VA_ARGS__))
# define FOR_EACH_13(what, x, ...)\
  what(x);\
  EXPAND(FOR_EACH_12(what,  __VA_ARGS__))
# define FOR_EACH_NARG(...) FOR_EACH_NARG_(__VA_ARGS__, FOR_EACH_RSEQ_N())
# define FOR_EACH_NARG_(...) EXPAND(FOR_EACH_ARG_N(__VA_ARGS__))
# define FOR_EACH_ARG_N(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, N, ...) N
# define FOR_EACH_RSEQ_N() 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
# define CONCATENATE(x,y) x##y
# define FOR_EACH_(N, what, ...) EXPAND(CONCATENATE(FOR_EACH_, N)(what, __VA_ARGS__))
# define FOR_EACH(what, ...) FOR_EACH_(FOR_EACH_NARG(__VA_ARGS__), what, __VA_ARGS__)

#else

// Respects ISO C99

# define CONCATENATE(arg1, arg2)   CONCATENATE1(arg1, arg2)
# define CONCATENATE1(arg1, arg2)  CONCATENATE2(arg1, arg2)
# define CONCATENATE2(arg1, arg2)  arg1##arg2

# define FOR_EACH_1(what, x)        \
    what(x)

# define FOR_EACH_2(what, x, ...)   \
    what(x);                        \
    FOR_EACH_1(what, __VA_ARGS__);

# define FOR_EACH_3(what, x, ...)   \
    what(x);                        \
    FOR_EACH_2(what, __VA_ARGS__);

# define FOR_EACH_4(what, x, ...)   \
    what(x);                        \
    FOR_EACH_3(what,  __VA_ARGS__);

# define FOR_EACH_5(what, x, ...)   \
    what(x);                        \
    FOR_EACH_4(what,  __VA_ARGS__);

# define FOR_EACH_6(what, x, ...)   \
  what(x);                          \
  FOR_EACH_5(what,  __VA_ARGS__);

# define FOR_EACH_7(what, x, ...)   \
    what(x);                        \
    FOR_EACH_6(what,  __VA_ARGS__);

# define FOR_EACH_8(what, x, ...)   \
    what(x);                        \
    FOR_EACH_7(what,  __VA_ARGS__);

# define FOR_EACH_9(what, x, ...)   \
    what(x);                        \
    FOR_EACH_8(what,  __VA_ARGS__);

# define FOR_EACH_10(what, x, ...)  \
    what(x);                        \
    FOR_EACH_9(what,  __VA_ARGS__);

# define FOR_EACH_11(what, x, ...)  \
    what(x);                        \
    FOR_EACH_10(what,  __VA_ARGS__);

# define FOR_EACH_12(what, x, ...)  \
    what(x);                        \
    FOR_EACH_11(what,  __VA_ARGS__);

# define FOR_EACH_13(what, x, ...)  \
    what(x);                        \
    FOR_EACH_12(what,  __VA_ARGS__);

# define FOR_EACH_NARG(...) FOR_EACH_NARG_(__VA_ARGS__, FOR_EACH_RSEQ_N())
# define FOR_EACH_NARG_(...) FOR_EACH_ARG_N(__VA_ARGS__)
# define FOR_EACH_ARG_N(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, N, ...) N
# define FOR_EACH_RSEQ_N() 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0

# define FOR_EACH_(N, what, ...) CONCATENATE(FOR_EACH_, N)(what, __VA_ARGS__)
# define FOR_EACH(what, ...) FOR_EACH_(FOR_EACH_NARG(__VA_ARGS__), what, __VA_ARGS__)

#endif

#ifndef SPS_UNREFERENCED_PARAMETER
# define SPS_UNREFERENCED_PARAMETER(x) ((void)(x))
#endif

#ifndef SPS_UNREFERENCED_PARAMETERS
# define SPS_UNREFERENCED_PARAMETERS(...) FOR_EACH( SPS_UNREFERENCED_PARAMETER, __VA_ARGS__)
#endif

#ifdef _MSC_VER
# ifndef SPS_ATTR_DESTRUCTOR
#  define SPS_ATTR_DESTRUCTOR
# endif
#else
# ifndef SPS_ATTR_DESTRUCTOR
#  define SPS_ATTR_DESTRUCTOR __attribute__((destructor(101)))
# endif
#endif

#if CXX14
# define SPS_DEPRECATED(since) [[deprecated("Since " #since)]]
# define SPS_DEPRECATED_FOR(since, replacement) \
  [[deprecated("Since " #since "; use " #replacement)]]
#else
# ifdef __GNUC__
#  define SPS_DEPRECATED(since) __attribute__((__deprecated__("Since " #since)))
#  define SPS_DEPRECATED_FOR(since, replacement) \
  __attribute__((__deprecated__("Since " #since "; use " #replacement)))
# elif defined(_MSC_VER)
#  if (_MSC_VER >= 1900)
#   define SPS_DEPRECATED(since) __declspec(deprecated)
#   define SPS_DEPRECATED_FOR(since, replacement) \
  __declspec(deprecated)
#  else
#   define SPS_DEPRECATED(since) __declspec(deprecated("Since " # since))
#   define SPS_DEPRECATED_FOR(since, replacement) \
  __declspec(deprecated("Since " #since "; use " #replacement))
#  endif
# endif
#else
# define SPS_DEPRECATED(since)
# define SPS_DEPRECATED_FOR(since, replacement)
#endif
/* Local variables: */
/* indent-tabs-mode: nil */
/* tab-width: 2 */
/* c-basic-offset: 2 */
/* End: */
