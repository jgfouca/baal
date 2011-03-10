#ifndef BaalCommon_hpp
#define BaalCommon_hpp

#include <climits>
#include <string>
#include <iosfwd>

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

  std::ostream& operator<<(std::ostream& out) const;

  unsigned row;
  unsigned col;
};

/**
 * Returns true if this file was compiled in optimized mode.
 *
 * Used for sanity-checking build system.
 */
bool is_opt();

std::ostream& operator<<(std::ostream& out, const Location& location);

void clear_screen();

}

#endif
