#include <cstddef>
#include <sps/cstdio>
#include <sps/multi_malloc.hpp>
#include <sps/multi_malloc.h>

#include <array>
#include <cstring>

template <typename T>
struct point_t : public std::array<T,3> {};

template <typename T>
struct euler_t {
  T alpha;
  T beta;
  T gamma;
};

template <typename T>
struct element_t {
  T hw;  /* Half width  */
  T hh;  /* Half height */
  point_t<T> center;
  euler_t<T> euler;
};

int main(int argc, char* argv[]) {
  SPS_UNREFERENCED_PARAMETERS(argc, argv);
  float** f = (float**) _mm_multi_malloc<float>(2,100,100);
  f[99][99] = 1.0f;
  _mm_multi_free<float>(f,2);


  f = (float**) _mm_multi_malloc<float,32>(2,100,100);
  f[99][99] = 1.0f;
  memset(f[0],0,100*100*sizeof(float));
//  ASSERT_EQ((uintptr_t)&f[0][0] % 32, 0);
    _mm_multi_free<float, 32>(f, 2);

  f = (float**) _mm_multi_malloc<float,4>(2,100,100);
  f[99][99] = 1.0f;
  _mm_multi_free<float,32>(f,2);

  float** g = (float**) _mm_multi_malloc<float,16>(2,1,100);
  memset((void*) g[0], 0, 100*sizeof(float));
  _mm_multi_free<float,16>(g,2);

  printf("sizeof(element_t<float>: %zu\n", sizeof(element_t<float>));
  printf("sizeof(element_t<double>: %zu\n", sizeof(element_t<double>));

  element_t<float>** e = (element_t<float>**) _mm_multi_malloc<element_t<float>, 64 >(2,100,100);
  e[0][0].hw = 1.0f;
  memset((void*) &(e[0][0]), 0, 100*100*sizeof(element_t<float>));
  _mm_multi_free<element_t<float>, 64>(e,2);

  element_t<float>* ee = (element_t<float>*) _mm_multi_malloc<element_t<float>, sizeof(element_t<float>) >(1,100);
  memset((void*) &(ee[0]), 0, 100*sizeof(element_t<float>));
  _mm_multi_free<element_t<float>, sizeof(element_t<float>) >(ee,1);

  printf("%zu\n",alignof(max_align_t));
  printf("%zu\n",std::alignment_of<max_align_t>::value);

  float** oldFloatArray2 = (float**) multi_malloc(sizeof(float),2,10,10);
  multi_free(oldFloatArray2,2);

  float* oldFloatArray1 = (float*) multi_malloc(sizeof(float),1,10);
  multi_free(oldFloatArray1,1);

#if 0
  // TODO: Test this
  sps::deleted_aligned_array<float> arr = sps::deleted_aligned_multi_array<float,2>(2, 10, 30);
  float **_arr = (float**) arr.get();
  _arr[0][0] = 3.0f;

  // Working
  sps::deleted_aligned_array<float> arr2 = sps::deleted_aligned_multi_array<float,3>(3, 10, 30, 10);
  float ***_arr2 = (float***) arr2.get();
  for (size_t i = 0 ; i < 10 ; i++)
    for (size_t j = 0 ; j < 30 ; j++)
      for (size_t k = 0 ; k < 10 ; k++)
        _arr2[i][j][k] = 3.0f;
#endif

#if 0
  // Not working
  sps::deleted_aligned_multi_array2<float,2,4> arr2_ = sps::deleted_aligned_multi_array2<float,2,4>();
  arr2_[1][3] = 3.0f;
  for (size_t i = 0 ; i < 2 ; i++)
    for (size_t j = 0 ; j < 4 ; j++)
      arr2_[i][j] = 3.0f;

  //  (arr2.get())[0][2][3] = 3.0f;
  //arr2[0];
  sps::deleted_aligned_multi_array3<float,3,4,2> arr3_ = sps::deleted_aligned_multi_array3<float,3,4,2>();
  for (size_t i = 0 ; i < 3 ; i++)
    for (size_t j = 0 ; j < 4 ; j++)
      for (size_t k = 0 ; k < 2 ; k++) {
        arr3_[i][j][k] = 3.0f;
      }
#endif
  return 0;
}
