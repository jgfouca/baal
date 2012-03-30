#include "CityImpl.hpp"
#include "Util.hpp"
#include "World.hpp"
#include "Engine.hpp"

#include <gtest/gtest.h>

#include <string>
#include <sstream>
#include <vector>

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

}

TEST(City, CityAI)
{

}

}
