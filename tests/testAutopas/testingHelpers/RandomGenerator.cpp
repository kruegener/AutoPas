/**
 * @file RandomGenerator.cpp
 * @author seckler
 * @date 22.05.18
 */

#include "RandomGenerator.h"

double RandomGenerator::fRand(double fMin, double fMax) {
  const double f = static_cast<double>(rand()) / RAND_MAX;
  return fMin + f * (fMax - fMin);
}

std::array<double, 3> RandomGenerator::randomPosition(const std::array<double, 3> &boxMin,
                                                      const std::array<double, 3> &boxMax) {
  std::array<double, 3> r{0, 0, 0};
  for (unsigned int d = 0; d < 3; ++d) {
    r[d] = fRand(boxMin[d], boxMax[d]);
  }
  return r;
}