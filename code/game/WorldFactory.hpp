#ifndef WorldFactory_hpp
#define WorldFactory_hpp

#include <string>

namespace baal {

class World;

class WorldFactory
{
 public:
  // Note, client is responsible for deletion
  static World& create();
  static World& load_from_file();

  static const std::string GENERATED_WORLD;
  static const std::string DEFAULT_WORLD;
};

}

#endif
