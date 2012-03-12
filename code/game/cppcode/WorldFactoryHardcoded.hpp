#ifndef WorldFactoryHardcoded_hpp
#define WorldFactoryHardcoded_hpp

#include <string>

namespace baal {

class World;
class Engine;

class WorldFactoryHardcoded
{
 public:
  static World& create(const std::string& world_config, Engine& engine);

  static const unsigned NUM_HARDCODED_WORLDS = 1;

 private:
  static World& generate_world_1(Engine& engine);
};

}

#endif
