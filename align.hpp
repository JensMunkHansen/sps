/**
 * @file   align.hpp
 * @author Jens Munk Hansen <jmh@debian9laptop.parknet.dk>
 * @date   Thu Sep 13 23:00:27 2018
 *
 * @brief
 *
 * Copyright 2018 Jens Munk Hansen
 */

#pragma once

#ifndef __cplusplus
# error This is a C++ header
#endif

namespace sps {

  template <unsigned int alignment>
  struct is_aligned {
    static_assert((alignment & (alignment - 1)) == 0, "Alignment must be a power of 2");
    static inline bool value(const void * ptr) {
      return (((uintptr_t)ptr) & (alignment - 1)) == 0;
    }
  };

  /*! \brief aligned struct
   *
   * @tparam A alignment
   *
   * VC++ doesn't like combining templates with __declspec. This is a
   * work-around using inheritance.
   *
   * e.g. it is possible to do
   *
   * template <class T>
   * class AlignedClass : sps::aligned<16> {
   *
   * };
   *
   * or even
   *
   * template <class T>
   * class AlignedToFourOfAKind : sps::aligned<4*sizeof(T)> {
   *
   * };
   *
   * Using G++ or Clang, there is no need for this work-around and
   * we can do stuff like
   *
   * template <class T>
   * struct __attribute__((aligned(4*sizeof(T)))) Data
   * {
   *   T a;
   * };
   *
   */
#ifdef _MSC_VER
#  pragma warning(push)
#  pragma warning(disable:4324)
  __declspec(align(1)) struct align1 {};
  __declspec(align(2)) struct align2 {};
  __declspec(align(4)) struct align4 {};
  __declspec(align(8)) struct align8 {};
  __declspec(align(16)) struct align16 {};
  __declspec(align(32)) struct align32 {};
  template<int A> struct aligned;
  template<> struct aligned<1> : align1 {};
  template<> struct aligned<2> : align2 {};
  template<> struct aligned<4> : align4 {};
  template<> struct aligned<8> : align8 {};
  template<> struct aligned<16> : align16 {};
  template<> struct aligned<32> : align32 {};
#  pragma warning(pop)
#else
  template<int A> struct aligned;
  template<> struct __attribute__((aligned(1))) aligned<1>   { };
  template<> struct __attribute__((aligned(2))) aligned<2>   { };
  template<> struct __attribute__((aligned(4))) aligned<4>   { };
  template<> struct __attribute__((aligned(8))) aligned<8>   { };
  template<> struct __attribute__((aligned(16))) aligned<16> { };
  template<> struct __attribute__((aligned(32))) aligned<32> { };
#endif
}  // namespace sps
