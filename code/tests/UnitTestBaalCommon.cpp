#include "BaalCommon.hpp"
#include "BaalExceptions.hpp"
#include "Engine.hpp"
#include "Util.hpp"

#include <gtest/gtest.h>

#include <string>
#include <sstream>
#include <vector>

using baal::Location;

SMART_ENUM(TestEnum, ONE, TWO, THREE, FOUR);

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
                     const std::vector<Location>& expected,
                     const baal::Engine& engine)
{
  auto adj_range = baal::get_adjacent_location_range(location, engine);
  if (static_cast<size_t>(boost::distance(adj_range)) != expected.size()) {
    return false;
  }
  return std::equal(std::begin(expected),
                    std::end(expected),
                    boost::const_begin(adj_range));
}

TEST(BaalCommon, AdjacentLocationRange)
{
  auto engine = baal::create_engine();

  {
    // Out-of-range location
    Location location(12, 12);
    EXPECT_THROW(baal::get_adjacent_location_range(location, *engine),
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
    EXPECT_TRUE(check_adjacency(location, expected, *engine));
  }

  {
    // Upper-left corner location
    Location location(0,0);
    std::vector<Location> expected =
      {
               {0,1},

        {1,0}, {1,1}
      };
    EXPECT_TRUE(check_adjacency(location, expected, *engine));
  }

  {
    // Upper-right corner location
    Location location(0,5);
    std::vector<Location> expected =
      {
        {0,4},

        {1,4}, {1,5}
      };
    EXPECT_TRUE(check_adjacency(location, expected, *engine));
  }

  {
    // Bottom-left corner location
    Location location(5,0);
    std::vector<Location> expected =
      {
        {4,0}, {4,1},

               {5,1}
      };
    EXPECT_TRUE(check_adjacency(location, expected, *engine));
  }

  {
    // Bottom-right corner location
    Location location(5,5);
    std::vector<Location> expected =
      {
        {4,4}, {4,5},

        {5,4}
      };
    EXPECT_TRUE(check_adjacency(location, expected, *engine));
  }

}

TEST(BaalCommon, split)
{
  {
    std::string input = "", sep = ":";
    std::vector<std::string> expected;
    EXPECT_EQ(expected, baal::split(input, sep));
  }

  {
    std::string input = "a", sep = ":";
    std::vector<std::string> expected = {"a"};
    EXPECT_EQ(expected, baal::split(input, sep));
  }

  {
    std::string input = "ab", sep = ":";
    std::vector<std::string> expected = {"ab"};
    EXPECT_EQ(expected, baal::split(input, sep));
  }

  {
    std::string input = "a:bc", sep = ":";
    std::vector<std::string> expected = {"a", "bc"};
    EXPECT_EQ(expected, baal::split(input, sep));
  }

  {
    std::string input = "a:bc:def", sep = ":";
    std::vector<std::string> expected = {"a", "bc", "def"};
    EXPECT_EQ(expected, baal::split(input, sep));
  }

  {
    std::string input = ":ab", sep = ":";
    std::vector<std::string> expected = {"", "ab"};
    EXPECT_EQ(expected, baal::split(input, sep));
  }

  {
    std::string input = "ab:", sep = ":";
    std::vector<std::string> expected = {"ab"};
    EXPECT_EQ(expected, baal::split(input, sep));
  }

  {
    std::string input = "aa, bbb, c", sep = ", ";
    std::vector<std::string> expected = {"aa", "bbb", "c"};
    EXPECT_EQ(expected, baal::split(input, sep));
  }
}

TEST(BaalCommon, smart_enum)
{
  using namespace baal;

  {
    std::ostringstream out;
    std::string expected("1");
    TestEnum e = ONE;
    out << e;
    EXPECT_EQ(expected, out.str());
  }

  {
    std::ostringstream out;
    std::string expected("ONE TWO THREE FOUR ");
    for (TestEnum e : iterate<TestEnum>()) {
      out << to_string(e) << " ";
    }
    EXPECT_EQ(expected, out.str());
  }

  {
    TestEnum expected = TWO;
    EXPECT_EQ(expected, from_string<TestEnum>("TWO"));
    EXPECT_EQ(expected, from_string<TestEnum>("two"));
  }

  {
    EXPECT_THROW(from_string<TestEnum>("ABCD"), UserError);
  }
}

}
