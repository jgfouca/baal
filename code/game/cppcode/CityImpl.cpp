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

//#define TRACE_CITY_AI

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
std::ostream& operator<<(std::ostream& out, CityImpl::Action action)
///////////////////////////////////////////////////////////////////////////////
{
  switch(action.m_action_id) {
  case CityImpl::BUILD_INFRA:
    out << "BUILD_INFRA";
    break;
  case CityImpl::BUILD_SETTLER:
    out << "BUILD_SETTLER";
    break;
  case CityImpl::BUILD_DEFENSE:
    out << "BUILD_DEFENSE";
    break;
  case CityImpl::NO_ACTION:
    out << "NO_ACTION";
    break;
  default:
    Require(false, "unhandled " << action.m_action_id);
  }

  Location location;
  if (action.m_affected_tile != nullptr) {
    location = action.m_affected_tile->location();
  }
  else {
    location = action.m_location;
  }

  if (is_valid(location)) {
    out << " at " << location;
  }
  return out;
}

///////////////////////////////////////////////////////////////////////////////
bool is_within_distance_of_any_city(Location location, int distance)
///////////////////////////////////////////////////////////////////////////////
{
  for (City* city : Engine::instance().world().cities()) {
    Location city_loc = city->location();
    if (std::abs(static_cast<int>(location.row - city_loc.row)) <= distance &&
        std::abs(static_cast<int>(location.col - city_loc.col)) <= distance) {
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
  const unsigned num_tiles_surrounding_city = boost::distance(location_range);
  food_tiles.reserve(num_tiles_surrounding_city);
  prod_tiles.reserve(num_tiles_surrounding_city);

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
float compute_city_loc_heuristic(Location location)
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
CityImpl::CityImpl(const std::string& name, Location location)
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
CityImpl::tile_vec_pair
CityImpl::examine_workable_tiles() const
///////////////////////////////////////////////////////////////////////////////
{
  return compute_nearby_food_and_prod_tiles(get_adjacent_location_range(m_location),
                                            FilterAlreadyWorked());
}

///////////////////////////////////////////////////////////////////////////////
CityImpl::tile_vec_pair
CityImpl::get_citizen_recommendation(const std::vector<WorldTile*>& food_tiles,
                                     const std::vector<WorldTile*>& prod_tiles)
  const
///////////////////////////////////////////////////////////////////////////////
{
  // Choose which tiles to work. We first make sure we have ample food, then
  // we look for production tiles.

  std::vector<WorldTile*> work_food_tiles, work_prod_tiles;
  work_food_tiles.reserve(food_tiles.size());
  work_prod_tiles.reserve(prod_tiles.size());

  float req_food = get_required_food();
  float food_gathered = FOOD_FROM_CITY_CENTER;
  const float tech_multiplier = Engine::instance().ai_player().tech_yield_multiplier();
  unsigned num_citizens_left = m_rank;

  // Determine how many workers should be allocated to gathering food. This
  // has the highest priority up until the minimum food is collected to
  // feed everyone.
  for (WorldTile* food_tile : food_tiles) {
    if (num_citizens_left == 0) {
      break;
    }

    if (food_gathered < req_food) {
      work_food_tiles.push_back(food_tile);
      food_gathered += food_tile->yield().m_food * tech_multiplier;
      --num_citizens_left;
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
    if (num_citizens_left == 0) {
      break;
    }

    if (prod_tile->yield().m_prod > PROD_FROM_SPECIALIST) {
      work_prod_tiles.push_back(prod_tile);
      --num_citizens_left;
    }
  }

  return std::make_pair(work_food_tiles, work_prod_tiles);
}

///////////////////////////////////////////////////////////////////////////////
CityImpl::resource_pair
CityImpl::assign_citizens(const std::vector<WorldTile*>& work_food_tiles,
                          const std::vector<WorldTile*>& work_prod_tiles)
///////////////////////////////////////////////////////////////////////////////
{
#ifdef TRACE_CITY_AI
  std::cout << "  assign_citizens:" << std::endl;
#endif

  float food_gathered = FOOD_FROM_CITY_CENTER,
        prod_gathered = PROD_FROM_CITY_CENTER;

  for (WorldTile* food_tile : work_food_tiles) {
    food_tile->work();
    food_gathered += food_tile->yield().m_food;

#ifdef TRACE_CITY_AI
    std::cout << "    Working food tile: " << food_tile->location() << " for " << food_tile->yield().m_food << " raw food" << std::endl;
#endif
  }

  for (WorldTile* prod_tile : work_prod_tiles) {
    prod_tile->work();
    prod_gathered += prod_tile->yield().m_prod;

#ifdef TRACE_CITY_AI
    std::cout << "    Working production tile: " << prod_tile->location() << " for " << prod_tile->yield().m_prod << " raw production" << std::endl;
#endif
  }

  // Remaining workers are specialists that contribute production
  const unsigned num_specialists =
    m_rank - work_food_tiles.size() - work_prod_tiles.size();
  prod_gathered += num_specialists * PROD_FROM_SPECIALIST;

  // AI get's a resource collection bonus from tech
  const float tech_multiplier = Engine::instance().ai_player().tech_yield_multiplier();
  food_gathered *= tech_multiplier;
  prod_gathered *= tech_multiplier;
  m_production  += prod_gathered; // prod is accumulated, all food is eaten
#ifdef TRACE_CITY_AI
  std::cout << "    Collected a total of " << food_gathered << " food and "
            << prod_gathered << " production." << std::endl;
#endif

  // Invariants
  Require(food_gathered > 0.0, "Negative food gathered");
  Require(prod_gathered > 0.0, "Negative prod gathered");
  Require(num_specialists <= m_rank, "Too many specialists");
  Require(work_food_tiles.size() + work_prod_tiles.size() <= m_rank,
          "Too many citizens allocated");

  return std::make_pair(food_gathered * tech_multiplier,
                        prod_gathered * tech_multiplier);
}

///////////////////////////////////////////////////////////////////////////////
void CityImpl::feed_people(float food)
///////////////////////////////////////////////////////////////////////////////
{
#ifdef TRACE_CITY_AI
  std::cout << "  feed_people(" << food << "):" << std::endl;
#endif
  unsigned orig_population = m_population;

  // Compute population multiplier, cannot exceed base by more than a factor of 4
  float food_multiplier = 1.0, req_food = get_required_food();
  if (food < req_food) {
    m_famine = true;
    food_multiplier = -req_food / food;
    if (food_multiplier < -1 * MAX_GROWTH_MODIFIER) {
      food_multiplier = -1 * MAX_GROWTH_MODIFIER;
    }
  }
  else {
    m_famine = false;
    food_multiplier = food / req_food;
    if (food_multiplier > MAX_GROWTH_MODIFIER) {
      food_multiplier = MAX_GROWTH_MODIFIER;
    }
  }

  // Grow city
  float pop_growth_rate = 1 + (food_multiplier * CITY_BASE_GROWTH_RATE);
#ifdef TRACE_CITY_AI
  std::cout << "    growth rate is " << pop_growth_rate << std::endl;
#endif
  m_population *= pop_growth_rate;
  if (m_population > m_next_rank_pop) {
    ++m_rank;
    m_next_rank_pop *= CITY_RANK_UP_MULTIPLIER;
  }
#ifdef TRACE_CITY_AI
  std::cout << "    pop is " << m_population
            << ", rank is " << m_rank << std::endl;
#endif

  // Invariants
  if (m_famine) {
    Require(orig_population > m_population,
            "Population should decrease when there is famine");
  }
  else {
    Require(orig_population < m_population,
            "Population should increase when there is not famine");
  }
}

///////////////////////////////////////////////////////////////////////////////
CityImpl::Action
CityImpl::get_recommended_production(const std::vector<WorldTile*>& food_tiles,
                                     const std::vector<WorldTile*>& prod_tiles,
                                     const std::vector<WorldTile*>& worked_food_tiles,
                                     const std::vector<WorldTile*>& worked_prod_tiles,
                                     float food_gathered,
                                     float prod_gathered)
///////////////////////////////////////////////////////////////////////////////
{
  // Decide on how to spend production. Options: City fortifications,
  // tile infrastructure, or settler. In the current system, the AI builds
  // general production points and it can use them to insta-buy the things
  // it wants. This is a bit unrealistic and should probably be replaced
  // with a system where the AI chooses to start building something and
  // all production points go to that thing until it is finished (a la civ).
  //
  // First, we need to decide what production item we should be saving
  // up for. Note that the AI will *not* build a cheaper item that is of
  // lower priority if it cannot afford the highest priority item. It will
  // always save-up for the highest priority. The following blocks of code
  // each evaluate whether an item should be built; the order in which the
  // blocks appear defines the priority.

  World& world = Engine::instance().world();

  // 1)
  // We want some of our workers doing something other than just
  // collecting food. If we are having to dedicate our workforce to
  // food, we probably need better food infrastructure. We need to verify
  // that there are nearby food tiles that we can enhance.
  {
    float pct_workers_on_food =
      static_cast<float>(worked_food_tiles.size()) / m_rank;
    if (pct_workers_on_food > TOO_MANY_FOOD_WORKERS || m_famine) {
      for (WorldTile* tile : food_tiles) {
        FoodTile* food_tile = dynamic_cast<FoodTile*>(tile);
        if (food_tile != nullptr &&
            (food_tile->infra_level() < LandTile::LAND_TILE_MAX_INFRA)) {
          return Action(BUILD_INFRA, food_tile);
        }
      }
    }
  }

  // We want a healthy level of production from this city if possible. We
  // need to verify that there are nearby production tiles that we can
  // enhance. We don't want to start pumping settlers before this city
  // has gotten-going.
  {
    if (prod_gathered < PROD_BEFORE_SETTLER) {
      for (WorldTile* tile : prod_tiles) {
        LandTile* prod_tile = dynamic_cast<LandTile*>(tile);
        Require(prod_tile != nullptr, "Production from a non-land tile?");
        if (prod_tile->infra_level() < LandTile::LAND_TILE_MAX_INFRA) {
          return Action(BUILD_INFRA, prod_tile);
        }
      }
    }
  }

  // We want to expand with settlers once a city has become large enough
  {
    // Check if building a settler is appropriate. New cities must be
    // "adjacent" to the city that created the settler.
    const int max_distance = 3;
    const int min_distance = 2;
    float heuristic_of_best_loc_so_far = 0.0;
    Location settler_loc;
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
          }
        }
      }
    }
    if (is_valid(settler_loc)) {
      return Action(BUILD_SETTLER, settler_loc);
    }
  }

  // We want a high level of production from this city if possible. We
  // need to verify that there are nearby production tiles that we can
  // enhance.
  {
    for (WorldTile* tile : prod_tiles) {
      LandTile* prod_tile = dynamic_cast<LandTile*>(tile);
      Require(prod_tile != nullptr, "Production from a non-land tile?");
      if (prod_tile->infra_level() < LandTile::LAND_TILE_MAX_INFRA) {
        return Action(BUILD_INFRA, prod_tile);
      }
    }
  }

  // Building defenses is always a decent option. Note that there is no upper
  // limit on the amount of defense a city can build.
  return Action(BUILD_DEFENSE);
}

