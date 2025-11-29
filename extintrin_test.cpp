#include <cstdlib>
#include <cstring>
#include <sps/extintrin.h>
#include <sps/mm_malloc.h>

#include <algorithm>
#include <climits>
#include <iostream>
#include <random>
#include <stdint.h>

#include <gtest/gtest.h>

/**
 * Test full multiplication of 32-bit signed integers
 *
 */
void _mm_mul_epi32_full_test()
{
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<int64_t> dis(LONG_MIN, LONG_MAX);

  ALIGN16_BEGIN int32_t as[4] ALIGN16_END;
  ALIGN16_BEGIN int32_t bs[4] ALIGN16_END;

  for (size_t i = 0; i < 4; i++)
  {
    as[i] = static_cast<int32_t>(dis(gen));
    bs[i] = static_cast<int32_t>(dis(gen));
    int64_t c = static_cast<int64_t>(as[i]) * static_cast<int64_t>(bs[i]);
    std::cout << "a: " << as[i] << " b: " << bs[i] << " c: " << c << std::endl;
  }

  __m128i a = _mm_load_si128(reinterpret_cast<__m128i*>(as));
  __m128i b = _mm_load_si128(reinterpret_cast<__m128i*>(bs));

  __m128i c1 = _mm_setzero_si128();
  __m128i c2 = _mm_setzero_si128();
  _mm_mul_epi32_full(c1, c2, a, b);

  std::cout << "int64[0]: " << v4i(c1).int64[0] << " int64[1]: " << v4i(c1).int64[1] << std::endl;
  std::cout << "int64[2]: " << v4i(c2).int64[0] << " int64[3]: " << v4i(c2).int64[1] << std::endl;
}

void sad(const uint8_t* input, uint8_t* output)
{
  /*
    i = mask2 * 4
    j = mask0-1 * 4
    for (k = 0; k < 8; k = k + 1) {
      t0 = abs(a[i + k + 0] - b[j + 0])
      t1 = abs(a[i + k + 1] - b[j + 1])
      t2 = abs(a[i + k + 2] - b[j + 2])
      t3 = abs(a[i + k + 3] - b[j + 3])
      r[k] = t0 + t1 + t2 + t3
    }
  */
  __m128i xm0 = _mm_load_si128(reinterpret_cast<const __m128i*>(input));
  __m128i xm1 = _mm_load_si128(reinterpret_cast<const __m128i*>(input + 16));
  __m128i xm2 = _mm_load_si128(reinterpret_cast<const __m128i*>(input + 32));

  __m128i xm3 = _mm_mpsadbw_epu8(xm1, xm0, 0);
  __m128i xm4 = _mm_mpsadbw_epu8(xm1, xm0, 5);
  xm4 = _mm_add_epi16(xm4, xm3);
  // Shift combined result 8*8-bit to the right
  __m128i xm5 = _mm_alignr_epi8(xm2, xm1, 8);
  __m128i xm6 = _mm_mpsadbw_epu8(xm5, xm0, 2);
  __m128i xm7 = _mm_mpsadbw_epu8(xm5, xm0, 7);
  xm7 = _mm_add_epi16(xm7, xm6);
  int offset = 0;
  _mm_store_si128(reinterpret_cast<__m128i*>(output + offset), xm4);
  // Increase 16-bytes (was 8)
  _mm_store_si128(reinterpret_cast<__m128i*>(output + offset + 16), xm7);
  return;
}

TEST(extintrin_test, test_rsqrt_nr)
{
  ALIGN16_BEGIN float vout[4] ALIGN16_END;

  float max_diff[] = { 0.0f, 0.0f };
  float x, diff;
  __m128 vx, vr;

  for (size_t i = 0; i < 1000; i++)
  {
    x = float(i + 1) / 100.0f;
    vx = _mm_set1_ps(x);
    vr = _mm_rsqrt_nr_ps(vx);
    memcpy(vout, &vr, 16);
    diff = fabs(vout[0] - 1.0f / sqrtf(x));
    max_diff[0] = std::max<float>(diff, max_diff[1]);
  }
  assert((max_diff[0] < 1.2e-6) && "_mm_rsqrt_nr_ps too inaccurate");

  ASSERT_LT(max_diff[0], 2.1e-6);
}

