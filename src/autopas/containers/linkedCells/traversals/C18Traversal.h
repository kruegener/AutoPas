/**
 * @file C18Traversal.h
 * @author nguyen
 * @date 06.09.2018
 */

#pragma once

#include "LinkedCellTraversalInterface.h"
#include "autopas/containers/cellPairTraversals/C18BasedTraversal.h"
#include "autopas/options/DataLayoutOption.h"
#include "autopas/pairwiseFunctors/CellFunctor.h"
#include "autopas/utils/ArrayUtils.h"
#include "autopas/utils/ThreeDimensionalMapping.h"
#include "autopas/utils/WrapOpenMP.h"

namespace autopas {

/**
 * This class provides the c18 traversal.
 *
 * The traversal uses the c18 base step performed on every single cell. Since
 * these steps overlap a domain coloring with eighteen colors is applied.
 *
 * @tparam ParticleCell the type of cells
 * @tparam PairwiseFunctor The functor that defines the interaction of two particles.
 * @tparam DataLayout
 * @tparam useNewton3
 */
template <class ParticleCell, class PairwiseFunctor, DataLayoutOption DataLayout, bool useNewton3>
class C18Traversal : public C18BasedTraversal<ParticleCell, PairwiseFunctor, DataLayout, useNewton3>,
                     public LinkedCellTraversalInterface<ParticleCell> {
 public:
  /**
   * Constructor of the c18 traversal.
   * @param dims The dimensions of the cellblock, i.e. the number of cells in x,
   * y and z direction.
   * @param pairwiseFunctor The functor that defines the interaction of two particles.
   * @param interactionLength Interaction length (cutoff + skin).
   * @param cellLength cell length.
   */
  explicit C18Traversal(const std::array<unsigned long, 3> &dims, PairwiseFunctor *pairwiseFunctor,
                        const double interactionLength = 1.0, const std::array<double, 3> &cellLength = {1.0, 1.0, 1.0})
      : C18BasedTraversal<ParticleCell, PairwiseFunctor, DataLayout, useNewton3>(dims, pairwiseFunctor,
                                                                                 interactionLength, cellLength),
        _cellFunctor(pairwiseFunctor, interactionLength) {
    computeOffsets();
  }

  void traverseParticlePairs() override;

  /**
   * Computes all interactions between the base
   * cell and adjacent cells with greater a ID.
   * @param cells vector of all cells.
   * @param x X-index of base cell.
   * @param y Y-index of base cell.
   * @param z Z-index of base cell.
   */
  void processBaseCell(std::vector<ParticleCell> &cells, unsigned long x, unsigned long y, unsigned long z);

  TraversalOption getTraversalType() const override { return TraversalOption::c18; }

  /**
   * C18 traversal is always usable.
   * @return
   */
  bool isApplicable() const override {
    int nDevices = 0;
#if defined(AUTOPAS_CUDA)
    cudaGetDeviceCount(&nDevices);
#endif
    if (DataLayout == DataLayoutOption::cuda)
      return nDevices > 0;
    else
      return true;
  }

  DataLayoutOption getDataLayout() const override { return DataLayout; }

  bool getUseNewton3() const override { return useNewton3; }

 private:
  /**
   * Computes pairs used in processBaseCell()
   */
  void computeOffsets();

  /**
   * CellFunctor to be used for the traversal defining the interaction between two cells.
   */
  internal::CellFunctor<typename ParticleCell::ParticleType, ParticleCell, PairwiseFunctor, DataLayout, useNewton3>
      _cellFunctor;

  /**
   * Type of an array containing offsets relative to the base cell and correspondent normalized 3d relationship vectors.
   * The vectors describe the imaginative line connecting the center of the base cell and the center of the cell defined
   * by the offset.
   */
  using offsetArray_t = std::vector<std::pair<unsigned long, std::array<double, 3>>>;

  /**
   * Pairs for processBaseCell(). overlap[0] x overlap[1] offsetArray_t for each special case in x and y direction.
   */
  std::vector<std::vector<offsetArray_t>> _cellOffsets;

  /**
   * Returns the index in the offsets array for the given position.
   * @param pos current position in dimension dim
   * @param dim current dimension
   * @return Index for the _cellOffsets Array.
   */
  unsigned long getIndex(const unsigned long pos, const unsigned int dim) const;
};

template <class ParticleCell, class PairwiseFunctor, DataLayoutOption DataLayout, bool useNewton3>
inline void C18Traversal<ParticleCell, PairwiseFunctor, DataLayout, useNewton3>::computeOffsets() {
  _cellOffsets.resize(2 * this->_overlap[1] + 1, std::vector<offsetArray_t>(2 * this->_overlap[0] + 1));
  const std::array<long, 3> _overlap_s = ArrayUtils::static_cast_array<long>(this->_overlap);

  const auto interactionLengthSquare(this->_interactionLength * this->_interactionLength);

  for (long z = 0l; z <= _overlap_s[2]; ++z) {
    for (long y = -_overlap_s[1]; y <= _overlap_s[1]; ++y) {
      for (long x = -_overlap_s[0]; x <= _overlap_s[0]; ++x) {
        const long offset = utils::ThreeDimensionalMapping::threeToOneD(
            x, y, z, ArrayUtils::static_cast_array<long>(this->_cellsPerDimension));

        if (offset < 0l) {
          continue;
        }
        // add to each applicable special case
        for (long yArray = -_overlap_s[1]; yArray <= _overlap_s[1]; ++yArray) {
          if (std::abs(yArray + y) <= _overlap_s[1]) {
            for (long xArray = -_overlap_s[0]; xArray <= _overlap_s[0]; ++xArray) {
              if (std::abs(xArray + x) <= _overlap_s[0]) {
                std::array<double, 3> pos = {};
                pos[0] = std::max(0l, (std::abs(x) - 1l)) * this->_cellLength[0];
                pos[1] = std::max(0l, (std::abs(y) - 1l)) * this->_cellLength[1];
                pos[2] = std::max(0l, (std::abs(z) - 1l)) * this->_cellLength[2];
                // calculate distance between base cell and other cell
                const double distSquare = ArrayMath::dot(pos, pos);
                // only add cell offset if cell is within cutoff radius
                if (distSquare <= interactionLengthSquare) {
                  _cellOffsets[yArray + _overlap_s[1]][xArray + _overlap_s[0]].push_back(
                      std::make_pair(offset, ArrayMath::normalize(pos)));
                }
              }
            }
          }
        }
      }
    }
  }
}

template <class ParticleCell, class PairwiseFunctor, DataLayoutOption DataLayout, bool useNewton3>
unsigned long C18Traversal<ParticleCell, PairwiseFunctor, DataLayout, useNewton3>::getIndex(
    const unsigned long pos, const unsigned int dim) const {
  unsigned long index;
  if (pos < this->_overlap[dim]) {
    index = pos;
  } else if (pos < this->_cellsPerDimension[dim] - this->_overlap[dim]) {
    index = this->_overlap[dim];
  } else {
    index = pos - this->_cellsPerDimension[dim] + 2 * this->_overlap[dim] + 1ul;
  }
  return index;
}

template <class ParticleCell, class PairwiseFunctor, DataLayoutOption DataLayout, bool useNewton3>
void C18Traversal<ParticleCell, PairwiseFunctor, DataLayout, useNewton3>::processBaseCell(
    std::vector<ParticleCell> &cells, unsigned long x, unsigned long y, unsigned long z) {
  const unsigned long baseIndex = utils::ThreeDimensionalMapping::threeToOneD(x, y, z, this->_cellsPerDimension);

  const unsigned long xArray = getIndex(x, 0);
  const unsigned long yArray = getIndex(y, 1);

  ParticleCell &baseCell = cells[baseIndex];
  offsetArray_t &offsets = this->_cellOffsets[yArray][xArray];
  for (auto const &[offset, r] : offsets) {
    unsigned long otherIndex = baseIndex + offset;
    ParticleCell &otherCell = cells[otherIndex];

    if (baseIndex == otherIndex) {
      this->_cellFunctor.processCell(baseCell);
    } else {
      this->_cellFunctor.processCellPair(baseCell, otherCell, r);
    }
  }
}

template <class ParticleCell, class PairwiseFunctor, DataLayoutOption DataLayout, bool useNewton3>
inline void C18Traversal<ParticleCell, PairwiseFunctor, DataLayout, useNewton3>::traverseParticlePairs() {
  auto &cells = *(this->_cells);
  this->c18Traversal([&](unsigned long x, unsigned long y, unsigned long z) { this->processBaseCell(cells, x, y, z); });
}

}  // namespace autopas
