#include "DrawMode.hpp"
#include "BaalExceptions.hpp"

namespace baal {

///////////////////////////////////////////////////////////////////////////////
DrawMode parse_draw_mode(const std::string& draw_mode_str)
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
  else if (draw_mode_str == "precip") {
    return RAINFALL;
  }
  else if (draw_mode_str == "dewpoint") {
    return DEWPOINT;
  }
  else if (draw_mode_str == "elevation") {
    return ELEVATION;
  }
  else if (draw_mode_str == "snowpack") {
    return SNOWPACK;
  }
  else if (draw_mode_str == "seasurfacetemp") {
    return SEASURFACETEMP;
  }
  else if (draw_mode_str == "precip") {
    return PRECIP;
  }
  else {
    RequireUser(false, "Unknown draw mode: " << draw_mode_str);
  }
}

///////////////////////////////////////////////////////////////////////////////
std::string draw_mode_to_str(DrawMode draw_mode)
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
    return "precip";
  case DEWPOINT:
    return "dewpoint";
  case ELEVATION:
    return "elevation";
  case SNOWPACK:
    return "snowpack";
  case SEASURFACETEMP:
    return "seasurfacetemp";
  case PRECIP:
    return "precip";
  default:
    Require(false, "Unhandled draw mode: " << draw_mode);
  }
}

///////////////////////////////////////////////////////////////////////////////
DrawMode& operator++(DrawMode& draw_mode)
///////////////////////////////////////////////////////////////////////////////
{
  Require(draw_mode != LAST, "Iterating off end of draw modes");

  int i = static_cast<int>(draw_mode);
  ++i;
  return draw_mode = static_cast<DrawMode>(i);
}

///////////////////////////////////////////////////////////////////////////////
std::string explain_draw_mode(DrawMode draw_mode)
///////////////////////////////////////////////////////////////////////////////
{
  switch (draw_mode) {
  case CIV:
    return "Draws things the civ has built, like cities and infrastructure.";
  case LAND:
    return "Draws the basic lay of the land.";
  case YIELD:
    return "Draws tile yields (food and production).";
  case ELEVATION:
    return "Draws elevation of land tiles in feet.";
  case SNOWPACK:
    return "Draws depth of snowpack in inches.";
  case SEASURFACETEMP:
    return "Draws temperature of water at sea surface in farenheit.";
  case MOISTURE:
    return "Draws soil moisture as a % of normal soil moisture.";
  case GEOLOGY:
    return "Draws the plate tectonics underneath the land.";
  case MAGMA:
    return "Draws the amount of magma buildup on a scale from 0 to 1";
  case TENSION:
    return "Draws the amount of plate tension buildup on a scale from 0 to 1";
  case WIND:
    return "Draws the speed and direction of the wind in mph.";
  case TEMPERATURE:
    return "Draws the current average (not high or low) temperature in degrees farenheit.";
  case PRESSURE:
    return "Draws the current pressure in millibars (1000 is average).";
  case PRECIP:
    return "Draws the amount of precip, in inches, that has fallen this season.";
  case DEWPOINT:
    return "Draws the current dewpoint in farenheit.";
  default:
    Require(false, "Unhandled draw mode: " << draw_mode);
  }
  return "";
}

}
