/**
 * @file ForceCalculationTest.h
 * @author F. Gratl
 * @date 13.04.18
 */

#pragma once

#include <gtest/gtest.h>
#include <algorithm>
#include <vector>
#include "AutoPasTestBase.h"
#include "autopas/AutoPas.h"
#include "testingHelpers/GridGenerator.h"

class ForceCalculationTest : public AutoPasTestBase {
 public:
  ForceCalculationTest() = default;

  ~ForceCalculationTest() override = default;

  /**
   * Creates a test with four particles placed on the corners of a square,
   * executes a LJFunctor and compares the results within a given tolerance.
   *
   * @param particleSpacing distance between two particles
   * @param cutoff cutoff radius for the LJ
   * @param dataLayoutOption aos or soa
   * @param expectedForces 2D array containing the forces which are expected for
   * each molecule in the end
   * @param tolerance error tolerance
   */
  void testLJ(double particleSpacing, double cutoff, autopas::DataLayoutOption dataLayoutOption,
              std::array<std::array<double, 3>, 4> expectedForces, double tolerance);
};
