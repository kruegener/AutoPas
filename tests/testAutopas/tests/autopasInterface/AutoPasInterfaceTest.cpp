/**
 * @file AutoPasInterfaceTest.cpp
 * @author seckler
 * @date 13.05.19
 */

#include "AutoPasInterfaceTest.h"
#include "testingHelpers/commonTypedefs.h"

constexpr double cutoff = 1.;
constexpr double skin = 0.2;
constexpr std::array<double, 3> boxMin{0., 0., 0.};
constexpr std::array<double, 3> boxMax{10., 10., 10.};
constexpr double eps = 1.;
constexpr double sigma = 1.;
constexpr double shift = 0.1;
constexpr std::array<double, 3> zeroArr = {0., 0., 0.};

template <typename AutoPasT>
void defaultInit(AutoPasT &autoPas) {
  autoPas.setBoxMin(boxMin);
  autoPas.setBoxMax(boxMax);
  autoPas.setCutoff(cutoff);
  autoPas.setVerletSkin(skin);
  autoPas.setVerletRebuildFrequency(2);
  autoPas.setNumSamples(2);
  // init autopas
  autoPas.init();
}

template <typename AutoPasT>
void defaultInit(AutoPasT &autoPas1, AutoPasT &autoPas2, size_t direction) {
  autoPas1.setBoxMin(boxMin);
  autoPas2.setBoxMax(boxMax);

  auto midLow = boxMin, midHigh = boxMax;
  midLow[direction] = (boxMax[direction] + boxMin[direction]) / 2;
  midHigh[direction] = (boxMax[direction] + boxMin[direction]) / 2;
  autoPas1.setBoxMax(midHigh);
  autoPas2.setBoxMin(midLow);

  for (auto aP : {&autoPas1, &autoPas2}) {
    aP->setCutoff(cutoff);
    aP->setVerletSkin(skin);
    aP->setVerletRebuildFrequency(2);
    aP->setNumSamples(2);
    // init autopas
    aP->init();
  }
}

/**
 * Convert the leaving particle to entering particles.
 * Hereby the periodic boundary position change is done.
 * @param leavingParticles
 * @return vector of particles that will enter the container.
 */
std::vector<Molecule> convertToEnteringParticles(const std::vector<Molecule> &leavingParticles) {
  std::vector<Molecule> enteringParticles{leavingParticles};
  for (auto &p : enteringParticles) {
    auto pos = p.getR();
    for (auto dim = 0; dim < 3; dim++) {
      if (pos[dim] < boxMin[dim]) {
        // has to be smaller than boxMax
        pos[dim] = std::min(std::nextafter(boxMax[dim], -1), pos[dim] + (boxMax[dim] - boxMin[dim]));
      } else if (pos[dim] >= boxMax[dim]) {
        // should at least be boxMin
        pos[dim] = std::max(boxMin[dim], pos[dim] - (boxMax[dim] - boxMin[dim]));
      }
    }
    p.setR(pos);
  }
  return enteringParticles;
}

/**
 * Identifies and sends particles that are in the halo of neighboring AutoPas instances or the same instance (periodic
 * boundaries).
 * @param autoPas
 * @return vector of particles that are already shifted for the next process.
 */
