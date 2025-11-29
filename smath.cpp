/**
 * @file   smath.cpp
 * @author Jens Munk Hansen <jmh@jmhlaptop>
 * @date   Sun Apr 30 22:52:50 2017
 *
 * @brief
 *
 * Copyright 2017 Jens Munk Hansen
 */

#include <sps/cmath>
#include <sps/config.h>
#include <sps/debug.h>
#include <sps/trigintrin.h>

#include <sps/smath.hpp>

#include <iostream>

namespace std
{
template struct SPS_EXPORT std::pair<float, float>;
template struct SPS_EXPORT std::pair<double, double>;
}

namespace sps
{

template <typename T>
bool SPS_EXPORT EigenValuesOfSymmetricMatrix(
  mat3_t<T> matrix, T* eigenValue1, T* eigenValue2, T* eigenValue3)
{
  if (!matrix.issymmetric())
    return false;

  // t := Tr(A) / 3
  T t = matrix.trace() / T(3);
  // K := A - tI
  sps::mat3_t<T> mK = matrix - t * sps::mat3_t<T>::identity;
  // q := |K|/2
  T q = mK.det() / T(2);
  // p := Tr(K^2)/6
  T p = (mK * mK).trace() / T(6.0);
  // y := Sqrt(p^3 - q^2)
  T y = p * p * p - q * q;
  y = y > 0 ? sqrt(y) : T(0);

  // phi := 1/3 * Angle([x,q])
  T phi = atan2(y, q);
  if (phi < T(0.0))
    phi = phi + T(M_PI);
  phi = phi / T(3.0);

  // Calculate eigen values
  p = sqrt(p);
  T f1 = p * cos(phi);
  T f2 = p * sqrt(3) * fabs(sin(phi));

  eigenValue1 = t + 2 * f1;
  eigenValue2 = t - f1 - f2;
  eigenValue3 = t - f1 + f2;

  // Sort them in non-decreasing order (we already know dEigen2 <= dEigen3)
  if (eigenValue1 > eigenValue2)
  {
    T temp = eigenValue1;
    eigenValue1 = eigenValue2;
    eigenValue2 = temp;
    if (eigenValue2 > eigenValue3)
    {
      temp = eigenValue2;
      eigenValue2 = eigenValue3;
      eigenValue3 = temp;
    }
  }
  return true;
}

template <typename T>
bool SPS_EXPORT svd(
  const sps::mat3_t<T>& mA, sps::mat3_t<T>* u, sps::point_t<T>* s, sps::mat3_t<T>* v)
{
  SPS_UNREFERENCE_PARAMETERS(mA, u, s, v);

  // bool retval = false; // TODO: Implement SVD properly
  sps::mat3_t<T> mT = mA.transpose();
  sps::mat3_t<T> mSqr = mT * mA;

  // Calculate the eigenvalues
  T dEigen1, dEigen2, dEigen3;
  EigenValuesOfSymmetricMatrix(mSqr, &dEigen1, &dEigen2, &dEigen3);
#if 0
  //Due to round off error eigenvalues can be negative.
  dEigen1 = max(dEigen1, 0);
  dEigen2 = max(dEigen2, 0);
  dEigen3 = max(dEigen3, 0);

  // Compare them
  Boolean b12Equal = fabs(dEigen1 - dEigen2) < RealMath.EpsilonDoubleWeak;
  Boolean b23Equal = fabs(dEigen2 - dEigen3) < RealMath.EpsilonDoubleWeak;
  // Look at different cases w.r.t. multiplicities
  if (b12Equal && b23Equal) // One eigen value with multiplicity 3
  {
    v = sps::mat3_t<T>.Identity;
    T d0 = sqrt(dEigen1);
    d = new Vector3d(d0, d0, d0);
  }
  else if (b12Equal) // One eigen value with multiplicity 2 (smallest), one with multiplicity 1
  {
    // Find eigen vector for the eigen value of multiplicity 1
    Vector3d vEigen3 = EigenVectorForEigenValueOfGeometricMultiplicityOne(mSqr, dEigen3);
    // Expand to an orthonormal basis
    Vector3d vEigen1 = vEigen3.MakeOrthogonalVector().Normalize();
    Vector3d vEigen2 = vEigen1.Cross(ref vEigen3).Normalize();
    v = new sps::mat3_t<T>(vEigen3, vEigen2, vEigen1);
    T d2 = sqrt(dEigen1);
    T d0 = sqrt(dEigen3);
    d = new Vector3d(d0, d2, d2);
  }
  else if (b23Equal) // One eigen value with multiplicity 2 (largest), one with multiplicity 1
  {
    // Find eigen vector for the eigen value of multiplicity 1
    Vector3d vEigen1 = EigenVectorForEigenValueOfGeometricMultiplicityOne(mSqr, dEigen1);
    // Expand to an orthonormal basis
    Vector3d vEigen2 = vEigen1.MakeOrthogonalVector().Normalize();
    Vector3d vEigen3 = vEigen1.Cross(ref vEigen2).Normalize();
    v = new sps::mat3_t<T>(vEigen3, vEigen2, vEigen1);
    T d2 = sqrt(dEigen1);
    T d0 = sqrt(dEigen3);
    d = new Vector3d(d0, d0, d2);
  }
  else // Three eigen values of multiplicity 1
  {
    // Find eigen vectors for two eigen values of multiplicity 1
    Vector3d vEigen2 = EigenVectorForEigenValueOfGeometricMultiplicityOne(mSqr, dEigen2);
    Vector3d vEigen3 = EigenVectorForEigenValueOfGeometricMultiplicityOne(mSqr, dEigen3);
    // When eigenvalues are near each other, numerical errors in EigenVectorForEigenvalueOfGeometricMultiplicityOne
    // can become large, and vEigen2 and vEigen3 might not be strictly orthonormal. Apply correction:
    vEigen3 = (vEigen3 - vEigen2 * vEigen3.Dot(vEigen2)).Normalize();

    // Expand to an orthonormal basis
    Vector3d vEigen1 = vEigen2.Cross(ref vEigen3).Normalize();
    v = new sps::mat3_t<T>(vEigen3, vEigen2, vEigen1);
    d = new Vector3d(sqrt(dEigen3), sqrt(dEigen2), sqrt(dEigen1));
  }

  if (dEigen3 <= RealMath.EpsilonDoubleWeak) //all singular values are zero
  {
    d = Vector3d.Zero;
    u = sps::mat3_t<T>.Zero;
    return false;
  }

  sps::mat3_t<T> uNotNorm = sps::mat3_t<T>.Mult(ref mA, ref v);
  if (dEigen2 <= RealMath.EpsilonDoubleWeak)
  {
    // Since ||uNotNorm.GetCol(0)|| = ||dEigen3 * v.GetCol(0)||, dEigen3 > 0 and v.GetCol(0) is unit,
    // ||uNotNorm.GetCol(0)|| cannot be zero
    u = new sps::mat3_t<T>(uNotNorm.GetCol(0).Normalize(), Vector3d.Zero, Vector3d.Zero);
    d.Y = 0;
    d.Z = 0;
    return false;
  }
  if (dEigen1 <= RealMath.EpsilonDoubleWeak)
  {
    u = new sps::mat3_t<T>(uNotNorm.GetCol(0).Normalize(), uNotNorm.GetCol(1).Normalize(), Vector3d.Zero);
    d.Z = 0;
    return false;
  }

  u = new sps::mat3_t<T>(uNotNorm.GetCol(0).Normalize(), uNotNorm.GetCol(1).Normalize(), uNotNorm.GetCol(2).Normalize());
  return true;
#endif
  return true;
}

/*
 */

template <typename T, typename U>
std::pair<T, T> minmax_delay(const T* xs, const U* ws, size_t nData)
{
  std::pair<T, T> result = std::make_pair(T(0.0), T(0.0));
  bool bFirst = true;
  if (xs)
  {
    if (ws)
    {
      for (size_t i = 0; i < nData; i++)
      {
        if (fabs(ws[i]) > T(0.0))
        {
          if (bFirst)
          {
            result.first = result.second = xs[i];
            bFirst = false;
          }
          else
          {
            result.first = std::min<T>(result.first, xs[i]);
            result.second = std::max<T>(result.second, xs[i]);
          }
        }
      }
    }
    else
    {
      for (size_t i = 0; i < nData; i++)
      {
        if (bFirst)
        {
          result.first = result.second = xs[i];
          bFirst = false;
        }
        else
        {
          result.first = std::min<T>(result.first, xs[i]);
          result.second = std::max<T>(result.second, xs[i]);
        }
      }
    }
  }
  return result;
}

template <typename T, RotationConvention conv>
void euler2rot(const sps::euler_t<T>& euler, sps::mat3_t<T>* mat)
{
  // TODO: Make this a class and partially specialize
  //       this for convention yxy
  sps::point_t<T> v0, v1, v2;
  basis_vectors<T, conv>(&v0, euler, 0);
  basis_vectors<T, conv>(&v1, euler, 1);
  basis_vectors<T, conv>(&v2, euler, 2);
  for (size_t i = 0; i < 3; ++i)
  {
    mat->data[i][0] = v0[i];
    mat->data[i][1] = v1[i];
    mat->data[i][2] = v2[i];
  }
}

// TODO(JMH): This is the yxy specialization - not generic
template <typename T, RotationConvention conv>
void SPS_EXPORT rot2euler(const sps::mat3_t<T>& rot, euler_t<T>* euler)
{
  size_t iFirstAxis = 1;
  int repetition = 1;
  size_t parity = 1;
  int intrin = 1; // rotating frame

  size_t iNextAxis[4] = { 1, 2, 0, 1 };

  size_t i = iFirstAxis;                // 1
  size_t j = iNextAxis[i + parity];     // 0
  size_t k = iNextAxis[i - parity + 1]; // 2

  euler->alpha = T(0.0);
  euler->beta = T(0.0);
  euler->gamma = T(0.0);

  T sy;
  bool bSingular;

  if (repetition)
  {
    sy = sqrt(SQUARE(rot.data[i][j]) + SQUARE(rot.data[i][k]));
    bSingular = sy < T(10.0) * std::numeric_limits<T>::epsilon();

    euler->alpha = atan2(rot.data[i][j], rot.data[i][k]);
    euler->beta = atan2(sy, rot.data[i][i]);
    euler->gamma = atan2(rot.data[j][i], -rot.data[k][i]);

    if (bSingular)
    {
      euler->alpha = atan2(-rot.data[j][k], rot.data[j][j]);
      euler->beta = atan2(sy, rot.data[i][i]);
      euler->gamma = T(0.0);
    }
  }
  else
  {
    sy = sqrt(SQUARE(rot.data[i][i]) + SQUARE(rot.data[j][i]));
    bSingular = sy < T(10.0) * std::numeric_limits<T>::epsilon();

    euler->alpha = atan2(rot.data[k][j], rot.data[k][k]);
    euler->beta = atan2(-rot.data[k][i], sy);
    euler->gamma = atan2(rot.data[j][i], rot.data[i][i]);

    if (bSingular)
    {
      euler->alpha = atan2(-rot.data[j][k], rot.data[j][j]);
      euler->beta = atan2(-rot.data[k][i], sy);
      euler->gamma = T(0.0);
    }
  }

  if (parity)
  {
    euler->alpha = -euler->alpha;
    euler->beta = -euler->beta;
    euler->gamma = -euler->gamma;
  }

  if (intrin)
  {
    std::swap(euler->alpha, euler->gamma);
  }
}

template <typename T, RotationConvention conv>
void basis_vectors(sps::point_t<T>* output, const euler_t<T>& euler, size_t index)
{
  const T alpha = euler.alpha;
  const T beta = euler.beta;
  const T gamma = euler.gamma;

  const T sa = sin(alpha);
  const T ca = cos(alpha);
  const T sb = sin(beta);
  const T cb = cos(beta);
  const T sc = sin(gamma);
  const T cc = cos(gamma);

#if ROTATION_CONVENTION == ROTATION_CONVENTION_EULER_ZYZ
#pragma message("Rotation used is zyz")
  switch (index)
  {
    case 0:
      (*output)[0] = ca * cc - cb * sa * sc;
      (*output)[1] = -cb * cc * sa - ca * sc;
      (*output)[2] = sa * sb;
      break;
    case 1:
      (*output)[0] = cc * sa + ca * cb * sc;
      (*output)[1] = ca * cb * cc - sa * sc;
      (*output)[2] = -ca * sb;
      break;
    case 2:
      (*output)[0] = sb * sc;
      (*output)[1] = cc * sb;
      (*output)[2] = cb;
      break;
  }
#elif ROTATION_CONVENTION == ROTATION_CONVENTION_EULER_YXY
#pragma message("Rotation used is yxy")
  // Intrinsic rotations, y, x and y'
  switch (index)
  {
    case 0:
      // Mult by (a,b,c) to get x component
      (*output)[0] = ca * cc - cb * sa * sc;
      (*output)[1] = sb * sc;
      (*output)[2] = -ca * cb * sc - cc * sa;
      break;
    case 1:
      (*output)[0] = sa * sb;
      (*output)[1] = cb;
      (*output)[2] = ca * sb;
      break;
    case 2:
      (*output)[0] = ca * sc + cb * cc * sa;
      (*output)[1] = -cc * sb;
      (*output)[2] = ca * cb * cc - sa * sc;
      break;
  }
#elif ROTATION_CONVENTION == ROTATION_CONVENTION_EXTRINSIC_XYZ
#pragma message("Rotation used is extrinsic xyz")
  switch (index)
  {
    case 0:
      (*output)[0] = cb * cc;
      (*output)[1] = cb * sc;
      (*output)[2] = -sb;
      break;
    case 1:
      (*output)[0] = -ca * sc + cc * sa * sb;
      (*output)[1] = ca * cc + sa * sb * sc;
      (*output)[2] = cb * sa;
      break;
    case 2:
      (*output)[0] = ca * cc * sb + sa * sc;
      (*output)[1] = ca * sb * sc - cc * sa;
      (*output)[2] = ca * cb;
      break;
  }
#endif
}

template <>
void basis_vectors<float, sps::EulerIntrinsicYXY>(
  sps::point_t<float>* output, const euler_t<float>& euler, size_t index)
{
  const float alpha = euler.alpha;
  const float beta = euler.beta;
  const float gamma = euler.gamma;

  const float sa = sin(alpha);
  const float ca = cos(alpha);
  const float sb = sin(beta);
  const float cb = cos(beta);
  const float sc = sin(gamma);
  const float cc = cos(gamma);

  // Intrinsic rotations, y, x and y'
  switch (index)
  {
    case 0:
      // Mult by (a,b,c) to get x component
      (*output)[0] = ca * cc - cb * sa * sc;
      (*output)[1] = sb * sc;
      (*output)[2] = -ca * cb * sc - cc * sa;
      break;
    case 1:
      (*output)[0] = sa * sb;
      (*output)[1] = cb;
      (*output)[2] = ca * sb;
      break;
    case 2:
      (*output)[0] = ca * sc + cb * cc * sa;
      (*output)[1] = -cc * sb;
      (*output)[2] = ca * cb * cc - sa * sc;
      break;
  }
}

template <>
void basis_vectors<double, sps::EulerIntrinsicYXY>(
  sps::point_t<double>* output, const euler_t<double>& euler, size_t index)
{
  const double alpha = euler.alpha;
  const double beta = euler.beta;
  const double gamma = euler.gamma;

  const double sa = sin(alpha);
  const double ca = cos(alpha);
  const double sb = sin(beta);
  const double cb = cos(beta);
  const double sc = sin(gamma);
  const double cc = cos(gamma);

  // Intrinsic rotations, y, x and y'
  switch (index)
  {
    case 0:
      // Mult by (a,b,c) to get x component
      (*output)[0] = ca * cc - cb * sa * sc;
      (*output)[1] = sb * sc;
      (*output)[2] = -ca * cb * sc - cc * sa;
      break;
    case 1:
      (*output)[0] = sa * sb;
      (*output)[1] = cb;
      (*output)[2] = ca * sb;
      break;
    case 2:
      (*output)[0] = ca * sc + cb * cc * sa;
      (*output)[1] = -cc * sb;
      (*output)[2] = ca * cb * cc - sa * sc;
      break;
  }
}

template <typename T, RotationConvention conv>
void basis_rotate(const sps::point_t<T>& input, const euler_t<T>& euler, sps::point_t<T>* output)
{
  sps::point_t<T> vec0, vec1, vec2;
  basis_vectors<T, conv>(&vec0, euler, 0);
  basis_vectors<T, conv>(&vec1, euler, 1);
  basis_vectors<T, conv>(&vec2, euler, 2);

  (*output)[0] = vec0[0] * input[0] + vec1[0] * input[1] + vec2[0] * input[2];
  (*output)[1] = vec0[1] * input[0] + vec1[1] * input[1] + vec2[1] * input[2];
  (*output)[2] = vec0[2] * input[0] + vec1[2] * input[1] + vec2[2] * input[2];
}

template <>
void basis_vectors(float* vec0, float* vec1, float* vec2, const sps::euler_t<float>& euler)
{
  __m128 a;

#ifdef _WIN32
  // Euler is not aligned on Windows
  a = _mm_loadu_ps(reinterpret_cast<const float*>(&euler));
#else
  a = _mm_load_ps(reinterpret_cast<const float*>(&euler));
#endif
  v4f c, s;
  _mm_sin_cos_ps(a, &s.v, &c.v);

  v4f* p0 = reinterpret_cast<v4f*>(vec0);
  v4f* p1 = reinterpret_cast<v4f*>(vec1);
  v4f* p2 = reinterpret_cast<v4f*>(vec2);

#if ROTATION_CONVENTION == ROTATION_CONVENTION_EULER_ZYZ
  p0->f32[0] = c.f32[0] * c.f32[2] - c.f32[2] * s.f32[0] * s.f32[2];
  p0->f32[1] = -c.f32[1] * c.f32[2] * s.f32[0] - c.f32[0] * s.f32[2];
  p0->f32[2] = s.f32[0] * s.f32[1];

  p1->f32[0] = c.f32[2] * s.f32[0] + c.f32[2] * c.f32[1] * s.f32[2];
  p1->f32[1] = c.f32[0] * c.f32[1] * c.f32[2] - s.f32[0] * s.f32[2];
  p1->f32[2] = -c.f32[0] * s.f32[1];

  p2->f32[0] = s.f32[1] * s.f32[2];
  p2->f32[1] = c.f32[2] * s.f32[1];
  p2->f32[2] = c.f32[1];
#elif ROTATION_CONVENTION == ROTATION_CONVENTION_EULER_YXY
  // Verify this
  p0->f32[0] = c.f32[0] * c.f32[2] - c.f32[1] * s.f32[0] * s.f32[2];
  p0->f32[1] = s.f32[1] * s.f32[2];
  p0->f32[2] = -c.f32[0] * c.f32[1] * s.f32[2] - c.f32[2] * s.f32[0];

  p1->f32[0] = s.f32[0] * s.f32[1];
  p1->f32[1] = c.f32[1];
  p1->f32[2] = c.f32[0] * s.f32[1];

  p2->f32[0] = c.f32[0] * s.f32[2] + c.f32[1] * c.f32[2] * s.f32[0];
  p2->f32[1] = -c.f32[2] * s.f32[1];
  p2->f32[2] = c.f32[0] * c.f32[1] * c.f32[2] - s.f32[0] * s.f32[2];
#endif
}

template <>
void basis_vectors(double* vec0, double* vec1, double* vec2, const sps::euler_t<double>& euler)
{
  // TODO(JMH): TEST ME
  const double alpha = euler.alpha;
  const double beta = euler.beta;
  const double gamma = euler.gamma;

  const double sa = sin(alpha);
  const double ca = cos(alpha);
  const double sb = sin(beta);
  const double cb = cos(beta);
  const double sc = sin(gamma);
  const double cc = cos(gamma);

  // Intrinsic rotations, y, x and y'
#if ROTATION_CONVENTION == ROTATION_CONVENTION_EULER_ZYZ
  vec0[0] = ca * cc - cb * sa * sc;
  vec0[1] = -cb * cc * sa - ca * sc;
  vec0[2] = sa * sb;

  vec1[0] = cc * sa + ca * cb * sc;
  vec1[1] = ca * cb * cc - sa * sc;
  vec2[2] = -ca * sb;

  vec2[0] = sb * sc;
  vec2[1] = cc * sb;
  vec2[2] = cb;
#elif ROTATION_CONVENTION == ROTATION_CONVENTION_EULER_YXY
  vec0[0] = ca * cc - cb * sa * sc;
  vec0[1] = sb * sc;
  vec0[2] = -ca * cb * sc - cc * sa;

  vec1[0] = sa * sb;
  vec1[1] = cb;
  vec1[2] = ca * sb;

  vec2[0] = ca * sc + cb * cc * sa;
  vec2[1] = -cc * sb;
  vec2[2] = ca * cb * cc - sa * sc;
#endif
}

template <typename T>
void dist_point_to_circle_local(
  const point_t<T>& point, const circle_t<T>& circle, T* r, T* z, T* distNear)
{
  sps::point_t<T> normal, uvector, vvector;
  sps::basis_vectors<T, sps::EulerIntrinsicYXY>(&uvector, circle.euler, 0);
  sps::basis_vectors<T, sps::EulerIntrinsicYXY>(&vvector, circle.euler, 1);
  sps::basis_vectors<T, sps::EulerIntrinsicYXY>(&normal, circle.euler, 2);

  sps::point_t<T> r2p = point - circle.center;

  *z = fabs(dot(r2p, normal));
  T dist2center = norm(r2p);

  // r-coordinate, r^2 = u^2 + v^2
  *r = sqrt(std::max<T>(SQUARE(dist2center) - SQUARE(*z), T(0.0)));

  if (*r < circle.radius)
  {
    // Projection is inside circle
    *distNear = fabs(*z);
  }
  else
  {
    *distNear = sqrt(std::max<T>(SQUARE(*z) + SQUARE(*r - circle.radius), T(0.0)));
  }
}

template <typename T>
void dist_point_to_circle_local(
  const point_t<T>& point, const circle_t<T>& circle, T* r, T* z, T* distNear, T* distFar)
{
  sps::point_t<T> normal, uvector, vvector;
  sps::basis_vectors<T, sps::EulerIntrinsicYXY>(&uvector, circle.euler, 0);
  sps::basis_vectors<T, sps::EulerIntrinsicYXY>(&vvector, circle.euler, 1);
  sps::basis_vectors<T, sps::EulerIntrinsicYXY>(&normal, circle.euler, 2);

  sps::point_t<T> r2p = point - circle.center;

  *z = fabs(dot(r2p, normal));
  T dist2center = norm(r2p);

  // r-coordinate, r^2 = u^2 + v^2
  *r = sqrt(std::max<T>(SQUARE(dist2center) - SQUARE(*z), T(0.0)));

  if (*r < circle.radius)
  {
    *distFar = sqrt(std::max<T>(SQUARE(*z) + SQUARE(circle.radius + *r), T(0.0)));
    *distNear = *z;
  }
  else
  {
    *distNear = sqrt(std::max<T>(SQUARE(*z) + SQUARE(*r - circle.radius), T(0.0)));
    *distFar = sqrt(std::max<T>(SQUARE(*z) + SQUARE(circle.radius + *r), T(0.0)));
  }
}

template <typename T>
T dist_point_to_circle(const point_t<T>& point, const circle_t<T>& circle)
{
  T distNear, r, z;
  dist_point_to_circle_local(point, circle, &r, &z, &distNear);
  return distNear;
}

template <typename T>
void arc_point_ellipsis(const sps::ellipsis_t<T>& ellipsis, const T& arc, sps::point_t<T>* point)
{
  (*point)[2] = T(0.0);
  if (sps::almost_equal(arc, T(M_PI_2), 1))
  {
    (*point)[0] = T(0.0);
    (*point)[1] = ellipsis.hh;
  }
  else if (sps::almost_equal(arc, T(M_3PI_2), 1))
  {
    (*point)[0] = T(0.0);
    (*point)[1] = -ellipsis.hh;
  }
  else
  {
    T a = ellipsis.hw;
    T b = ellipsis.hh;
    if (sps::almost_equal(b, T(0.0), 1))
    {
      (*point)[1] = T(0.0);
      if (fabs(arc) < M_PI_2)
      {
        (*point)[0] = a;
      }
      else
      {
        (*point)[0] = -a;
      }
    }
    else
    {
      if ((arc > M_PI_2) && (arc < M_3PI_2))
      {
        (*point)[0] = -a * b / sqrt(SQUARE(b) + SQUARE(a * tan(arc)));
        (*point)[1] = -a * b * tan(arc) / sqrt(SQUARE(b) + SQUARE(a * tan(arc)));
      }
      else
      {
        // 0 <= arc < M_PI_2 or M_3PI_2 < arc <= M_2PI
        (*point)[0] = a * b / sqrt(SQUARE(b) + SQUARE(a * tan(arc)));
        (*point)[1] = a * b * tan(arc) / sqrt(SQUARE(b) + SQUARE(a * tan(arc)));
      }
    }
  }
}
template <typename T>
void tan_point_ellipsis(
  const sps::ellipsis_t<T>& ellipsis, const T& y, const T& x, sps::point_t<T>* point)
{
  (*point)[2] = T(0.0);
  // Catches sign
  if (sps::almost_equal(x, T(0.0), 1))
  {
    (*point)[0] = T(0.0);
    if (y > 0)
    {
      (*point)[1] = ellipsis.hh;
    }
    else
    {
      (*point)[1] = -ellipsis.hh;
    }
  }
  else if (sps::almost_equal(y, T(0.0), 1))
  {
    (*point)[1] = T(0.0);
    if (x > 0)
    {
      (*point)[0] = ellipsis.hw;
    }
    else
    {
      (*point)[0] = -ellipsis.hw;
    }
  }
  else
  {
    T a = ellipsis.hw;
    T b = ellipsis.hh;
    T tan = y / x;
    (*point)[0] = a * b / sqrt(SQUARE(b) + SQUARE(a * tan));
    (*point)[1] = (a * b * tan) / sqrt(SQUARE(b) + SQUARE(a * tan));
    if (x < 0)
    {
      (*point)[0] *= T(-1);
      (*point)[1] *= T(-1);
    }
  }
}

// WRONG
template <typename T>
void intcp_line_rect(
  const sps::element_rect_t<T>& rect, const T& y, const T& x, sps::point_t<T>* point)
{
  (*point)[2] = T(0.0);
  if (sps::almost_equal(x, T(0.0), 1))
  {
    (*point)[0] = T(0.0);
    if (y > 0)
    {
      (*point)[1] = rect.hh;
    }
    else
    {
      (*point)[1] = -rect.hh;
    }
  }
  else if (sps::almost_equal(y, T(0.0), 1))
  {
    (*point)[1] = T(0.0);
    if (x > 0)
    {
      (*point)[0] = rect.hw;
    }
    else
    {
      (*point)[0] = -rect.hw;
    }
  }
  else
  {
    T tan = y / x;
    if (fabs(tan * rect.hw) > rect.hh)
    {
      (*point)[0] = std::copysign(T(1.0), x) * T(1.0) / fabs(tan) * rect.hh;
      (*point)[1] = std::copysign(T(1.0), y) * rect.hh;
    }
    else
    {
      (*point)[0] = std::copysign(T(1.0), x) * rect.hw;
      (*point)[1] = std::copysign(T(1.0), y) * fabs(tan) * rect.hw;
    }
  }
}

#ifdef _WIN32
template class sps::aligned_array<float, 4U>;
template class sps::aligned_array<double, 4U>;
#endif

template struct euler_t<float>;
template struct point_t<float>;
template struct mat3_t<float>;
template struct element_rect_t<float>;

#ifdef _WIN32
// Not possible to move to fnm library
template class sps::aligned_array<sps::point_t<float>, 4U>;
template class sps::aligned_array<sps::point_t<double>, 4U>;
#endif

template point_t<float> operator-(const point_t<float>& a, const point_t<float>& b);
template point_t<float> operator+(const point_t<float>& a, const point_t<float>& b);
template point_t<float> operator*(const float& a, const point_t<float>& b);
template float dot(const point_t<float>& a, const point_t<float>& b);
template point_t<float> cross(const point_t<float>& a, const point_t<float>& b);
template float norm(const point_t<float>& a);
template float dist_point_to_line(
  const point_t<float>& point, const point_t<float>& pointOnLine, const point_t<float>& direction);

// These are explicit specializations, instantiation has no effect
// template void SPS_EXPORT
// basis_vectors<float, sps::EulerIntrinsicYXY>(sps::point_t<float>* output,
//     const sps::euler_t<float>& euler, size_t index);

template void SPS_EXPORT euler2rot<float, sps::EulerIntrinsicYXY>(
  const sps::euler_t<float>& euler, sps::mat3_t<float>* rot);

template void SPS_EXPORT rot2euler<float, sps::EulerIntrinsicYXY>(
  const sps::mat3_t<float>& rot, euler_t<float>* euler);

template std::ostream& operator<<(std::ostream& out, const point_t<float>& point);

template std::ostream& operator<<(std::ostream& out, const mat3_t<float>& point);

// template void
// SPS_EXPORT basis_vectors(float* vec0, float* vec1, float* vec2, const sps::euler_t<float>&
// euler);

template void SPS_EXPORT basis_rotate<float, sps::EulerIntrinsicYXY>(
  const sps::point_t<float>& input, const sps::euler_t<float>& euler, sps::point_t<float>* output);

template void SPS_EXPORT dists_most_distant_and_closest(
  const sps::bbox_t<float>& box0, const sps::bbox_t<float>& box1, float* distNear, float* distFar);

// Is this a problem, when it is exported (it must be export using SOFUS_EXPORT when used in SOFUS)
template std::pair<float, float> minmax_delay<float, float>(
  const float* xs, const float* ws, size_t nData);

template float SPS_EXPORT dist_point_to_circle<float>(
  const sps::point_t<float>& point, const sps::circle_t<float>& circle);
template void SPS_EXPORT dist_point_to_circle_local<float>(const sps::point_t<float>& point,
  const sps::circle_t<float>& circle, float* r, float* z, float* distNear, float* distFar);
template void SPS_EXPORT dist_point_to_circle_local<float>(const sps::point_t<float>& point,
  const sps::circle_t<float>& circle, float* r, float* z, float* distNear);

template void intcp_line_rect(const sps::element_rect_t<float>& rect, const float& y,
  const float& x, sps::point_t<float>* point);

template void arc_point_ellipsis(
  const sps::ellipsis_t<float>& ellipsis, const float& arc, sps::point_t<float>* point);

template void tan_point_ellipsis(const sps::ellipsis_t<float>& ellipsis, const float& y,
  const float& x, sps::point_t<float>* point);

template struct SPS_EXPORT element_rect_t<double>;

template point_t<double> operator-(const point_t<double>& a, const point_t<double>& b);
template point_t<double> operator+(const point_t<double>& a, const point_t<double>& b);
template point_t<double> operator*(const double& a, const point_t<double>& b);
template double dot(const point_t<double>& a, const point_t<double>& b);
template point_t<double> cross(const point_t<double>& a, const point_t<double>& b);
template double norm(const point_t<double>& a);
template double dist_point_to_line(const point_t<double>& point, const point_t<double>& pointOnLine,
  const point_t<double>& direction);

// template void SPS_EXPORT basis_vectors<double, EulerIntrinsicYXY>(
//   sps::point_t<double>* output, const sps::euler_t<double>& euler, size_t index);
template std::ostream& operator<<(std::ostream& out, const point_t<double>& point);
// template void SPS_EXPORT basis_vectors(
//   double* vec0, double* vec1, double* vec2,
//   const sps::euler_t<double>& euler);
template void SPS_EXPORT basis_rotate<double, EulerIntrinsicYXY>(const sps::point_t<double>& input,
  const sps::euler_t<double>& euler, sps::point_t<double>* output);

template void SPS_EXPORT dists_most_distant_and_closest(const sps::bbox_t<double>& box0,
  const sps::bbox_t<double>& box1, double* distNear, double* distFar);

template double SPS_EXPORT dist_point_to_circle<double>(
  const sps::point_t<double>& point, const sps::circle_t<double>& circle);
template void SPS_EXPORT dist_point_to_circle_local<double>(const sps::point_t<double>& point,
  const sps::circle_t<double>& circle, double* r, double* z, double* distNear, double* distFar);
template void SPS_EXPORT dist_point_to_circle_local<double>(const sps::point_t<double>& point,
  const sps::circle_t<double>& circle, double* r, double* z, double* distNear);

template std::pair<double, double> minmax_delay<double, double>(
  const double* xs, const double* ws, size_t nData);
} // namespace sps

/* Local variables: */
/* indent-tabs-mode: nil */
/* tab-width: 2 */
/* c-basic-offset: 2 */
/* End: */
