#include "BaalCommon.hpp"
#include "BaalExceptions.hpp"

#include <sstream>

namespace baal {

///////////////////////////////////////////////////////////////////////////////
Location::Location(const std::string& str_location)
///////////////////////////////////////////////////////////////////////////////
{
  std::istringstream iss(str_location);

  iss >> row;
  Require(!iss.fail(), "Parse failure while getting row");

  char comma;
  iss >> comma;
  Require(!iss.fail(), "Parse failure while getting comma");
  Require(comma == ',', "Expected ',' after row");
  
  iss >> col;
  Require(!iss.fail(), "Parse failure while getting column");

  Require(iss.eof(), "Leftover text in str location");
}

}

///////////////////////////////////////////////////////////////////////////////
bool baal::is_opt()
///////////////////////////////////////////////////////////////////////////////
{
#ifndef NDEBUG
  return false;
#else
  return true;
#endif
}