auto identifyAndSendHaloParticles(autopas::AutoPas<Molecule, FMCell> &autoPas) {
  std::vector<Molecule> haloParticles;

  for (short x : {-1, 0, 1}) {
    for (short y : {-1, 0, 1}) {
      for (short z : {-1, 0, 1}) {
        if (x == 0 and y == 0 and z == 0) continue;
        std::array<short, 3> direction{x, y, z};
        std::array<double, 3> min{}, max{}, shiftVec{};
        for (size_t dim = 0; dim < 3; ++dim) {
          // The search domain has to be enlarged as the position of the particles is not certain.
          bool needsShift = false;
          if (direction[dim] == -1) {
            min[dim] = autoPas.getBoxMin()[dim] - skin;
            max[dim] = autoPas.getBoxMin()[dim] + cutoff + skin;
            if (autoPas.getBoxMin()[dim] == boxMin[dim]) {
              needsShift = true;
            }
          } else if (direction[dim] == 1) {
            min[dim] = autoPas.getBoxMax()[dim] - cutoff - skin;
            max[dim] = autoPas.getBoxMax()[dim] + skin;
            if (autoPas.getBoxMax()[dim] == boxMax[dim]) {
              needsShift = true;
            }
          } else {  // 0
            min[dim] = autoPas.getBoxMin()[dim] - skin;
            max[dim] = autoPas.getBoxMax()[dim] + skin;
          }
          if (needsShift) {
            shiftVec[dim] = -(boxMax[dim] - boxMin[dim]) * direction[dim];
          } else {
            shiftVec[dim] = 0;
          }
        }
        // here it is important to only iterate over the owned particles!
        for (auto iter = autoPas.getRegionIterator(min, max, autopas::IteratorBehavior::ownedOnly); iter.isValid();
             ++iter) {
          auto particleCopy = *iter;
          particleCopy.addR(shiftVec);
          haloParticles.push_back(particleCopy);
        }
      }
    }
  }

  return haloParticles;
}

size_t addEnteringParticles(autopas::AutoPas<Molecule, FMCell> &autoPas, std::vector<Molecule> enteringParticles) {
  size_t numAdded = 0;
  for (auto &p : enteringParticles) {
    if (autopas::utils::inBox(p.getR(), autoPas.getBoxMin(), autoPas.getBoxMax())) {
      autoPas.addParticle(p);
      ++numAdded;
    }
  }
  return numAdded;
}

void addHaloParticles(autopas::AutoPas<Molecule, FMCell> &autoPas, std::vector<Molecule> haloParticles) {
  for (auto &p : haloParticles) {
    autoPas.addOrUpdateHaloParticle(p);
  }
}

template <typename Functor>
void doSimulationLoop(autopas::AutoPas<Molecule, FMCell> &autoPas, Functor *functor) {
  // 1. update Container; return value is vector of invalid == leaving particles!
  auto [invalidParticles, updated] = autoPas.updateContainer();

  if (updated) {
    // 2. leaving and entering particles
    const auto &sendLeavingParticles = invalidParticles;
    // 2b. get+add entering particles (addParticle)
    const auto &enteringParticles = convertToEnteringParticles(sendLeavingParticles);
    auto numAdded = addEnteringParticles(autoPas, enteringParticles);

    EXPECT_EQ(numAdded, enteringParticles.size());
  }
  // 3. halo particles
  // 3a. identify and send inner particles that are in the halo of other autopas instances or itself.
  auto sendHaloParticles = identifyAndSendHaloParticles(autoPas);

  // 3b. get halo particles
  const auto &recvHaloParticles = sendHaloParticles;
  addHaloParticles(autoPas, recvHaloParticles);

  // 4. iteratePairwise
  autoPas.iteratePairwise(functor);
}

template <typename Functor>
void doSimulationLoop(autopas::AutoPas<Molecule, FMCell> &autoPas1, autopas::AutoPas<Molecule, FMCell> &autoPas2,
                      Functor *functor1, Functor *functor2) {
  // 1. update Container; return value is vector of invalid = leaving particles!
  auto [invalidParticles1, updated1] = autoPas1.updateContainer();
  auto [invalidParticles2, updated2] = autoPas2.updateContainer();

  ASSERT_EQ(updated1, updated2);
  if (updated1) {
    // 2. leaving and entering particles
    const auto &sendLeavingParticles1 = invalidParticles1;
    const auto &sendLeavingParticles2 = invalidParticles2;
    // 2b. get+add entering particles (addParticle)
    const auto &enteringParticles2 = convertToEnteringParticles(sendLeavingParticles1);
    const auto &enteringParticles1 = convertToEnteringParticles(sendLeavingParticles2);

    // the particles may either still be in the same container (just going over periodic boundaries) or in the other.
    size_t numAdded = 0;
    numAdded += addEnteringParticles(autoPas1, enteringParticles1);
    numAdded += addEnteringParticles(autoPas1, enteringParticles2);
    numAdded += addEnteringParticles(autoPas2, enteringParticles1);
    numAdded += addEnteringParticles(autoPas2, enteringParticles2);

    ASSERT_EQ(numAdded, enteringParticles1.size() + enteringParticles2.size());
  }
  // 3. halo particles
  // 3a. identify and send inner particles that are in the halo of other autopas instances or itself.
  auto sendHaloParticles1 = identifyAndSendHaloParticles(autoPas1);
  auto sendHaloParticles2 = identifyAndSendHaloParticles(autoPas2);

  // 3b. get halo particles
  const auto &recvHaloParticles2 = sendHaloParticles1;
  const auto &recvHaloParticles1 = sendHaloParticles2;
  addHaloParticles(autoPas1, recvHaloParticles1);
  addHaloParticles(autoPas2, recvHaloParticles2);

  // 4. iteratePairwise
  autoPas1.iteratePairwise(functor1);
  autoPas2.iteratePairwise(functor2);
}

