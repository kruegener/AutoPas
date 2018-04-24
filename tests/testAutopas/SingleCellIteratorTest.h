/*
 * SingleCellIteratorTest.h
 *
 *  Created on: 19 Jan 2018
 *      Author: tchipevn
 */

#ifndef TESTS_TESTAUTOPAS_SINGLECELLITERATORTEST_H_
#define TESTS_TESTAUTOPAS_SINGLECELLITERATORTEST_H_

#include "autopasIncludes.h"
#include "gtest/gtest.h"
#include "AutoPasTest.h"

class SingleCellIteratorTest : public AutoPasTest {
 public:
  SingleCellIteratorTest() = default;

  void SetUp() override;

  void TearDown() override;

  ~SingleCellIteratorTest() override = default;

  template <class Cell>
  void fillWithParticles(Cell *pc) {
    for (auto &m : _vecOfMolecules) {
      pc->addParticle(m);
    }
  }

 protected:
  // needs to be protected, because the test fixtures generate a derived class
  // for each unit test.
  std::vector<autopas::MoleculeLJ> _vecOfMolecules;
};

#endif /* TESTS_TESTAUTOPAS_SINGLECELLITERATORTEST_H_ */
