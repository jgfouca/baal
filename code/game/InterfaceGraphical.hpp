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

  virtual void spell_report(const std::string& report) {}

  virtual void human_wins();

  virtual void ai_wins();

};

}

#endif