template <typename Functor>
void doAssertions(autopas::AutoPas<Molecule, FMCell> &autoPas, Functor *functor) {
  std::array<Molecule, 2> molecules{};
  size_t numParticles = 0;
  for (auto iter = autoPas.begin(autopas::IteratorBehavior::ownedOnly); iter.isValid(); ++iter) {
    ASSERT_LT(numParticles, 2) << "Too many particles owned by this container.";
    molecules[numParticles++] = *iter;
  }
  ASSERT_EQ(numParticles, 2) << "The container should own exactly two particles!";

  for (auto &mol : molecules) {
    EXPECT_DOUBLE_EQ(autopas::ArrayMath::dot(mol.getF(), mol.getF()), 390144. * 390144)
        << "wrong force calculated.";  // this value should be correct already
  }

  EXPECT_DOUBLE_EQ(functor->getUpot(), 16128.1) << "wrong upot calculated";
  EXPECT_DOUBLE_EQ(functor->getVirial(), 195072.) << "wrong virial calculated";
}

template <typename Functor>
void doAssertions(autopas::AutoPas<Molecule, FMCell> &autoPas1, autopas::AutoPas<Molecule, FMCell> &autoPas2,
                  Functor *functor1, Functor *functor2) {
  std::array<Molecule, 2> molecules{};
  size_t numParticles = 0;
  for (auto iter = autoPas1.begin(autopas::IteratorBehavior::ownedOnly); iter.isValid(); ++iter) {
    ASSERT_LT(numParticles, 2) << "Too many owned particles.";
    molecules[numParticles++] = *iter;
  }
  for (auto iter = autoPas2.begin(autopas::IteratorBehavior::ownedOnly); iter.isValid(); ++iter) {
    ASSERT_LT(numParticles, 2) << "Too many owned particles.";
    molecules[numParticles++] = *iter;
  }
  ASSERT_EQ(numParticles, 2) << "There should be exactly two owned particles!";

  for (auto &mol : molecules) {
    EXPECT_DOUBLE_EQ(autopas::ArrayMath::dot(mol.getF(), mol.getF()), 390144. * 390144) << "wrong force calculated.";
  }

  EXPECT_DOUBLE_EQ(functor1->getUpot() + functor2->getUpot(), 16128.1) << "wrong upot calculated";
  EXPECT_DOUBLE_EQ(functor1->getVirial() + functor2->getVirial(), 195072.) << "wrong virial calculated";
}

