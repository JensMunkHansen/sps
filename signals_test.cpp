/**
 * @file   signals_test.cpp
 * @author Jens Munk Hansen <jmh@jmhlaptop.parknet.dk>
 * @date   Sat May 30 16:09:42 2015
 *
 * @brief  Test of signal routines
 *
 * Copyright 2017 Jens Munk Hansen
 */

// Compile using g++ -I../ -c signals.cpp -std=c++11
//               g++ -I../ signals.o signals_test.cpp -std=c++11 -lfftw3f -lfftw3l -lfftw3

#include <sps/sps_export.h>
#include <sps/math.h>
#include <sps/msignals.hpp>
#include <sps/profiler.h>

#ifdef _MSC_VER
# include <algorithm>
#endif

#include <float.h>

#include <cstdlib>
#include <iostream>
#include <cassert>

#include <stdint.h>

#include <gtest/gtest.h>

using namespace sps;

template <typename T>
T test_fft_speed(const size_t na)
{

  size_t n = 16;
  // Fix Me
  signal1D<T> a(n);

  for (size_t i = 0 ; i < n ; i++) {
    a.data[i] = (T) i;
  }

  signal1D<std::complex<T> > c;

  bool result = fft<float>(a,n,c);
  SPS_UNREFERENCED_PARAMETER(result);
#if 0
  for (size_t i = 0 ; i < n/2 + 1 ; i++) {
    std::cout << c.data[i] << ", ";
  }
  std::cout << std::endl;
#endif

  signal1D<T> a1(na);

  for (size_t i = 0 ; i < na ; i++) {
    a1.data[i] = (T) i;
  }

  signal1D<std::complex<T> > c1;

  float ftotal = 0.0f;

  double time = sps::profiler::time();

  for (size_t i = 0 ; i < 10000000 ; i++) {
    result = fft<float>(a1,n,c1);
    ftotal += std::real(c1.data[na-1]);
    c1.data[0] = std::complex<float>(0.0f);
  }

  std::cout << "Elapsed: " << sps::profiler::time() - time << " seconds" << std::endl;

  std::cout << "ftotal: " << ftotal << std::endl;

  return T(0);//Signal1DPlan
}

template <typename T>
T test_fft(const size_t n)
{

  signal1D<T> a(n);

  for (size_t i = 0 ; i < n ; i++) {
    a.data[i] = (T) i;
  }

  signal1D<std::complex<T> > b;

  bool result = fft<T>(a,2*n,b);

  signal1D<T> c(n);

  result = ifft<T>(b,2*n,c);

#if 0
  for (size_t i = 0 ; i < n ; i++) {
    std::cout << c.data[i] << ", ";
  }
  std::cout << std::endl;
#endif

  T max_diff = 0.0f;

  for (size_t i=0 ; i < n ; i++) {
    max_diff = std::max<T>(max_diff, fabs(c.data[i] - a.data[i]));
  }
  return max_diff;
}

template <typename T>
T test_mfft(const size_t n)
{

  msignal1D<T> a(n);

  for (size_t i = 0 ; i < n ; i++) {
    a.m_data.get()[i] = (T) i;
  }

  msignal1D<std::complex<T> > b;

  bool result = mfft<T>(a,2*n,b);

  msignal1D<T> c(n);

  T max_diff = 0.0f;

  result = mifft<T>(b,2*n,c);

#if 0
  for (size_t i = 0 ; i < n ; i++) {
    std::cout << c.data.get()[i] << ", ";
  }
  std::cout << std::endl;
#endif

  for (size_t i=0 ; i < n ; i++) {
    max_diff = std::max<T>(max_diff, fabs(c.m_data.get()[i] - a.m_data.get()[i]));
  }

  return max_diff;
}

template <typename T>
T test_conv(const size_t na, const size_t nb)
{

  signal1D<T> a(na);

  for (size_t i = 0 ; i < na ; i++) {
    a.data[i] = (T) i;
  }

  signal1D<T> b(nb);

  for (size_t i = 0 ; i < nb ; i++) {
    b.data[i] = (T) i;
  }

  signal1D<T> c;
  bool result = conv_fft<T>(b,a,c);

  T max_diff = 0.0f;

  signal1D<T> c1;

  result = conv<T>(b,a,c1);

  SPS_UNREFERENCED_PARAMETER(result);

  for (size_t i=0 ; i < c1.ndata ; i++) {
    max_diff = std::max<T>(max_diff, fabs(c1.data[i] - c.data[i]));
  }
  return max_diff;
}

