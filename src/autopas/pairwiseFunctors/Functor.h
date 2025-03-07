/**
 * @file Functor.h
 *
 * @date 17 Jan 2018
 * @author tchipevn
 */

#pragma once

#include "autopas/cells/ParticleCell.h"
#include "autopas/utils/AlignedAllocator.h"
#include "autopas/utils/CudaSoA.h"
#include "autopas/utils/ExceptionHandler.h"
#include "autopas/utils/SoAView.h"

#include <type_traits>

#if defined(AUTOPAS_CUDA)
#include "autopas/pairwiseFunctors/FunctorCuda.cuh"
#endif

namespace autopas {

template <class Particle>
class VerletListHelpers;

namespace internal {
/**
 * Dummy class to provide empty arrays.
 * This class is needed to provide a default argument to the implementation type (Impl_t) template parameter of Functor.
 * @tparam Particle
 */
template <class Particle>
class Dummy final {
 public:
  /**
   * @copydoc Functor::getNeededAttr()
   */
  constexpr static const std::array<typename Particle::AttributeNames, 0> getNeededAttr() {
    return std::array<typename Particle::AttributeNames, 0>{};
  }

  /**
   * @copydoc Functor::getComputedAttr()
   */
  constexpr static const std::array<typename Particle::AttributeNames, 0> getComputedAttr() {
    return std::array<typename Particle::AttributeNames, 0>{};
  }
};
}  // namespace internal

/**
 * Functor class. This class describes the pairwise interactions between
 * particles.
 * Both an array of structure (AoS) and a structure of array (SoA) are supported
 * to be used with functors.
 * Newton3: A functor does not have to implement both a newton3 and a
 * non-newton3 version. Instead you can specify, which version you use by
 * overriding allowsNonNewton3 resp. allowsNewton3
 *
 * @tparam Particle the type of Particle
 * @tparam ParticleCell_t the type of ParticleCell
 */
template <class Particle, class ParticleCell_t, class SoAArraysType = typename Particle::SoAArraysType,
          typename Impl_t = internal::Dummy<Particle>>
class Functor {
 public:
  /**
   * Constructor
   * @param cutoff
   */
  Functor(typename Particle::ParticleFloatingPointType cutoff) : _cutoff(cutoff){};

  virtual ~Functor() = default;

  /**
   * This function is called at the start of each traversal.
   * Use it for resetting global values or initializing them.
   */
  virtual void initTraversal(){};

  /**
   * This function is called at the end of each traversal.
   * You may accumulate values in this step.
   * @param newton3
   */
  virtual void endTraversal(bool newton3){};

  /**
   * @brief Functor for arrays of structures (AoS).
   *
   * This functor should calculate the forces or any other pair-wise interaction
   * between two particles.
   * This should include a cutoff check if needed!
   * @param i Particle i
   * @param j Particle j
   * @param newton3 defines whether or whether not to use newton 3
   */
  virtual void AoSFunctor(Particle &i, Particle &j, bool newton3) {
    utils::ExceptionHandler::exception("Functor::AoSFunctor: not yet implemented");
  }

  /**
   * Get attributes needed for computation.
   * @return Attributes needed for computation.
   * @todo C++20: make this function virtual
   */
  constexpr static const std::array<typename Particle::AttributeNames, 0> getNeededAttr() {
    return std::array<typename Particle::AttributeNames, 0>{};
  }

  /**
   * Get attributes needed for computation without N3 optimization.
   * @return Attributes needed for computation.
   * @todo C++20: make this function virtual
   */
  constexpr static const std::array<typename Particle::AttributeNames, 0> getNeededAttr(std::false_type) {
    return Impl_t::getNeededAttr();
  }

  /**
   * Get attributes computed by this functor.
   * @return Attributes computed by this functor.
   * @todo C++20: make this function virtual
   */
  constexpr static const std::array<typename Particle::AttributeNames, 0> getComputedAttr() {
    return std::array<typename Particle::AttributeNames, 0>{};
  }

  /**
   * @brief Functor for structure of arrays (SoA)
   *
   * This functor should calculate the forces or any other pair-wise interaction
   * between all particles in an soa.
   * This should include a cutoff check if needed!
   *
   * @param soa Structure of arrays
   * @param newton3 defines whether or whether not to use newton 3
   */
  virtual void SoAFunctor(SoAView<SoAArraysType> soa, bool newton3) {
    utils::ExceptionHandler::exception("Functor::SoAFunctor(one soa): not yet implemented");
  }

  /**
   * @brief Functor for structure of arrays (SoA) for neighbor lists
   *
   * This functor should calculate the forces or any other pair-wise interaction
   * between the particles in the SoA that are marked by the verlet list
   * This should include a cutoff check if needed!
   *
   * iFrom an iTo define the range inside of the neighborList that should be
   * iterated over. The starting index is i = iFrom. The iteration will continue
   * while i < iTo.
   *
   * @param soa Structure of arrays
   * @param neighborList The list of neighbors
   * @param iFrom the starting index of the vector neighborList that should be
   * iterated over
   * @param iTo the first index that should not be iterated over. (Should be at
   * least iFrom and less than soa.size())
   * @param newton3 defines whether or whether not to use newton 3
   */
  virtual void SoAFunctor(SoAView<SoAArraysType> soa,
                          const std::vector<std::vector<size_t, autopas::AlignedAllocator<size_t>>> &neighborList,
                          size_t iFrom, size_t iTo, bool newton3) {
    utils::ExceptionHandler::exception("Functor::SoAFunctor(verlet): not yet implemented");
  }

