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
  const char* color = "";
  if (speed < 10) {
    color = GREEN;
  }
  else if (speed < 20) {
    color = YELLOW;
  }
  else {
    color = RED;
  }

  out << BOLD_COLOR << color       // set color and bold text
      << std::setw(2) << speed     // print value
      << CLEAR_ALL;                // clear color and boldness
}

///////////////////////////////////////////////////////////////////////////////
void draw_dewpoint(std::ostream& out, int dewpoint)
///////////////////////////////////////////////////////////////////////////////
{
  out << " ";

  const char* color = "";
  if (dewpoint < 32) {
    color = RED;
  }
  else if (dewpoint < 55) {
    color = YELLOW;
  }
  else {
    color = GREEN;
  }

  out << BOLD_COLOR << color       // set color and bold text
      << std::left << std::setw(3) << dewpoint  // print value
      << CLEAR_ALL;                // clear color and boldness
  out << std::right << " ";
}

///////////////////////////////////////////////////////////////////////////////
void draw_temperature(std::ostream& out, int temperature)
///////////////////////////////////////////////////////////////////////////////
{
  out << " ";

  const char* color = "";
  if (temperature < 32) {
    color = BLUE;
  }
  else if (temperature < 80) {
    color = YELLOW;
  }
  else {
    color = RED;
  }

  out << BOLD_COLOR << color       // set color and bold text
      << std::left << std::setw(3) << temperature  // print value
      << CLEAR_ALL;                // clear color and boldness
  out << std::right << " ";
}

///////////////////////////////////////////////////////////////////////////////
void draw_pressure(std::ostream& out, unsigned pressure)
///////////////////////////////////////////////////////////////////////////////
{
  out << " ";

  const char* color = "";
  if (pressure < 975) {
    color = GREEN;
  }
  else if (pressure < 1025) {
    color = YELLOW;
  }
  else {
    color = RED;
  }

  out << BOLD_COLOR << color   // set color and bold text
      << std::left << std::setw(4) << pressure  // print value
      << CLEAR_ALL;            // clear color and boldness
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
