#ifndef WorldFactoryHardcoded_hpp
#define WorldFactoryHardcoded_hpp

#include <string>
#include <memory>

namespace baal {

class World;
class Engine;

class WorldFactoryHardcoded
{
 public:
  static std::shared_ptr<World> create(const std::string& world_config, Engine& engine);

  static const unsigned NUM_HARDCODED_WORLDS = 1;

 private:
  static std::shared_ptr<World> generate_world_1(Engine& engine);
};

}

#endif
