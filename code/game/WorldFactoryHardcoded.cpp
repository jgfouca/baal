#include "WorldFactoryHardcoded.hpp"
#include "World.hpp"
#include "BaalExceptions.hpp"

#include <sstream>

using namespace baal;

///////////////////////////////////////////////////////////////////////////////
World& WorldFactoryHardcoded::create(const std::string& world_config)
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
World& WorldFactoryHardcoded::generate_world_1()
///////////////////////////////////////////////////////////////////////////////
{
  /*
    This is the world that will be generated:

    T P H M L O
    D D M H L O
    D M H L O O
    H M L L O O
    P P L O O O
    O O O O O O
   */
  World& world = *(new World(6, 6));

  world.m_tiles[0][0] = new TundraTile();
  world.m_tiles[0][1] = new PlainsTile();
  world.m_tiles[0][2] = new HillsTile();
  world.m_tiles[0][3] = new MountainTile(5000);
  world.m_tiles[0][4] = new LushTile();
  world.m_tiles[0][5] = new OceanTile(1000);

  world.m_tiles[1][0] = new DesertTile();
  world.m_tiles[1][1] = new DesertTile();
  world.m_tiles[1][2] = new MountainTile(5000);
  world.m_tiles[1][3] = new HillsTile();
  world.m_tiles[1][4] = new LushTile();
  world.m_tiles[1][5] = new OceanTile(1000);

  world.m_tiles[2][0] = new DesertTile();
  world.m_tiles[2][1] = new MountainTile(5000);
  world.m_tiles[2][2] = new HillsTile();
  world.m_tiles[2][3] = new LushTile();
  world.m_tiles[2][4] = new OceanTile(1000);
  world.m_tiles[2][5] = new OceanTile(1000);

  world.m_tiles[3][0] = new HillsTile();
  world.m_tiles[3][1] = new MountainTile(5000);
  world.m_tiles[3][2] = new LushTile();
  world.m_tiles[3][3] = new LushTile();
  world.m_tiles[3][4] = new OceanTile(1000);
  world.m_tiles[3][5] = new OceanTile(1000);

  world.m_tiles[4][0] = new PlainsTile();
  world.m_tiles[4][1] = new LushTile();
  world.m_tiles[4][2] = new LushTile();
  world.m_tiles[4][3] = new OceanTile(1000);
  world.m_tiles[4][4] = new OceanTile(1000);
  world.m_tiles[4][5] = new OceanTile(1000);

  world.m_tiles[5][0] = new OceanTile(1000);
  world.m_tiles[5][1] = new OceanTile(1000);
  world.m_tiles[5][2] = new OceanTile(1000);
  world.m_tiles[5][3] = new OceanTile(1000);
  world.m_tiles[5][4] = new OceanTile(1000);
  world.m_tiles[5][5] = new OceanTile(1000);

  return world;
}