///////////////////////////////////////////////////////////////////////////////
bool CityImpl::produce_item(Action action)
///////////////////////////////////////////////////////////////////////////////
{
#ifdef TRACE_CITY_AI
  std::cout << "  produce_item(" << action << ")" << std::endl;
  std::cout << "    production accumulated: " << m_production << std::endl;
#endif
  World& world = Engine::instance().world();
  bool was_produced = false;
  float orig_production = m_production;

  // Now we actually try to build stuff. The order in which we check the bools
  // defines the priorities of the items.
  switch(action.m_action_id) {
  case BUILD_INFRA:
    Require(action.m_affected_tile != nullptr, "Error, null tile");
    was_produced = build_infra(*action.m_affected_tile);
    break;
  case BUILD_SETTLER:
    if (m_production >= SETTLER_PROD_COST) {
      world.place_city(action.m_location);
      m_production -= SETTLER_PROD_COST;
      was_produced = true;
    }
    else {
      was_produced = false;
    }
    break;
  case BUILD_DEFENSE:
    if (m_production >= (m_defense_level * CITY_DEF_PROD_COST)) {
      m_production -= (m_defense_level * CITY_DEF_PROD_COST);
      ++m_defense_level;
      was_produced = true;
    }
    else {
      was_produced = false;
    }
    break;
  case NO_ACTION:
  default:
    Require(false, "Nothing worth building?");
  }

#ifdef TRACE_CITY_AI
  std::cout << "    item was " << (was_produced ? "" : " not ") << "produced" << std::endl;
  std::cout << "    production remaining: " << m_production << std::endl;
#endif

  // Invariants
  if (was_produced) {
    Require(orig_production > m_production, "Production should have decreased");
  }
  else {
    Require(orig_production == m_production, "Production should not have changed");
  }
  Require(m_production >= 0.0, "Negative production");

  return was_produced;
}

