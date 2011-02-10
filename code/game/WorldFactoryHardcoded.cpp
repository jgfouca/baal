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
  /*
    This is the world that will be generated:

    P P M M P O
    P P M P P O
    P M P P O O
    M M P P O O
    P P P O O O
    O O O O O O
   */
  World* world = new World(6, 6);

  world->get_tile(Location(0, 0)) = WorldTile(PLAIN);
  world->get_tile(Location(0, 1)) = WorldTile(PLAIN);
  world->get_tile(Location(0, 2)) = WorldTile(MTN);
  world->get_tile(Location(0, 3)) = WorldTile(MTN);
  world->get_tile(Location(0, 4)) = WorldTile(PLAIN);
  world->get_tile(Location(0, 5)) = WorldTile(OCEAN);

  world->get_tile(Location(1, 0)) = WorldTile(PLAIN);
  world->get_tile(Location(1, 1)) = WorldTile(PLAIN);
  world->get_tile(Location(1, 2)) = WorldTile(MTN);
  world->get_tile(Location(1, 3)) = WorldTile(PLAIN);
  world->get_tile(Location(1, 4)) = WorldTile(PLAIN);
  world->get_tile(Location(1, 5)) = WorldTile(OCEAN);

  world->get_tile(Location(2, 0)) = WorldTile(PLAIN);
  world->get_tile(Location(2, 1)) = WorldTile(MTN);
  world->get_tile(Location(2, 2)) = WorldTile(PLAIN);
  world->get_tile(Location(2, 3)) = WorldTile(PLAIN);
  world->get_tile(Location(2, 4)) = WorldTile(OCEAN);
  world->get_tile(Location(2, 5)) = WorldTile(OCEAN);

  world->get_tile(Location(3, 0)) = WorldTile(MTN);
  world->get_tile(Location(3, 1)) = WorldTile(MTN);
  world->get_tile(Location(3, 2)) = WorldTile(PLAIN);
  world->get_tile(Location(3, 3)) = WorldTile(PLAIN);
  world->get_tile(Location(3, 4)) = WorldTile(OCEAN);
  world->get_tile(Location(3, 5)) = WorldTile(OCEAN);

  world->get_tile(Location(4, 0)) = WorldTile(PLAIN);
  world->get_tile(Location(4, 1)) = WorldTile(PLAIN);
  world->get_tile(Location(4, 2)) = WorldTile(PLAIN);
  world->get_tile(Location(4, 3)) = WorldTile(OCEAN);
  world->get_tile(Location(4, 4)) = WorldTile(OCEAN);
  world->get_tile(Location(4, 5)) = WorldTile(OCEAN);

  world->get_tile(Location(5, 0)) = WorldTile(OCEAN);
  world->get_tile(Location(5, 1)) = WorldTile(OCEAN);
  world->get_tile(Location(5, 2)) = WorldTile(OCEAN);
  world->get_tile(Location(5, 3)) = WorldTile(OCEAN);
  world->get_tile(Location(5, 4)) = WorldTile(OCEAN);
  world->get_tile(Location(5, 5)) = WorldTile(OCEAN);

  return world;
}
