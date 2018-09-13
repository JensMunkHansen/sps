/**
 * @file   align.h
 * @author Jens Munk Hansen <jmh@debian9laptop.parknet.dk>
 * @date   Thu Sep 13 22:17:44 2015
 *
 * @brief
 *
 * Copyright 2015 Jens Munk Hansen
 */

#ifndef SPS_ALIGN_H
#define SPS_ALIGN_H

#ifdef __GNUC__

/*! \brief Alignment structure
 *
 * Inherit from this to enforce stack alignment. For heap allocation
 * use an aligned version of malloc
 *
 * struct Aligned {
 *   sps_align16_t _alignment;
 *   float data[4];
 * };
 */
  typedef struct sps_align16 {
} sps_align16_t __attribute__((aligned(16)));
typedef struct sps_align32 {
} sps_align32_t __attribute__((aligned(32)));
typedef struct sps_align64 {
} sps_align64_t __attribute__((aligned(64)));
#elif defined(_MSC_VER)
typedef __declspec(align(16)) struct sps_align16 {
} sps_align16_t;
typedef __declspec(align(32)) struct sps_align16 {
} sps_align32_t;
typedef __declspec(align(64)) struct sps_align16 {
} sps_align64_t;
#endif

#endif  // SPS_ALIGN_H
