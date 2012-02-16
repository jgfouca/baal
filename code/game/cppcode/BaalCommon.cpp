#include "BaalCommon.hpp"
#include "BaalExceptions.hpp"

#include <sstream>
#include <iostream>
#include <cstdlib>

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
bool is_opt()
///////////////////////////////////////////////////////////////////////////////
{
#ifndef NDEBUG
  return false;
#else
  return true;
#endif
}

///////////////////////////////////////////////////////////////////////////////
void clear_screen()
///////////////////////////////////////////////////////////////////////////////
{
#ifndef WINDOWS
  std::system("clear");
#endif
}

}
