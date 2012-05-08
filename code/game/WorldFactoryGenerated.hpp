#ifndef WorldFactoryGenerated_hpp
#define WorldFactoryGenerated_hpp

#include <memory>

namespace baal {

class World;
class Engine;

class WorldFactoryGenerated
{
 public:
  static std::shared_ptr<World> create(Engine& engine);
};

}

#endif
