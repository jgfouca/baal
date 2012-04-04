#ifndef Drawable_hpp
#define Drawable_hpp

#include <iosfwd>

namespace baal {

// Couples to everything in the code base that knows how to draw itself, but
// I don't know of a better way to do this.
// TODO: Support all the DrawModes below.
// Do not change the first or last enum value without changing Drawbable::FIRST
// or Drawable::LAST. Do NOT assign custom values to enums.
enum DrawMode
{
  CIV,     // Coupled with Drawable::FIRST
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
  PRECIP   // Coupled with Drawable::LAST
};

class Drawable
{
 public:
  virtual ~Drawable() {}

  virtual void draw_text(std::ostream& out) const = 0;

  virtual void draw_graphics() const = 0;

  static DrawMode parse_draw_mode(const std::string& draw_mode_str);

  static std::string draw_mode_to_str(DrawMode draw_mode);

  static const DrawMode FIRST = CIV;
  static const DrawMode LAST  = PRECIP;

 protected:
  static DrawMode s_draw_mode;

 private:
  static void set_draw_mode(DrawMode mode) { s_draw_mode = mode; }

  // only friends can change draw mode
  friend class DrawCommand;
};

DrawMode& operator++(DrawMode& draw_mode);

std::string explain_draw_mode(DrawMode draw_mode);

}

#endif
