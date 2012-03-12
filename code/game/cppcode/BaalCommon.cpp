#include "BaalCommon.hpp"
#include "BaalExceptions.hpp"
#include "Engine.hpp"
#include "World.hpp"

#include <sstream>
#include <iostream>

using std::ostream;

namespace baal {

const unsigned INVALID    = INT_MAX;
const char*    BLACK      = "30m";
const char*    RED        = "31m";
const char*    GREEN      = "32m";
const char*    YELLOW     = "33m";
const char*    BLUE       = "34m";
const char*    MAGENTA    = "35m";
const char*    CYAN       = "36m";
const char*    WHITE      = "37m";
const char*    BOLD_COLOR = "\033[1;";
const char*    CLEAR_ALL  = "\033[0m";

///////////////////////////////////////////////////////////////////////////////
Location::Location(const std::string& str_location)
///////////////////////////////////////////////////////////////////////////////
{
  std::istringstream iss(str_location);

  iss >> row;
  RequireUser(!iss.fail(), "Parse failure while getting row");

  char comma;
  iss >> comma;
  RequireUser(!iss.fail(), "Parse failure while getting comma");
  RequireUser(comma == ',', "Expected ',' after row");

  iss >> col;
  RequireUser(!iss.fail(), "Parse failure while getting column");

  RequireUser(iss.eof(), "Leftover text in str location");
}

///////////////////////////////////////////////////////////////////////////////
void AdjacentLocationIterator::advance()
///////////////////////////////////////////////////////////////////////////////
{
  const World& world = m_engine.world();
  int row_delta = -1, col_delta = -1;
  unsigned center_row = m_center.row, center_col = m_center.col;

  // Case 1: center is valid, but current is not -> this is first advance call
  if (is_valid(m_center) && !is_valid(m_current)) {
    Require(world.in_bounds(m_center), m_center << " is not in bounds");
  }
  // Case 2: both are valid -> normal iteration
  else if (is_valid(m_center) && is_valid(m_current)) {
    row_delta = m_current.row - m_center.row;
    col_delta = m_current.col - m_center.col;
    ++col_delta;
  }
  // Case 3: both are invalid -> we have reached the end
  else if (!is_valid(m_center) && !is_valid(m_current)) {
    // no-op
    return;
  }
  // Case 4: center is invalid, but current is valid -> not legal
  else {
    Require(false, "Invalid iterator state");
    return;
  }

  // Scan from current delta onwards, looking for in-bounds location
  for ( ; row_delta <= 1; ++row_delta) {
    for ( ; col_delta <= 1; ++col_delta) {
      // Adjacency does not include center tile
      if (row_delta != 0 || col_delta != 0) {
        Location loc_delta(center_row + row_delta, center_col + col_delta);
        if (world.in_bounds(loc_delta)) {
          m_current = loc_delta;
          return;
        }
      }
    }
    col_delta = -1;
  }

  // We're at the end
  m_center  = Location();
  m_current = Location();
}

}
