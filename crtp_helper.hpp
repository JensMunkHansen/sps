/**
 * @file   crtp_helper.hpp
 * @author Jens Munk Hansen <jmh@debian9laptop.parknet.dk>
 * @date   Sun Apr 29 14:43:51 2018
 *
 * @brief
 *
 * Copyright 2018 Jens Munk Hansen
 */
#pragma once
namespace sps {
template <typename T, template<typename> class crtpType>
class CRTP {
 public:
  /**
   *
   *
   *
   * @return
   */
  T& underlying() {
    return static_cast<T&>(*this);
  }

  /**
   *
   *
   *
   * @return
   */
  T const& underlying() const {
    return static_cast<T const&>(*this);
  }

  /**
   *
   *
   *
   * @return
   */
  T* thisp() {
    return static_cast<T*>(this);
  }

  /**
   *
   *
   *
   * @return
   */
  const T* thisp() const {
    return static_cast<const T*>(this);
  }

 private:
  CRTP() {}
  friend crtpType<T>;
};
}  // namespace sps
