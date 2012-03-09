#include "BaalCommon.hpp"
#include "BaalExceptions.hpp"
#include "Util.hpp"

#include <gtest/gtest.h>

#include <string>
#include <sstream>
#include <vector>

using baal::Location;

namespace {

TEST(BaalCommon, LocationBasics)
{
  // Test Location class API and related free functions

  Location l1, l2;
  EXPECT_EQ(l1, l2);
  EXPECT_EQ(l1.row, baal::INVALID);
  EXPECT_EQ(l1.col, baal::INVALID);
  EXPECT_FALSE(baal::is_valid(l1));

  Location l3(2, 3);
  EXPECT_EQ(l3.row, 2);
  EXPECT_EQ(l3.col, 3);
  EXPECT_NE(l3, l1);
  EXPECT_TRUE(baal::is_valid(l3));

  Location l4("2,3");
  Location l5(" 2 , 3");
  EXPECT_EQ(l3, l4);
  EXPECT_EQ(l3, l5);
  EXPECT_THROW(Location("2 3"), baal::UserError);
  EXPECT_THROW(Location("23"), baal::UserError);
  EXPECT_THROW(Location(""), baal::UserError);
  EXPECT_THROW(Location("2,3 "), baal::UserError);

  std::ostringstream out;
  out << l4;
  EXPECT_EQ(out.str(), std::string("2,3"));
}

bool check_adjacency(Location location,
                     const std::vector<Location>& expected)
{
  auto adj_range = baal::get_adjacent_location_range(location);
  if (static_cast<size_t>(boost::distance(adj_range)) != expected.size()) {
    return false;
  }
  return std::equal(std::begin(expected),
                    std::end(expected),
                    boost::const_begin(adj_range));
}

TEST(BaalCommon, AdjacentLocationRange)
{
  baal::setup_singletons();

  {
    // Out-of-range location
    Location location(12, 12);
    EXPECT_THROW(baal::get_adjacent_location_range(location),
                 baal::ProgramError);
  }

  {
    // Central location
    Location location(3,3);
    std::vector<Location> expected =
      {
        {2,2}, {2,3}, {2,4},

        {3,2},        {3,4},

        {4,2}, {4,3}, {4,4}
      };
    EXPECT_TRUE(check_adjacency(location, expected));
  }

  {
    // Upper-left corner location
    Location location(0,0);
    std::vector<Location> expected =
      {
               {0,1},

        {1,0}, {1,1}
      };
    EXPECT_TRUE(check_adjacency(location, expected));
  }

  {
    // Upper-right corner location
    Location location(0,5);
    std::vector<Location> expected =
      {
        {0,4},

        {1,4}, {1,5}
      };
    EXPECT_TRUE(check_adjacency(location, expected));
  }

  {
    // Bottom-left corner location
    Location location(5,0);
    std::vector<Location> expected =
      {
        {4,0}, {4,1},

               {5,1}
      };
    EXPECT_TRUE(check_adjacency(location, expected));
  }

  {
    // Bottom-right corner location
    Location location(5,5);
    std::vector<Location> expected =
      {
        {4,4}, {4,5},

        {5,4}
      };
    EXPECT_TRUE(check_adjacency(location, expected));
  }

}

}