  /**
   * @brief Functor for structure of arrays (SoA)
   *
   * This functor should calculate the forces or any other pair-wise interaction
   * between all particles of soa1 and soa2.
   * This should include a cutoff check if needed!
   *
   * @param soa1 First structure of arrays.
   * @param soa2 Second structure of arrays.
   * @param newton3 defines whether or whether not to use newton 3
   */
  virtual void SoAFunctor(SoAView<SoAArraysType> soa1, SoAView<SoAArraysType> soa2, bool newton3) {
    utils::ExceptionHandler::exception("Functor::SoAFunctor(two soa): not yet implemented");
  }

  /**
   * @brief Functor using Cuda on SoA in device Memory
   *
   * This Functor calculates the pair-wise interactions between particles in the device_handle on the GPU
   *
   * @param device_handle soa in device memory
   * @param newton3 defines whether or whether not to use newton
   */
  virtual void CudaFunctor(CudaSoA<typename Particle::CudaDeviceArraysType> &device_handle, bool newton3) {
    utils::ExceptionHandler::exception("Functor::CudaFunctorNoN3: not yet implemented");
  }

  /**
   * @brief Functor using Cuda on SoAs in device Memory
   *
   * This Functor calculates the pair-wise interactions between particles in the device_handle1 and device_handle2 on
   * the GPU
   *
   * @param device_handle1 first soa in device memory
   * @param device_handle2 second soa in device memory
   * @param newton3 defines whether or whether not to use newton
   */
  virtual void CudaFunctor(CudaSoA<typename Particle::CudaDeviceArraysType> &device_handle1,
                           CudaSoA<typename Particle::CudaDeviceArraysType> &device_handle2, bool newton3) {
    utils::ExceptionHandler::exception("Functor::CudaFunctorNoN3(two cells): not yet implemented");
  }

  /**
   * @brief Copies the SoA data of the given cell to the Cuda device.
   *
   * @param soa  Structure of arrays where the data is loaded.
   * @param device_handle soa in device memory where the data is copied to
   */
  virtual void deviceSoALoader(::autopas::SoA<SoAArraysType> &soa,
                               CudaSoA<typename Particle::CudaDeviceArraysType> &device_handle) {
    utils::ExceptionHandler::exception("Functor::CudaDeviceSoALoader: not yet implemented");
  }

  /**
   * @brief Copies the data stored on the Cuda device back to the SoA overwrites the data in the soa
   *
   * @param soa  Structure of arrays where the data copied to.
   * @param device_handle soa in device memory where the data is loaded from
   */
  virtual void deviceSoAExtractor(::autopas::SoA<SoAArraysType> &soa,
                                  CudaSoA<typename Particle::CudaDeviceArraysType> &device_handle) {
    utils::ExceptionHandler::exception("Functor::CudaDeviceSoAExtractor: not yet implemented");
  }

  /**
   * @brief Copies the AoS data of the given cell in the given soa.
   *
   * @param cell Cell from where the data is loaded.
   * @param soa  Structure of arrays where the data is copied to.
   * @param offset Offset within the SoA. The data of the cell should be added
   * to the SoA with the specified offset.
   */
  virtual void SoALoader(ParticleCell<Particle> &cell, ::autopas::SoA<SoAArraysType> &soa, size_t offset = 0) {
    SoALoaderImpl(cell, soa, offset, std::make_index_sequence<Impl_t::getNeededAttr().size()>{});
  }

  /**
   * @brief Copies the data stored in the soa back into the cell.
   *
   * @param cell Cell where the data should be stored.
   * @param soa  Structure of arrays from where the data is loaded.
   * @param offset Offset within the SoA. The data of the soa should be
   * extracted starting at offset.
   */
  virtual void SoAExtractor(ParticleCell<Particle> &cell, ::autopas::SoA<SoAArraysType> &soa, size_t offset = 0) {
    SoAExtractorImpl(cell, soa, offset, std::make_index_sequence<Impl_t::getComputedAttr().size()>{});
  }

  /**
   * Specifies whether the functor is capable of Newton3-like functors.
   * If the functor provides an interface to soa or aos functions that utilize
   * Newton's third law of motion (actio = reactio) to reduce the computational
   * complexity this function should return true. If this is not the case this
   * function should return false.
   * @return true if and only if this functor provides an interface to
   * Newton3-like functions.
   */
  virtual bool allowsNewton3() = 0;

