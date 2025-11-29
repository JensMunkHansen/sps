/**
 * @file   show_types.hpp
 * @author Jens Munk Hansen <jens.munk.hansen@gmail.com>
 * @date   Tue Feb 20 21:51:18 2018
 *
 * @brief
 *
 *
 */
#pragma once

#include <sps/cenv.h>

#include <iostream>

#ifdef _MSC_VER
#include <string_view>
using string_view = std::basic_string_view<char>;
#else
#if __cplusplus >= 201402
#include <experimental/string_view>
using std::experimental::string_view;
#endif
#endif

namespace sps
{
template <class T>
// TODO(JMH): Make work using C++11
constexpr string_view type_name()
{
#ifdef __clang__
  string_view p = __PRETTY_FUNCTION__;
  return experimental::string_view(p.data() + 34, p.size() - 34 - 1);
#elif defined(__GNUC__)
  string_view p = __PRETTY_FUNCTION__;
#if __cplusplus <= 201402
  // C++14 - TODO(JMH): Filter out encapsulating stuff
  return string_view(p.data() + 36, p.size() - 36 - 1);
#else
  //  return string_view(p.data() + 46, p.size() - 46 - 1);
  //  return string_view(p.data() + 46, p.size() - 46); // Kept ']'

  // For 6.3.0-18 using std::experimental::string_view
  return string_view(p.data() + 46 + 34, p.size() - 46 - 146);
#endif
#elif defined(_MSC_VER)
  string_view p = __FUNCSIG__;
  return string_view(p.data() + 38, p.size() - 38 - 7);
#endif
}
} // namespace sps

#if 0

#include <cstddef>
#include <cstring>
#include <ostream>
#include <stdexcept>

#include <iostream>

#ifndef _MSC_VER
#if __cplusplus < 201103
#define CONSTEXPR11_TN
#define CONSTEXPR14_TN
#define NOEXCEPT_TN
#elif __cplusplus < 201402
#define CONSTEXPR11_TN constexpr
#define CONSTEXPR14_TN
#define NOEXCEPT_TN noexcept
#else
#define CONSTEXPR11_TN constexpr
#define CONSTEXPR14_TN constexpr
#define NOEXCEPT_TN noexcept
#endif
#else // _MSC_VER
#if _MSC_VER < 1900
#define CONSTEXPR11_TN
#define CONSTEXPR14_TN
#define NOEXCEPT_TN
#elif _MSC_VER < 2000
#define CONSTEXPR11_TN constexpr
#define CONSTEXPR14_TN
#define NOEXCEPT_TN noexcept
#else
#define CONSTEXPR11_TN constexpr
#define CONSTEXPR14_TN constexpr
#define NOEXCEPT_TN noexcept
#endif
#endif // _MSC_VER

class static_string {
  const char* const p_;
  const std::size_t sz_;

 public:
  typedef const char* const_iterator;

  template <std::size_t N>
  CONSTEXPR11_TN static_string(const char(&a)[N]) NOEXCEPT_TN
 :
  p_(a), sz_(N-1) {
  }

  CONSTEXPR11_TN static_string(const char* p, std::size_t N) NOEXCEPT_TN
 :
  p_(p), sz_(N) {
  }

  CONSTEXPR11_TN const char* data() const NOEXCEPT_TN {
    return p_;
  }
  CONSTEXPR11_TN std::size_t size() const NOEXCEPT_TN {
    return sz_;
  }

  CONSTEXPR11_TN const_iterator begin() const NOEXCEPT_TN {
    return p_;
  }
  CONSTEXPR11_TN const_iterator end()   const NOEXCEPT_TN {
    return p_ + sz_;
  }

  CONSTEXPR11_TN char operator[](std::size_t n) const {
    return n < sz_ ? p_[n] : throw std::out_of_range("static_string");
  }
};

inline std::ostream& operator<<(std::ostream& os, static_string const& s) {
  return os.write(s.data(), s.size());
}

template <class T>
CONSTEXPR14_TN
static_string
type_name() {
#ifdef __clang__
  static_string p = __PRETTY_FUNCTION__;
  return static_string(p.data() + 31, p.size() - 31 - 1);
  // 31 should be 34 for clang supporting c++17
#elif defined(__GNUC__)
  static_string p = __PRETTY_FUNCTION__;
#if __cplusplus < 201402
  return static_string(p.data() + 36, p.size() - 36 - 1);
#else
  return static_string(p.data() + 46, p.size() - 46 - 1);
#endif
#elif defined(_MSC_VER)
  static_string p = __FUNCSIG__;
  return static_string(p.data() + 38, p.size() - 38 - 7);
#endif
}

#endif

/* Local variables: */
/* indent-tab-mode: nil */
/* tab-width: 2 */
/* c-basic-offset: 2 */
/* indent-tabs-mode: nil */
/* End: */
