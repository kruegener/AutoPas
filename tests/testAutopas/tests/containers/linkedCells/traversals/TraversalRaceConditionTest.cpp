/**
 * @file TraversalRaceConditionTest.cpp
 * @author seckler
 * @date 18.04.18
 */

#include "TraversalRaceConditionTest.h"
#include "autopas/containers/CompatibleTraversals.h"
#include "autopas/utils/StringUtils.h"
#include "testingHelpers/GridGenerator.h"
#include "testingHelpers/NumThreadGuard.h"

/**
 * Idea: create mesh of particles and iterate with the SimpleFunctor.
 * All non-border particles should have F=0 at the end.
 *
 * Failing this test means that the traversal is incomplete or a race condition
 * occurred. Passing this test does not guarantee that there is no race
 * condition. Multiple execution is advised until a deterministic test is
 * implemented.
 *
 * Attention: If the traversal traverses over no particles this test will pass.
 * TODO: when periodic boundaries are implemented also border particles will
 * have F=0
 */
TEST_F(TraversalRaceConditionTest, testRCNonDeterministic) {
  double cellLength = 1;
  std::array<size_t, 3> particlesPerDimension = {30, 30, 30};
  std::array<double, 3> boxMin = {0., 0., 0.};
  std::array<double, 3> boxMax = {(double)particlesPerDimension[0], (double)particlesPerDimension[1],
                                  (double)particlesPerDimension[2]};

  NumThreadGuard numThreadGuard(8);

  /// @todo: test all containers similar to Newton3OnOffTest
  for (auto &container : {autopas::ContainerOption::linkedCells}) {
    for (auto &traversal : autopas::compatibleTraversals::allCompatibleTraversals(container)) {
      if (traversal == autopas::TraversalOption::c01 or traversal == autopas::TraversalOption::c01CombinedSoA or
          traversal == autopas::TraversalOption::c04SoA) {
        // c01 traversal does not work with newton3.
        // Here only one traversal is tested.
        continue;
      }

      if (traversal == autopas::TraversalOption::c01Cuda) {
        // c01Cuda traversal does not work with dataLayout Option AoS used in this test.
        continue;
      }

      // @TODO: extend Simple Functor for SoA
      for (auto &dataLayout : /*autopas::allDataLayoutOptions*/ {autopas::DataLayoutOption::aos}) {
        autopas::AutoPas<Particle, FPCell> autoPas;

        // generates one cell per particle + 1 halo layer
        autoPas.setBoxMin(boxMin);
        autoPas.setBoxMax(boxMax);
        autoPas.setCutoff(cellLength);
        autoPas.setAllowedContainers({container});
        autoPas.setAllowedTraversals({traversal});
        autoPas.setAllowedDataLayouts({dataLayout});
        autoPas.init();

        auto defaultParticle = Particle({0, 0, 0}, {0, 0, 0}, 0);
        GridGenerator::fillWithParticles(autoPas, particlesPerDimension, defaultParticle);

        SimpleFunctor functor(cellLength);

        autoPas.iteratePairwise(&functor);

        for (auto particleIterator = autoPas.begin(); particleIterator.isValid(); ++particleIterator) {
          if (particleIterator->getR()[0] == .5 || particleIterator->getR()[0] == particlesPerDimension[0] - .5 ||
              particleIterator->getR()[1] == .5 || particleIterator->getR()[1] == particlesPerDimension[1] - .5 ||
              particleIterator->getR()[2] == .5 || particleIterator->getR()[2] == particlesPerDimension[2] - .5)
            continue;
          // for debugging:
          //    particleIterator->print();

          // although these are doubles this should be exactly zero
          ASSERT_EQ(particleIterator->getF()[0], 0)
              << "in traversal: " << autopas::utils::StringUtils::to_string(traversal)
              << " data layout: " << autopas::utils::StringUtils::to_string(dataLayout);
          ASSERT_EQ(particleIterator->getF()[1], 0)
              << "in traversal: " << autopas::utils::StringUtils::to_string(traversal)
              << " data layout: " << autopas::utils::StringUtils::to_string(dataLayout);
          ASSERT_EQ(particleIterator->getF()[2], 0)
              << "in traversal: " << autopas::utils::StringUtils::to_string(traversal)
              << " data layout: " << autopas::utils::StringUtils::to_string(dataLayout);
        }
      }
    }
  }
}
