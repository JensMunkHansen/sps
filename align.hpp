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

#include <sps/debug.h>
#include <sps/cenv.h>  // TODO(JMH): Split this up
#include <sps/mm_malloc.h>

#include <cstdint>  // uintptr_t
#include <stdexcept>

#include <sps/crtp_helper.hpp>

#ifndef CXX11
# error This header requires at least C++11
#endif

namespace sps {

template <unsigned int alignment>
struct is_aligned {
  static_assert((alignment & (alignment - 1)) == 0,
                "Alignment must be a power of 2");
  static inline bool value(const void * ptr) {
    return (((uintptr_t)ptr) & (alignment - 1)) == 0;
  }
};

/*! \brief aligned struct for stack alignment
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
__declspec(align(64)) struct align64 {};
template<int A> struct aligned;
template<> struct aligned<1> : align1 {};
template<> struct aligned<2> : align2 {};
template<> struct aligned<4> : align4 {};
template<> struct aligned<8> : align8 {};
template<> struct aligned<16> : align16 {};
template<> struct aligned<32> : align32 {};
template<> struct aligned<64> : align64 {};
#  pragma warning(pop)
#else
template<int A> struct aligned;
template<> struct __attribute__((aligned(1))) aligned<1>   { };
template<> struct __attribute__((aligned(2))) aligned<2>   { };
template<> struct __attribute__((aligned(4))) aligned<4>   { };
template<> struct __attribute__((aligned(8))) aligned<8>   { };
template<> struct __attribute__((aligned(16))) aligned<16> { };
template<> struct __attribute__((aligned(32))) aligned<32> { };
template<> struct __attribute__((aligned(64))) aligned<64> { };
#endif



/*! \brief Alignment helper for ensuring heap alignment
 *
 * A class supplying providing aligned single and array
 * allocators. The class is implemented using the curious recurring
 * template pattern (CRTP). When C++17 is used and structures are
 * aligned using alignas, the default allocators and deallocators
 * respect alignment on the heap. For C++14, this is not the case.
 *
 * @tparam T The descendant, which satisfy alignment on stack and heap
 * @tparam Alignment The alignment, which must be a power of two
 *
 */
template <typename T, size_t Alignment = 16>
class dynaligned : public sps::aligned<Alignment> {
 public:
  void* operator new(std::size_t size) throw() {
    debug_print("aligned\n");
    T* ptr = static_cast<T*>(SPS_MM_MALLOC(size, Alignment));
    return ptr ? ptr : throw std::bad_alloc{};
  }

  void operator delete(void* ptr) {
    debug_print("aligned\n");
    SPS_MM_FREE(ptr);
  }

  void* operator new[]( std::size_t size) throw() {
    debug_print("aligned\n");
    T* ptr = static_cast<T*>(SPS_MM_MALLOC(size, Alignment));
    return ptr ? ptr : throw std::bad_alloc{};
  }

  void operator delete[](void* ptr, size_t count) {
    SPS_MM_FREE(ptr);
  }

#if CXX17
  /** @name C++17 provide over-aligned allocators and deallocators
   *
   * The (de)allocators are used provided that the structure is
   * aligned using alignas() and alignment is larger than
   * __STDCPP_DEFAULT_NEW_ALIGNMENT__
   */
  ///@{

  void* operator new(std::size_t size, std::align_val_t al) throw() {
    debug_print("aligned\n");
    static_assert(al == Alignment, "Alignment mismatch");
#if 1
    return ::operator new(size, al);
#else
    T* ptr = static_cast<T*>(SPS_MM_MALLOC(size, al));
    return ptr ? ptr : throw std::bad_alloc{};
#endif
  }

  void operator delete(void* ptr, std::align_val_t align) {
    debug_print("aligned\n");
#if 1
    return ::operator delete(ptr);
#else
    SPS_MM_FREE(ptr);
#endif
  }

  // Called if the above is left undefined
  void operator delete(void* ptr, std::size_t size, std::align_val_t align) {
    debug_print("aligned\n");
#if 1
    return ::operator delete(ptr);
#else
    SPS_MM_FREE(ptr);
#endif
  }

  void* operator new[]( std::size_t count, std::align_val_t al) throw() {
    debug_print("aligned\n");
    return ::operator new[](count, al);
  }

  void operator delete[](void* ptr, std::size_t count, std::align_val_t al) {
    debug_print("aligned\n");
    return ::operator delete[](ptr, count, al);  // Calls many dtors, free
  }

#endif
  ///@}

 private:
  dynaligned() = default;
  friend T;
};

}  // namespace sps

// dynamic heap alignment
