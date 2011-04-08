#include "City.hpp"
#include "BaalExceptions.hpp"
#include "World.hpp"

#include <list>

using namespace baal;

///////////////////////////////////////////////////////////////////////////////
City::City(const std::string& name, World& world, const Location& location)
///////////////////////////////////////////////////////////////////////////////
  : m_name(name),
    m_rank(1),
    m_population(CITY_STARTING_POP),
    m_next_rank_pop(CITY_STARTING_POP * CITY_RANK_UP_MULTIPLIER),
    m_production(0.0),
    m_world(world),
    m_city_location(location)
{}

///////////////////////////////////////////////////////////////////////////////
void City::cycle_turn()
///////////////////////////////////////////////////////////////////////////////
{
  // TODO: Make tile yields go up based on tech-level

  Require(m_population > 0,
          "This city has no people and should have been deleted");

  // Gather resources based on nearby worked tiles. At this time, cities will
  // only be able to harvest adjacent tiles.

  // Evaluate nearby tiles, put in to sorted lists (best-to-worst) for each
  // of the two yield types
  unsigned row_loc = m_city_location.row;
  unsigned col_loc = m_city_location.col;
  std::list<WorldTile*> food_tiles, prod_tiles; // sorted highest to lowest
  for (int row_delta = -1; row_delta <= 1; ++row_delta) {
    for (int col_delta = -1; col_delta <= 1; ++col_delta) {
      Location loc_delta(row_loc + row_delta, col_loc + col_delta);
      if (m_world.in_bounds(loc_delta)) {
        WorldTile& tile = m_world.get_tile(loc_delta);
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

  for (std::list<WorldTile*>::iterator itr = prod_tiles.begin();
       itr != prod_tiles.end() && num_workers > 0; ++itr) {
    if ((*itr)->yield().m_prod > PROD_FROM_SPECIALIST) {
      (*itr)->work();
      --num_workers;
      prod_gathered += (*itr)->yield().m_prod;
    }
  }

  // Remaining workers are specialists that contribute production
  prod_gathered += num_workers * PROD_FROM_SPECIALIST;

  // Accumulate production
  m_production += prod_gathered;

  // Decide on how to spend production. Options: City fortifications,
  // tile infrastructure, or settler. In the current system, the AI builds
  // general production points and it can use them to insta-buy the things
  // it wants. This is a bit unrealistic and should probably be replaced
  // with a system where the AI chooses to start building something and
  // all production points go to that thing until it is finished (a la civ).

  // We want some of our workers doing something other than just
  // collecting food.  If we are having to dedicate our workforce to
  // food, we probably need better food infrastructure. By putting this
  // first, we are giving it the highest priority.
  float pct_workers_on_food =
    static_cast<float>(num_workers_used_for_food) / m_rank;
  float expected_production = m_rank * EXPECTED_PROD_PER_SIZE;
  if (pct_workers_on_food > TOO_MANY_FOOD_WORKDERS) {
    for (std::list<WorldTile*>::iterator itr = food_tiles.begin();
         itr != food_tiles.end();
         ++itr) {
      FoodTile* food_tile = dynamic_cast<FoodTile*>(*itr);
      if (food_tile != NULL) {
        try_to_build_infra(*food_tile);
      }
    }
  }
  // See if we want to build any production infrastructure
  else if (prod_gathered < expected_production) {
    for (std::list<WorldTile*>::iterator itr = prod_tiles.begin();
         itr != prod_tiles.end();
         ++itr) {
      LandTile* land_tile = dynamic_cast<LandTile*>(*itr);
      Require(land_tile != NULL, "Production from a non-land tile?");
      try_to_build_infra(*land_tile);
    }
  }
  else {
    // TODO: We're good as far as food and production, make a settler or
    // city defenses.
  }

  // Handle population growth

  // Compute multiplier, cannot exceed base by more than a factor of 4
  float food_multiplier;
  if (food_gathered < req_food) {
    food_multiplier = -req_food / food_gathered;
    if (food_multiplier < -1 * MAX_GROWTH_MODIFIER) {
      food_multiplier = -1 * MAX_GROWTH_MODIFIER;
    }
  }
  else {
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
bool City::try_to_build_infra(LandTile& land_tile)
///////////////////////////////////////////////////////////////////////////////
{
  unsigned infra_level = land_tile.infra_level();
  if (infra_level < LandTile::LAND_TILE_MAX_INFRA) {
    unsigned next_infra_level = infra_level + 1;
    float prod_cost = next_infra_level * INFRA_PROD_COST;
    if (prod_cost < m_production) {
      m_production -= prod_cost;
      land_tile.build_infra();
      return true;
    }
  }
  return false;
}
