/**
 * @file   sse2-linear-search.hpp
 * @author Jens Munk Hansen <jens.munk.hansen@gmail.com>
 * @date   Thu Nov 30 19:02:46 2017
 *
 * @brief
 *
 * Copyright 2017 Jens Munk Hansen
 */

#include <cstddef>
#include <cstdint>

namespace sps
{

template <typename T, int A = 0>
class SSE2LinearSearch
{
public:
  SSE2LinearSearch(const T* pData, const size_t& nData)
    : m_pData(pData)
    , m_nData(nData)
  {
  }
  int operator=(const T& key) const;
  int operator>=(const T& key) const;

private:
  const T* m_pData;
  size_t m_nData;
};

template <typename T, int Algorithm = 0>
class SSELinearQuadSearch
{
public:
  SSELinearQuadSearch(const T* pData, const size_t& nData)
    : m_pData(pData)
    , m_nData(nData)
  {
  }
  int Find(const T (&value)[4], int indices[4]) const;
  inline int operator>=(const T& value) const { return Find(value); }

private:
  const T* m_pData;
  size_t m_nData;
};

} // namespace sps
