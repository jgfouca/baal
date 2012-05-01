#include "DrawMode.hpp"
#include "BaalExceptions.hpp"

namespace baal {

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
