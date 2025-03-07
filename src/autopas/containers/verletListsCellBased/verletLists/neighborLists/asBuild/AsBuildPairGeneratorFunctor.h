/**
 * @file AsBuildPairGeneratorFunctor.h
 * @author humig
 * @date 09.07.19
 */

#pragma once

namespace autopas {

template <class Particle>
class VerletNeighborListAsBuild;

namespace internal {

/**
 * This functor can generate or check variable verlet lists using the typical pairwise
 * traversal.
 *
 * @tparam Particle The particle class.
 * @tparam callCheckInstead If false, generate a neighbor list. If true, check the current for validity. Checking
 * validity only works with the AoSFunctor().
 */
template <class Particle, bool callCheckInstead = false>
class AsBuildPairGeneratorFunctor
    : public autopas::Functor<Particle, typename VerletListHelpers<Particle>::VerletListParticleCellType,
                              typename VerletListHelpers<Particle>::SoAArraysType> {
  /// typedef for soa's of verlet list's linked cells (only id and position needs to be stored)
  typedef typename utils::SoAType<Particle *, double, double, double>::Type SoAArraysType;

  /// attributes for soa's of verlet list's linked cells (only id and position needs to be stored)
  enum AttributeNames : int { ptr, posX, posY, posZ };

 public:
  bool allowsNewton3() override { return true; }
  bool allowsNonNewton3() override { return true; }

  /**
   * Constructor of the functor.
   * @param neighborList The neighbor list to fill.
   * @param cutoffskin The cutoff skin to use.
   */
  AsBuildPairGeneratorFunctor(VerletNeighborListAsBuild<Particle> &neighborList, double cutoffskin)
      : autopas::Functor<Particle, typename VerletListHelpers<Particle>::VerletListParticleCellType,
                         typename VerletListHelpers<Particle>::SoAArraysType>(cutoffskin),
        _list(neighborList),
        _cutoffskinsquared(cutoffskin * cutoffskin) {}

  bool isRelevantForTuning() override { return false; }

  /**
   * Adds the given pair to the neighbor list.
   * @param i The first particle of the pair.
   * @param j The second particle of the pair.
   * @param newton3 Not used!
   */
  void AoSFunctor(Particle &i, Particle &j, bool newton3) override {
    auto dist = ArrayMath::sub(i.getR(), j.getR());
    double distsquare = ArrayMath::dot(dist, dist);
    if (distsquare < _cutoffskinsquared) {
      if (callCheckInstead) {
        _list.checkPair(&i, &j);
      } else {
        _list.addPair(&i, &j);
      }
    }
  }

  /**
   * Adds all pairs of the SoA to the neighbor list.
   * @param soa The SoA to add.
   * @param newton3 Whether to use newton 3 or not.
   */
  void SoAFunctor(SoAView<SoAArraysType> soa, bool newton3) override {
    if (soa.getNumParticles() == 0) return;

    auto **const __restrict__ ptrptr = soa.template begin<AttributeNames::ptr>();
    double *const __restrict__ xptr = soa.template begin<AttributeNames::posX>();
    double *const __restrict__ yptr = soa.template begin<AttributeNames::posY>();
    double *const __restrict__ zptr = soa.template begin<AttributeNames::posZ>();

    size_t numPart = soa.getNumParticles();
    for (unsigned int i = 0; i < numPart; ++i) {
      for (unsigned int j = i + 1; j < numPart; ++j) {
        const double drx = xptr[i] - xptr[j];
        const double dry = yptr[i] - yptr[j];
        const double drz = zptr[i] - zptr[j];

        const double drx2 = drx * drx;
        const double dry2 = dry * dry;
        const double drz2 = drz * drz;

        const double dr2 = drx2 + dry2 + drz2;

        if (dr2 < _cutoffskinsquared) {
          _list.addPair(ptrptr[i], ptrptr[j]);
          if (not newton3) {
            _list.addPair(ptrptr[j], ptrptr[i]);
          }
        }
      }
    }
  }

  /**
   * Adds all pairs (p1, p2), p1 element soa1, p2 element soa2 to the neighbor list.
   * @param soa1
   * @param soa2
   */
  void SoAFunctor(SoAView<SoAArraysType> soa1, SoAView<SoAArraysType> soa2, bool /*newton3*/) override {
    if (soa1.getNumParticles() == 0 || soa2.getNumParticles() == 0) return;

    auto **const __restrict__ ptrptr1 = soa1.template begin<AttributeNames::ptr>();
    double *const __restrict__ x1ptr = soa1.template begin<AttributeNames::posX>();
    double *const __restrict__ y1ptr = soa1.template begin<AttributeNames::posY>();
    double *const __restrict__ z1ptr = soa1.template begin<AttributeNames::posZ>();

    auto **const __restrict__ ptrptr2 = soa2.template begin<AttributeNames::ptr>();
    double *const __restrict__ x2ptr = soa2.template begin<AttributeNames::posX>();
    double *const __restrict__ y2ptr = soa2.template begin<AttributeNames::posY>();
    double *const __restrict__ z2ptr = soa2.template begin<AttributeNames::posZ>();

    size_t numPart1 = soa1.getNumParticles();
    for (unsigned int i = 0; i < numPart1; ++i) {
      size_t numPart2 = soa2.getNumParticles();
      for (unsigned int j = 0; j < numPart2; ++j) {
        const double drx = x1ptr[i] - x2ptr[j];
        const double dry = y1ptr[i] - y2ptr[j];
        const double drz = z1ptr[i] - z2ptr[j];

        const double drx2 = drx * drx;
        const double dry2 = dry * dry;
        const double drz2 = drz * drz;

        const double dr2 = drx2 + dry2 + drz2;

        if (dr2 < _cutoffskinsquared) {
          _list.addPair(ptrptr1[i], ptrptr2[j]);
        }
      }
    }
  }

  /**
   * Loads all particles of the cell into the SoA.
   * @param cell
   * @param soa
   * @param offset
   */
  void SoALoader(ParticleCell<Particle> &cell, SoA<SoAArraysType> &soa, size_t offset = 0) override {
    if (offset != 0ul) {
      utils::ExceptionHandler::exception("offset must be 0, is: {}", offset);
    }
    soa.resizeArrays(cell.numParticles());

    if (cell.numParticles() == 0) return;

    auto *const __restrict__ ptrptr = soa.template begin<AttributeNames::ptr>();
    double *const __restrict__ xptr = soa.template begin<AttributeNames::posX>();
    double *const __restrict__ yptr = soa.template begin<AttributeNames::posY>();
    double *const __restrict__ zptr = soa.template begin<AttributeNames::posZ>();

    auto cellIter = cell.begin();
    // load particles in SoAs.
    for (size_t i = 0; cellIter.isValid(); ++cellIter, ++i) {
      Particle *pptr = &(*cellIter);
      ptrptr[i] = pptr;
      xptr[i] = cellIter->getR()[0];
      yptr[i] = cellIter->getR()[1];
      zptr[i] = cellIter->getR()[2];
    }
  }

  /**
   * Does nothing
   * @param cell
   * @param soa
   * @param offset
   */
  void SoAExtractor(ParticleCell<Particle> &cell, SoA<SoAArraysType> &soa, size_t offset = 0) override {}

 private:
  /**
   * The neighbor list to fill.
   */
  VerletNeighborListAsBuild<Particle> &_list;
  /**
   * The squared cutoff skin to determine if a pair should be added to the list.
   */
  double _cutoffskinsquared;
};

}  // namespace internal

}  // namespace autopas