#pragma once

#include <sps/sps_export.h>
#include <sps/smath_types.hpp>

namespace sps {
template <typename T>
struct
#ifdef __GNUG__
__attribute__((aligned(4*sizeof(T))))
#endif
SPS_EXPORT mat3_t {
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
  T* operator[](size_t index) {
    return data[index];
  }
  const T* operator[](size_t index) const {
    return data[index];
  }

  T data[3][3];
};

template <typename T>
inline mat3_t<T> operator*(const T& s, const mat3_t<T>& mat) {
  mat3_t<T> result;
  for (size_t i = 0 ; i < 3 ; i++) {
    for (size_t j = 0 ; j < 3 ; j++) {
      result[i] = result[i] * s;
    }
  }
  return result;
}

}  // namespace sps
