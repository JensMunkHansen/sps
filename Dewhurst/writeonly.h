//
// Copyright (c) 2017 by Daniel Saks and Stephen C. Dewhurst.
//
// Permission to use, copy, modify, distribute, and sell this
// software for any purpose is hereby granted without fee, provided
// that the above copyright notice appears in all copies and
// that both that copyright notice and this permission notice
// appear in supporting documentation.
// The authors make no representation about the suitability of this
// software for any purpose. It is provided "as is" without express
// or implied warranty.
//
#ifndef WRITEONLY_H
#define WRITEONLY_H

#include <stddef.h>

namespace DandS
{

template <typename T>
class write_only
{
public:
  write_only() { check(); }
  write_only(T const& v)
    : m_(v)
  {
    check();
  }
  void operator=(T const& v) { m_ = v; }

private:
  T m_;
  write_only(write_only const&) = delete;
  write_only& operator=(write_only const&) = delete;
  static constexpr void check()
  {
    static_assert(sizeof(write_only) == sizeof(T), "writeonly size problem");
    static_assert(alignof(write_only) == alignof(T), "writeonly alignment problem");
    static_assert(offsetof(write_only, m_) == 0, "problem with offset in write_only");
  }
};

template <typename T>
class read_only
{
public:
  read_only() { check(); }
  operator T const&() const { return m_; }
  T const* operator&() const { return &m_; }

private:
  T m_;
  read_only(read_only const&) = delete;
  read_only& operator=(read_only const&) = delete;
  static constexpr void check()
  {
    static_assert(sizeof(read_only) == sizeof(T), "read_only size problem");
    static_assert(alignof(read_only) == alignof(T), "read_only alignment problem");
    static_assert(offsetof(read_only, m_) == 0, "problem with offset in read_only");
  }
};

template <typename T>
class unused
{
public:
  unused() { check(); }
  unused(unused const&) {}
  unused& operator=(unused const&) { return *this; }

private:
  T unused_;
  static constexpr void check()
  {
    static_assert(sizeof(unused) == sizeof(T), "unused size problem");
    static_assert(alignof(unused) == alignof(T), "unused alignment problem");
    static_assert(offsetof(unused, unused_) == 0, "problem with offset in unused");
  }
};

}

#endif
