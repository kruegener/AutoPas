/**
 * @file LinkedCellsTest.h
 * @author seckler
 * @date 27.04.18
 */

#pragma once

#include "AutoPasTestBase.h"
#include "autopas/cells/FullParticleCell.h"
#include "autopas/containers/linkedCells/LinkedCells.h"
#include "autopas/particles/Particle.h"
#include "testingHelpers/commonTypedefs.h"

class LinkedCellsTest : public AutoPasTestBase {
 public:
  LinkedCellsTest();

 protected:
  autopas::LinkedCells<Particle, FPCell> _linkedCells;
};
