#include "Weather.hpp"
#include "Engine.hpp"
#include "World.hpp"

#include <gtest/gtest.h>
#include <sstream>

namespace {

TEST(World, basic)
{
  using namespace baal;

  auto engine = baal::create_engine();
  World& world = engine->world();

  const unsigned width = world.width();
  const unsigned height = world.height();

  EXPECT_FALSE(world.in_bounds(Location(width, height)));
  EXPECT_FALSE(world.in_bounds(Location(width, height - 1)));
  EXPECT_FALSE(world.in_bounds(Location(width - 1, height)));
  EXPECT_TRUE(world.in_bounds(Location(width - 1, height - 1)));

  EXPECT_EQ(world.get_tile(Location(4, 2)).location(), Location(4, 2));

  const std::vector<City*>& cities = world.cities();
  EXPECT_EQ(cities.size(), 1u);

  world.remove_city(*cities[0]);

  EXPECT_EQ(world.cities().size(), 0u);
}

void check_adjacency(baal::Location location,
                     const std::vector<baal::Location>& expected,
                     const baal::World& world)
{
  auto adj_range = world.valid_nearby_tile_range(location);
  EXPECT_EQ( static_cast<size_t>(boost::distance(adj_range)), expected.size());

  if (static_cast<size_t>(boost::distance(adj_range)) == expected.size()) {
    auto adj_itr = boost::const_begin(adj_range);
    for (baal::Location loc : expected) {
      EXPECT_EQ(loc, *adj_itr);
      ++adj_itr;
    }
  }
}

TEST(World, NearbyTileRange)
{
  using namespace baal;

  auto engine = baal::create_engine();
  World& world = engine->world();

  {
    // Central location
    Location location(3,3);
    std::vector<Location> expected =
      {
        {2,2}, {2,3}, {2,4},

        {3,2}, {3,3}, {3,4},

        {4,2}, {4,3}, {4,4}
      };
    check_adjacency(location, expected, world);
  }

  {
    // Upper-left corner location
    Location location(0,0);
    std::vector<Location> expected =
      {
        {0,0}, {0,1},

        {1,0}, {1,1}
      };
    check_adjacency(location, expected, world);
  }

  {
    // Upper-right corner location
    Location location(0,5);
    std::vector<Location> expected =
      {
        {0,4}, {0,5},

        {1,4}, {1,5}
      };
    check_adjacency(location, expected, world);
  }

  {
    // Bottom-left corner location
    Location location(5,0);
    std::vector<Location> expected =
      {
        {4,0}, {4,1},

        {5,0}, {5,1}
      };
    check_adjacency(location, expected, world);
  }

  {
    // Bottom-right corner location
    Location location(5,5);
    std::vector<Location> expected =
      {
        {4,4}, {4,5},

        {5,4}, {5,5}
      };
    check_adjacency(location, expected, world);
  }
}

}
