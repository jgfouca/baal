#include "CityImpl.hpp"
#include "Util.hpp"
#include "World.hpp"

#include <gtest/gtest.h>

#include <string>
#include <sstream>
#include <vector>

using baal::CityImpl;
using baal::Location;

namespace {

TEST(City, CityBasics)
{
  //
  // Test simple City queries
  //

  // Create a city
  std::string city_name = "testCity";
  Location location(4, 2);
  CityImpl city(city_name, location);

  EXPECT_EQ(city_name, city.name());
  EXPECT_EQ(location, city.location());
  EXPECT_EQ(CityImpl::CITY_STARTING_POP, city.population());
  EXPECT_EQ(1, city.rank());
  EXPECT_EQ(false, city.famine());
  EXPECT_EQ(CityImpl::CITY_STARTING_DEFENSE, city.defense());
  EXPECT_EQ(0.0, city.m_production);
}

TEST(City, CityAdvanced)
{
  
}

TEST(City, CityAI)
{

}

}
