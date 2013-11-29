#include "WorldFactoryHardcoded.hpp"
#include "World.hpp"
#include "BaalExceptions.hpp"
#include "Weather.hpp"
#include "Geology.hpp"
#include "City.hpp"

#include <sstream>

namespace baal {

///////////////////////////////////////////////////////////////////////////////
std::shared_ptr<World> WorldFactoryHardcoded::create(const std::string& world_config,
                                                     Engine& engine)
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
    return generate_world_1(engine);
  }
  else {
    Require(false, "Should never make it here");
  }
}

#define MT(A, B, C, D) std::vector<int> {A, B, C, D}
#define MP(A, B, C, D) std::vector<float>{A, B, C, D}
#define MW(A) std::vector<Wind>(4, A)

///////////////////////////////////////////////////////////////////////////////
std::shared_ptr<World> WorldFactoryHardcoded::generate_world_1(Engine& engine)
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
  static const unsigned ROW = 6;
  static const unsigned COL = 6;

  LocationIterator<ROW, COL> loc_itr;

  auto world = std::shared_ptr<World>(new World(ROW, COL, engine));

  world->m_tiles[0][0] = new TundraTile(*loc_itr++,
                                       *new Climate(MT(10, 30, 50, 30),
                                                    MP(4, 2, .5, 2),
                                                    MW(Wind(10, WSW))),
                                       *new Inactive);
  world->m_tiles[0][1] = new PlainsTile(*loc_itr++,
                                       *new Climate(MT(20, 40, 60, 40),
                                                    MP(5, 2.5, 1, 2.5),
                                                    MW(Wind(10, WSW))),
                                       *new Inactive);
  world->m_tiles[0][2] = new HillsTile(*loc_itr++,
                                      *new Climate(MT(15, 35, 50, 35),
                                                   MP(6, 3.5, 2, 3.5),
                                                   MW(Wind(15, WSW))),
                                      *new Inactive);
  world->m_tiles[0][3] = new MountainTile(*loc_itr++,
                                         5000,
                                         *new Climate(MT(10, 25, 40, 25),
                                                      MP(12, 7, 8, 7),
                                                      MW(Wind(25, WSW))),
                                         *new Subducting(2.0));
  world->m_tiles[0][4] = new LushTile(*loc_itr++,
                                     *new Climate(MT(50, 60, 70, 60),
                                                  MP(8, 8, 8, 8),
                                                  MW(Wind(10, WSW))),
                                     *new Subducting(2.0));
  world->m_tiles[0][5] = new OceanTile(*loc_itr++,
                                      1000,
                                      *new Climate(MT(65, 70, 75, 70),
                                                   MP(9, 9, 9, 9),
                                                   MW(Wind(10, SW))),
                                      *new Subducting(2.0));


  world->m_tiles[1][0] = new DesertTile(*loc_itr++,
                                       *new Climate(MT(25, 50, 75, 50),
                                                    MP(4, 1.5, 1, 1.5),
                                                    MW(Wind(10, SW))),
                                       *new Inactive);
  world->m_tiles[1][1] = new DesertTile(*loc_itr++,
                                       *new Climate(MT(30, 55, 80, 55),
                                                    MP(4, 1.5, 1, 1.5),
                                                    MW(Wind(10, SW))),
                                       *new Inactive);
  world->m_tiles[1][2] = new MountainTile(*loc_itr++,
                                         5000,
                                         *new Climate(MT(12, 27, 42, 27),
                                                      MP(12, 7, 8, 7),
                                                      MW(Wind(25, SW))),
                                         *new Inactive);
  world->m_tiles[1][3] = new HillsTile(*loc_itr++,
                                      *new Climate(MT(40, 55, 70, 55),
                                                   MP(10, 10, 10, 10),
                                                   MW(Wind(15, SW))),
                                      *new Subducting(2.0));
  world->m_tiles[1][4] = new LushTile(*loc_itr++,
                                     *new Climate(MT(52, 62, 72, 62),
                                                  MP(8, 8, 8, 8),
                                                  MW(Wind(10, SW))),
                                     *new Subducting(2.0));
  world->m_tiles[1][5] = new OceanTile(*loc_itr++,
                                      1000,
                                      *new Climate(MT(67, 72, 77, 72),
                                                   MP(9, 9, 9, 9),
                                                   MW(Wind(10, SSW))),
                                      *new Subducting(2.0));


  world->m_tiles[2][0] = new DesertTile(*loc_itr++,
                                       *new Climate(MT(30, 55, 80, 55),
                                                    MP(4, 1.5, 1, 1.5),
                                                    MW(Wind(10, S))),
                                       *new Inactive);
  world->m_tiles[2][1] = new MountainTile(*loc_itr++,
                                         5000,
                                         *new Climate(MT(14, 29, 44, 29),
                                                      MP(13, 8, 10, 8),
                                                      MW(Wind(25, SSW))),
                                         *new Inactive);
  world->m_tiles[2][2] = new HillsTile(*loc_itr++,
                                      *new Climate(MT(42, 57, 72, 57),
                                                   MP(11, 11, 11, 11),
                                                   MW(Wind(15, SSW))),
                                      *new Subducting(3.0));
  world->m_tiles[2][3] = new LushTile(*loc_itr++,
                                     *new Climate(MT(55, 65, 75, 65),
                                                  MP(9, 9, 9, 9),
                                                  MW(Wind(10, SSW))),
                                     *new Subducting(3.0));
  world->m_tiles[2][4] = new OceanTile(*loc_itr++,
                                      1000,
                                      *new Climate(MT(70, 75, 80, 75),
                                                   MP(10, 10, 10, 10),
                                                   MW(Wind(10, S))),
                                      *new Subducting(3.0));
  world->m_tiles[2][5] = new OceanTile(*loc_itr++,
                                      1000,
                                      *new Climate(MT(70, 75, 80, 75),
                                                   MP(10, 10, 10, 10),
                                                   MW(Wind(10, S))),
                                      *new Inactive);

  world->m_tiles[3][0] = new HillsTile(*loc_itr++,
                                      *new Climate(MT(30, 50, 65, 50),
                                                   MP(4, 4, 4, 4),
                                                   MW(Wind(15, S))),
                                      *new Inactive);
  world->m_tiles[3][1] = new MountainTile(*loc_itr++,
                                         5000,
                                         *new Climate(MT(18, 33, 48, 33),
                                                      MP(10, 9, 13, 9),
                                                      MW(Wind(25, S))),
                                         *new Inactive);
  world->m_tiles[3][2] = new LushTile(*loc_itr++,
                                     *new Climate(MT(60, 70, 80, 70),
                                                  MP(8, 10, 12, 10),
                                                  MW(Wind(10, S))),
                                     *new Subducting(2.0));
  world->m_tiles[3][3] = new LushTile(*loc_itr++,
                                     *new Climate(MT(60, 70, 80, 70),
                                                  MP(8, 10, 12, 10),
                                                  MW(Wind(10, S))),
                                     *new Subducting(2.0));
  world->m_tiles[3][4] = new OceanTile(*loc_itr++,
                                      1000,
                                      *new Climate(MT(75, 80, 85, 80),
                                                   MP(11, 11, 11, 11),
                                                   MW(Wind(10, SSE))),
                                      *new Subducting(2.0));
  world->m_tiles[3][5] = new OceanTile(*loc_itr++,
                                      1000,
                                       *new Climate(MT(75, 80, 85, 80),
                                                    MP(11, 11, 11, 11),
                                                    MW(Wind(10, SSE))),
                                      *new Inactive);

  world->m_tiles[4][0] = new PlainsTile(*loc_itr++,
                                       *new Climate(MT(40, 70, 90, 70),
                                                    MP(3, 4, 8, 4),
                                                    MW(Wind(10, SSE))),
                                       *new Transform(2.0));
  world->m_tiles[4][1] = new LushTile(*loc_itr++,
                                     *new Climate(MT(57, 67, 77, 67),
                                                  MP(6, 8, 16, 8),
                                                  MW(Wind(10, SSE))),
                                     *new Transform(2.0));
  world->m_tiles[4][2] = new LushTile(*loc_itr++,
                                     *new Climate(MT(59, 69, 79, 69),
                                                  MP(8, 10, 16, 8),
                                                  MW(Wind(10, SSE))),
                                     *new Transform(2.0));
  world->place_city("Capital", Location(4, 2)); // city
  world->m_tiles[4][3] = new OceanTile(*loc_itr++,
                                      1000,
                                       *new Climate(MT(75, 80, 85, 80),
                                                    MP(12, 12, 12, 12),
                                                    MW(Wind(10, SE))),
                                      *new Subducting(1.0));
  world->m_tiles[4][4] = new OceanTile(*loc_itr++,
                                      1000,
                                      *new Climate(MT(75, 80, 85, 80),
                                                   MP(12, 12, 12, 12),
                                                   MW(Wind(10, SE))),
                                      *new Inactive);
  world->m_tiles[4][5] = new OceanTile(*loc_itr++,
                                      1000,
                                      *new Climate(MT(75, 80, 85, 80),
                                                   MP(12, 12, 12, 12),
                                                   MW(Wind(10, SE))),
                                      *new Inactive);

  world->m_tiles[5][0] = new OceanTile(*loc_itr++,
                                      1000,
                                      *new Climate(MT(80, 85, 90, 85),
                                                   MP(12, 12, 12, 12),
                                                   MW(Wind(10, ESE))),
                                      *new Inactive);
  world->m_tiles[5][1] = new OceanTile(*loc_itr++,
                                      1000,
                                      *new Climate(MT(80, 85, 90, 85),
                                                   MP(12, 12, 12, 12),
                                                   MW(Wind(10, ESE))),
                                      *new Inactive);
  world->m_tiles[5][2] = new OceanTile(*loc_itr++,
                                      1000,
                                      *new Climate(MT(80, 85, 90, 85),
                                                   MP(12, 12, 12, 12),
                                                   MW(Wind(10, ESE))),
                                      *new Inactive);
  world->m_tiles[5][3] = new OceanTile(*loc_itr++,
                                      1000,
                                      *new Climate(MT(80, 85, 90, 85),
                                                   MP(12, 12, 12, 12),
                                                   MW(Wind(10, ESE))),
                                      *new Inactive);
  world->m_tiles[5][4] = new OceanTile(*loc_itr++,
                                      1000,
                                      *new Climate(MT(80, 85, 90, 85),
                                                   MP(12, 12, 12, 12),
                                                   MW(Wind(10, ESE))),
                                      *new Inactive);
  world->m_tiles[5][5] = new OceanTile(*loc_itr,
                                      1000,
                                      *new Climate(MT(80, 85, 90, 85),
                                                   MP(12, 12, 12, 12),
                                                   MW(Wind(10, ESE))),
                                      *new Inactive);

  return world;
}

#undef MT
#undef MP
#undef MW

}
