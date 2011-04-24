#include "City.hpp"
#include "BaalExceptions.hpp"
#include "World.hpp"
#include "Engine.hpp"
#include "PlayerAI.hpp"

#include <cstdlib>
#include <cmath>
#include <list>
#include <iostream>

using namespace baal;

namespace {

///////////////////////////////////////////////////////////////////////////////
bool is_within_distance_of_any_city(const Location& location, int distance)
///////////////////////////////////////////////////////////////////////////////
{
  const std::vector<City*>& cities = Engine::instance().world().cities();
  for (std::vector<City*>::const_iterator
       itr = cities.begin(); itr != cities.end(); ++itr) {
    const Location& city_loc = (*itr)->location();
    int loc_row = location.row;
    int loc_col = location.col;
    int city_loc_row = city_loc.row;
    int city_loc_col = city_loc.col;
    if (std::abs(loc_row - city_loc_row) <= distance &&
        std::abs(loc_col - city_loc_col) <= distance) {
      return true;
    }
  }
  return false;
}

///////////////////////////////////////////////////////////////////////////////
float compute_city_loc_heuristic(const Location& location)
///////////////////////////////////////////////////////////////////////////////
{
  World& world = Engine::instance().world();
  const unsigned row_loc = location.row;
  const unsigned col_loc = location.col;
  float available_food = 0.0;
  float available_prod = 0.0;
  for (int row_delta = -1; row_delta <= 1; ++row_delta) {
    for (int col_delta = -1; col_delta <= 1; ++col_delta) {
      // Cannot work tile that has city
      if (row_delta != 0 || col_delta != 0) {
        Location loc_delta(row_loc + row_delta, col_loc + col_delta);
        if (world.in_bounds(loc_delta) &&
            !is_within_distance_of_any_city(loc_delta, 1)) {
          WorldTile& tile = world.get_tile(loc_delta);
          if (tile.yield().m_food > 0) {
            available_food += tile.yield().m_food;
          }
          else {
            available_prod += tile.yield().m_prod;
          }
        }
      }
    }
  }

  // Favor city locations with a good balance of food and production
  return std::sqrt( (available_food + 1.0) * (available_prod + 1.0) );
}

} // empty namespace

///////////////////////////////////////////////////////////////////////////////
City::City(const std::string& name, const Location& location)
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
void City::cycle_turn()
///////////////////////////////////////////////////////////////////////////////
{
  Require(m_population > 0,
          "This city has no people and should have been deleted");

  Engine& engine = Engine::instance();
  World& world = engine.world();

  // Gather resources based on nearby worked tiles. At this time, cities will
  // only be able to harvest adjacent tiles.

  // Evaluate nearby tiles, put in to sorted lists (best-to-worst) for each
  // of the two yield types
  const unsigned row_loc = m_location.row;
  const unsigned col_loc = m_location.col;
  std::list<WorldTile*> food_tiles, prod_tiles; // sorted highest to lowest
  for (int row_delta = -1; row_delta <= 1; ++row_delta) {
    for (int col_delta = -1; col_delta <= 1; ++col_delta) {
      // Cannot work tile that has city
      if (row_delta != 0 || col_delta != 0) {
        Location loc_delta(row_loc + row_delta, col_loc + col_delta);
        if (world.in_bounds(loc_delta)) {
          WorldTile& tile = world.get_tile(loc_delta);
          if (!tile.worked()) {
            if (tile.yield().m_food > 0) {
              ordered_insert(food_tiles, tile);
            }
            else {
              ordered_insert(prod_tiles, tile);
            }
          }
        }
      }
    }
  }

  // Choose which tiles to work. We first make sure we have ample food, then
  // we look for production tiles.

  float req_food = static_cast<float>(m_population) / POP_THAT_EATS_ONE_FOOD;
  float food_gathered = FOOD_FROM_CITY_CENTER;
  float prod_gathered = PROD_FROM_CITY_CENTER;
  unsigned num_workers = m_rank;
  unsigned num_workers_used_for_food = 0;
  for (std::list<WorldTile*>::iterator itr = food_tiles.begin();
       itr != food_tiles.end() && num_workers > 0; ++itr) {
    if (food_gathered < req_food) {
      (*itr)->work();
      --num_workers;
      ++num_workers_used_for_food;
      food_gathered += (*itr)->yield().m_food;
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
  for (std::list<WorldTile*>::iterator itr = prod_tiles.begin();
       itr != prod_tiles.end() && num_workers > 0; ++itr) {
    if ((*itr)->yield().m_prod > PROD_FROM_SPECIALIST) {
      (*itr)->work();
      --num_workers;
      prod_gathered += (*itr)->yield().m_prod;
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
      for (std::list<WorldTile*>::iterator itr = food_tiles.begin();
           itr != food_tiles.end();
           ++itr) {
        food_tile = dynamic_cast<FoodTile*>(*itr);
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
      for (std::list<WorldTile*>::iterator itr = prod_tiles.begin();
           itr != prod_tiles.end();
           ++itr) {
        prod_tile = dynamic_cast<LandTile*>(*itr);
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
        Location loc_delta(row_loc + row_delta, col_loc + col_delta);

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
    for (std::list<WorldTile*>::iterator itr = prod_tiles.begin();
         itr != prod_tiles.end();
         ++itr) {
      prod_tile = dynamic_cast<LandTile*>(*itr);
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
void City::kill(unsigned killed)
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
void City::ordered_insert(std::list<WorldTile*>& tile_list,
                          WorldTile& tile) const
///////////////////////////////////////////////////////////////////////////////
{
  for (std::list<WorldTile*>::iterator
       itr = tile_list.begin(); itr != tile_list.end(); ++itr) {
    WorldTile* curr_tile = *itr;
    float curr_tile_food = curr_tile->yield().m_food;
    float curr_tile_prod = curr_tile->yield().m_prod;
    float new_tile_food  = tile.yield().m_food;
    float new_tile_prod  = tile.yield().m_prod;
    if (curr_tile_food > 0) {
      Require(new_tile_food > 0, "Mismatch between list and new tile");
      if (new_tile_food > curr_tile_food) {
        tile_list.insert(itr, &tile); // OK since iteration is over
        return;
      }
    }
    else {
      Require(curr_tile_prod > 0, "Curr tile is neither food nor prod?");
      Require(new_tile_prod > 0, "Mismatch between list and new tile");
      if (new_tile_prod > curr_tile_prod) {
        tile_list.insert(itr, &tile); // OK since iteration is over
        return;
      }
    }
  }

  tile_list.push_back(&tile);
}

///////////////////////////////////////////////////////////////////////////////
bool City::build_infra(LandTile& land_tile)
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
