/**
 * @file   magic_ring_buffer.hpp
 * @author Jens Munk Hansen <jens.munk.hansen@gmail.com>
 * @date   Thu Oct 19 01:34:56 2017
 *
 * @brief  Magic ring buffer.
 *
 * Copyright 2017 Jens Munk Hansen
 *
 */
/*
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
 *  TODO: support NUMA nodes, SHMAT and MMAP
 * numa_mem_id() or cpu_to_mem()
 */

#pragma once

namespace sps
{
class IMagicRingBuffer
{
public:
  virtual ~IMagicRingBuffer() = default;
  IMagicRingBuffer(size_t size, unsigned int flags = 0);
  size_t write(size_t len, void* pData);
  size_t read(size_t len, void* pData);
};
} // namespace sps
