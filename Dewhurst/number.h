//
// Copyright (c) 2017 by Daniel Saks and Stephen C. Dewhurst.
//
// Permission to use, copy, modify, distribute, and sell this
// software for any purpose is hereby granted without fee, provided
// that the above copyright notice appears in all copies and
// that both that copyright notice and this permission notice
// appear in supporting documentation.
// The authors make no representation about the suitability of this
// software for any purpose. It is provided "as is" without express
// or implied warranty.
//
#ifndef INCLUDED_NUMBER
#define INCLUDED_NUMBER

#include <iostream>
#include <type_traits>

//=============================================================
//
// Inspired by Fedor Picus's restricted value types.
//

namespace DandS
{

//
// Recursively get the underlying arithmetic type of our Numeric types.
// The implementation will be completed with an additional partial
// specialization to follow below.
//
template <typename N>
struct underlying_arithmetic_type
{
  using type = N;
};

template <typename N>
using underlying_arithmetic_type_t = typename underlying_arithmetic_type<N>::type;

//
// A simple Number implementation with a wide CRTP hierarchy.
// For compilers that can handle MI EBO.
//
template <typename N, template <typename...> class... CRTPs>
class Number : public CRTPs<Number<N, CRTPs...>>...
{
public:
  using S = std::decay_t<underlying_arithmetic_type_t<N>>;
  constexpr Number() // note: intentionally uninitialized
  {
    check();
  }
  constexpr Number(S value)
    : value_(value)
  {
    check();
  }
  constexpr S value() const { return value_; }
  constexpr void set_value(S a) { value_ = a; }

private:
  static constexpr void check()
  {
    constexpr auto is_a = std::is_arithmetic<underlying_arithmetic_type_t<N>>::value;
    constexpr auto is_sl = std::is_standard_layout<Number>::value;
    static_assert(sizeof(Number) == sizeof(N), "problem with size of Number");
    static_assert(alignof(Number) == alignof(N), "problem with alignment of Number");
    static_assert(is_a, "underlying type for Number must be arithmetic");
    static_assert(is_sl, "Number is not standard layout");
  }
  N value_;
};

//
// The rest of the "underlying" implementation started above.
//
template <typename N, template <typename...> class... CRTPs>
struct underlying_arithmetic_type<Number<N, CRTPs...>>
{
  using type = underlying_arithmetic_type_t<N>;
};

//
// Components for building the Number of your choice.
//

// A terminator for a deep single inheritance heirarchy.
// (Not used here.)
//
class Last
{
};

template <typename T, typename Next = Last>
struct Stream_i : Next
{
  friend std::ostream& operator<<(std::ostream& a, T b) { return a << b.value(); }
};

template <typename T, typename Next = Last>
struct Add_i : Next
{
  friend T operator+(T a, T b) { return a.value() + b.value(); }
};

template <typename T, typename Next = Last>
struct Shift_i : Next
{
  friend T operator<<(T a, T b) { return T(a.value() << b.value()); }
  friend T operator>>(T a, T b) { return T(a.value() >> b.value()); }
};

template <typename T, typename Next = Last>
struct Eq_i : Next
{
  friend constexpr bool operator==(T a, T b) { return a.value() == b.value(); }
  friend constexpr bool operator!=(T a, T b) { return a.value() != b.value(); }
};

template <typename T, typename Next = Last>
struct Bit_i : Next
{
  using U = underlying_arithmetic_type_t<T>;

  friend constexpr T operator&(T a, T b) { return T(a.value() & b.value()); }
  friend constexpr T operator|(T a, T b) { return T(a.value() | b.value()); }
  friend constexpr T operator~(T a) { return T(~a.value()); }

  friend constexpr T operator&(U a, T b) { return T(a & b.value()); }
  friend constexpr T operator|(U a, T b) { return T(a | b.value()); }

  friend constexpr T operator&(T a, U b) { return T(a.value() & b); }
  friend constexpr T operator|(T a, U b) { return T(a.value() | b); }
};

}

#endif
