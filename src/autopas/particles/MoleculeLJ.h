/**
 * @file MoleculeLJ.h
 *
 * @date 17 Jan 2018
 * @author tchipevn
 */

#pragma once

#include <vector>
#include "autopas/particles/Particle.h"

namespace autopas {

/**
 * lennard jones molecule class
 */
template <typename floatType>
class MoleculeLJBase : public ParticleBase<floatType> {
 public:
  MoleculeLJBase() = default;

  /**
   * constructor of a lennard jones molecule
   * @param r position of the molecule
   * @param v velocity of the molecule
   * @param id id of the molecule
   */
  explicit MoleculeLJBase(std::array<double, 3> r, std::array<double, 3> v, unsigned long id)
      : ParticleBase<floatType>(r, v, id) {}

  virtual ~MoleculeLJBase() = default;

  /**
   * get epsilon (characteristic energy of the lj potential)
   * @return epsilon
   */
  static double getEpsilon() { return EPSILON; }

  /**
   * set epsilon (characteristic energy of the lj potential)
   * @param epsilon
   */
  static void setEpsilon(double epsilon) { EPSILON = epsilon; }

  /**
   * get sigma (characteristic length of the lj potential)
   * @return sigma
   */
  static double getSigma() { return SIGMA; }

  /**
   * set sigma (characteristic length of the lj potential)
   * @param sigma
   */
  static void setSigma(double sigma) { SIGMA = sigma; }

 private:
  static double EPSILON, SIGMA;
};

/// Alias for double precision LJ Moleclue
typedef MoleculeLJBase<double> MoleculeLJ;

}  // namespace autopas
