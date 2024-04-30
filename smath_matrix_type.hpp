#pragma once

#include <sps/sps_export.h>
#include <cstddef>

namespace sps {

template <typename T>
bool isNear(const T& a, const T& b)
{
  // TODO: Specialize
  return fabs(a-b) < T(1e-8);
}

template <typename T>
struct
#ifdef __GNUG__
__attribute__((aligned(4*sizeof(T))))
#endif
SPS_EXPORT mat3_t {
  static const mat3_t<T> identity;

  mat3_t() {
    memset(this->data, 0, 9*sizeof(T));
  }
  mat3_t(const T& e00, const T& e01, const T& e02,
         const T& e10, const T& e11, const T& e12,
         const T& e20, const T& e21, const T& e22) {
    this->data[0][0] = e00;
    this->data[0][1] = e01;
    this->data[0][2] = e02;
    this->data[1][0] = e10;
    this->data[1][1] = e11;
    this->data[1][2] = e12;
    this->data[2][0] = e20;
    this->data[2][1] = e21;
    this->data[2][2] = e22;
  }
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

  T* operator[](size_t index) {
      return data[index];
  }

  const T* operator[](size_t index) const {
      return data[index];
  }

  mat3_t<T> operator+(const mat3_t<T>& other) {
    mat3_t<T> result;
    for (size_t i = 0 ; i < 3 ; i++) {
      for (size_t j = 0 ; j < 3 ; j++) {
        result.data[i][j] = (*this).data[i][j] + other.data[i][j];
      }
    }
    return result;
  }
  mat3_t<T> transpose() {
    mat3_t<T> result;
    for (size_t i = 0 ; i < 3 ; i++) {
      result.data[i][i] = (*this).data[i][i];
    }
    result.data[0][1] = (*this).data[1][0];
    result.data[0][2] = (*this).data[2][0];
    result.data[1][0] = (*this).data[0][1];
    result.data[1][2] = (*this).data[2][1];
    result.data[2][0] = (*this).data[0][2];
    result.data[2][1] = (*this).data[1][2];
    return result;
  }
  T trace() {
    return (*this).data[0][0] + (*this).data[1][1] + (*this).data[2][2];
  }
  bool issymmetric() const {
    return isNear(this->data[0][1], this->data[1][0]) &&
        isNear(this->data[1][2], this->data[2][1]) &&
        isNear(this->data[0][2], this->data[2][0]);

  }
#if 0
  point_t<T> operator*(const point_t<typename T>& vec) {
      point_t<T> result;
      result[0] = T(0.0);
      result[1] = T(0.0);
      result[2] = T(0.0);

    for (size_t i = 0 ; i < 3 ; i++) {
      for (size_t j = 0 ; j < 3 ; j++) {
        result[i] = result[i] + data[i][j] * vec[j];
      }
    }
    return result;
  }
#endif
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

template <typename T>
const mat3_t<T> mat3_t<T>::identity = mat3_t<T>(T(1.0), T(0.0), T(0.0),
                                                T(0.0), T(1.0), T(0.0),
                                                T(0.0), T(0.0), T(1.0));


}  // namespace sps
