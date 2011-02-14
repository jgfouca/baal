#ifndef BaalCommon_hpp
#define BaalCommon_hpp

#include <climits>
#include <string>

// Put simple, generic free functions in this file

namespace baal {

const unsigned INVALID = INT_MAX;

// Basically a named pair defining a location in a 2D space
struct Location
{
  Location() : row(INVALID), col(INVALID) {}

  Location(unsigned r, unsigned c) : row(r), col(c) {}

  Location(const std::string& str_location);
  
  unsigned row;
  unsigned col;
};

/**
 * Returns true if this file was compiled in optimized mode.
 *
 * Used for sanity-checking build system.
 */
bool is_opt();

}

#endif
