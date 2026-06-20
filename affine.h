#include <cstdint>
#include <immintrin.h>

static inline __m128 dot3_ps(__m128 a, __m128 b)
{
  __m128 m = _mm_mul_ps(a, b);
  __m128 shuf = _mm_shuffle_ps(m, m, _MM_SHUFFLE(2, 1, 0, 3));
  __m128 sum = _mm_add_ps(m, shuf);
  shuf = _mm_shuffle_ps(sum, sum, _MM_SHUFFLE(1, 0, 3, 2));
  sum = _mm_add_ps(sum, shuf);
  return _mm_shuffle_ps(sum, sum, _MM_SHUFFLE(0, 0, 0, 0)); // broadcast
}

// cross(a,b) for xyz in lanes 0..2, lane3 ignored
static inline __m128 cross3_ps(__m128 a, __m128 b)
{
  __m128 a_yzx = _mm_shuffle_ps(a, a, _MM_SHUFFLE(3, 0, 2, 1));
  __m128 b_yzx = _mm_shuffle_ps(b, b, _MM_SHUFFLE(3, 0, 2, 1));
  __m128 c = _mm_sub_ps(_mm_mul_ps(a, b_yzx), _mm_mul_ps(a_yzx, b));
  return _mm_shuffle_ps(c, c, _MM_SHUFFLE(3, 0, 2, 1));
}

// Invert affine matrix (row-major 4x4). Assumes last row is [0 0 0 1].
static inline void invert_affine_4x4_sse(const float* M, float* Out)
{
  // Load top 3 rows; keep xyz in lanes 0..2, and tx/ty/tz in lane 3 for convenience.
  __m128 r0 = _mm_loadu_ps(M + 0); // [r00 r01 r02 tx]
  __m128 r1 = _mm_loadu_ps(M + 4); // [r10 r11 r12 ty]
  __m128 r2 = _mm_loadu_ps(M + 8); // [r20 r21 r22 tz]

  // Extract translation t = [tx ty tz 0]
  __m128 t = _mm_set_ps(0.0f, M[11], M[7], M[3]); // [0 tz ty tx] (lane order is reversed)
  // Better: build as [tx ty tz 0] in lanes 0..3:
  t = _mm_setr_ps(M[3], M[7], M[11], 0.0f);

  // Build 3x3 columns (or rows). We'll treat vectors as rows in xyz lanes.
  __m128 a = _mm_and_ps(r0, _mm_castsi128_ps(_mm_setr_epi32(-1, -1, -1, 0)));
  __m128 b = _mm_and_ps(r1, _mm_castsi128_ps(_mm_setr_epi32(-1, -1, -1, 0)));
  __m128 c = _mm_and_ps(r2, _mm_castsi128_ps(_mm_setr_epi32(-1, -1, -1, 0)));

  // Cofactors via cross products (for row-based inverse):
  __m128 rcp0 = cross3_ps(b, c); // = cofactor row0 (up to transpose convention)
  __m128 rcp1 = cross3_ps(c, a);
  __m128 rcp2 = cross3_ps(a, b);

  // Determinant = dot(a, cross(b,c))
  __m128 det = dot3_ps(a, rcp0);

  // Reciprocal det (you can do NR refinement if you want)
  __m128 invDet = _mm_div_ps(_mm_set1_ps(1.0f), det);

  rcp0 = _mm_mul_ps(rcp0, invDet);
  rcp1 = _mm_mul_ps(rcp1, invDet);
  rcp2 = _mm_mul_ps(rcp2, invDet);

  // Now rcp0/1/2 are rows of R^{-1} *if* you’re consistent with the cofactor layout.
  // For row-major R, the inverse is adj(R)/det, which is transpose of cofactor matrix.
  // The cross-product construction above gives you *rows* of adj(R) for row-major rows.
  // If you find it comes out transposed in your convention, swap shuffles accordingly.

  // Compute tinv = -Rinv * t
  __m128 tx = dot3_ps(rcp0, t);
  __m128 ty = dot3_ps(rcp1, t);
  __m128 tz = dot3_ps(rcp2, t);
  __m128 neg = _mm_set1_ps(-1.0f);
  tx = _mm_mul_ps(tx, neg);
  ty = _mm_mul_ps(ty, neg);
  tz = _mm_mul_ps(tz, neg);

  // Store Out rows: [Rinv row | tinv component]
  __m128 o0 = _mm_move_ss(rcp0, tx); // put tx in lane0 temporarily? easier to assemble explicitly
  // Assemble cleanly as row vectors:
  __m128 out0 =
    _mm_setr_ps(((float*)&rcp0)[0], ((float*)&rcp0)[1], ((float*)&rcp0)[2], ((float*)&tx)[0]);
  __m128 out1 =
    _mm_setr_ps(((float*)&rcp1)[0], ((float*)&rcp1)[1], ((float*)&rcp1)[2], ((float*)&ty)[0]);
  __m128 out2 =
    _mm_setr_ps(((float*)&rcp2)[0], ((float*)&rcp2)[1], ((float*)&rcp2)[2], ((float*)&tz)[0]);
  __m128 out3 = _mm_setr_ps(0.0f, 0.0f, 0.0f, 1.0f);

  _mm_storeu_ps(Out + 0, out0);
  _mm_storeu_ps(Out + 4, out1);
  _mm_storeu_ps(Out + 8, out2);
  _mm_storeu_ps(Out + 12, out3);
}