void testSimulationLoop(testingTuple options) {
  // create AutoPas object
  autopas::AutoPas<Molecule, FMCell> autoPas;

  auto containerOption = std::get<0>(std::get<0>(options));
  auto traversalOption = std::get<1>(std::get<0>(options));
  auto dataLayoutOption = std::get<1>(options);
  auto newton3Option = std::get<2>(options);
  auto cellSizeOption = std::get<3>(options);

  autoPas.setAllowedContainers({containerOption});
  autoPas.setAllowedTraversals({traversalOption});
  autoPas.setAllowedDataLayouts({dataLayoutOption});
  autoPas.setAllowedNewton3Options({newton3Option});
  autoPas.setAllowedCellSizeFactors(autopas::NumberSetFinite<double>(std::set<double>({cellSizeOption})));

  defaultInit(autoPas);

  // create two particles with distance .5
  double distance = .5;
  std::array<double, 3> pos1{9.99, 5., 5.};
  std::array<double, 3> distVec{0., distance, 0.};
  std::array<double, 3> pos2 = autopas::ArrayMath::add(pos1, distVec);

  {
    Molecule particle1(pos1, {0., 0., 0.}, 0);
    Molecule particle2(pos2, {0., 0., 0.}, 1);

    // add the two particles!
    autoPas.addParticle(particle1);
    autoPas.addParticle(particle2);
  }

  autopas::LJFunctor<Molecule, FMCell, autopas::FunctorN3Modes::Both, true /*calculate globals*/> functor(cutoff, eps,
                                                                                                          sigma, shift);
  // do first simulation loop
  doSimulationLoop(autoPas, &functor);

  doAssertions(autoPas, &functor);

  // update positions a bit (outside of domain!) + reset F
  {
    std::array<double, 3> moveVec{skin / 3., 0., 0.};
    for (auto iter = autoPas.begin(autopas::IteratorBehavior::ownedOnly); iter.isValid(); ++iter) {
      iter->setR(autopas::ArrayMath::add(iter->getR(), moveVec));
      iter->setF(zeroArr);
    }
  }

  // do second simulation loop
  doSimulationLoop(autoPas, &functor);

  doAssertions(autoPas, &functor);

  // no position update this time, but resetF!
  {
    for (auto iter = autoPas.begin(autopas::IteratorBehavior::ownedOnly); iter.isValid(); ++iter) {
      iter->setF(zeroArr);
    }
  }
  // do third simulation loop, tests rebuilding of container.
  doSimulationLoop(autoPas, &functor);

  doAssertions(autoPas, &functor);
}

TEST_P(AutoPasInterfaceTest, SimulatonLoopTest) {
  // this test checks the correct behavior of the autopas interface.
  auto options = GetParam();
  try {
    testSimulationLoop(options);
  } catch (autopas::utils::ExceptionHandler::AutoPasException &autoPasException) {
    std::string str = autoPasException.what();
    if (str.find("Trying to execute a traversal that is not applicable") != std::string::npos) {
      GTEST_SKIP() << "skipped with exception: " << autoPasException.what() << std::endl;
    } else {
      // rethrow
      throw;
    }
  }
}

using ::testing::Combine;
using ::testing::UnorderedElementsAreArray;
using ::testing::Values;
using ::testing::ValuesIn;

INSTANTIATE_TEST_SUITE_P(
    Generated, AutoPasInterfaceTest,
    // proper indent
    Combine(ValuesIn([]() -> std::vector<std::tuple<autopas::ContainerOption, autopas::TraversalOption>> {
              auto allContainerOptions = autopas::allContainerOptions;
              /// @TODO no verletClusterLists yet, so we erase it for now.
              allContainerOptions.erase(allContainerOptions.find(autopas::ContainerOption::verletClusterLists));
              std::vector<std::tuple<autopas::ContainerOption, autopas::TraversalOption>> tupleVector;
              for (const auto &containerOption : allContainerOptions) {
                auto compatibleTraversals = autopas::compatibleTraversals::allCompatibleTraversals(containerOption);
                for (const auto &traversalOption : compatibleTraversals) {
                  tupleVector.emplace_back(containerOption, traversalOption);
                }
              }
              return tupleVector;
            }()),
            ValuesIn([]() -> std::set<autopas::DataLayoutOption> {
              auto all = autopas::allDataLayoutOptions;
              /// @TODO no cuda yet, so we erase it for now (if it is there)
              if (all.find(autopas::DataLayoutOption::cuda) != all.end()) {
                all.erase(all.find(autopas::DataLayoutOption::cuda));
              }
              return all;
            }()),
            ValuesIn(autopas::allNewton3Options), Values(0.5, 1., 1.5)),
    AutoPasInterfaceTest::PrintToStringParamName());

///////////////////////////////////////// TWO containers //////////////////////////////////////////////////////////

