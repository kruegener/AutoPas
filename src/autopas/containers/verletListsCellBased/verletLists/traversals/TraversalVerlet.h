/**
 * @file TraversalVerlet.h
 *
 * @date 7.4.2019
 * @author jspahl
 */

#pragma once

#include "VerletTraversalInterface.h"
#include "autopas/containers/cellPairTraversals/CellPairTraversal.h"
#include "autopas/containers/verletListsCellBased/verletLists/VerletListHelpers.h"
#include "autopas/options/DataLayoutOption.h"

namespace autopas {

/**
 * This class provides a Traversal for the verlet lists container.
 *
 * @tparam ParticleCell the type of cells
 * @tparam PairwiseFunctor The functor that defines the interaction of two particles.
 * @tparam dataLayout
 * @tparam useNewton3
 */
template <class ParticleCell, class PairwiseFunctor, DataLayoutOption dataLayout, bool useNewton3>
class TraversalVerlet
    : public TraversalInterface,
      public VerletTraversalInterface<
          typename VerletListHelpers<typename ParticleCell::ParticleType>::VerletListParticleCellType> {
  using Particle = typename ParticleCell::ParticleType;
  typedef
      typename VerletListHelpers<typename ParticleCell::ParticleType>::VerletListParticleCellType LinkedParticleCell;

 public:
  /**
   * Constructor for Verlet Traversal
   * @param pairwiseFunctor Functor to be used with this Traversal
   */
  explicit TraversalVerlet(PairwiseFunctor *pairwiseFunctor) : _functor(pairwiseFunctor) {}

  TraversalOption getTraversalType() const override { return TraversalOption::verletTraversal; }

  DataLayoutOption getDataLayout() const override { return dataLayout; }

  bool getUseNewton3() const override { return useNewton3; }

  bool isApplicable() const override {
    return dataLayout == DataLayoutOption::aos || dataLayout == DataLayoutOption::soa;
  }

  void initTraversal() override {
    auto &cells = *(this->_cells);
    if (dataLayout == DataLayoutOption::soa) {
      size_t offset = 0;
      for (auto &cell : cells) {
        _functor->SoALoader(cell, _soa, offset);
        offset += cell.numParticles();
      }
    }
  }

  void endTraversal() override {
    auto &cells = *(this->_cells);
    if (dataLayout == DataLayoutOption::soa) {
      size_t offset = 0;
      for (auto &cell : cells) {
        _functor->SoAExtractor(cell, _soa, offset);
        offset += cell.numParticles();
      }
    }
  }

  void traverseParticlePairs() override {
    auto &aosNeighborLists = *(this->_aosNeighborLists);
    auto &soaNeighborLists = *(this->_soaNeighborLists);
    switch (dataLayout) {
      case DataLayoutOption::aos: {
#if defined(AUTOPAS_OPENMP)
        if (not useNewton3) {
          size_t buckets = aosNeighborLists.bucket_count();
          // @todo find a sensible chunk size
#pragma omp parallel for schedule(dynamic)
          for (size_t b = 0; b < buckets; b++) {
            auto endIter = aosNeighborLists.end(b);
            for (auto it = aosNeighborLists.begin(b); it != endIter; ++it) {
              Particle &i = *(it->first);
              for (auto j_ptr : it->second) {
                Particle &j = *j_ptr;
                _functor->AoSFunctor(i, j, false);
              }
            }
          }
        } else
#endif
        {
          for (auto &list : aosNeighborLists) {
            Particle &i = *list.first;
            for (auto j_ptr : list.second) {
              Particle &j = *j_ptr;
              _functor->AoSFunctor(i, j, useNewton3);
            }
          }
        }
        return;
      }

      case DataLayoutOption::soa: {
        const size_t iFrom = 0;
        const size_t iTo = soaNeighborLists.size();

#if defined(AUTOPAS_OPENMP)
        if (not useNewton3) {
          // @todo find a sensible chunk size
          const size_t chunkSize = std::max((iTo - iFrom) / (omp_get_max_threads() * 10), 1ul);
#pragma omp parallel for schedule(dynamic, chunkSize)
          for (size_t i = iFrom; i < iTo; i++) {
            _functor->SoAFunctor(_soa, soaNeighborLists, i, i + 1, useNewton3);
          }
        } else
#endif
        {
          // iterate over SoA
          _functor->SoAFunctor(_soa, soaNeighborLists, iFrom, iTo, useNewton3);
        }
        return;
      }
      default: { utils::ExceptionHandler::exception("VerletList dataLayout {} not available", dataLayout); }
    }
  }

 private:
  /**
   * Functor for Traversal
   */
  PairwiseFunctor *_functor;

  /**
   *global SoA of verlet lists
   */
  SoA<typename Particle::SoAArraysType> _soa;
};

}  // namespace autopas