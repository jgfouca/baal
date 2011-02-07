#ifndef Interface_hpp
#define Interface_hpp

namespace baal {

class World;

class Interface
{
 public:
  virtual void draw(const World& world) const = 0;

 private:
};

}

#endif