  /**
   * Specifies whether the functor is capable of non-Newton3-like functors.
   * If the functor provides an interface to soa or aos functions that do not
   * utilize Newton's third law of motion (actio = reactio) this function should
   * return true. If this is not the case this function should return false.
   * @return true if and only if this functor provides an interface to functions
   * that do not utilize Newton3.
   */
  virtual bool allowsNonNewton3() = 0;

  /**
   * Specifies whether the functor should be considered for the auto-tuning process.
   * @return true if and only if this functor is relevant for auto-tuning.
   */
  virtual bool isRelevantForTuning() = 0;

#if defined(AUTOPAS_CUDA)
  /**
   * Provides an interface for traversals to directly access Cuda Functions
   * @return Pointer to CudaWrapper of the Functor
   */
  virtual CudaWrapperInterface<typename Particle::ParticleFloatingPointType> *getCudaWrapper() { return nullptr; }

  /**
   * Creates Cuda SoA object containing all the relevant pointers from the generic Cuda SoA
   * @return unique pointer to the object
   */
  virtual std::unique_ptr<FunctorCudaSoA<typename Particle::ParticleFloatingPointType>> createFunctorCudaSoA(
      CudaSoA<typename Particle::CudaDeviceArraysType> &device_handle) {
    return std::make_unique<FunctorCudaSoA<typename Particle::ParticleFloatingPointType>>();
  }
#endif

  /**
   * Getter for the functor's cutoff
   * @return
   */
  typename Particle::ParticleFloatingPointType getCutoff() const { return _cutoff; }

 private:
  /**
   * Implements loading of SoA buffers.
   * @tparam cell_t Cell type.
   * @tparam I Attribute.
   * @param cell Cell from where the data is loaded.
   * @param soa  Structure of arrays where the data is copied to.
   * @param offset Offset within the SoA. The data of the cell should be added
   * to the SoA with the specified offset.
   */
  template <typename cell_t, std::size_t... I>
  void SoALoaderImpl(cell_t &cell, ::autopas::SoA<SoAArraysType> &soa, size_t offset, std::index_sequence<I...>) {
    soa.resizeArrays(offset + cell.numParticles());

    if (cell.numParticles() == 0) return;

    /**
     * Store the start address of all needed arrays inside the SoA buffer in a tuple. This avoids unnecessary look ups
     * in the following loop.
     */
    // maybe_unused necessary because gcc doesnt understand that pointer is used later
    [[maybe_unused]] auto const pointer = std::make_tuple(soa.template begin<Impl_t::getNeededAttr()[I]>()...);

    auto cellIter = cell.begin();
    // load particles in SoAs
    for (size_t i = offset; cellIter.isValid(); ++cellIter, ++i) {
      /**
       * The following statement writes the values of all attributes defined in neededAttr into the respective position
       * inside the SoA buffer. I represents the index inside neededAttr. The whole expression is folded sizeof...(I)
       * times over the comma operator. E.g. like this (std::index_sequence<I...> = 0, 1):
       * ((std::get<0>(pointer)[i] = cellIter->template get<Impl_t::neededAttr[0]>()),
       * (std::get<1>(pointer)[i] = cellIter->template get<Impl_t::neededAttr[1]>()))
       */
      ((std::get<I>(pointer)[i] = cellIter->template get<Impl_t::getNeededAttr()[I]>()), ...);
    }
  }

  /**
   * Implements extraction of SoA buffers.
   * @tparam cell_t Cell type.
   * @tparam I Attribute.
   * @param cell Cell.
   * @param soa SoA buffer.
   * @param offset Offset
   */
  template <typename cell_t, std::size_t... I>
  void SoAExtractorImpl(cell_t &cell, ::autopas::SoA<SoAArraysType> &soa, size_t offset, std::index_sequence<I...>) {
    if (cell.numParticles() == 0) return;

    /**
     * Store the start address of all needed arrays inside the SoA buffer in a tuple. This avoids unnecessary look ups
     * in the following loop.
     */
    // maybe_unused necessary because gcc doesnt understand that pointer is used later
    [[maybe_unused]] auto const pointer = std::make_tuple(soa.template begin<Impl_t::getComputedAttr()[I]>()...);

    auto cellIter = cell.begin();
    // write values in SoAs back to particles
    for (size_t i = offset; cellIter.isValid(); ++cellIter, ++i) {
      /**
       * The following statement writes the value of all attributes defined in computedAttr back into the particle.
       * I represents the index inside computedAttr.
       * The whole expression is folded sizeof...(I) times over the comma operator. E.g. like this
       * (std::index_sequence<I...> = 0, 1):
       * (cellIter->template set<Impl_t::computedAttr[0]>(std::get<0>(pointer)[i]),
       * cellIter->template set<Impl_t::computedAttr[1]>(std::get<1>(pointer)[i]))
       */
      (cellIter->template set<Impl_t::getComputedAttr()[I]>(std::get<I>(pointer)[i]), ...);
    }
  }

  typename Particle::ParticleFloatingPointType _cutoff;
};

}  // namespace autopas

#include "autopas/containers/verletListsCellBased/verletLists/VerletListHelpers.h"
