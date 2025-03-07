/**
 * @file C04SoATraversal.h
 * @author C. Menges
 * @date 02.06.2019
 */

#pragma once

#include "C04SoACellHandler.h"
#include "LinkedCellTraversalInterface.h"
#include "autopas/containers/cellPairTraversals/C04BasedTraversal.h"
#include "autopas/pairwiseFunctors/CellFunctor.h"
#include "autopas/utils/WrapOpenMP.h"

namespace autopas {

/**
 * This class provides the c04 traversal.
 *
 * The traversal uses the c04 base step performed on every single cell. Since
 * these steps overlap a domain coloring with eight colors is applied.
 *
 * @tparam ParticleCell the type of cells
 * @tparam PairwiseFunctor The functor that defines the interaction of two particles.
 * @tparam useSoA
 * @tparam useNewton3
 */
template <class ParticleCell, class PairwiseFunctor, DataLayoutOption DataLayout, bool useNewton3>
class C04SoATraversal : public C04BasedTraversal<ParticleCell, PairwiseFunctor, DataLayout, useNewton3, 2>,
                        public LinkedCellTraversalInterface<ParticleCell> {
 public:
  /**
   * Constructor of the c04 traversal.
   * @param dims The dimensions of the cellblock, i.e. the number of cells in x,
   * y and z direction.
   * @param pairwiseFunctor The functor that defines the interaction of two particles.
   * @param cutoff Cutoff radius.
   * @param cellLength cell length.
   */
  explicit C04SoATraversal(const std::array<unsigned long, 3> &dims, PairwiseFunctor *pairwiseFunctor,
                           const double cutoff = 1.0, const std::array<double, 3> &cellLength = {1.0, 1.0, 1.0})
      : C04BasedTraversal<ParticleCell, PairwiseFunctor, DataLayout, useNewton3, 2>(dims, pairwiseFunctor, cutoff,
                                                                                    cellLength),
        _cellHandler(pairwiseFunctor, this->_cellsPerDimension, cutoff, cellLength, this->_overlap) {}

  void traverseParticlePairs() override;

  TraversalOption getTraversalType() const override { return TraversalOption::c04SoA; }

  DataLayoutOption getDataLayout() const override { return DataLayout; }

  bool getUseNewton3() const override { return useNewton3; }

  /**
   * c04SoA traversals are only usable with DataLayout SoA.
   * @return
   */
  bool isApplicable() const override { return DataLayout == DataLayoutOption::soa; }

 private:
  C04SoACellHandler<ParticleCell, PairwiseFunctor, DataLayout, useNewton3> _cellHandler;
};

template <class ParticleCell, class PairwiseFunctor, DataLayoutOption DataLayout, bool useNewton3>
inline void C04SoATraversal<ParticleCell, PairwiseFunctor, DataLayout, useNewton3>::traverseParticlePairs() {
  _cellHandler.resizeBuffers();
  auto &cells = *(this->_cells);
  this->c04Traversal(
      [&](unsigned long x, unsigned long y, unsigned long z) { _cellHandler.processBaseCell(cells, x, y, z); });
}

}  // namespace autopas
