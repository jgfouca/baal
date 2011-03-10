#include "BaalCommon.hpp"
#include "BaalExceptions.hpp"

#include <sstream>
#include <iostream>
#include <cstdlib>

using std::ostream;
using namespace baal;

const unsigned baal::INVALID    = INT_MAX;
const char*    baal::BLACK      = "30m";
const char*    baal::RED        = "31m";
const char*    baal::GREEN      = "32m";
const char*    baal::YELLOW     = "33m";
const char*    baal::BLUE       = "34m";
const char*    baal::MAGENTA    = "35m";
const char*    baal::CYAN       = "36m";
const char*    baal::WHITE      = "37m";
const char*    baal::BOLD_COLOR = "\033[1;";
const char*    baal::CLEAR_ALL  = "\033[0m";

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
ostream& Location::operator<<(ostream& out) const
///////////////////////////////////////////////////////////////////////////////
{
  return out << row << ',' << col;
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

///////////////////////////////////////////////////////////////////////////////
ostream& baal::operator<<(ostream& out, const Location& location)
///////////////////////////////////////////////////////////////////////////////
{
  return location.operator<<(out);
}

///////////////////////////////////////////////////////////////////////////////
void baal::clear_screen()
///////////////////////////////////////////////////////////////////////////////
{
#ifndef WINDOWS
  std::system("clear");
#endif
}
