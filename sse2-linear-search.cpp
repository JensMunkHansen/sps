#include <sps/cenv.h>
#include <sps/sse2-linear-search.hpp>

#include <sps/extintrin.h>

#include <algorithm>

// __lzcnt() exposed by Microsoft

#ifdef __GNUC__
# define clz(x) __builtin_clz(x)
# define ctz(x) __builtin_ctz(x)
#elif defined(_MSC_VER)
# include <intrin.h>
uint32_t __inline ctz(uint32_t value) {
  unsigned long trailing_zero = 0;

  if (_BitScanForward(&trailing_zero, value)) {
    return trailing_zero;
  } else {
    return 32;
  }
}
# if _MSC_VER < 1900
uint32_t __inline clz(uint32_t value) {
  unsigned long leading_zero = 0;
  // Intrinsic calling lzcnt (if available)
  if (_BitScanReverse(&leading_zero, value)) {
    return 31 - leading_zero;
  } else {
    return 32;
  }
}
# else
# define clz(x) __lzcnt(x)
# endif
#endif

namespace sps {

template <>
int SSELinearQuadSearch<float, 0>::
Find(const float (&value)[4], int indices[4]) const {
  const float *ptr = m_pData;
  int i = 0;

  // Possibly misaligned start
  int misalignment = (uintptr_t)(ptr + i) & 0xF;

  __m128 v_value[4];
  __m128 cmp = _mm_setzero_ps();
  __m128 arrVec = _mm_loadu_ps(static_cast<const float*>(ptr + i));

  for (size_t j = 0 ; j < 4 ; j++) {
    v_value[j] = _mm_set1_ps(value[j]);
  }

  int mask;
  int allfound = 1;
  indices[0] = indices[1] = indices[2] = indices[3] = static_cast<int>(m_nData);

  for (size_t j = 0 ; j < 4 ; j++) {
    cmp = _mm_cmpge_ps(arrVec, v_value[j]);
    mask = _mm_movemask_ps(cmp);
    if (mask != 0) {
      indices[j] = i + ctz(mask);
    }
    allfound *= mask;
  }

  if (allfound) {
    return 0;
  }

  auto offset = (16 - misalignment) / 4;
  i += offset;

  for (; i < m_nData ; i += 4) {
    arrVec = _mm_loadu_ps(static_cast<const float*>(ptr + i));
    allfound = 1;
    for (size_t j = 0 ; j < 4 ; j++) {
      cmp = _mm_cmpge_ps(arrVec, v_value[j]);
      mask = _mm_movemask_ps(cmp);
      if (mask != 0) {
        indices[j] = std::min(indices[j],
                              i + static_cast<int>(ctz(mask)));
      }
      allfound *= mask;
      if (allfound) {
        return 0;
      }
    }
  }
  return -1;
}

template <>
int SSE2LinearSearch<int32_t,0>::operator=(const int32_t& key) const {
  auto vecX = _mm_set1_epi32(key);
  const int *ptr = m_pData;
  int i = 0;  // Guess

  // Possibly misaligned start
  int misalignment = (uintptr_t)(ptr + i) & 15;
  auto arrVec = _mm_loadu_si128(reinterpret_cast<const __m128i*>(ptr + i));

  auto cmp = _mm_cmpeq_epi32(arrVec, vecX);
  cmp = _mm_packs_epi32(cmp, _mm_setzero_si128());
  int mask = _mm_movemask_ps(_mm_castsi128_ps(cmp));
  if (mask) {
    return i + ctz(mask);
  }

  auto offset = (16 - misalignment) / 4;
  i += offset;
  for (; i < m_nData ; i += 16) {
    auto av0 = _mm_load_si128(reinterpret_cast<const __m128i*>(ptr + i));
    auto av1 = _mm_load_si128(reinterpret_cast<const __m128i*>(ptr + i + 4));
    auto av2 = _mm_load_si128(reinterpret_cast<const __m128i*>(ptr + i + 8));
    auto av3 = _mm_load_si128(reinterpret_cast<const __m128i*>(ptr + i + 12));
    auto cmp0 = _mm_cmpeq_epi32(av0, vecX);
    auto cmp1 = _mm_cmpeq_epi32(av1, vecX);
    auto cmp2 = _mm_cmpeq_epi32(av2, vecX);
    auto cmp3 = _mm_cmpeq_epi32(av3, vecX);

    cmp =
      _mm_packs_epi16(
        _mm_packs_epi32(cmp0, cmp1),
        _mm_packs_epi32(cmp2, cmp3));
    mask = _mm_movemask_epi8(cmp);
    if (mask)
      return i + ctz(mask);
  }

  // Last unaligned part

  return -1;
}

template <>
int SSE2LinearSearch<float, 0>::operator>=(const float& key) const {
  const __m128 keys = _mm_set1_ps(key);
  const int n = static_cast<int>(m_nData);
  const int rounded = 8 * (n/8);

  for (int i=0; i < rounded; i += 8) {
    const __m128 vec1 =
      _mm_loadu_ps(static_cast<const float*>(&m_pData[i]));
    const __m128 vec2 =
      _mm_loadu_ps(static_cast<const float*>(&m_pData[i + 4]));

    const __m128 cmp1 = _mm_cmpge_ps(vec1, keys);
    const __m128 cmp2 = _mm_cmpge_ps(vec2, keys);

    const __m128i tmp  = _mm_packs_epi32(_mm_castps_si128(cmp1),
                                         _mm_castps_si128(cmp2));
    const uint32_t mask = _mm_movemask_epi8(tmp);

    if (mask != 0) {
      return i + ctz(mask)/2;
    }
  }

  for (int i = rounded; i < n; i++) {
    if (m_pData[i] >= key) {
      return i;
    }
  }

  return -1;
}

template <>
int SSE2LinearSearch<uint32_t, 0>::operator>=(const uint32_t& key) const {
  SPS_UNREFERENCED_PARAMETER(key);
  return -1;
}

template <>
int SSE2LinearSearch<uint32_t, 0>::operator=(const uint32_t& key) const {
  const __m128i keys = _mm_set1_epi32(key);
  const int n = static_cast<int>(m_nData);
  const int rounded = 8 * (n/8);

  for (int i=0; i < rounded; i += 8) {
    const __m128i vec1 =
      _mm_loadu_si128(reinterpret_cast<const __m128i*>(&m_pData[i]));
    const __m128i vec2 =
      _mm_loadu_si128(reinterpret_cast<const __m128i*>(&m_pData[i + 4]));

    const __m128i cmp1 = _mm_cmpeq_epi32(vec1, keys);
    const __m128i cmp2 = _mm_cmpeq_epi32(vec2, keys);

    const __m128i tmp  = _mm_packs_epi32(cmp1, cmp2);
    const uint32_t mask = _mm_movemask_epi8(tmp);

    if (mask != 0) {
      return i + ctz(mask)/2;
    }
  }

  for (int i = rounded; i < n; i++) {
    if (m_pData[i] == key) {
      return i;
    }
  }

  return -1;
}
}  // namespace sps

