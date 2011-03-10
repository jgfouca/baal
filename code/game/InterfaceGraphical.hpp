#ifndef InterfaceGraphical_hpp
#define InterfaceGraphical_hpp

namespace baal {

// TODO
class InterfaceGraphical : public Interface
{
 public:
  virtual void draw() {}

  virtual void interact() {}

  virtual void help(const std::string& helpmsg) {}
};

}

#endif
