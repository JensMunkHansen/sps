#pragma once

/*
  The compiler, must be one of: (SPS_CC_x)

  MSVC     - Microsoft Visual C/C++, Intel C++ for Windows
  GNU      - GNU C++
  INTEL    - Intel C++ for Linux, Intel C++ for Windows
  CLANG    - C++ front-end for the LLVM compiler

*/

#if defined(_MSC_VER)
# ifdef __clang__
#  define SPS_CC_CLANG ((__clang_major__ * 100) + __clang_minor__)
# endif
# define SPS_CC_MSVC (_MSC_VER)
# define SPS_CC_MSVC_NET
# define SPS_OUTOFLINE_TEMPLATE inline
# define SPS_COMPILER_MANGLES_RETURN_TYPE
# define SPS_FUNC_INFO __FUNCSIG__
# define SPS_ALIGNOF(type) __alignof(type)
# define SPS_DECL_ALIGN(n) __declspec(align(n))
# define SPS_ASSUME_IMPL(expr) __assume(expr)
# define SPS_UNREACHABLE_IMPL() __assume(0)
# define SPS_NORETURN __declspec(noreturn)
# define SPS_DECL_DEPRECATED __declspec(deprecated)
# ifndef SPS_CC_CLANG
#  define SPS_DECL_DEPRECATED_X(text) __declspec(deprecated(text))
# endif
# define SPS_DECL_EXPORT __declspec(dllexport)
# define SPS_DECL_IMPORT __declspec(dllimport)
# define QT_MAKE_UNCHECKED_ARRAY_ITERATOR(x) stdext::make_unchecked_array_iterator(x) // Since _MSC_VER >= 1800
# define QT_MAKE_CHECKED_ARRAY_ITERATOR(x, N) stdext::make_checked_array_iterator(x, size_t(N)) // Since _MSC_VER >= 1500
/*Intel C++ disguising as Visual C++: the `using' keyword avoids warnings */
# if defined(__INTEL_COMPILER)
#  define SPS_DECL_VARIABLE_DEPRECATED
#  define SPS_CC_INTEL  __INTEL_COMPILER
# endif
#elif defined(__GNUC__)
# define SPS_CC_GNU          (__GNUC__ * 100 + __GNUC_MINOR__)
# if defined(__MINGW32__)
#  define SPS_CC_MINGW
# endif
# if defined(__INTEL_COMPILER)
/*Intel C++ also masquerades as GCC */
#  define SPS_CC_INTEL      (__INTEL_COMPILER)
#  ifdef __clang__
/*Intel C++ masquerades as Clang masquerading as GCC */
#   define SPS_CC_CLANG    305
#  endif
#  define SPS_ASSUME_IMPL(expr)  __assume(expr)
#  define SPS_UNREACHABLE_IMPL() __builtin_unreachable()
#  if __INTEL_COMPILER >= 1300 && !defined(__APPLE__)
#   define SPS_DECL_DEPRECATED_X(text) __attribute__ ((__deprecated__(text)))
#  endif
# elif defined(__clang__)
/* Clang also masquerades as GCC */
#  if defined(__apple_build_version__)
#   /* http://en.wikipedia.org/wiki/Xcode#Toolchain_Versions */
#   if __apple_build_version__ >= 8020041
#    define SPS_CC_CLANG 309
#   elif __apple_build_version__ >= 8000038
#    define SPS_CC_CLANG 308
#   elif __apple_build_version__ >= 7000053
#    define SPS_CC_CLANG 306
#   elif __apple_build_version__ >= 6000051
#    define SPS_CC_CLANG 305
#   elif __apple_build_version__ >= 5030038
#    define SPS_CC_CLANG 304
#   elif __apple_build_version__ >= 5000275
#    define SPS_CC_CLANG 303
#   elif __apple_build_version__ >= 4250024
#    define SPS_CC_CLANG 302
#   elif __apple_build_version__ >= 3180045
#    define SPS_CC_CLANG 301
#   elif __apple_build_version__ >= 2111001
#    define SPS_CC_CLANG 300
#   else
#    error "Unknown Apple Clang version"
#   endif
#  else
#   define SPS_CC_CLANG ((__clang_major__ * 100) + __clang_minor__)
#  endif
#  if __has_builtin(__builtin_assume)
#   define SPS_ASSUME_IMPL(expr)   __builtin_assume(expr)
#  else
#   define SPS_ASSUME_IMPL(expr)  if (expr){} else __builtin_unreachable()
#  endif
#  define SPS_UNREACHABLE_IMPL() __builtin_unreachable()
#  if !defined(__has_extension)
#   /* Compatibility with older Clang versions */
#   define __has_extension __has_feature
#  endif
#  if defined(__APPLE__)
/* Apple/clang specific features */
#   define SPS_DECL_CF_RETURNS_RETAINED __attribute__((cf_returns_retained))
#   ifdef __OBJC__
#    define SPS_DECL_NS_RETURNS_AUTORELEASED __attribute__((ns_returns_autoreleased))
#   endif
#  endif
#  ifdef __EMSCRIPTEN__
#   define SPS_CC_EMSCRIPTEN
#  endif
# else
/* Plain GCC */
#  if SPS_CC_GNU >= 405
#   define SPS_ASSUME_IMPL(expr)  if (expr){} else __builtin_unreachable()
#   define SPS_UNREACHABLE_IMPL() __builtin_unreachable()
#   define SPS_DECL_DEPRECATED_X(text) __attribute__ ((__deprecated__(text)))
#  endif
# endif