///////////////////////////////////////////////////////////////////////////////
void CityImpl::cycle_turn()
///////////////////////////////////////////////////////////////////////////////
{
#ifdef TRACE_CITY_AI
  std::cout << "cycle_turn for city: " << name() << std::endl;
#endif
  Require(m_population > 0,
          "This city has no people and should have been deleted");

  // 1)
  // Evaluate nearby tiles, put in to sorted lists (best-to-worst) for each
  // of the two yield types
  auto tile_pair = examine_workable_tiles();
  const std::vector<WorldTile*>& food_tiles = tile_pair.first;
  const std::vector<WorldTile*>& prod_tiles = tile_pair.second;

  // 2)
  // Get recommended citizen allocation
  auto work_tile_pair = get_citizen_recommendation(food_tiles, prod_tiles);
  const std::vector<WorldTile*>& work_food_tiles = work_tile_pair.first;
  const std::vector<WorldTile*>& work_prod_tiles = work_tile_pair.second;

  // 3)
  // Assign citizens based on recommendations
  auto resources_gathered = assign_citizens(work_food_tiles, work_prod_tiles);
  const float food_gathered = resources_gathered.first;
  const float prod_gathered = resources_gathered.second;

  // 4)
  // Feed people and grow population. Needs to happen *before* we decide
  // on what to build so we can use information based on this harvest
  // to help determine what to build.
  feed_people(food_gathered);

  // 5)
  // Decide on how to spend production.
  Action recommended_build = get_recommended_production(food_tiles,
                                                        prod_tiles,
                                                        work_food_tiles,
                                                        work_prod_tiles,
                                                        food_gathered,
                                                        prod_gathered);

  // 6)
  // Produce recommended item if possible
  produce_item(recommended_build);
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
  xmlNodePtr City_node = xmlNewNode(nullptr, BAD_CAST "City");

  std::ostringstream rank_oss;
  rank_oss << m_rank;
  xmlNewChild(City_node, nullptr, BAD_CAST "m_rank", BAD_CAST rank_oss.str().c_str());

  std::ostringstream population_oss;
  population_oss << m_population;
  xmlNewChild(City_node, nullptr, BAD_CAST "m_population", BAD_CAST population_oss.str().c_str());

  std::ostringstream next_rank_pop_oss;
  next_rank_pop_oss << m_next_rank_pop;
  xmlNewChild(City_node, nullptr, BAD_CAST "m_next_rank_pop", BAD_CAST next_rank_pop_oss.str().c_str());

  std::ostringstream production_oss;
  production_oss << m_production;
  xmlNewChild(City_node, nullptr, BAD_CAST "m_production", BAD_CAST production_oss.str().c_str());

  std::ostringstream location_oss;
  location_oss << m_location;
  xmlNewChild(City_node, nullptr, BAD_CAST "m_location", BAD_CAST location_oss.str().c_str());

  std::ostringstream defense_level_rank_oss;
  defense_level_rank_oss << m_defense_level;
  xmlNewChild(City_node, nullptr, BAD_CAST "m_defense_level", BAD_CAST defense_level_rank_oss.str().c_str());

  std::ostringstream famine_oss;
  famine_oss << m_famine;
  xmlNewChild(City_node, nullptr, BAD_CAST "m_famine", BAD_CAST famine_oss.str().c_str());

  return City_node;
}

}
}
