#ifndef DrawMode_hpp
#define DrawMode_hpp

#include "BaalCommon.hpp"

#include <string>

// Couples to everything in the code base that knows how to draw itself, but
// I don't know of a better way to do this.
// TODO: Support all the DrawModes below.
SMART_ENUM(DrawMode,
           CIV,
           LAND,
           YIELD,
           MOISTURE,
           GEOLOGY,
           MAGMA,
           TENSION,
           WIND,
           TEMPERATURE,
           PRESSURE,
           PRECIP,
           DEWPOINT,
           ELEVATION,
           SNOWPACK,
           SEASURFACETEMP);

namespace baal {

std::string explain_draw_mode(DrawMode draw_mode);

}

#endif
