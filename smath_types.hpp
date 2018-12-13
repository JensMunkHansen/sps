/**
 * @file   smath_types.hpp
 * @author Jens Munk Hansen <jens.munk.hansen@gmail.com>
 * @date   Sat Mar 18 13:55:23 2017
 *
 * @brief  Mathematical structures
 *
 * Copyright 2017 Jens Munk Hansen
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

#include <sps/sps_export.h>

#ifdef _MSC_VER
# include <sps/aligned_array.hpp>
#else
# include <array>
#endif

#include <sps/memory>   // For sps::aligned

#ifdef _WIN32
namespace std {
template class SPS_EXPORT aligned_array<float, 4>;
template class SPS_EXPORT aligned_array<double, 4>;
}
#endif

/** @addtogroup SPS */
/*@{*/

namespace sps {

/*! \brief Point type (aligned)
 *
 *
 *  Point as an aligned array
 */
#ifdef _MSC_VER

template <typename T>
struct SPS_EXPORT point_t : public std::aligned_array<T, 4>

// Microsoft VS2013 does not allow mxing declspec(align()) with template

#else

template <typename T>
struct __attribute__((aligned(4*sizeof(T))))
SPS_EXPORT point_t : public std::array<T, 4>

#endif
{
  static const point_t xaxis;   ///< x-axis
  static const point_t yaxis;   ///< y-axis
  static const point_t zaxis;   ///< z-axis

  point_t() = default;

  point_t(const T& a, const T& b, const T&c) {
    (*this)[0] = a;
    (*this)[1] = b;
    (*this)[2] = c;
  }
#ifdef _MSC_VER
  point_t(std::initializer_list<T> args) :
    std::aligned_array<T, 4>(reinterpret_cast<std::aligned_array<T, 4> const&>(
                               *(args.begin()))) {}
#else
  using std::array<T, 4>::array;

  point_t(std::initializer_list<T> args) :
    std::array<T, 4>(reinterpret_cast<std::array<T, 4> const&>(
                       *(args.begin()))) {}
#endif
  point_t<T>& operator+=(const point_t<T>& other) {
    (*this)[0] += other[0];
    (*this)[1] += other[1];
    (*this)[2] += other[2];
    return *this;
  }
};



template <typename T>
const point_t<T> point_t<T>::xaxis = point_t<T>(T(1.0), T(0.0), T(0.0));

template <typename T>
const point_t<T> point_t<T>::yaxis = point_t<T>(T(0.0), T(1.0), T(0.0));

template <typename T>
const point_t<T> point_t<T>::zaxis = point_t<T>(T(0.0), T(0.0), T(1.0));

/*! \brief Vector aliased as point
 *
 */
template <typename T>
using vector_t = point_t<T>;

// Non-optimal
template <typename T>
struct SPS_EXPORT mat3_t {
  mat3_t<T> operator*(const mat3_t<T>& other) {
    mat3_t<T> result;
    for (size_t i = 0 ; i < 3 ; i++) {
      for (size_t j = 0 ; j < 3 ; j++) {
        result.data[i][j] = T(0.0);
        for (size_t k = 0 ; k < 3 ; k++) {
          result.data[i][j] =
            result.data[i][j]
            + this->data[i][k] * other.data[k][j];
        }
      }
    }
    return result;
  }

  point_t<T> operator*(const point_t<T>& vec) {
    point_t<T> result = {T(0.0), T(0.0), T(0.0)};

    for (size_t i = 0 ; i < 3 ; i++) {
      for (size_t j = 0 ; j < 3 ; j++) {
        result[i] = result[i] + data[i][j] * vec[j];
      }
    }
    return result;
  }

