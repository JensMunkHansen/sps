/**
 * @file   resource.cpp
 * @author Jens Munk Hansen <jmh@debian9laptop.parknet.dk>
 * @date   Mon Mar  5 21:01:20 2018
 *
 * @brief
 *
 * Copyright 2017 Jens Munk Hansen
 *
 */

#include <sps/resource.hpp>
#include <cstdlib>
#include <atomic>

namespace sps {
std::uint32_t Resource::UIDCreate() {
  static std::atomic<std::uint32_t> g_uid = {0};
  std::uint32_t retval = std::atomic_fetch_add(&g_uid, 1U);
  if (retval + 1 == 0) {
    exit(-1);
  }
  return retval;
}
}  // namespace sps

/* Local variables: */
/* indent-tabs-mode: nil */
/* tab-width: 2 */
/* c-basic-offset: 2 */
/* End: */
