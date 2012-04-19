#ifndef DrawMode_hpp
#define DrawMode_hpp

#include <iosfwd>

namespace baal {

// Couples to everything in the code base that knows how to draw itself, but
// I don't know of a better way to do this.
// TODO: Support all the DrawModes below.
// Do not change the first or last enum value without changing FIRST
// or LAST. Do NOT assign custom values to enums.
enum DrawMode
{
  FIRST = 0,
  CIV = FIRST,
  LAND,
  YIELD,
  MOISTURE,
  GEOLOGY,
  MAGMA,
  TENSION,
  WIND,
  TEMPERATURE,
  PRESSURE,
  RAINFALL,
  DEWPOINT,
  ELEVATION,
  SNOWPACK,
  SEASURFACETEMP,
  PRECIP,
  LAST = PRECIP
};

// TODO - Need better way of handling enums!

DrawMode parse_draw_mode(const std::string& draw_mode_str);

std::string draw_mode_to_str(DrawMode draw_mode);

DrawMode& operator++(DrawMode& draw_mode);

std::string explain_draw_mode(DrawMode draw_mode);

}

#endif