void testSimulationLoop(autopas::ContainerOption containerOption1, autopas::ContainerOption containerOption2,
                        size_t autoPasDirection) {
  // create AutoPas object
  autopas::AutoPas<Molecule, FMCell> autoPas1;
  autoPas1.setAllowedContainers(std::set<autopas::ContainerOption>{containerOption1});
  autopas::AutoPas<Molecule, FMCell> autoPas2;
  autoPas1.setAllowedContainers(std::set<autopas::ContainerOption>{containerOption2});

  defaultInit(autoPas1, autoPas2, autoPasDirection);

  // create two particles with distance .5
  double distance = .5;
  std::array<double, 3> pos1{9.99, 5., 5.};
  std::array<double, 3> distVec{0., distance, 0.};
  std::array<double, 3> pos2 = autopas::ArrayMath::add(pos1, distVec);

  {
    Molecule particle1(pos1, {0., 0., 0.}, 0);
    Molecule particle2(pos2, {0., 0., 0.}, 1);

    // add the two particles!
    for (auto p : {&particle1, &particle2}) {
      if (autopas::utils::inBox(p->getR(), autoPas1.getBoxMin(), autoPas1.getBoxMax())) {
        autoPas1.addParticle(*p);
      } else {
        autoPas2.addParticle(*p);
      }
    }
  }

  autopas::LJFunctor<Molecule, FMCell, autopas::FunctorN3Modes::Both, true /*calculate globals*/> functor1(
      cutoff, eps, sigma, shift);
  autopas::LJFunctor<Molecule, FMCell, autopas::FunctorN3Modes::Both, true /*calculate globals*/> functor2(
      cutoff, eps, sigma, shift);

  // do first simulation loop
  doSimulationLoop(autoPas1, autoPas2, &functor1, &functor2);

  doAssertions(autoPas1, autoPas2, &functor1, &functor2);

  // update positions a bit (outside of domain!) + reset F
  {
    std::array<double, 3> moveVec{skin / 3., 0., 0.};
    for (auto aP : {&autoPas1, &autoPas2}) {
      for (auto iter = aP->begin(autopas::IteratorBehavior::ownedOnly); iter.isValid(); ++iter) {
        iter->setR(autopas::ArrayMath::add(iter->getR(), moveVec));
        iter->setF(zeroArr);
      }
    }
  }

  // do second simulation loop
  doSimulationLoop(autoPas1, autoPas2, &functor1, &functor2);

  doAssertions(autoPas1, autoPas2, &functor1, &functor2);

  // reset F
  for (auto aP : {&autoPas1, &autoPas2}) {
    for (auto iter = aP->begin(autopas::IteratorBehavior::ownedOnly); iter.isValid(); ++iter) {
      iter->setF(zeroArr);
    }
  }

  // do third simulation loop, no position update
  doSimulationLoop(autoPas1, autoPas2, &functor1, &functor2);

  doAssertions(autoPas1, autoPas2, &functor1, &functor2);
}

TEST_P(AutoPasInterface2ContainersTest, SimulatonLoopTest) {
  // this test checks the correct behavior of the autopas interface.
  auto containerOptionTuple = GetParam();
  testSimulationLoop(std::get<0>(containerOptionTuple), std::get<1>(containerOptionTuple), 0);
}

using ::testing::Combine;
using ::testing::UnorderedElementsAreArray;
using ::testing::ValuesIn;

/// @todo: use this instead of below to enable testing of VerletClusterLists.
// INSTANTIATE_TEST_SUITE_P(Generated, ContainerSelectorTest,
//                         Combine(ValuesIn(autopas::allContainerOptions), ValuesIn(autopas::allContainerOptions)),
//                         ContainerSelectorTest::PrintToStringParamName());

INSTANTIATE_TEST_SUITE_P(Generated, AutoPasInterface2ContainersTest,
                         Combine(ValuesIn([]() -> std::set<autopas::ContainerOption> {
                                   auto all = autopas::allContainerOptions;
                                   all.erase(all.find(autopas::ContainerOption::verletClusterLists));
                                   return all;
                                 }()),
                                 ValuesIn([]() -> std::set<autopas::ContainerOption> {
                                   auto all = autopas::allContainerOptions;
                                   all.erase(all.find(autopas::ContainerOption::verletClusterLists));
                                   return all;
                                 }())),
                         AutoPasInterface2ContainersTest::PrintToStringParamName());
