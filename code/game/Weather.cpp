#include "Weather.hpp"
#include "BaalExceptions.hpp"
#include "WorldTile.hpp"

#include <iomanip>

using namespace baal;

namespace {

///////////////////////////////////////////////////////////////////////////////
std::string direction_str(Direction direction)
///////////////////////////////////////////////////////////////////////////////
{
  switch(direction) {
  case N:   return "N";
  case NNE: return "NNE";
  case NE:  return "NE";
  case ENE: return "ENE";
  case E:   return "E";
  case ESE: return "ESE";
  case SE:  return "SE";
  case SSE: return "SSE";
  case S:   return "S";
  case SSW: return "SSW";
  case SW:  return "SW";
  case WSW: return "WSW";
  case W:   return "W";
  case WNW: return "WNW";
  case NW:  return "NW";
  case NNW: return "NNW";
  default:
    Require(false, "Should never make it here");
  }
}

///////////////////////////////////////////////////////////////////////////////
void draw_wind(std::ostream& out, const Wind& wind)
///////////////////////////////////////////////////////////////////////////////
{
  out << std::setw(3) << std::left << direction_str(wind.m_direction);
  out << std::right;

  unsigned speed = wind.m_speed;
  int color = -1; // init to invalid
  if (speed < 10) {
    color = 32; // green
  }
  else if (speed < 20) {
    color = 33; // yellow
  }
  else {
    color = 31; // red
  }

  out << "\033[1;" << color << "m" // set color and bold text
      << std::setw(2) << speed     // print value
      << "\033[0m";                // clear color and boldness
}

///////////////////////////////////////////////////////////////////////////////
void draw_dewpoint(std::ostream& out, int dewpoint)
///////////////////////////////////////////////////////////////////////////////
{
  out << " ";

  int color = -1; // init to invalid
  if (dewpoint < 32) {
    color = 31; // red (dry)
  }
  else if (dewpoint < 55) {
    color = 33; // yellow
  }
  else {
    color = 32; // green (moist)
  }

  out << "\033[1;" << color << "m" // set color and bold text
      << std::left << std::setw(3) << dewpoint  // print value
      << "\033[0m";                // clear color and boldness
  out << std::right << " ";
}

///////////////////////////////////////////////////////////////////////////////
void draw_temperature(std::ostream& out, int temperature)
///////////////////////////////////////////////////////////////////////////////
{
  out << " ";

  int color = -1; // init to invalid
  if (temperature < 32) {
    color = 34; // blue
  }
  else if (temperature < 80) {
    color = 33; // yellow
  }
  else {
    color = 31; // red
  }

  out << "\033[1;" << color << "m" // set color and bold text
      << std::left << std::setw(3) << temperature  // print value
      << "\033[0m";                // clear color and boldness
  out << std::right << " ";

}

///////////////////////////////////////////////////////////////////////////////
void draw_pressure(std::ostream& out, unsigned pressure)
///////////////////////////////////////////////////////////////////////////////
{
  out << " ";

  int color = -1;
  if (pressure < 975) {
    color = 32; // green
  }
  else if (pressure < 1025) {
    color = 33; // yellow
  }
  else {
    color = 31; // red
  }

  out << "\033[1;" << color << "m" // set color and bold text
      << std::left << std::setw(4) << pressure  // print value
      << "\033[0m";                // clear color and boldness
  out << std::right;
}

} // empty namespace

///////////////////////////////////////////////////////////////////////////////
Atmosphere::Atmosphere(const Climate& climate)
///////////////////////////////////////////////////////////////////////////////
  : m_temperature(climate.temperature()),
    m_pressure(NORMAL_PRESSURE),
    m_wind(climate.wind())
{
  m_dewpoint = compute_dewpoint();
}

///////////////////////////////////////////////////////////////////////////////
void Atmosphere::draw_text(std::ostream& out) const
///////////////////////////////////////////////////////////////////////////////
{
  switch (s_draw_mode) {
  case WIND:
    draw_wind(out, m_wind);
    break;
  case DEWPOINT:
    draw_dewpoint(out, m_dewpoint);
    break;
  case TEMPERATURE:
    draw_temperature(out, m_temperature);
    break;
  case PRESSURE:
    draw_pressure(out, m_pressure);
    break;
  default:
    Require(false, "Should not draw atmosphere in mode: " << s_draw_mode);
  }
}

///////////////////////////////////////////////////////////////////////////////
int Atmosphere::compute_dewpoint() const
///////////////////////////////////////////////////////////////////////////////
{
  // TODO
  return m_temperature;
}

///////////////////////////////////////////////////////////////////////////////
bool Atmosphere::is_atmospheric(DrawMode mode)
///////////////////////////////////////////////////////////////////////////////
{
  return mode == WIND        ||
         mode == DEWPOINT    ||
         mode == TEMPERATURE ||
         mode == PRESSURE;
}
