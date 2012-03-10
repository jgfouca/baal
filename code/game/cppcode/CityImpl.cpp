#include "CityImpl.hpp"
#include "City.hpp"
#include "BaalExceptions.hpp"
#include "World.hpp"
#include "Engine.hpp"
#include "PlayerAI.hpp"

#include <cstdlib>
#include <cmath>
#include <list>
#include <vector>
#include <algorithm>
#include <iostream>

namespace baal {
namespace details {

struct FoodGetter
{
  float operator()(const WorldTile* tile) const
  {
    return tile->yield().m_food;
  }
};

struct ProdGetter
{
  float operator()(const WorldTile* tile) const
  {
    return tile->yield().m_prod;
  }
};

template <class Getter>
struct ReverseOrd
{
  bool operator()(const WorldTile* lhs, const WorldTile* rhs)
  {
    Getter getter;
    return getter(lhs) > getter(rhs);
  }
};

struct AcceptAll
{
  bool operator()(const WorldTile* tile) const
  {
    return true;
  }
};

struct FilterTooCloseToOtherCities
{
  FilterTooCloseToOtherCities(int distance) : m_distance(distance) {}

  bool operator()(const WorldTile& tile) const
  {
    return !is_within_distance_of_any_city(tile.location(), m_distance);
  }

