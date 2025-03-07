/**
 * @file CellBorderAndFlagManager.h
 * @author seckler
 * @date 14.05.18
 */

#pragma once
#include <cstdlib>

namespace autopas {
namespace internal {
/**
 * Interface class to handle cell borders and cell types of cells.
 * @todo: add cell border handling
 */
class CellBorderAndFlagManager {
  /**
   * The index type to access the particle cells.
   */
  typedef std::size_t index_t;

 public:
  /**
   * Cestructor
   */
  virtual ~CellBorderAndFlagManager() = default;

  /**
   * Checks if the cell with the one-dimensional index index1d can contain halo particles.
   * @param index1d the one-dimensional index of the cell that should be checked
   * @return true if the cell can contain halo particles
   */
  virtual bool cellCanContainHaloParticles(index_t index1d) const = 0;

  /**
   * Checks if the cell with the one-dimensional index index1d can contain owned particles.
   * @param index1d the one-dimensional index of the cell that should be checked
   * @return true if the cell can contain owned particles
   */
  virtual bool cellCanContainOwnedParticles(index_t index1d) const = 0;
};
}  // namespace internal
}  // namespace autopas
