#include "CityImpl.hpp"
#include "Util.hpp"
#include "World.hpp"
#include "Engine.hpp"

#include <gtest/gtest.h>

#include <string>
#include <sstream>
#include <vector>
#include <limits>

using baal::details::CityImpl;
using baal::Location;

namespace {

TEST(City, CityBasics)
{
  //
  // Test simple City queries
  //

  // Create a city
  baal::Engine engine;
  std::string city_name = "testCity";
  Location location(4, 2);
  CityImpl city(city_name, location, engine);

  // Need to do the following to avoid link errors
  unsigned expected_pop = CityImpl::CITY_STARTING_POP;
  unsigned expected_defense = CityImpl::CITY_STARTING_DEFENSE;

  EXPECT_EQ(city_name, city.name());
  EXPECT_EQ(location, city.location());
  EXPECT_EQ(expected_pop, city.population());
  EXPECT_EQ(1, city.rank());
  EXPECT_FALSE(city.famine());
  EXPECT_EQ(expected_defense, city.defense());
  EXPECT_EQ(0.0, city.m_production);
}

TEST(City, CityAdvanced)
{
  //
  // Test more-advanced city operations
  //

  // Create a city
  baal::Engine engine;
  std::string city_name = "testCity";
  Location location(4, 2);
  CityImpl city(city_name, location, engine);
  unsigned starting_defense = CityImpl::CITY_STARTING_DEFENSE;
  unsigned starting_pop = CityImpl::CITY_STARTING_POP;

  // Test producting some stuff
  CityImpl::Action build_defense(CityImpl::BUILD_DEFENSE);
  EXPECT_FALSE(city.produce_item(build_defense));
  city.m_production = CityImpl::CITY_DEF_PROD_COST;
  EXPECT_TRUE(city.produce_item(build_defense));
  EXPECT_EQ(0.0, city.m_production);
  EXPECT_EQ(starting_defense + 1, city.defense());

  // Test destroy defense
  city.destroy_defense(1);
  EXPECT_EQ(starting_defense, city.defense());

  // Test population mechanics
  city.feed_people(city.get_required_food());
  EXPECT_GT(city.population(), starting_pop);

  city.m_population = city.m_next_rank_pop;
  city.feed_people(city.get_required_food());
  EXPECT_EQ(2, city.rank());

  city.kill(city.population() - starting_pop);
  EXPECT_EQ(starting_pop, city.population());
  EXPECT_EQ(1, city.rank());

  city.feed_people(city.get_required_food() / 2);
  EXPECT_GT(starting_pop, city.population());
  EXPECT_TRUE(city.famine());

  // Test build-infra
  Location infra_location(3, 3);
  baal::LandTile& infra_tile =
    dynamic_cast<baal::LandTile&>(engine.world().get_tile(infra_location));
  CityImpl::Action build_infra(CityImpl::BUILD_INFRA, &infra_tile);
  city.m_production = CityImpl::INFRA_PROD_COST;
  EXPECT_TRUE(city.produce_item(build_infra));
  EXPECT_EQ(1, infra_tile.infra_level());
  EXPECT_EQ(0.0, city.m_production);
}

TEST(City, CityAI)
{
  // Test functionality of CityAI

  // Create a city
  baal::Engine engine;
  std::string city_name = "testCity";
  Location location(4, 2);
  CityImpl city(city_name, location, engine);

  // Test examination of nearby tiles
  auto tiles_pair = city.examine_workable_tiles();
  EXPECT_GT(tiles_pair.first.size(), 0);
  EXPECT_GT(tiles_pair.second.size(), 0);
  EXPECT_EQ(8, tiles_pair.first.size() + tiles_pair.second.size());
  typedef decltype(baal::Yield(0, 0).m_food) yield_type;

  yield_type last = std::numeric_limits<yield_type>::max();
  for (baal::WorldTile* tile : tiles_pair.first) {
    yield_type current = tile->yield().m_food;
    EXPECT_GT(current, 0);
    EXPECT_LE(current, last);
    last = current;
  }

  last = std::numeric_limits<yield_type>::max();
  for (baal::WorldTile* tile : tiles_pair.second) {
    yield_type current = tile->yield().m_prod;
    EXPECT_GT(current, 0);
    EXPECT_LE(current, last);
    last = current;
  }

  // Test citizen recommendation. Unit testing this stuff becomes hard
  // because there is no "right" answer.
  auto work_tiles_pair = city.get_citizen_recommendation(tiles_pair.first,
                                                         tiles_pair.second);
  EXPECT_EQ(city.rank(), work_tiles_pair.first.size() + work_tiles_pair.second.size());

  // Test citizen assignment
  auto resources_gathered = city.assign_citizens(work_tiles_pair.first,
                                                 work_tiles_pair.second);
  yield_type expected_food = CityImpl::FOOD_FROM_CITY_CENTER;
  yield_type expected_prod = CityImpl::PROD_FROM_CITY_CENTER;
  for (const baal::WorldTile* tile : work_tiles_pair.first) {
    EXPECT_TRUE(tile->worked());
    expected_food += tile->yield().m_food;
  }
  for (const baal::WorldTile* tile : work_tiles_pair.second) {
    EXPECT_TRUE(tile->worked());
    expected_prod += tile->yield().m_prod;
  }
  EXPECT_EQ(expected_food, resources_gathered.first);
  EXPECT_EQ(expected_prod, resources_gathered.second);

  // Test recommended build. Unit testing this stuff becomes hard
  // because there is no "right" answer.
  CityImpl::Action action = city.get_recommended_production(
    tiles_pair.first,
    tiles_pair.second,
    work_tiles_pair.first,
    work_tiles_pair.second,
    resources_gathered.first,
    resources_gathered.second);
  EXPECT_NE(CityImpl::NO_ACTION, action.m_action_id);
}

}
