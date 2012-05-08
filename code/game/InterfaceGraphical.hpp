#ifndef InterfaceGraphical_hpp
#define InterfaceGraphical_hpp

#include "Interface.hpp"

namespace baal {

class Engine;
class Geology;
class Player;
class PlayerAI;
class Time;
class Atmosphere;
class Anomaly;
class World;
class WorldTile;

// TODO
class InterfaceGraphical : public Interface
{
 public:
  InterfaceGraphical(Engine& engine);

  virtual void draw() {}

  virtual void draw(const Geology&) { }
  virtual void draw(const Player&) { }
  virtual void draw(const PlayerAI&) { }
  virtual void draw(const Time&) { }
  virtual void draw(const Atmosphere&) { };
  virtual void draw(const Anomaly&) { };
  virtual void draw(const World&) { }
  virtual void draw(const WorldTile&) { }

  virtual void interact() {}

  virtual void help(const std::string& helpmsg) {}

  virtual void spell_report(const std::string& report) {}

  virtual void human_wins() {}

  virtual void ai_wins() {}

 private:
  Engine& m_engine;
};

}

#endif