template <typename T>
T test_conv_in_place(const size_t na, const size_t nb)
{

  size_t nInternal = 2 * next_power_two<size_t>(na+nb+1) + 2;

  signal1D<T> a(nb,nInternal*sizeof(float));

  signal1D<T> b(nb);

  for (size_t i = 0 ; i < na ; i++) {
    a.data[i] = (T) i;
  }

  for (size_t i = 0 ; i < nb ; i++) {
    b.data[i] = (T) i;
  }

  signal1D<T> c;
  bool result  = conv_fft<T>(b,a,c);

  SPS_UNREFERENCED_PARAMETER(result);

  T max_diff = 0.0f;

  conv_fft_out_in<float>(b,a);

  for (size_t i=0 ; i < a.ndata ; i++) {
    max_diff = std::max<T>(max_diff, fabs(a.data[i] - c.data[i]));
  }

  return max_diff;
}

template <typename T>
T test_mconv(const size_t na, const size_t nb)
{

  signal1D<T>   a(na);
  msignal1D<T> ma(na);

  for (size_t i = 0 ; i < na ; i++) {
    a.data[i] = (T) i;
    ma.m_data.get()[i] = (T) i;
  }

  signal1D<T> b(nb);
  msignal1D<T> mb(nb);

  for (size_t i = 0 ; i < nb ; i++) {
    b.data[i] = (T) i;
    mb.m_data.get()[i] = (T) i;
  }

  msignal1D<T> mc;
  bool result  = mconv_fft<T>(mb,ma,mc);

  T max_diff = 0.0f;

  signal1D<T> c;

  result  = conv<T>(b,a,c);

  SPS_UNREFERENCED_PARAMETER(result);

  for (size_t i=0 ; i < c.ndata ; i++) {
    max_diff = std::max<T>(max_diff, fabs(mc.m_data.get()[i] - c.data[i]));
  }
  return max_diff;
}

template <typename T>
T test_mconv_unmanaged(const size_t na, const size_t nb)
{

  signal1D<T>   a(na);
  msignal1D<T> ma(na);

  for (size_t i = 0 ; i < na ; i++) {
    a.data[i] = (T) i;
    ma.m_data.get()[i] = (T) i;
  }

  signal1D<T> b(nb);
  msignal1D<T> mb(nb);

  for (size_t i = 0 ; i < nb ; i++) {
    b.data[i] = (T) i;
    mb.m_data.get()[i] = (T) i;
  }

  signal1D<T> c;
  bool result  = mconv_fft<T>(mb,ma,c);

  T max_diff = 0.0f;

  signal1D<T> c1;

  result  = conv<T>(b,a,c1);

  SPS_UNREFERENCED_PARAMETER(result);

  for (size_t i=0 ; i < c.ndata ; i++) {
    max_diff = std::max<T>(max_diff, fabs(c.data[i] - c1.data[i]));
  }
  return max_diff;
}

TEST(signals_test, sample_test)
{
  EXPECT_EQ(1, 1);
}

TEST(signals_test, test_conv_float)
{
  size_t na = 7;
  size_t nb = 6;
  size_t nc = na + nb - 1;

  float fmax_diff = test_conv<float>(na,nb);
  // std::cout << "Maximum difference: " << fmax_diff << std::endl;
  assert((fmax_diff  / (2*next_power_two<size_t>(nc)) < 1.1 * FLT_EPSILON ) && "conv_fft too inaccurate");
  ASSERT_LT( (fmax_diff  / (2*next_power_two<size_t>(nc))), 1.1 * FLT_EPSILON);
}

TEST(signals_test, test_conv_float1)
{
  size_t na = 1;
  size_t nb = 1;
  size_t nc = na + nb - 1;

  float fmax_diff = test_conv<float>(na,nb);
  // std::cout << "Maximum difference: " << fmax_diff << std::endl;
  assert((fmax_diff  / (2*next_power_two<size_t>(nc)) < 1.1 * FLT_EPSILON ) && "conv_fft too inaccurate");
  ASSERT_LT( (fmax_diff  / (2*next_power_two<size_t>(nc))), 1.1 * FLT_EPSILON);
}

TEST(signals_test, test_conv_double)
{
  size_t na = 7;
  size_t nb = 6;
  size_t nc = na + nb - 1;

  double dmax_diff = test_conv<double>(na,nb);
  assert((dmax_diff  / (2*next_power_two<size_t>(nc)) < 1.1 * DBL_EPSILON ) && "conv_fft too inaccurate");
  ASSERT_LT((dmax_diff  / (2*next_power_two<size_t>(nc))), 1.1 * DBL_EPSILON);
}