  T data[3][3];
};




/*! \brief Rectangle
 *
 *
 *  Rectangle structure as an array of \ref sps::point_t
 */
#ifdef _MSC_VER
template <typename T>
struct SPS_EXPORT rect_t : public std::aligned_array<point_t<T>, 4> {};
#else
template <typename T>
struct __attribute__((aligned(4*sizeof(T)))) SPS_EXPORT rect_t :
public std::array<point_t<T>, 4> {};
#endif

/*! \brief Bounding box
 *
 *
 *  The bounding box is used for fast parallel computation and to
 *  optimize the cache usage.
 */
template <typename T>
struct SPS_EXPORT bbox_t {
  /// Boundary minimum
  point_t<T> min;
  /// Boundary maximum
  point_t<T> max;
};

/*! \brief Euler angles
 *
 *
 *  Euler angles, the z-x-z' convention is used.
 */
template <typename T>
struct SPS_EXPORT euler_t : sps::aligned<4*sizeof(T)> {
  /// alpha
  T alpha;
  /// beta
  T beta;
  /// gamma
  T gamma;
  /// Dummy used by alignment
  T dummy;
};

template <typename T>
struct SPS_EXPORT circle_t : sps::aligned<4*sizeof(T)> {
  sps::point_t<T> center;  ///< Center position
  sps::euler_t<T> euler;   ///< Euler angles
  T radius;                ///< Radius
};

template <typename T>
struct SPS_EXPORT ellipsis_t : sps::aligned<4*sizeof(T)> {
  sps::point_t<T> center;  ///< Center position
  sps::euler_t<T> euler;   ///< Euler angles
  T hw;                    ///< Half width (half axis)
  T hh;                    ///< Half height (half axis)
};


/*! \brief Element representation
 *
 *  The element representation is used no matter if the
 *  PP(Plane-parallel)-wave approximation is used or the more
 *  accurate analytical responses.
 */

#if defined(_WIN32)
template <typename T>
struct SPS_EXPORT element_rect_t : sps::aligned<4*sizeof(T)>
#else
template <typename T>
struct __attribute__((aligned(4*sizeof(T)))) SPS_EXPORT element_rect_t
#endif
{
  SPS_ALIGNAS(sizeof(T) * 4) sps::point_t<T> center;  ///< Center position
  SPS_ALIGNAS(sizeof(T) * 4) sps::euler_t<T> euler;   ///< Euler angles
  /** @name Cached variables (careful with alignment)
   *
   */
  ///@{

  /// Normal vector
  SPS_ALIGNAS(sizeof(T) * 4) T normal[4] ALIGN32_END;
  /// First basis vector
  SPS_ALIGNAS(sizeof(T) * 4) T uvector[4] ALIGN32_END;
  /// Second basis vector
  SPS_ALIGNAS(sizeof(T) * 4) T vvector[4] ALIGN32_END;

  /// Four vertices, xyz major-order
  SPS_ALIGNAS(sizeof(T) * 4) T vertices[3][4] ALIGN32_END;

  ///@}
  T hw;        ///< Half width
  T hh;        ///< Half height
  T dummy[2];  ///< Dummy for alignment
};

template <typename T>
struct SPS_EXPORT element_circular_t : sps::aligned<4*sizeof(T)> {
  sps::circle_t<T> circle;  // Circular element

  /** @name Cached variables - alignas(4*sizeof(T))
   *
   */
  ///@{
#if defined(_MSC_VER) && (_MSC_VER < 1900)
  /// Normal vector
  ALIGN16_BEGIN T normal[4] ALIGN16_END;
  /// First basis vector
  ALIGN16_BEGIN T uvector[4] ALIGN16_END;
  /// Second basis vector
  ALIGN16_BEGIN T vvector[4] ALIGN16_END;
#else
  /// Normal vector
  alignas(4*sizeof(T)) T normal[4];
  /// First basis vector
  alignas(4*sizeof(T)) T uvector[4];
  /// Second basis vector
  alignas(4*sizeof(T)) T vvector[4];
#endif
  ///@}
};

#ifdef _MSC_VER
template struct SPS_EXPORT bbox_t<float>;
template struct SPS_EXPORT bbox_t<double>;
#endif
}  // namespace sps

/*@}*/

/* Local variables: */
/* indent-tabs-mode: nil */
/* tab-width: 2 */
/* c-basic-offset: 2 */
/* End: */
