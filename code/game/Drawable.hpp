#ifndef Drawable_hpp
#define Drawable_hpp

#include <iosfwd>

namespace baal {

// Couples to everything in the code base that knows how to draw itself, but
// I don't know of a better way to do this.
// TODO: Support all the DrawModes below.
enum DrawMode
{
  NORMAL,
  GEOLOGY,
  MAGMA,
  TENSION,
  WIND,
  TEMPERATURE,
  PRESSURE,
  DEWPOINT
};

class Drawable
{
 public:
  virtual ~Drawable() {}

  virtual void draw_text(std::ostream& out) const = 0;

  virtual void draw_graphics() const = 0;

  static DrawMode parse_draw_mode(const std::string& draw_mode_str);

 protected:
  static DrawMode m_draw_mode;

 private:
  static void set_draw_mode(DrawMode mode) { m_draw_mode = mode; }

  // only friends can change draw mode
  friend class DrawCommand;
};

}

#endif
