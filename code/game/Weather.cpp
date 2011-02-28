#include "Weather.hpp"
#include "BaalExceptions.hpp"

using namespace baal;

///////////////////////////////////////////////////////////////////////////////
void Atmosphere::draw_text(std::ostream& out) const
///////////////////////////////////////////////////////////////////////////////
{
  // TODO: This method needs much more work

  int invalid_color = -1;
  int color = invalid_color;
  char invalid_symbol = '?';
  char symbol = invalid_symbol;

  switch (m_draw_mode) {
  case WIND:
    break;
  case DEWPOINT:
    break;
  case TEMPERATURE:
    if (m_temperature < 32) {
      color = 34; // blue
      symbol = 'C';
    }
    else if (m_temperature < 80) {
      color = 33; // yellow
      symbol = 'M';
    }
    else {
      color = 31; //red
      symbol = 'H';
    }
    break;
  case PRESSURE:
    if (m_pressure < 975) {
      color = 32; // green
      symbol = 'L';
    }
    else if (m_pressure < 1025) {
      color = 33; // yellow
      symbol = 'M';
    }
    else {
      color = 31; //red
      symbol = 'H';
    }
    break;
  default:
    Require(false, "Should not be trying to draw geology in this mode");
  }

  Require(color  != invalid_color , "Color was not set");
  Require(symbol != invalid_symbol, "Symbol was not set");

  out << "\033[1;" << color << "m" // set color and bold text
      << symbol                    // print symbol
      << "\033[0m"                 // clear color and boldness
      << " ";                      // separator
}
