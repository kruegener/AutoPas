/**
 * @file AutoPasTest.h
 * @author seckler
 * @date 24.04.18
 */

#include <gtest/gtest.h>
#include "autopas/utils/Logger.h"

#pragma once

class AutoPasTestBase : public testing::Test {
 public:
  AutoPasTestBase() { autopas::Logger::create(); }

  virtual ~AutoPasTestBase() { autopas::Logger::unregister(); }
};
