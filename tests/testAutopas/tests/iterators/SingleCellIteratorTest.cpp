/**
 * @file SingleCellIteratorTest.cpp
 * @author tchipev
 * @date 19.01.18
 */

#include "SingleCellIteratorTest.h"
#include <gtest/gtest.h>
#include "autopas/autopasIncludes.h"

using namespace autopas;

void SingleCellIteratorTest::SetUp() {
  for (int i = 0; i < 4; ++i) {
    std::array<double, 3> arr{};
    for (auto &a : arr) {
      a = static_cast<double>(i);
    }
    MoleculeLJ m(arr, {0., 0., 0.}, static_cast<unsigned long>(i));
    _vecOfMolecules.push_back(m);
  }
}

void SingleCellIteratorTest::TearDown() {}

TEST_F(SingleCellIteratorTest, testFullParticleCell) {
  FullParticleCell<MoleculeLJ> fpc;

  fillWithParticles(&fpc);

  auto iter = fpc.begin();
  int i = 0;
  for (; iter.isValid(); ++iter, ++i) {
    for (int d = 0; d < 3; ++d) {
      ASSERT_DOUBLE_EQ(iter->getR()[d], _vecOfMolecules[i].getR()[d]);
    }
    ASSERT_EQ(iter->getID(), _vecOfMolecules[i].getID());
  }
}

TEST_F(SingleCellIteratorTest, testRMMParticleCell) {
  RMMParticleCell<MoleculeLJ> fpc;

  fillWithParticles(&fpc);

  auto iter = fpc.begin();
  int i = 0;
  for (; iter.isValid(); ++iter, ++i) {
    for (int d = 0; d < 3; ++d) {
      ASSERT_DOUBLE_EQ(iter->getR()[d], _vecOfMolecules[i].getR()[d]);
    }

    //		IDs are not set by the RMM Cell yet!
    //		ASSERT_EQ(iter->getID(), _vecOfMolecules[i].getID());
  }
}
