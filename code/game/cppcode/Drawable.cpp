#include "Drawable.hpp"
#include "BaalExceptions.hpp"

namespace baal {

DrawMode Drawable::s_draw_mode = CIV;

///////////////////////////////////////////////////////////////////////////////
DrawMode Drawable::parse_draw_mode(const std::string& draw_mode_str)
///////////////////////////////////////////////////////////////////////////////
{
  if (draw_mode_str == "civ") {
    return CIV;
  }
  else if (draw_mode_str == "land") {
    return LAND;
  }
  else if (draw_mode_str == "yield") {
    return YIELD;
  }
  else if (draw_mode_str == "moisture") {
    return MOISTURE;
  }
  else if (draw_mode_str == "geology") {
    return GEOLOGY;
  }
  else if (draw_mode_str == "magma") {
    return MAGMA;
  }
  else if (draw_mode_str == "tension") {
    return TENSION;
  }
  else if (draw_mode_str == "wind") {
    return WIND;
  }
  else if (draw_mode_str == "temperature") {
    return TEMPERATURE;
  }
  else if (draw_mode_str == "pressure") {
    return PRESSURE;
  }
  else if (draw_mode_str == "rainfall") {
    return RAINFALL;
  }
  else if (draw_mode_str == "dewpoint") {
    return DEWPOINT;
  }
  else {
    RequireUser(false, "Unknown draw mode: " << draw_mode_str);
  }
}

///////////////////////////////////////////////////////////////////////////////
std::string Drawable::draw_mode_to_str(DrawMode draw_mode)
///////////////////////////////////////////////////////////////////////////////
{
  switch(draw_mode) {
  case CIV:
    return "civ";
  case LAND:
    return "land";
  case YIELD:
    return "yield";
  case MOISTURE:
    return "moisture";
  case GEOLOGY:
    return "geology";
  case MAGMA:
    return "magma";
  case TENSION:
    return "tension";
  case WIND:
    return "wind";
  case TEMPERATURE:
    return "temperature";
  case PRESSURE:
    return "pressure";
  case RAINFALL:
    return "rainfall";
  case DEWPOINT:
    return "dewpoint";
  default:
    Require(false, "Unhandled draw mode: " << draw_mode);
  }
}

///////////////////////////////////////////////////////////////////////////////
DrawMode& operator++(DrawMode& draw_mode)
///////////////////////////////////////////////////////////////////////////////
{
  Require(draw_mode != Drawable::LAST, "Iterating off end of draw modes");

  int i = static_cast<int>(draw_mode);
  ++i;
  return draw_mode = static_cast<DrawMode>(i);
}

}
