#ifndef WorldFactory_hpp
#define WorldFactory_hpp

#include <string>

namespace baal {

class World;
class Engine;

class WorldFactory
{
 public:
  // Note, client is responsible for deletion
  static World& create(Engine& engine);

  static const std::string GENERATED_WORLD;
  static const std::string DEFAULT_WORLD;
};

}

#endif
