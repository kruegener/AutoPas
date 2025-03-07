/**
 * @file Newton3Option.h
 * @author F. Gratl
 * @date 01/02/2019
 */

#pragma once

#include <set>

namespace autopas {
/**
 * Possible choices for the particle data layout. Values consistent with bool.
 */
enum Newton3Option { disabled = 0, enabled = 1 };

/**
 * Provides a way to iterate over the possible choices of data layouts.
 */
static const std::set<Newton3Option> allNewton3Options = {
    Newton3Option::enabled,
    Newton3Option::disabled,
};

}  // namespace autopas
