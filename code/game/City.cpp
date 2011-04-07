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
  for (std::list<WorldTile*>::iterator itr = food_tiles.begin();
       itr != food_tiles.end() && num_workers > 0; ) {
    if (food_gathered < req_food) {
      (*itr)->work();
      --num_workers;
      food_gathered += (*itr)->yield().m_food;
      food_tiles.pop_front();
      itr = food_tiles.begin();
    }
    else {
      // TODO: AI should continue to pile up food to increase it's growth
      // rate unless it's already hit the max growth rate or if there are
      // good production tiles available or it really needs production.
      break;
    }
  }

  for (std::list<WorldTile*>::iterator itr = prod_tiles.begin();
       itr != prod_tiles.end() && num_workers > 0; ) {
    if ((*itr)->yield().m_prod > PROD_FROM_SPECIALIST) {
      (*itr)->work();
      --num_workers;
      prod_gathered += (*itr)->yield().m_prod;
      prod_tiles.pop_front();
      itr = prod_tiles.begin();
    }
  }

  // Remaining workers are specialists that contribute production
  prod_gathered += num_workers * PROD_FROM_SPECIALIST;

  // Accumulate production
  m_production += prod_gathered;

  // TODO - Decide on how to spend production

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
  // Cites can be wiped out, but that is done via the World object that owns
  // them.
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
