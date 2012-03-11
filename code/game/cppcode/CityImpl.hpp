#ifndef CityImpl_hpp
#define CityImpl_hpp

#include "BaalCommon.hpp"

#include <string>

#include <libxml/parser.h>


namespace baal {

class LandTile;
class WorldTile;

namespace details { // Clients, stay away!

/**
 * Implementation for City class. Nothing is private in this class
 * in order to make unit-testing easier.
 */
class CityImpl
{
 public:

  //
  // ==== Public API ====
  //

  CityImpl(const std::string& name, Location location);

  CityImpl & operator=(const CityImpl&) = delete;
  CityImpl(const CityImpl&)             = delete;
  CityImpl()                            = delete;

  //
  // Query / Getter API
  //

  const std::string& name() const { return m_name; }

  unsigned population() const { return m_population; }

  unsigned rank() const { return m_rank; }

  Location location() const { return m_location; }

  bool famine() const { return m_famine; }

  unsigned defense() const { return m_defense_level; }

  xmlNodePtr to_xml() const;

  //
  // Modification API
  //

  /**
   * Tell this city that the turn is cycling
   */
  void cycle_turn();

  /**
   * Kill off some of this city's citizens
   */
  void kill(unsigned killed);

  /**
   * Destroy city defenses, lowering them by an amount equal to the
   * 'levels' parameter.
   */
  void destroy_defense(unsigned levels);

  //
  // ==== Internal methods ====
  //

  typedef std::pair<std::vector<WorldTile*>, std::vector<WorldTile*> > tile_vec_pair;
  typedef std::pair<float, float> resource_pair;

  enum ActionId {
    BUILD_INFRA,
    BUILD_SETTLER,
    BUILD_DEFENSE,
    NO_ACTION
  };

  struct Action {
    Action(ActionId action_id, LandTile* tile = nullptr) :
      m_action_id(action_id),
      m_affected_tile(tile),
      m_location()
    {}

    Action(ActionId action_id, Location location) :
      m_action_id(action_id),
      m_affected_tile(nullptr),
      m_location(location)
    {}

    Action() :
      m_action_id(NO_ACTION),
      m_affected_tile(nullptr),
      m_location()
    {}

    ActionId   m_action_id;
    LandTile*  m_affected_tile;
    Location   m_location;
  };

  //
  // Methods with side-effects (try to limit the number of these).
  //

  /**
   * Add a level of infrastructure to a tile.
   */
  bool build_infra(LandTile& land_tile);

  /**
   * Assign citizens based on recommendations.
   *
   * Return resources gathered.
   */
  resource_pair assign_citizens(const std::vector<WorldTile*>& work_food_tiles,
                                const std::vector<WorldTile*>& work_prod_tiles);

  /**
   * Give the people food. The population will change based on how much food
   * was provided.
   */
  void feed_people(float food);

  /**
   * Try to build something.
   *
   * Returns true if the item was built.
   */
  bool produce_item(Action action);

  //
  // Side-effect free methods (maximize use of these, they are easier
  // to unit-test)
  //

  /**
   * Examine harvestable nearby tiles, returning two sorted (best->worst)
   * vectors of tiles. The first for food, second for production.
   */
  tile_vec_pair examine_workable_tiles() const;

  /**
   * Compute recommended allocation of available citizens to various
   * roles like working tiles and serving as specialist.
   *
   * Returns a pair of vectors, first representing food_tiles that should
   * be worked, second representing production tiles that should be worked.
   * All remaining workers are assumed to be specialists.
   */
  tile_vec_pair get_citizen_recommendation(const std::vector<WorldTile*>& food_tiles,
                                           const std::vector<WorldTile*>& prod_tiles) const;

  Action get_recommended_production(const std::vector<WorldTile*>& food_tiles,
                                    const std::vector<WorldTile*>& prod_tiles,
                                    const std::vector<WorldTile*>& worked_food_tiles,
                                    const std::vector<WorldTile*>& worked_prod_tiles,
                                    float food_gathered,
                                    float prod_gathered);

  /**
   * How much food is required to avoid starvation.
   */
  float get_required_food() const
  {
    return static_cast<float>(m_population) / POP_THAT_EATS_ONE_FOOD;
  }


  //
  // ==== Members ====
  //

  std::string m_name;
  unsigned    m_rank;
  unsigned    m_population;
  unsigned    m_next_rank_pop;
  float       m_production;
  Location    m_location;
  unsigned    m_defense_level;
  bool        m_famine;

  //
  // ==== Class constants ====
  //

  // City growth/food/production contants
  static constexpr float CITY_BASE_GROWTH_RATE = 0.01; // 1% per turn
  static constexpr float MAX_GROWTH_MODIFIER   = 4.0;
  static constexpr unsigned CITY_RANK_UP_MULTIPLIER = 2;
  static constexpr unsigned CITY_STARTING_POP = 1000;
  static constexpr unsigned POP_THAT_EATS_ONE_FOOD = 1000;
  static constexpr float FOOD_FROM_CITY_CENTER = 1.0;
  static constexpr float PROD_FROM_CITY_CENTER = 1.0;
  static constexpr float PROD_FROM_SPECIALIST  = 1.0;

  // Constants for production costs of various buildable items
  static constexpr unsigned SETTLER_PROD_COST  = 200;
  static constexpr unsigned INFRA_PROD_COST    = 50;
  static constexpr unsigned CITY_DEF_PROD_COST = 400;

  // AI constants
  static constexpr float TOO_MANY_FOOD_WORKERS = 0.66;
  static constexpr float PROD_BEFORE_SETTLER   = 7.0;
};

//
// Free functions. Normally, these would be empty-namespace functions in
// the .cpp, but again, we publicize these for unit-testing.
//

bool is_within_distance_of_any_city(Location location, int distance);

float compute_city_loc_heuristic(Location location);

}
}

#endif
