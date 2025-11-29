#include <float.h>
#include <gtest/gtest.h>
#include <sps/math.h>
#include <sps/sps_export.h>
#include <stdint.h>

#include <cassert>
#include <cstdlib>
#include <sps/cmath>

#include <iostream>
#include <vector>

#include <sps/smath.hpp>

using namespace sps;

TEST(smath_test, test_alignment)
{
  sps::point_t<float> p0 = sps::point_t<float>(1.0f, 2.0f, 3.0f);
  ASSERT_EQ(reinterpret_cast<uintptr_t>(&p0[0]) & 0x0F, 0U);
  sps::point_t<float> p1{ 1.0f, 2.0f, 3.0f };
  ASSERT_EQ(reinterpret_cast<uintptr_t>(&p1[0]) & 0x0F, 0U);
  sps::point_t<float> a = {};
  ASSERT_EQ(a[0], 0.0f);
  SPS_UNREFERENCED_PARAMETER(a);
}

TEST(smath_test, test_basis_vectors_double)
{
  euler_t<double> euler; //  = { 0.1, 0.2, 0.3, 0.0 }; Not working on _MSC_VER
  euler.alpha = 0.1;
  euler.beta = 0.2;
  euler.gamma = 0.3;
  double vec0[4], vec1[4], vec2[4];

  basis_vectors(vec0, vec1, vec2, euler);

  point_t<double> vec = point_t<double>();
  basis_vectors<double, EulerIntrinsicYXY>(&vec, euler, 0);

  double diff =
    sqrt(SQUARE(vec[0] - vec0[0]) + SQUARE(vec[1] - vec0[1]) + SQUARE(vec[2] - vec0[2]));
  ASSERT_LT(diff, FLT_EPSILON);
  basis_vectors<double, EulerIntrinsicYXY>(&vec, euler, 1);
  diff = sqrt(SQUARE(vec[0] - vec1[0]) + SQUARE(vec[1] - vec1[1]) + SQUARE(vec[2] - vec1[2]));
  ASSERT_LT(diff, FLT_EPSILON);
  basis_vectors<double, EulerIntrinsicYXY>(&vec, euler, 2);
  diff = sqrt(SQUARE(vec[0] - vec2[0]) + SQUARE(vec[1] - vec2[1]) + SQUARE(vec[2] - vec2[2]));
  ASSERT_LT(diff, FLT_EPSILON);
}

TEST(smath_test, test_min_max)
{
  std::vector<float> xs = { 1.0f, 2.0f, 3.0f };
  std::vector<float> ws = { 0.0f, 1.0f, 0.0f };
  auto result = sps::minmax_weighted_element<std::vector<float>::const_iterator,
    std::vector<float>::const_iterator>(xs.begin(), xs.end(), ws.begin());
  std::cout << "min: " << *(result.first) << "max: " << *(result.second) << std::endl;
  ASSERT_TRUE(true);
}

TEST(smath_test, test_point_to_circle)
{
  point_t<float> p = { 0.0f, 0.0f, 4.0f };
  circle_t<float> c = circle_t<float>();
  c.center[0] = 0.0f;
  c.center[1] = 0.0f;
  c.center[2] = 0.0f;
  c.radius = 2.0f;
  c.euler.alpha = 0.0f;
  c.euler.beta = 0.0f;
  c.euler.gamma = 0.0f;

  float dist = dist_point_to_circle<float>(p, c);
  float diff = fabs(dist - 4.0f);
  ASSERT_LT(diff, FLT_EPSILON);

  p[1] = 6.0f;
  dist = dist_point_to_circle<float>(p, c);
  diff = fabs(dist - sqrt(32.0f));
  ASSERT_LT(diff, FLT_EPSILON);

  p[0] = 1.0f;
  p[1] = 0.0f;
  dist = dist_point_to_circle<float>(p, c);
  diff = fabs(dist - 4.0f);
  ASSERT_LT(diff, FLT_EPSILON);

  float r, z, distNear, distFar;
  dist_point_to_circle_local<float>(p, c, &r, &z, &distNear, &distFar);
  ASSERT_LT(fabs(distNear - dist), FLT_EPSILON);
}

TEST(smath_test, test_point_to_circle_local)
{
  point_t<float> p = { 0.0f, 0.0f, 4.0f };
  circle_t<float> c = circle_t<float>();
  c.center[0] = 0.0f;
  c.center[1] = 0.0f;
  c.center[2] = 0.0f;
  c.radius = 2.0f;
  c.euler.alpha = 0.0f;
  c.euler.beta = 0.0f;
  c.euler.gamma = 0.0f;

  float r, z, dist, distFar;

  dist_point_to_circle_local<float>(p, c, &r, &z, &dist, &distFar);
  float diff = fabs(dist - 4.0f);
  ASSERT_LT(diff, FLT_EPSILON);
  ASSERT_LT(dist, distFar);

  p[1] = 6.0f;
  dist_point_to_circle_local<float>(p, c, &r, &z, &dist, &distFar);
  diff = fabs(dist - sqrt(32.0f));
  ASSERT_LT(diff, FLT_EPSILON);
  ASSERT_LT(dist, distFar);

  p[0] = 1.0f;
  p[1] = 0.0f;
  dist_point_to_circle_local<float>(p, c, &r, &z, &dist, &distFar);
  diff = fabs(dist - 4.0f);
  ASSERT_LT(diff, FLT_EPSILON);
  ASSERT_LT(dist, distFar);

  p[0] = 0.1f;
  dist_point_to_circle_local<float>(p, c, &r, &z, &dist, &distFar);
  ASSERT_LT(dist, distFar);
}

