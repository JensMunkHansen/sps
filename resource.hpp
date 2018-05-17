/**
 * @file   resource.hpp
 * @author Jens Munk Hansen <jmh@debian9laptop.parknet.dk>
 * @date   Mon Mar  5 21:00:13 2018
 *
 * @brief
 *
 * Copyright 2017 Jens Munk Hansen
 */
#pragma once

#include <cstdint>

namespace sps {
class Resource {
 public:
  static std::uint32_t UIDCreate();
};
}  // namespace sps

/* Local variables: */
/* indent-tabs-mode: nil */
/* tab-width: 2 */
/* c-basic-offset: 2 */
/* End: */
