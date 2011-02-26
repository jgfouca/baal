#ifndef Drawable_hpp
#define Drawable_hpp

#include <iosfwd>

namespace baal {

class Drawable
{
 public:
  virtual ~Drawable() {}

  virtual void draw_text(std::ostream& out) const = 0;

  virtual void draw_graphics() const = 0;
};

}

#endif
