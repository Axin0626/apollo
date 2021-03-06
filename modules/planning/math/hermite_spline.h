/******************************************************************************
 * Copyright 2017 The Apollo Authors. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *****************************************************************************/

/**
 * @file hermite_spline.h
 **/

#ifndef MODULES_PLANNING_MATH_HERMITE_SPLINE_H_
#define MODULES_PLANNING_MATH_HERMITE_SPLINE_H_

#include <array>
#include <utility>

#include "modules/common/log.h"

namespace apollo {
namespace planning {

// Hermite spline implementation that works for 1d and 2d space interpolation.
// Valid input type T: double, Eigen::Vector2d
template <typename T, std::uint32_t N>
class HermiteSpline {
 public:
  HermiteSpline(std::array<T, (N + 1) / 2> x0, std::array<T, (N + 1) / 2> x1,
                const double z0 = 0.0, const double z1 = 1.0);

  virtual ~HermiteSpline() = default;

  virtual T Evaluate(const std::uint32_t order, const double z) const;

 private:
  std::array<T, (N + 1) / 2> x0_;

  std::array<T, (N + 1) / 2> x1_;

  double z0_ = 0.0;

  double delta_z_ = 0.0;
};

template <typename T, std::uint32_t N>
inline HermiteSpline<T, N>::HermiteSpline(std::array<T, (N + 1) / 2> x0,
                                          std::array<T, (N + 1) / 2> x1,
                                          const double z0, const double z1)
    : x0_(std::move(x0)), x1_(std::move(x1)), z0_(z0), delta_z_(z1 - z0) {
  CHECK(N == 3 || N == 5)
      << "Error: currently we only support cubic and quintic hermite splines!";
}

template <typename T, std::uint32_t N>
inline T HermiteSpline<T, N>::Evaluate(const std::uint32_t order,
                                       const double z) const {
  CHECK_LE(z0_, z);
  CHECK_LE(z, z0_ + delta_z_);

  // if N == 3, cubic hermite spline, N == 5, qunitic hermite spline
  if (N == 3) {
    const T& p0 = x0_[0];
    const T& v0 = x0_[1];
    const T& p1 = x1_[0];
    const T& v1 = x1_[1];
    switch (order) {
      case 0: {
        double t = (z - z0_) / delta_z_;
        double t2 = t * t;
        double t3 = t2 * t;

        return (2.0 * t3 - 3.0 * t2 + 1.0) * p0 + (t3 - 2 * t2 + t) * v0 +
               (-2.0 * t3 + 3.0 * t2) * p1 + (t3 - t2) * v1;
      }
      case 1: {
        double t = (z - z0_) / delta_z_;
        double t2 = t * t;

        return (6.0 * t2 - 6.0 * t) * p0 + (3.0 * t2 - 4 * t + 1.0) * v0 +
               (-6.0 * t2 + 6.0 * t) * p1 + (3.0 * t2 - 2.0 * t) * v1;
      }
      case 2: {
        double t = (z - z0_) / delta_z_;
        return (12.0 * t - 6.0) * p0 + (6.0 * t - 4.0) * v0 +
               (-12.0 * t + 6.0) * p1 + (6.0 * t - 2.0) * v1;
      }
      case 3: {
        return 12.0 * p0 + 6.0 * v0 - 12.0 * p1 + 6.0 * v1;
      }
      default: { break; }
    }
  } else {
    CHECK_EQ(5, N);
    const T& p0 = x0_[0];
    const T& v0 = x0_[1];
    const T& a0 = x0_[2];
    const T& p1 = x1_[0];
    const T& v1 = x1_[1];
    const T& a1 = x1_[2];

    switch (order) {
      case 0: {
        double t = (z - z0_) / delta_z_;
        double t2 = t * t;
        double t3 = t * t2;
        double t4 = t2 * t2;
        double t5 = t2 * t3;
        double det0 = t3 - t4;
        double det1 = t4 - t5;
        double h0 = 1.0 - 10.0 * t3 + 15.0 * t4 - 6.0 * t5;
        double h1 = t - 6.0 * t3 + 8.0 * t4 - 3.0 * t5;
        double h2 = 0.5 * (t2 - t5) - 1.5 * det0;
        double h3 = 10.0 * t3 - 15.0 * t4 + 6.0 * t5;
        double h4 = -4.0 * det0 + 3.0 * det1;
        double h5 = 0.5 * (det0 - det1);

        return h0 * p0 + h1 * v0 + h2 * a0 + h3 * p1 + h4 * v1 + h5 * a1;
      }
      case 1: {
        double t = (z - z0_) / delta_z_;
        double t2 = t * t;
        double t3 = t * t2;
        double t4 = t2 * t2;
        double det0 = t2 - t3;
        double det1 = t3 - t4;
        double dh0 = -30.0 * det0 + 30.0 * det1;
        double dh1 = 1 - 18.0 * t2 + 32.0 * t3 - 15.0 * t4;
        double dh2 = t - 4.5 * t2 + 6.0 * t3 - 2.5 * t4;
        double dh3 = 30.0 * det0 - 30.0 * det1;
        double dh4 = -12.0 * t2 + 28.0 * t3 - 15.0 * t4;
        double dh5 = 1.5 * det0 - 2.5 * det1;

        return dh0 * p0 + dh1 * v0 + dh2 * a0 + dh3 * p1 + dh4 * v1 + dh5 * a1;
      }
      case 2: {
        double t = (z - z0_) / delta_z_;
        double t2 = t * t;
        double t3 = t * t2;
        double det0 = t - t2;
        double det1 = t2 - t3;
        double ddh0 = -60.0 * det0 + 120.0 * det1;
        double ddh1 = -36.0 * det0 + 60.0 * det1;
        double ddh2 = 1.0 - 9.0 * t + 18.0 * t2 - 10.0 * t3;
        double ddh3 = 60.0 * det0 - 120.0 * det1;
        double ddh4 = -24.0 * det0 + 60.0 * det1;
        double ddh5 = 3.0 * t - 12.0 * t2 + 10.0 * t3;

        return ddh0 * p0 + ddh1 * v0 + ddh2 * a0 + ddh3 * p1 + ddh4 * v1 +
               ddh5 * a1;
      }
      case 3: {
        double t = (z - z0_) / delta_z_;
        double t2 = t * t;
        double det = t - t2;
        double dddh0 = -60.0 + 360.0 * det;
        double dddh1 = -36.0 + 192.0 * t - 180.0 * t2;
        double dddh2 = -9.0 + 36.0 * t - 30.0 * t2;
        double dddh3 = 60.0 - 360.0 * det;
        double dddh4 = -24.0 + 168.0 * t - 180.0 * t2;
        double dddh5 = 3.0 - 24.0 * t + 30.0 * t2;

        return dddh0 * p0 + dddh1 * v0 + dddh2 * a0 + dddh3 * p1 + dddh4 * v1 +
               dddh5 * a1;
      }
      case 4: {
        double t = (z - z0_) / delta_z_;
        double d4h0 = 360.0 - 720.0 * t;
        double d4h1 = 192.0 - 360.0 * t;
        double d4h2 = 36.0 - 60.0 * t;
        double d4h3 = -360.0 + 720.0 * t;
        double d4h4 = 168.0 - 360.0 * t;
        double d4h5 = -24.0 + 60.0 * t;

        return d4h0 * p0 + d4h1 * v0 + d4h2 * a0 + d4h3 * p1 + d4h4 * v1 +
               d4h5 * a1;
      }
      case 5: {
        double d5h0 = -720.0;
        double d5h1 = -360.0;
        double d5h2 = -60.0;
        double d5h3 = 720.0;
        double d5h4 = -360.0;
        double d5h5 = 60.0;

        return d5h0 * p0 + d5h1 * v0 + d5h2 * a0 + d5h3 * p1 + d5h4 * v1 +
               d5h5 * a1;
      }
      default: { break; }
    }
  }
  return T();
}

}  // namespace planning
}  // namespace apollo

#endif /* MODULES_PLANNING_MATH_HERMITE_SPLINE_H_ */
