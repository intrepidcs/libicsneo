#ifndef __ICSNEO_OPTIONAL_H_
#define __ICSNEO_OPTIONAL_H_

#include "nonstd/optional.hpp"

/**
 * We use icsneo::optional throughout the C++ API to allow for polyfilling
 * std::optional into C++11 and C++14 environments. In a C++17 or better
 * environment, icsneo::optional will be an alias of std::optional.
 */

namespace icsneo {

	using nonstd::optional;
	using nonstd::bad_optional_access;
	using nonstd::make_optional;

	using nonstd::nullopt;
	using nonstd::nullopt_t;

} // namespace icsneo

#endif