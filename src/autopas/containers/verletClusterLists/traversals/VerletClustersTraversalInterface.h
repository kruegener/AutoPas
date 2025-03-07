/**
 * @file VerletClustersTraversalInterface.h
 * @author humig
 * @date 20.06.19
 */

#pragma once

#include "autopas/containers/verletClusterLists/VerletClusterLists.h"
#include "autopas/containers/verletClusterLists/VerletClusterMaths.h"

namespace autopas {

/**
 * Interface for traversals of the VerletClusterLists container.
 */
template <class Particle>
class VerletClustersTraversalInterface {
 public:
  /**
   * virtual default destructor.
   */
  virtual ~VerletClustersTraversalInterface() = default;

  /**
   * Sets the cluster list for the traversal to iterate over.
   * @param verletClusterLists the cluster list to iterate over.
   */
  virtual void setClusterLists(VerletClusterLists<Particle> &verletClusterLists) {
    _verletClusterLists = &verletClusterLists;
  }

 protected:
  /**
   * The cluster list to iterate over.
   */
  VerletClusterLists<Particle> *_verletClusterLists;
};
}  // namespace autopas