  int m_distance;
};

struct FilterAlreadyWorked
{
  bool operator()(const WorldTile& tile) const
  {
    return !tile.worked();
  }
};

///////////////////////////////////////////////////////////////////////////////
bool is_within_distance_of_any_city(const Location& location, int distance)
///////////////////////////////////////////////////////////////////////////////
{
  for (City* city : Engine::instance().world().cities()) {
    const Location& city_loc = city->location();
    if (std::abs(location.row - city_loc.row) <= distance &&
        std::abs(location.col - city_loc.col) <= distance) {
      return true;
    }
  }
  return false;
}

///////////////////////////////////////////////////////////////////////////////
template <typename Range, class Filter>
std::pair<std::vector<WorldTile*>, std::vector<WorldTile*> >
compute_nearby_food_and_prod_tiles(Range location_range,
                                   Filter filter = AcceptAll())
///////////////////////////////////////////////////////////////////////////////
{
  // Returns a 2-ple of (food-tiles, production-tiles); both lists are sorted
  // from highest to lowest based on yield.

  World& world = Engine::instance().world();
  std::vector<WorldTile*> food_tiles, prod_tiles;
  static const unsigned NUM_TILES_SURROUNDING_CITY = 8;
  food_tiles.reserve(NUM_TILES_SURROUNDING_CITY);
  prod_tiles.reserve(NUM_TILES_SURROUNDING_CITY);

  for (Location location : location_range) {
    WorldTile& tile = world.get_tile(location);
    if (filter(tile)) {
      if (tile.yield().m_food > 0) {
        food_tiles.push_back(&tile);
      }
      else {
        prod_tiles.push_back(&tile);
      }
    }
  }

  std::sort(std::begin(food_tiles), std::end(food_tiles), ReverseOrd<FoodGetter>());
  std::sort(std::begin(prod_tiles), std::end(prod_tiles), ReverseOrd<ProdGetter>());

  // This would be expensive in old C++, but move semantics should make this
  // fast.
  return std::make_pair(food_tiles, prod_tiles);
}

///////////////////////////////////////////////////////////////////////////////
float compute_city_loc_heuristic(const Location& location)
///////////////////////////////////////////////////////////////////////////////
{
  const int min_distance = 1;
  auto tile_pair = compute_nearby_food_and_prod_tiles(get_adjacent_location_range(location),
                                                      FilterTooCloseToOtherCities(min_distance));
  float available_food = 0.0, available_prod = 0.0;

  for (WorldTile* tile : tile_pair.first) {
    available_food += tile->yield().m_food;
  }

  for (WorldTile* tile : tile_pair.second) {
    available_prod += tile->yield().m_prod;
  }

  // Favor city locations with a good balance of food and production
  return available_food * available_prod;
}

///////////////////////////////////////////////////////////////////////////////
CityImpl::CityImpl(const std::string& name, const Location& location)
///////////////////////////////////////////////////////////////////////////////
  : m_name(name),
    m_rank(1),
    m_population(CITY_STARTING_POP),
    m_next_rank_pop(CITY_STARTING_POP * CITY_RANK_UP_MULTIPLIER),
    m_production(0.0),
    m_location(location),
    m_defense_level(1),
    m_famine(false)
{}

///////////////////////////////////////////////////////////////////////////////
CityImpl::tile_vec_pair CityImpl::examine_workable_tiles() const
///////////////////////////////////////////////////////////////////////////////
{
  return compute_nearby_food_and_prod_tiles(get_adjacent_location_range(m_location),
                                            FilterAlreadyWorked());
}

///////////////////////////////////////////////////////////////////////////////
void CityImpl::cycle_turn()
///////////////////////////////////////////////////////////////////////////////
{
  Require(m_population > 0,
          "This city has no people and should have been deleted");

  Engine& engine = Engine::instance();
  World& world   = engine.world();

  // Gather resources based on nearby worked tiles. At this time, cities will
  // only be able to harvest adjacent tiles.

  // Evaluate nearby tiles, put in to sorted lists (best-to-worst) for each
  // of the two yield types
  tile_vec_pair tile_pair = examine_workable_tiles();
  std::vector<WorldTile*>& food_tiles = tile_pair.first;
  std::vector<WorldTile*>& prod_tiles = tile_pair.second;

  // Choose which tiles to work. We first make sure we have ample food, then
  // we look for production tiles.

  float req_food = static_cast<float>(m_population) / POP_THAT_EATS_ONE_FOOD;
  float food_gathered = FOOD_FROM_CITY_CENTER;
  float prod_gathered = PROD_FROM_CITY_CENTER;
  unsigned num_workers = m_rank;
  unsigned num_workers_used_for_food = 0;
  for (WorldTile* food_tile : food_tiles) {
    if (food_gathered < req_food) {
      food_tile->work();
      --num_workers;
      ++num_workers_used_for_food;
      food_gathered += food_tile->yield().m_food;
    }
    else {
      // TODO: AI should continue to pile up food to increase it's growth
      // rate unless it's already hit the max growth rate or if there are
      // good production tiles available or it really needs production.
      break;
    }
  }

  // TODO: If city is getting close to being food capped, do not sacrafice
  // good production tiles for marginal food tiles.
  for (WorldTile* prod_tile : prod_tiles) {
    if (prod_tile->yield().m_prod > PROD_FROM_SPECIALIST) {
      prod_tile->work();
      --num_workers;
      prod_gathered += prod_tile->yield().m_prod;
    }
  }

  // Remaining workers are specialists that contribute production
  prod_gathered +=
    num_workers * PROD_FROM_SPECIALIST * engine.ai_player().tech_yield_multiplier();

  // Accumulate production
  m_production += prod_gathered;

  // Decide on how to spend production. Options: City fortifications,
  // tile infrastructure, or settler. In the current system, the AI builds
  // general production points and it can use them to insta-buy the things
  // it wants. This is a bit unrealistic and should probably be replaced
  // with a system where the AI chooses to start building something and
  // all production points go to that thing until it is finished (a la civ).

  // First, we need to decide what production item we should be saving
  // up for. Note that the AI will *not* build a cheaper item that is of
  // lower priority if it cannot afford the highest priority item. It will
  // always save-up for the highest priority.

  // We want some of our workers doing something other than just
  // collecting food. If we are having to dedicate our workforce to
  // food, we probably need better food infrastructure. We need to verify
  // that there are nearby food tiles that we can enhance.
  bool build_food_infra = false;
  FoodTile* food_tile = NULL;
  {
    float pct_workers_on_food =
      static_cast<float>(num_workers_used_for_food) / m_rank;
    if (pct_workers_on_food > TOO_MANY_FOOD_WORKERS ||
        food_gathered < req_food) {
      for (WorldTile* tile : food_tiles) {
        food_tile = dynamic_cast<FoodTile*>(tile);
        if (food_tile != NULL &&
            (food_tile->infra_level() < LandTile::LAND_TILE_MAX_INFRA)) {
          build_food_infra = true;
          break;
        }
      }
    }
  }

  // We want a healthy level of production from this city if possible. We
  // need to verify that there are nearby production tiles that we can
  // enhance.
  bool build_prod_infra = false;
  LandTile* prod_tile = NULL;
  {
    if (prod_gathered < PROD_BEFORE_SETTLER) {
      for (WorldTile* tile : prod_tiles) {
        prod_tile = dynamic_cast<LandTile*>(tile);
        Require(prod_tile != NULL, "Production from a non-land tile?");
        if (prod_tile->infra_level() < LandTile::LAND_TILE_MAX_INFRA) {
          build_prod_infra = true;
          break;
        }
      }
    }
  }

  // We want to expand with settlers once a city has become large enough
  bool build_settler = false;
  Location settler_loc;
  {
    // Check if building a settler is appropriate. New cities must be
    // "adjacent" to the city that created the settler.
    const int max_distance = 3;
    const int min_distance = 2;
    float heuristic_of_best_loc_so_far = 0.0;
    for (int row_delta = -max_distance; row_delta <= max_distance; ++row_delta) {
      for (int col_delta = -max_distance; col_delta <= max_distance; ++col_delta) {
        Location loc_delta(m_location.row + row_delta,
                           m_location.col + col_delta);

        // Check if this is a valid city loc
        if (world.in_bounds(loc_delta) &&
            world.get_tile(loc_delta).supports_city() &&
            !is_within_distance_of_any_city(loc_delta, min_distance - 1)) {

          float heuristic = compute_city_loc_heuristic(loc_delta);
          if (heuristic > heuristic_of_best_loc_so_far) {
            settler_loc = loc_delta;
            heuristic_of_best_loc_so_far = heuristic;
            build_settler = true;
          }
        }
      }
    }
  }

  // We want a high level of production from this city if possible. We
  // need to verify that there are nearby production tiles that we can
  // enhance.
  bool fallback_build_prod_infra = false;
  {
    for (WorldTile* tile : prod_tiles) {
      prod_tile = dynamic_cast<LandTile*>(tile);
      Require(prod_tile != NULL, "Production from a non-land tile?");
      if (prod_tile->infra_level() < LandTile::LAND_TILE_MAX_INFRA) {
        fallback_build_prod_infra = true;
        break;
      }
    }
  }

  // Building defenses is always a decent option. Note that there is no upper
  // limit on the amount of defense a city can build.
  bool build_defenses = true;

  // Now we actually try to build stuff. The order in which we check the bools
  // defines the priorities of the items.
  if (build_food_infra) {
    Require(food_tile != NULL, "Error during food infra check");
    build_infra(*food_tile);
  }
  else if (build_prod_infra) {
    Require(prod_tile != NULL, "Error during prod infra check");
    build_infra(*prod_tile);
  }
  else if (build_settler) {
    if (m_production >= SETTLER_PROD_COST) {
      world.place_city(settler_loc);
      m_production -= SETTLER_PROD_COST;
    }
  }
  else if (fallback_build_prod_infra) {
    Require(prod_tile != NULL, "Error during fallback prod infra check");
    build_infra(*prod_tile);
  }
  else if (build_defenses) {
    // No settler expansion is possible, build city defenses. This is the
    // lowest priority item to build.
    unsigned cost = m_defense_level * CITY_DEF_PROD_COST;
    if (m_production >= cost) {
      ++m_defense_level;
      m_production -= cost;
    }
  }
  else {
    Require(false, "Nothing worth building?");
  }

  // Handle population growth

  // Compute multiplier, cannot exceed base by more than a factor of 4
  float food_multiplier;
  if (food_gathered < req_food) {
    m_famine = true;
    food_multiplier = -req_food / food_gathered;
    if (food_multiplier < -1 * MAX_GROWTH_MODIFIER) {
      food_multiplier = -1 * MAX_GROWTH_MODIFIER;
    }
  }
  else {
    m_famine = false;
    food_multiplier = food_gathered / req_food;
    if (food_multiplier > MAX_GROWTH_MODIFIER) {
      food_multiplier = MAX_GROWTH_MODIFIER;
    }
  }

  // Grow city
  float pop_growth_rate = 1 + (food_multiplier * CITY_BASE_GROWTH_RATE);
  m_population *= pop_growth_rate;
  if (m_population > m_next_rank_pop) {
    ++m_rank;
    m_next_rank_pop *= CITY_RANK_UP_MULTIPLIER;
  }
}

///////////////////////////////////////////////////////////////////////////////
void CityImpl::kill(unsigned killed)
///////////////////////////////////////////////////////////////////////////////
{
  Require(m_population >= killed, "Invalid killed: " << killed);

  m_population -= killed;

  if (m_population > 0) {
    while (m_population < m_next_rank_pop / CITY_RANK_UP_MULTIPLIER) {
      --m_rank;
      m_next_rank_pop /= CITY_RANK_UP_MULTIPLIER;
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
bool CityImpl::build_infra(LandTile& land_tile)
///////////////////////////////////////////////////////////////////////////////
{
  unsigned infra_level = land_tile.infra_level();
  Require(infra_level < LandTile::LAND_TILE_MAX_INFRA, "Error in build eval");
  unsigned next_infra_level = infra_level + 1;
  float prod_cost = next_infra_level * INFRA_PROD_COST;
  if (prod_cost < m_production) {
    m_production -= prod_cost;
    land_tile.build_infra();
    return true;
  }
  return false;
}

///////////////////////////////////////////////////////////////////////////////
xmlNodePtr CityImpl::to_xml() const
///////////////////////////////////////////////////////////////////////////////
{
  xmlNodePtr City_node = xmlNewNode(NULL, BAD_CAST "City");

  std::ostringstream rank_oss;
  rank_oss << m_rank;
  xmlNewChild(City_node, NULL, BAD_CAST "m_rank", BAD_CAST rank_oss.str().c_str());

  std::ostringstream population_oss;
  population_oss << m_population;
  xmlNewChild(City_node, NULL, BAD_CAST "m_population", BAD_CAST population_oss.str().c_str());

  std::ostringstream next_rank_pop_oss;
  next_rank_pop_oss << m_next_rank_pop;
  xmlNewChild(City_node, NULL, BAD_CAST "m_next_rank_pop", BAD_CAST next_rank_pop_oss.str().c_str());

  std::ostringstream production_oss;
  production_oss << m_production;
  xmlNewChild(City_node, NULL, BAD_CAST "m_production", BAD_CAST production_oss.str().c_str());

  std::ostringstream location_oss;
  location_oss << m_location;
  xmlNewChild(City_node, NULL, BAD_CAST "m_location", BAD_CAST location_oss.str().c_str());

  std::ostringstream defense_level_rank_oss;
  defense_level_rank_oss << m_defense_level;
  xmlNewChild(City_node, NULL, BAD_CAST "m_defense_level", BAD_CAST defense_level_rank_oss.str().c_str());

  std::ostringstream famine_oss;
  famine_oss << m_famine;
  xmlNewChild(City_node, NULL, BAD_CAST "m_famine", BAD_CAST famine_oss.str().c_str());

  return City_node;
}

}
}
