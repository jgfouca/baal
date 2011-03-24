#include "WorldFactoryHardcoded.hpp"
#include "World.hpp"
#include "BaalExceptions.hpp"
#include "Weather.hpp"
#include "Geology.hpp"
#include "City.hpp"

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
    P L L O O O
    O O O O O O
   */
  World& world = *(new World(6, 6));

  world.m_tiles[0][0] = new TundraTile(*new Climate(30, 10, Wind(10, WSW)),
                                       *new Inactive);
  world.m_tiles[0][1] = new PlainsTile(*new Climate(40, 12, Wind(10, WSW)),
                                       *new Inactive);
  world.m_tiles[0][2] = new HillsTile(*new Climate(35, 14, Wind(15, WSW)),
                                      *new Inactive);
  world.m_tiles[0][3] = new MountainTile(5000,
                                         *new Climate(20, 40, Wind(25, WSW)),
                                         *new Subducting(2.0));
  world.m_tiles[0][4] = new LushTile(*new Climate(60, 30, Wind(10, WSW)),
                                     *new Subducting(2.0));
  world.m_tiles[0][5] = new OceanTile(1000,
                                      *new Climate(70, 35, Wind(10, SW)),
                                      *new Subducting(2.0));


  world.m_tiles[1][0] = new DesertTile(*new Climate(50, 10, Wind(10, SW)),
                                       *new Inactive);
  world.m_tiles[1][1] = new DesertTile(*new Climate(50, 10, Wind(10, SW)),
                                       *new Inactive);
  world.m_tiles[1][2] = new MountainTile(5000,
                                         *new Climate(20, 40, Wind(25, SW)),
                                         *new Inactive);
  world.m_tiles[1][3] = new HillsTile(*new Climate(40, 40, Wind(15, SW)),
                                      *new Subducting(2.0));
  world.m_tiles[1][4] = new LushTile(*new Climate(62, 30, Wind(10, SW)),
                                     *new Subducting(2.0));
  world.m_tiles[1][5] = new OceanTile(1000,
                                      *new Climate(72, 35, Wind(10, SSW)),
                                      *new Subducting(2.0));


  world.m_tiles[2][0] = new DesertTile(*new Climate(55, 10, Wind(10, S)),
                                       *new Inactive);
  world.m_tiles[2][1] = new MountainTile(5000,
                                         *new Climate(25, 45, Wind(25, SSW)),
                                         *new Inactive);
  world.m_tiles[2][2] = new HillsTile(*new Climate(45, 40, Wind(15, SSW)),
                                      *new Subducting(3.0));
  world.m_tiles[2][3] = new LushTile(*new Climate(65, 35, Wind(10, SSW)),
                                     *new Subducting(3.0));
  world.m_tiles[2][4] = new OceanTile(1000,
                                      *new Climate(75, 35, Wind(10, S)),
                                      *new Subducting(3.0));
  world.m_tiles[2][5] = new OceanTile(1000,
                                      *new Climate(75, 40, Wind(10, S)),
                                      *new Inactive);

  world.m_tiles[3][0] = new HillsTile(*new Climate(55, 15, Wind(15, S)),
                                      *new Inactive);
  world.m_tiles[3][1] = new MountainTile(5000,
                                         *new Climate(30, 50, Wind(25, S)),
                                         *new Inactive);
  world.m_tiles[3][2] = new LushTile(*new Climate(70, 40, Wind(10, S)),
                                     *new Subducting(2.0));
  world.m_tiles[3][3] = new LushTile(*new Climate(70, 40, Wind(10, S)),
                                     *new Subducting(2.0));
  world.m_tiles[3][4] = new OceanTile(1000,
                                      *new Climate(80, 45, Wind(10, SSE)),
                                      *new Subducting(2.0));
  world.m_tiles[3][5] = new OceanTile(1000,
                                      *new Climate(80, 45, Wind(10, SSE)),
                                      *new Inactive);

  world.m_tiles[4][0] = new PlainsTile(*new Climate(70, 20, Wind(10, SSE)),
                                       *new Transform(2.0));
  world.m_tiles[4][1] = new LushTile(*new Climate(70, 30, Wind(10, SSE)),
                                     *new Transform(2.0));
  world.m_tiles[4][2] = new LushTile(*new Climate(70, 30, Wind(10, SSE)),
                                     *new Transform(2.0));
  dynamic_cast<LandTile*>(world.m_tiles[4][2])->place_city(*new City("Capital"));
  world.m_tiles[4][3] = new OceanTile(1000,
                                      *new Climate(80, 45, Wind(10, SE)),
                                      *new Subducting(1.0));
  world.m_tiles[4][4] = new OceanTile(1000,
                                      *new Climate(80, 45, Wind(10, SE)),
                                      *new Inactive);
  world.m_tiles[4][5] = new OceanTile(1000,
                                      *new Climate(80, 45, Wind(10, SE)),
                                      *new Inactive);

  world.m_tiles[5][0] = new OceanTile(1000,
                                      *new Climate(85, 45, Wind(10, ESE)),
                                      *new Inactive);
  world.m_tiles[5][1] = new OceanTile(1000,
                                      *new Climate(85, 45, Wind(10, ESE)),
                                      *new Inactive);
  world.m_tiles[5][2] = new OceanTile(1000,
                                      *new Climate(85, 45, Wind(10, ESE)),
                                      *new Inactive);
  world.m_tiles[5][3] = new OceanTile(1000,
                                      *new Climate(85, 45, Wind(10, ESE)),
                                      *new Inactive);
  world.m_tiles[5][4] = new OceanTile(1000,
                                      *new Climate(85, 45, Wind(10, ESE)),
                                      *new Inactive);
  world.m_tiles[5][5] = new OceanTile(1000,
                                      *new Climate(85, 45, Wind(10, ESE)),
                                      *new Inactive);

  return world;
}
