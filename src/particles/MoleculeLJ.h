/*
 * MoleculeLJ.h
 *
 *  Created on: 17 Jan 2018
 *      Author: tchipevn
 */

#ifndef SRC_PARTICLES_MOLECULELJ_H_
#define SRC_PARTICLES_MOLECULELJ_H_

#include "Particle.h"

namespace autopas {

    class MoleculeLJ : public Particle {
    public:
        MoleculeLJ() = default;

        explicit MoleculeLJ(std::array<double, 3> r, std::array<double, 3> v, unsigned long id) : Particle(r, v, id) {}

        virtual ~MoleculeLJ() = default;;

        static double getEpsilon() { return EPSILON; }

        static void setEpsilon(double epsilon) { EPSILON = epsilon; }

        static double getSigma() { return SIGMA; }

        static void setSigma(double sigma) { SIGMA = sigma; }

    private:
        static double EPSILON, SIGMA;
    };

} /* namespace autopas */

#endif /* SRC_PARTICLES_MOLECULELJ_H_ */
