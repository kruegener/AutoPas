/**
 * @file C01TraversalVerlet.h
 * @date 09 Jan 2019
 * @author seckler
 */

#pragma once

#include "autopas/containers/cellPairTraversals/C01BasedTraversal.h"
#include "autopas/containers/verletListsCellBased/verletListsCells/traversals/VerletListsCellsTraversal.h"
#include "autopas/utils/WrapOpenMP.h"

namespace autopas {

/**
 * This class provides the c01 traversal.
 *
 * The traversal uses the c01 base step performed on every single cell.
 * newton3 cannot be applied!
 *
 * @tparam ParticleCell the type of cells
 * @tparam PairwiseFunctor The functor that defines the interaction of two particles.
 * @tparam DataLayout
 * @tparam useNewton3
 */
template <class ParticleCell, class PairwiseFunctor, DataLayoutOption DataLayout, bool useNewton3>
class C01TraversalVerlet : public C01BasedTraversal<ParticleCell, PairwiseFunctor, DataLayout, useNewton3>,
                           public VerletListsCellsTraversal<typename ParticleCell::ParticleType> {
 public:
  /**
   * Constructor of the c01 traversal.
   * @param dims The dimensions of the cellblock, i.e. the number of cells in x,
   * y and z direction.
   * @param pairwiseFunctor The functor that defines the interaction of two particles.
   */
  explicit C01TraversalVerlet(const std::array<unsigned long, 3> &dims, PairwiseFunctor *pairwiseFunctor)
      : C01BasedTraversal<ParticleCell, PairwiseFunctor, DataLayout, useNewton3>(dims, pairwiseFunctor),
        _functor(pairwiseFunctor) {}

  void traverseParticlePairs() override;

  TraversalOption getTraversalType() const override { return TraversalOption::c01Verlet; }

  bool isApplicable() const override { return (not useNewton3) && (DataLayout == DataLayoutOption::aos); }

  DataLayoutOption getDataLayout() const override { return DataLayout; }

  bool getUseNewton3() const override { return useNewton3; }

 private:
  PairwiseFunctor *_functor;
};

template <class ParticleCell, class PairwiseFunctor, DataLayoutOption DataLayout, bool useNewton3>
inline void C01TraversalVerlet<ParticleCell, PairwiseFunctor, DataLayout, useNewton3>::traverseParticlePairs() {
  this->c01Traversal([&](unsigned long x, unsigned long y, unsigned long z) {
    unsigned long baseIndex = utils::ThreeDimensionalMapping::threeToOneD(x, y, z, this->_cellsPerDimension);
    this->template iterateVerletListsCell<PairwiseFunctor, useNewton3>(*(this->_verletList), baseIndex, _functor);
  });
}

}  // namespace autopas
