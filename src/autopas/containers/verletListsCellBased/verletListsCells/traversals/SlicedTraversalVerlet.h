/**
 * @file SlicedTraversalVerlet.h
 *
 * @date 09 Jan 2019
 * @author seckler
 */

#pragma once

#include <algorithm>
#include "autopas/containers/cellPairTraversals/SlicedBasedTraversal.h"
#include "autopas/containers/verletListsCellBased/verletListsCells/traversals/VerletListsCellsTraversal.h"
#include "autopas/utils/ThreeDimensionalMapping.h"
#include "autopas/utils/WrapOpenMP.h"

namespace autopas {

/**
 * This class provides the sliced traversal.
 *
 * The traversal finds the longest dimension of the simulation domain and cuts
 * the domain in one slice (block) per thread along this dimension. Slices are
 * assigned to the threads in a round robin fashion. Each thread locks the cells
 * on the boundary wall to the previous slice with one lock. This lock is lifted
 * as soon the boundary wall is fully processed.
 *
 * @tparam ParticleCell the type of cells
 * @tparam PairwiseFunctor The functor that defines the interaction of two particles.
 * @tparam useSoA
 * @tparam useNewton3
 */
template <class ParticleCell, class PairwiseFunctor, DataLayoutOption DataLayout, bool useNewton3>
class SlicedTraversalVerlet : public SlicedBasedTraversal<ParticleCell, PairwiseFunctor, DataLayout, useNewton3>,
                              public VerletListsCellsTraversal<typename ParticleCell::ParticleType> {
 public:
  /**
   * Constructor of the sliced traversal.
   * @param dims The dimensions of the cellblock, i.e. the number of cells in x,
   * y and z direction.
   * @param pairwiseFunctor The functor that defines the interaction of two particles.
   */
  explicit SlicedTraversalVerlet(const std::array<unsigned long, 3> &dims, PairwiseFunctor *pairwiseFunctor)
      : SlicedBasedTraversal<ParticleCell, PairwiseFunctor, DataLayout, useNewton3>(dims, pairwiseFunctor),
        _functor(pairwiseFunctor) {}

  void traverseParticlePairs() override;

  DataLayoutOption getDataLayout() const override { return DataLayout; }

  bool getUseNewton3() const override { return useNewton3; }

  TraversalOption getTraversalType() const override { return TraversalOption::slicedVerlet; }

  bool isApplicable() const override { return DataLayout == DataLayoutOption::aos; }

 private:
  PairwiseFunctor *_functor;
};

template <class ParticleCell, class PairwiseFunctor, DataLayoutOption DataLayout, bool useNewton3>
inline void SlicedTraversalVerlet<ParticleCell, PairwiseFunctor, DataLayout, useNewton3>::traverseParticlePairs() {
  this->slicedTraversal([&](unsigned long x, unsigned long y, unsigned long z) {
    auto baseIndex = utils::ThreeDimensionalMapping::threeToOneD(x, y, z, this->_cellsPerDimension);
    this->template iterateVerletListsCell<PairwiseFunctor, useNewton3>(*(this->_verletList), baseIndex, _functor);
  });
}

}  // namespace autopas
