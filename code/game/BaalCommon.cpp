#include "BaalCommon.hpp"
#include "BaalExceptions.hpp"
#include "Engine.hpp"
#include "World.hpp"

#include <sstream>
#include <iostream>
#include <cmath>
#include <algorithm>

using std::ostream;

namespace baal {

const unsigned INVALID = INT_MAX;

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
unsigned Location::distance(const Location& location) const
///////////////////////////////////////////////////////////////////////////////
{
  return std::max( std::abs(static_cast<int>(row - location.row)),
                   std::abs(static_cast<int>(col - location.col)) );
}

///////////////////////////////////////////////////////////////////////////////
vecstr_t split(const std::string& str,
               const std::string& sep)
///////////////////////////////////////////////////////////////////////////////
{
  vecstr_t rv;
  size_t pos = 0;
  while (pos < str.size()) {
    size_t next_colon = str.find(sep, pos);
    if (next_colon == std::string::npos) {
      rv.push_back(str.substr(pos));
      break;
    }
    else {
      rv.push_back(str.substr(pos, next_colon - pos));
      pos = next_colon + sep.size();
    }
  }
  return rv;
}

}
