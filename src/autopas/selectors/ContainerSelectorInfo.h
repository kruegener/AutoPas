/**
 * @file ContainerSelectorInfo.h
 * @author Jan Nguyen
 * @date 05.06.19
 */

#pragma once
#include <array>
#include <memory>

namespace autopas {
/**
 * Info to generate a container.
 */
class ContainerSelectorInfo {
 public:
  /**
   * Default Constructor.
   */
  ContainerSelectorInfo() : cellSizeFactor(1.), verletSkin(0.) {}

  /**
   * Constructor.
   * @param cellSizeFactor Cell size factor to be used in this container (only relevant for LinkedCells, VerletLists and
   * VerletListsCells).
   * @param verletSkin Length added to the cutoff for the verlet lists' skin.
   */
  explicit ContainerSelectorInfo(double cellSizeFactor, double verletSkin)
      : cellSizeFactor(cellSizeFactor), verletSkin(verletSkin) {}

  /**
   * Equality between ContainerSelectorInfo
   * @param other
   * @return True iff all member equal
   */
  bool operator==(const ContainerSelectorInfo &other) const {
    return cellSizeFactor == other.cellSizeFactor and verletSkin == other.verletSkin;
  }

  /**
   * Inequality between ContainerSelectorInfo
   * @param other
   * @return False iff all member euqal
   */
  bool operator!=(const ContainerSelectorInfo &other) const { return !(*this == other); }

  /**
   * Comparison operator for ContainerSelectorInfo objects.
   * Configurations are compared member wise in the order: _cellSizeFactor, _verletSkin, _verletRebuildFrequency
   *
   * @param other
   * @return
   */
  bool operator<(const ContainerSelectorInfo &other) {
    return std::tie(cellSizeFactor, verletSkin) < std::tie(other.cellSizeFactor, other.verletSkin);
  }

  /**
   * cellSizeFactor Cell size factor to be used in this container (only relevant for LinkedCells)
   */
  double cellSizeFactor;
  /**
   * Length added to the cutoff for the verlet lists' skin.
   */
  double verletSkin;
};

}  // namespace autopas