TEST(signals_test, test_conv_in_place_float)
{
  size_t na = 7;
  size_t nb = 6;
  size_t nc = na + nb - 1;

  float fmax_diff = test_conv_in_place<float>(na,nb);
  assert((fmax_diff  / (2*next_power_two<size_t>(nc)) < 1.1 * FLT_EPSILON ) && "conv_in_place too inaccurate");
  ASSERT_LT( (fmax_diff  / (2*next_power_two<size_t>(nc))), 1.1 * FLT_EPSILON);
}

TEST(signals_test, test_mconv_float)
{
  size_t na = 7;
  size_t nb = 6;
  size_t nc = na + nb - 1;

  float fmax_diff = test_mconv<float>(na,nb);
  // std::cout << "Maximum difference: " << fmax_diff << std::endl;
  assert((fmax_diff  / (2*next_power_two<size_t>(nc)) < 1.1 * FLT_EPSILON ) && "conv_fft too inaccurate");
  ASSERT_LT( (fmax_diff  / (2*next_power_two<size_t>(nc))), 1.1 * FLT_EPSILON);
}

TEST(signals_test, test_mconv_unmanaged_float)
{
  size_t na = 7;
  size_t nb = 6;
  size_t nc = na + nb - 1;

  float fmax_diff = test_mconv_unmanaged<float>(na,nb);
  assert((fmax_diff  / (2*next_power_two<size_t>(nc)) < 1.1 * FLT_EPSILON ) && "conv_fft too inaccurate");
  ASSERT_LT( (fmax_diff  / (2*next_power_two<size_t>(nc))), 1.1 * FLT_EPSILON);
}

TEST(signals_test, test_mconv_float1)
{
  size_t na = 1;
  size_t nb = 1;
  size_t nc = na + nb - 1;

  float fmax_diff = test_mconv<float>(na,nb);
  assert((fmax_diff  / (2*next_power_two<size_t>(nc)) < 1.1 * FLT_EPSILON ) && "conv_fft too inaccurate");
  ASSERT_LT( (fmax_diff  / (2*next_power_two<size_t>(nc))), 1.1 * FLT_EPSILON);
}

TEST(signals_test, test_fft)
{
  const size_t n = 8;
  float fmax_diff = test_fft<float>(n);

  assert((fmax_diff  / (2.0*next_power_two<size_t>(n)) < 1.1 * FLT_EPSILON ) && "fft too inaccurate");
  ASSERT_LT( (fmax_diff  / (2.0*next_power_two<size_t>(n))), 1.1 * FLT_EPSILON);
}

TEST(signals_test, test_mfft)
{
  const size_t n = 8;
  float fmax_diff = test_mfft<float>(n);

  assert((fmax_diff  / (2.0*next_power_two<size_t>(n)) < 1.1 * FLT_EPSILON ) && "fft too inaccurate");
  ASSERT_LT( (fmax_diff  / (2.0*next_power_two<size_t>(n))), 1.1 * FLT_EPSILON);
}

TEST(signals_test, test_mfft_double)
{
  const size_t n = 8;
  double fmax_diff = test_mfft<double>(n);

  assert((fmax_diff  / (2.0*next_power_two<size_t>(n)) < 1.1 * FLT_EPSILON ) && "fft too inaccurate");
  ASSERT_LT( (fmax_diff  / (2.0*next_power_two<size_t>(n))), 1.1 * FLT_EPSILON);
}

TEST(signals_test, test_scale_array)
{
  for (size_t i = 1 ; i < 20 ; i++) {
    float* fInput = (float*) malloc(i*sizeof(float));
    for (size_t j = 0 ; j < i ; j++) {
      fInput[j] = (float)i;
    }
    float scale = float(i);

    DivideArray<float>(fInput, i, scale);

    float sum = 0.0f;
    for (size_t j = 0 ; j < i ; j++) {
      sum += fInput[j];
    }
    ASSERT_LT(fabs(sum - (float)i), FLT_EPSILON);
    free(fInput);
  }

  for (size_t i = 1 ; i < 20 ; i++) {
    double* fInput = (double*) malloc(i*sizeof(double));
    for (size_t j = 0 ; j < i ; j++) {
      fInput[j] = (double)i;
    }
    double scale = double(i);

    DivideArray<double>(fInput, i, scale);

    double sum = 0.0f;
    for (size_t j = 0 ; j < i ; j++) {
      sum += fInput[j];
    }
    ASSERT_LT(fabs(sum - (double)i), FLT_EPSILON);
    free(fInput);
  }
}

int main(int argc, char** argv)
{
  testing::InitGoogleTest(&argc, argv);
  //    testing::GTEST_FLAG(filter) = "*float*";
  //    testing::GTEST_FLAG(filter) = "*test_conv_float";
  return RUN_ALL_TESTS();
}


