/**
 * @file StringUtilsTest.h
 * @author F. Gratl
 * @date 1/15/19
 */

#pragma once

#include <gmock/gmock-matchers.h>
#include <functional>
#include "AutoPasTestBase.h"
#include "autopas/utils/StringUtils.h"

class StringUtilsTest : public AutoPasTestBase {};

/**
 * Tests a parsing function which takes a string and returns a set of values.
 * @tparam T Type of the object resulting form parsing.
 * @param allOptions Set of all expected options.
 * @param optionsString String that shall be parsed.
 * @param parseFun Parsing Function.
 */
template <class T>
void testParseMultiple(const std::set<T> &allOptions, const std::string &optionsString,
                       std::function<std::set<T>(const std::string &, bool)> &&parseFun) {
  auto parsedOptions = parseFun(optionsString, false);

  EXPECT_THAT(parsedOptions, ::testing::ElementsAreArray(allOptions));
}

/**
 * Tests a parsing function which takes a string and returns a value.
 * @tparam T Type of the object resulting form parsing.
 * @param allOptions Set of all expected options.
 * @param optionsStrings Vector of strings that shall be parsed.
 * @param parseFun Parsing function.
 */
template <class T>
void testParseSingle(const std::set<T> &allOptions, const std::vector<std::string> &optionsStrings,
                     std::function<T(const std::string &)> &&parseFun) {
  ASSERT_EQ(allOptions.size(), optionsStrings.size()) << "Not all options tested!";

  std::set<T> parsedOptions;

  for (auto &string : optionsStrings) {
    parsedOptions.insert(parseFun(string));
  }

  ASSERT_THAT(parsedOptions, ::testing::ElementsAreArray(allOptions));
}

/**
 * Test to to_string function for a given list of options.
 * @tparam T Type of the options to be tested.
 * @param goodOptions Options expected to be printable.
 * @param badOptions Options expected to return 'Unknown option'.
 */
template <class T>
void testToString(const std::set<T> &goodOptions, const std::set<T> &badOptions) {
  for (auto &op : goodOptions) {
    std::string createdString = autopas::utils::StringUtils::to_string(op);
    EXPECT_THAT(createdString, Not(::testing::HasSubstr("Unknown")));
  }
  for (auto &op : badOptions) {
    std::string createdString = autopas::utils::StringUtils::to_string(op);
    EXPECT_THAT(createdString, ::testing::HasSubstr("Unknown"));
  }
}