# ifdef SPS_OS_WIN
#  define SPS_DECL_EXPORT     __declspec(dllexport)
#  define SPS_DECL_IMPORT     __declspec(dllimport)
# elif defined(QT_VISIBILITY_AVAILABLE)
#  define SPS_DECL_EXPORT     __attribute__((visibility("default")))
#  define SPS_DECL_IMPORT     __attribute__((visibility("default")))
#  define SPS_DECL_HIDDEN     __attribute__((visibility("hidden")))
# endif

# define SPS_FUNC_INFO       __PRETTY_FUNCTION__
# define SPS_ALIGNOF(type)   __alignof__(type)
# define SPS_TYPEOF(expr)    __typeof__(expr)
# define SPS_DECL_DEPRECATED __attribute__ ((__deprecated__))
# define SPS_DECL_ALIGN(n)   __attribute__((__aligned__(n)))
# define SPS_DECL_UNUSED     __attribute__((__unused__))
# define SPS_LIKELY(expr)    __builtin_expect(!!(expr), true)
# define SPS_UNLIKELY(expr)  __builtin_expect(!!(expr), false)
# define SPS_NORETURN        __attribute__((__noreturn__))
# define SPS_REQUIRED_RESULT __attribute__ ((__warn_unused_result__))
# define SPS_DECL_PURE_FUNCTION __attribute__((pure))
# define SPS_DECL_CONST_FUNCTION __attribute__((const))
# define SPS_DECL_COLD_FUNCTION __attribute__((cold))
# if !defined(QT_MOC_CPP)
#  define SPS_PACKED __attribute__ ((__packed__))
#  ifndef __ARM_EABI__
#   define QT_NO_ARM_EABI
#  endif
# endif
# if SPS_CC_GNU >= 403 && !defined(SPS_CC_CLANG)
#  define SPS_ALLOC_SIZE(x) __attribute__((alloc_size(x)))
# endif
#else
#  error "Unsupported compiler"
#endif

/*
 * Warning/diagnostic handling
 */

