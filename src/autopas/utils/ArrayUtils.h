/**
 * @file ArrayUtils.h
 *
 * @date 11.07.2019.
 * @author C.Menges
 */

#pragma once

#include <array>
#include <sstream>

namespace autopas::ArrayUtils {

/**
 * Creates a new array by performing an element-wise static_cast<>.
 * @tparam output_t Output type.
 * @tparam input_t Input type.
 * @tparam SIZE Size of the array.
 * @param a Input array.
 * @return Array of type std::array<output_t, SIZE>.
 */
template <class output_t, class input_t, std::size_t SIZE>
[[nodiscard]] constexpr std::array<output_t, SIZE> static_cast_array(const std::array<input_t, SIZE> &a) {
  std::array<output_t, SIZE> result{};
  for (std::size_t d = 0; d < SIZE; ++d) {
    result[d] = static_cast<output_t>(a[d]);
  }
  return result;
}

/**
 * Generates a string representation of a container which fulfills the Container requirement (provide cbegin and cend).
 * @tparam T Type of Container.
 * @param a Container.
 * @param delimiter String delimiter.
 * @return String representation of a.
 */
template <class T>
[[nodiscard]] std::string to_string(T &&a, const std::string &delimiter = ", ") {
  auto it = std::cbegin(a);
  const auto end = std::cend(a);
  if (it == end) {
    return "";
  }
  std::ostringstream strStream;
  strStream << *it;
  for (++it; it != end; ++it) {
    strStream << delimiter << *it;
  }

  return strStream.str();
}

}  // namespace autopas::ArrayUtils
