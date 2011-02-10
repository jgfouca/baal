#ifndef BaalCommon_hpp
#define BaalCommon_hpp

// Put simple, generic free functions in this file

namespace baal {

// Basically a named pair defining a location in a 2D space
struct Location
{
  Location(unsigned r, unsigned c) : row(r), col(c) {}
  
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
