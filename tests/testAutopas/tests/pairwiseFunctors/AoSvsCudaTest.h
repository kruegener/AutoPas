/**
 * @file AoSvsCudaTest.h
 * @author jspahl
 * @date 11.02.19
 */

#if defined(AUTOPAS_CUDA)

#pragma once

#include <gtest/gtest.h>
#include <chrono>
#include "AutoPasTestBase.h"
#include "autopas/autopasIncludes.h"

class AoSvsCudaTest : public AutoPasTestBase {
 public:
  void generateParticles(std::vector<autopas::Particle> *particles);
};

#endif  // AUTOPAS_CUDA