TEST(extintrin_test, test_rsqrt)
{
  ALIGN16_BEGIN float vout[4] ALIGN16_END;

  float max_diff[] = { 0.0f, 0.0f };
  float x, diff;
  __m128 vx, vr;

  for (size_t i = 0; i < 1000; i++)
  {
    x = float(i + 1) / 100.0f;
    vx = _mm_set1_ps(x);
    vr = _mm_rsqrt_ps(vx);
    memcpy(vout, &vr, 16);
    diff = fabs(vout[0] - 1.0f / sqrtf(x));
    max_diff[0] = std::max<float>(diff, max_diff[0]);
  }
  assert((max_diff[0] < 1.2e-3) && "_mm_rsqrt_ps too inaccurate");

  ASSERT_LT(max_diff[0], 2.1e-3);
}

TEST(extintrin_test, test_tranpose4x4)
{
  ALIGN16_BEGIN float a[4][4] ALIGN16_END;
  ALIGN16_BEGIN float b[4][4] ALIGN16_END;

  for (size_t i = 0; i < 4; i++)
  {
    for (size_t j = 0; j < 4; j++)
    {
      a[i][j] = float(i * 4 + j);
    }
  }

  _mm_transpose_4x4_ps(a, b);

  for (size_t i = 0; i < 4; i++)
  {
    for (size_t j = 0; j < 4; j++)
    {
      ASSERT_EQ(a[i][j], b[j][i]);
    }
  }
}

TEST(extintrin_test, test_tranpose8x8)
{
  ALIGN32_BEGIN float a[8][8] ALIGN32_END;
  ALIGN32_BEGIN float b[8][8] ALIGN32_END;

  for (size_t i = 0; i < 8; i++)
  {
    for (size_t j = 0; j < 8; j++)
    {
      a[i][j] = float(i * 8 + j);
    }
  }

  _mm_transpose_8x8_ps(a, b);

  for (size_t i = 0; i < 8; i++)
  {
    for (size_t j = 0; j < 8; j++)
    {
      ASSERT_EQ(a[i][j], b[j][i]);
    }
  }
}

TEST(extintrin_test, test_transpose4x8)
{
  const size_t N = 8;
  const size_t M = 4;
  float xyzs[M][N]{};
  for (size_t i = 0; i < M; i++)
  {
    for (size_t j = 0; j < N; j++)
    {
      xyzs[i][j] = static_cast<float>((j + 1) * (i + 1));
    }
  }
  __m256 x0 = _mm256_loadu_ps(xyzs[0]);
  __m256 y0 = _mm256_loadu_ps(xyzs[1]);
  __m256 z0 = _mm256_loadu_ps(xyzs[2]);
  __m256 t0 = _mm256_loadu_ps(xyzs[3]);

  // Act
  _MM_TRANSPOSE8_LANE4_PS(x0, y0, z0, t0);

  // Store data into array memory
  alignas(32) float xyzTest[4][8];
  _mm256_store_ps(xyzTest[0], x0);
  _mm256_store_ps(xyzTest[1], y0);
  _mm256_store_ps(xyzTest[2], z0);
  _mm256_store_ps(xyzTest[3], t0);

  // Assert
  for (size_t i = 0; i < 4; i++)
  {
    // Test transpose of the first 4x4 matrix
    for (size_t j = 0; j < 4; j++)
    {
      ASSERT_EQ(xyzTest[i][j], xyzs[j][i]);
    }
    // Test transpose of the second 4x4 matrix
    for (size_t j = 5; j < 8; j++)
    {
      ASSERT_EQ(xyzTest[i][j], xyzs[j - 4][i + 4]);
    }
  }
}

int main(int argc, char* argv[])
{

  testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();

#if 0
  size_t nInputs = 24;
  uint16_t* input = (uint16_t*) _mm_malloc(nInputs*sizeof(uint16_t),16);

  uint16_t* output = (uint16_t*) _mm_malloc(nInputs*sizeof(uint16_t),16);
  memset(output,0,nInputs*sizeof(uint16_t));

  for (size_t i = 0 ; i < nInputs ; i++) {
    if (i < 3) {
      input[i] = (uint16_t)(i);
    } else if ((i > 7) && (i<12)) {
      input[i] = 7;
    } else {
      input[i] = 0;
    }
    std::cout << uint16_t(input[i]) << " ";
  }
  std::cout << std::endl;

  sad((uint8_t*)input,(uint8_t*)output);

  for (size_t i = 0 ; i < nInputs ; i++) {
    std::cout << uint16_t(output[i]) << " ";
  }
  std::cout << std::endl;

  _mm_free(input);
  _mm_free(output);

  _mm_mul_epi32_full_test();

  int64_t a = int64_t(1) << 62;
  int64_t b = int64_t(1) << 62;

  std::cout << a << std::endl;
  int64_t c = mulshift(a,b,62);
  std::cout << c << std::endl;
#endif
}