TEST(smath_test, test_dists_most_distant_and_closest)
{
  // Overlapping boxes
  bbox_t<float> box0 = bbox_t<float>();
  bbox_t<float> box1 = bbox_t<float>();
  box0.min[0] = -2.0f;
  box0.max[0] = 2.0f;
  box0.min[1] = -2.0f;
  box0.max[1] = 2.0f;
  box0.min[2] = -2.0f;
  box0.max[2] = 2.0f;

  box1.min[0] = -1.0f;
  box1.max[0] = 3.0f;
  box1.min[1] = -1.0f;
  box1.max[1] = 1.0f;
  box1.min[2] = -1.0f;
  box1.max[2] = 1.0f;

  float distNear = 0.0f;
  float distFar = 0.0f;
  dists_most_distant_and_closest(box0, box1, &distNear, &distFar);

  ASSERT_EQ(distNear, 0.0f);
  ASSERT_LT(fabs(distFar - sqrtf(43.0f)), FLT_EPSILON);
}

TEST(smath, arc_point_ellipsis)
{
  sps::ellipsis_t<float> ellipse;
  ellipse.hh = 1.0;
  ellipse.hw = 2.0;
  sps::point_t<float> point;
  float arc = 0.0f;

  sps::arc_point_ellipsis<float>(ellipse, arc, &point);
  std::cout << sps::almost_equal(point[0], ellipse.hw, 1);
  std::cout << sps::almost_equal(point[1], 0.0f, 1);

  arc = float(M_PI_2);
  sps::arc_point_ellipsis(ellipse, arc, &point);
  std::cout << sps::almost_equal(point[0], 0.0f, 1);
  std::cout << sps::almost_equal(point[1], ellipse.hh, 1);

  arc = float(M_PI);
  sps::arc_point_ellipsis(ellipse, arc, &point);
  std::cout << sps::almost_equal(point[0], -ellipse.hw, 1);
  //  std::cout << sps::almost_equal(point[1], 0.0f, 1);

  arc = float(M_3PI_2);
  sps::arc_point_ellipsis(ellipse, arc, &point);
  std::cout << sps::almost_equal(point[0], 0.0f, 1);
  std::cout << sps::almost_equal(point[1], -ellipse.hh, 1);

  arc = float(M_2PI);
  sps::arc_point_ellipsis(ellipse, arc, &point);
  std::cout << sps::almost_equal(point[0], ellipse.hw, 1);
  //  std::cout << sps::almost_equal(point[1], 0.0f, 1);

  // Perfect
  std::cout << std::endl << sps::almost_equal(-0.0f, 0.0f, 1) << std::endl;
}

TEST(smath, intcp_line_rect)
{
  sps::element_rect_t<float> rect;
  rect.hh = 1.0f;
  rect.hw = 2.0f;
  rect.center = { 0.0f, 0.0f, 0.0f };
  sps::point_t<float> point;
  float x = 1.0f;
  float y = 0.0f;
  intcp_line_rect(rect, y, x, &point);
  std::cout << point << std::endl;
  x = 0.0f;
  y = 1.0f;
  intcp_line_rect(rect, y, x, &point);
  std::cout << point << std::endl;
  x = -1.0f;
  y = 0.0f;
  intcp_line_rect(rect, y, x, &point);
  std::cout << point << std::endl;
  x = 0.0f;
  y = -1.0f;
  intcp_line_rect(rect, y, x, &point);
  std::cout << point << std::endl;
}

TEST(smath, matmul)
{
  sps::mat3_t<float> m0, m1, m2;
  for (size_t i = 0; i < 3; i++)
    for (size_t j = 0; j < 3; j++)
    {
      m0.data[i][j] = static_cast<float>(i * 3 + j);
      m1.data[i][j] = static_cast<float>(i * 3 + j);
    }
  m2 = m0 * m1;
  // std::cout << "m2[0,0]: " << m2.data[0][0] << std::endl;
  // std::cout << "m2[0,1]: " << m2.data[0][1] << std::endl;
  // std::cout << "m2[2,2]: " << m2.data[2][2] << std::endl;
}

/*
// TODO: Use this alternative for testing
T angle = atan2(v,u);
nearest[0] = cos(angle)*circle.radius;
nearest[1] = sin(angle)*circle.radius;
nearest[2] = T(0.0);
*/

int main(int argc, char** argv)
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

/* Local variables: */
/* indent-tab-mode: nil */
/* tab-width: 2 */
/* c-basic-offset: 2 */
/* End: */