#define SPS_DO_PRAGMA(text)                      _Pragma(#text)
#if defined(SPS_CC_INTEL) && defined(SPS_CC_MSVC)
/* icl.exe: Intel compiler on Windows */
#  undef SPS_DO_PRAGMA                           /* not needed */
#  define SPS_WARNING_PUSH                       __pragma(warning(push))
#  define SPS_WARNING_POP                        __pragma(warning(pop))
#  define SPS_WARNING_DISABLE_MSVC(number)
#  define SPS_WARNING_DISABLE_INTEL(number)      __pragma(warning(disable: number))
#  define SPS_WARNING_DISABLE_CLANG(text)
#  define SPS_WARNING_DISABLE_GCC(text)
#  define SPS_WARNING_DISABLE_DEPRECATED         SPS_WARNING_DISABLE_INTEL(1478 1786)
#elif defined(SPS_CC_INTEL)
/* icc: Intel compiler on Linux or OS X */
#  define SPS_WARNING_PUSH                       SPS_DO_PRAGMA(warning(push))
#  define SPS_WARNING_POP                        SPS_DO_PRAGMA(warning(pop))
#  define SPS_WARNING_DISABLE_INTEL(number)      SPS_DO_PRAGMA(warning(disable: number))
#  define SPS_WARNING_DISABLE_MSVC(number)
#  define SPS_WARNING_DISABLE_CLANG(text)
#  define SPS_WARNING_DISABLE_GCC(text)
#  define SPS_WARNING_DISABLE_DEPRECATED         SPS_WARNING_DISABLE_INTEL(1478 1786)
#elif defined(SPS_CC_MSVC) && !defined(SPS_CC_CLANG)
#  undef SPS_DO_PRAGMA                           /* not needed */
#  define SPS_WARNING_PUSH                       __pragma(warning(push))
#  define SPS_WARNING_POP                        __pragma(warning(pop))
#  define SPS_WARNING_DISABLE_MSVC(number)       __pragma(warning(disable: number))
#  define SPS_WARNING_DISABLE_INTEL(number)
#  define SPS_WARNING_DISABLE_CLANG(text)
#  define SPS_WARNING_DISABLE_GCC(text)
#  define SPS_WARNING_DISABLE_DEPRECATED         SPS_WARNING_DISABLE_MSVC(4996)
#elif defined(SPS_CC_CLANG)
#  define SPS_WARNING_PUSH                       SPS_DO_PRAGMA(clang diagnostic push)
#  define SPS_WARNING_POP                        SPS_DO_PRAGMA(clang diagnostic pop)
#  define SPS_WARNING_DISABLE_CLANG(text)        SPS_DO_PRAGMA(clang diagnostic ignored text)
#  define SPS_WARNING_DISABLE_GCC(text)
#  define SPS_WARNING_DISABLE_INTEL(number)
#  define SPS_WARNING_DISABLE_MSVC(number)
#  define SPS_WARNING_DISABLE_DEPRECATED         SPS_WARNING_DISABLE_CLANG("-Wdeprecated-declarations")
#elif defined(SPS_CC_GNU) && (__GNUC__ * 100 + __GNUC_MINOR__ >= 406)
#  define SPS_WARNING_PUSH                       SPS_DO_PRAGMA(GCC diagnostic push)
#  define SPS_WARNING_POP                        SPS_DO_PRAGMA(GCC diagnostic pop)
#  define SPS_WARNING_DISABLE_GCC(text)          SPS_DO_PRAGMA(GCC diagnostic ignored text)
#  define SPS_WARNING_DISABLE_CLANG(text)
#  define SPS_WARNING_DISABLE_INTEL(number)
#  define SPS_WARNING_DISABLE_MSVC(number)
#  define SPS_WARNING_DISABLE_DEPRECATED         SPS_WARNING_DISABLE_GCC("-Wdeprecated-declarations")
#else       // All other compilers, GCC < 4.6 and MSVC < 2008
#  define SPS_WARNING_DISABLE_GCC(text)
#  define SPS_WARNING_PUSH
#  define SPS_WARNING_POP
#  define SPS_WARNING_DISABLE_INTEL(number)
#  define SPS_WARNING_DISABLE_MSVC(number)
#  define SPS_WARNING_DISABLE_CLANG(text)
#  define SPS_WARNING_DISABLE_GCC(text)
#  define SPS_WARNING_DISABLE_DEPRECATED
#endif

#ifndef SPS_IGNORE_DEPRECATIONS
#define SPS_IGNORE_DEPRECATIONS(statement) \
    SPS_WARNING_PUSH \
    SPS_WARNING_DISABLE_DEPRECATED \
    statement \
    SPS_WARNING_POP
#endif
