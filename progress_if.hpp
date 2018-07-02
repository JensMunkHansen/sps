/**
 * @file   progress_if.hpp
 * @author Jens Munk Hansen <jmh@debian9laptop.parknet.dk>
 * @date   Sun Jul  1 18:30:09 2018
 *
 * @brief
 *
 *  Copyright 2018 Jens Munk Hansen
 *
 *  This file is part of SOFUS.
 *
 *  SOFUS is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  SOFUS is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with SOFUS.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#pragma once

#include <sps/sps_export.h>

namespace sps {
class SPS_EXPORT ProgressBarInterface {
 public:
  virtual void show(float percent) = 0;
  virtual ~ProgressBarInterface() = default;

  template <typename P>
  void wait(const P& period);
};
}  // namespace sps
