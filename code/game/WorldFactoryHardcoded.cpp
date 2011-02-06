#include "WorldFactoryHardcoded.hpp"
#include "World.hpp"
#include "BaalExceptions.hpp"

#include <sstream>

using namespace baal;

///////////////////////////////////////////////////////////////////////////////
World* WorldFactoryHardcoded::create(const std::string& world_config)
///////////////////////////////////////////////////////////////////////////////
{
  // Convert to int
  unsigned world_id;
  std::istringstream(world_config) >> world_id;

  // Check validity
  RequireUser(world_id > 0 && world_id <= NUM_HARDCODED_WORLDS,
              "Hardcoded world choice " << world_id <<
              " is out of bounds, choose 1-" << NUM_HARDCODED_WORLDS);

  if (world_id == 1) {
    return generate_world_1();
  }
  else {
    Require(false, "Should never make it here");
  }
}

///////////////////////////////////////////////////////////////////////////////
World* WorldFactoryHardcoded::generate_world_1()
///////////////////////////////////////////////////////////////////////////////
{
  // TODO
  return new World;
}
