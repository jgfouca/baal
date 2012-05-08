#include "WorldFactory.hpp"
#include "WorldFactoryHardcoded.hpp"
#include "WorldFactoryGenerated.hpp"
#include "WorldFactoryFromFile.hpp"
#include "Configuration.hpp"
#include "BaalExceptions.hpp"
#include "Engine.hpp"

#include <cstdlib>

namespace baal {

const std::string WorldFactory::GENERATED_WORLD  = "g";
const std::string WorldFactory::DEFAULT_WORLD = "1";

///////////////////////////////////////////////////////////////////////////////
std::shared_ptr<World> WorldFactory::create(Engine& engine)
///////////////////////////////////////////////////////////////////////////////
{
  // Get user's choice of world
  const Configuration& config = engine.config();
  std::string world_config = config.get_world_config();
  if (world_config == "") {
    world_config = DEFAULT_WORLD;
  }

  // Parse world config, if it's numeric, the user is requesting a hardcoded
  // world.
  bool numeric = true;
  for (unsigned i = 0; i < world_config.size() && numeric; ++i) {
    if (!isdigit(world_config[i])) {
      numeric = false;
    }
  }

  // Create and return the desired world
  if (numeric) {
    return WorldFactoryHardcoded::create(world_config, engine);
  }
  else if (world_config == GENERATED_WORLD) {
    return WorldFactoryGenerated::create(engine);
  }
  else if (is_baal_map_file(world_config)) {
    return WorldFactoryFromFile::create(world_config, engine);
  }
  else {
    RequireUser(false, "Invalid choice of world: " << world_config);
  }
}

}
