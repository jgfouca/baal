#include "Drawable.hpp"
#include "BaalExceptions.hpp"

using namespace baal;

DrawMode Drawable::m_draw_mode = NORMAL;

///////////////////////////////////////////////////////////////////////////////
DrawMode Drawable::parse_draw_mode(const std::string& draw_mode_str)
///////////////////////////////////////////////////////////////////////////////
{
  if (draw_mode_str == "NORMAL") {
    return NORMAL;
  }
  else if (draw_mode_str == "GEOLOGY") {
    return GEOLOGY;
  }
  else if (draw_mode_str == "MAGMA") {
    return MAGMA;
  }
  else if (draw_mode_str == "TENSION") {
    return TENSION;
  }
  else if (draw_mode_str == "WIND") {
    return WIND;
  }
  else if (draw_mode_str == "TEMPERATURE") {
    return TEMPERATURE;
  }
  else if (draw_mode_str == "PRESSURE") {
    return PRESSURE;
  }
  else if (draw_mode_str == "DEWPOINT") {
    return DEWPOINT;
  }
  else {
    Require(false, "Unknown draw mode: " << draw_mode_str);
  }
}
