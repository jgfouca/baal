#ifndef BaalCommon_hpp
#define BaalCommon_hpp

#include <climits>
#include <string>
#include <iostream>

// Put simple, generic free functions in this file

namespace baal {

extern const unsigned INVALID;

// Common ascii colors
extern const char* BLACK;
extern const char* RED;
extern const char* GREEN;
extern const char* YELLOW;
extern const char* BLUE;
extern const char* MAGENTA;
extern const char* CYAN;
extern const char* WHITE;

// Ascii bold prefix/postfix
extern const char* BOLD_COLOR;
extern const char* CLEAR_ALL;

// Basically a named pair defining a location in a 2D space
struct Location
{
  Location() : row(INVALID), col(INVALID) {}

  Location(unsigned r, unsigned c) : row(r), col(c) {}

  Location(const std::string& str_location);

  bool operator==(const Location& location) const;

  bool operator!=(const Location& location) const;

  unsigned row;
  unsigned col;
};

// Inlined methods

///////////////////////////////////////////////////////////////////////////////
inline
bool Location::operator==(const Location& location) const
///////////////////////////////////////////////////////////////////////////////
{
  return (row == location.row && col == location.col);
}

///////////////////////////////////////////////////////////////////////////////
inline
bool Location::operator!=(const Location& location) const
///////////////////////////////////////////////////////////////////////////////
{
  return !(*this == location);
}

///////////////////////////////////////////////////////////////////////////////
inline
std::ostream& operator<<(std::ostream& out, const Location& location)
///////////////////////////////////////////////////////////////////////////////
{
  return out << location.row << ',' << location.col;
}

//
// Misc free functions
//

/**
 * Returns true if this file was compiled in optimized mode.
 *
 * Used for sanity-checking build system.
 */
bool is_opt();

void clear_screen();

}

#endif
