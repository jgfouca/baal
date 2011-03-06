#include "Drawable.hpp"
#include "BaalExceptions.hpp"

using namespace baal;

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
  else if (draw_mode_str == "dewpoint") {
    return DEWPOINT;
  }
  else {
    RequireUser(false, "Unknown draw mode: " << draw_mode_str);
  }
}
