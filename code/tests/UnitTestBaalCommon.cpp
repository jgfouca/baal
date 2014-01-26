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
    std::string expected("ONE");
    TestEnum e = ONE;
    out << e;
    EXPECT_EQ(expected, out.str());
  }

  {
    EXPECT_EQ(baal::size<TestEnum>(), 4);
  }

  {
    std::ostringstream out, out2;
    std::string expected("ONE TWO THREE FOUR ");
    for (TestEnum e : iterate<TestEnum>()) {
      out << to_string(e) << " ";
      out2 << e << " ";
    }
    EXPECT_EQ(expected, out.str());
    EXPECT_EQ(expected, out2.str());
  }

  {
    TestEnum expected = TWO;
    EXPECT_EQ(expected, from_string<TestEnum>("TWO"));
    EXPECT_EQ(expected, from_string<TestEnum>("two"));
  }

  {
    EXPECT_THROW(from_string<TestEnum>("ABCD"), UserError);
  }

  {
    TestEnum first = get_first<TestEnum>();
    TestEnum last  = get_last<TestEnum>();
    EXPECT_EQ(first, ONE);
    EXPECT_EQ(last, FOUR);
  }
}

TEST(BaalCommon, location_iterator)
{
  using namespace baal;

  {
    static const unsigned ROW = 2;
    static const unsigned COL = 2;

    LocationIterator loc_itr(ROW, COL);

    EXPECT_EQ(std::distance(loc_itr, loc_itr.end()), 4u);

    EXPECT_EQ(*loc_itr++, Location(0, 0));
    EXPECT_EQ(*loc_itr++, Location(0, 1));
    EXPECT_EQ(*loc_itr++, Location(1, 0));
    EXPECT_EQ(*loc_itr,   Location(1, 1));
  }

  {
    static const unsigned MIN_ROW = 2;
    static const unsigned MIN_COL = 2;
    static const unsigned MAX_ROW = 4;
    static const unsigned MAX_COL = 4;

    LocationIterator loc_itr(MIN_ROW, MIN_COL, MAX_ROW, MAX_COL);

    EXPECT_EQ(std::distance(loc_itr, loc_itr.end()), 4u);

    EXPECT_EQ(*loc_itr++, Location(2, 2));
    EXPECT_EQ(*loc_itr++, Location(2, 3));
    EXPECT_EQ(*loc_itr++, Location(3, 2));
    EXPECT_EQ(*loc_itr,   Location(3, 3));
  }

  {
    static const unsigned RADIUS = 1;

    LocationIterator loc_itr(Location(1, 1), RADIUS);

    EXPECT_EQ(std::distance(loc_itr, loc_itr.end()), 9u);

    EXPECT_EQ(*loc_itr++, Location(0, 0));
    EXPECT_EQ(*loc_itr++, Location(0, 1));
    EXPECT_EQ(*loc_itr++, Location(0, 2));
    EXPECT_EQ(*loc_itr++, Location(1, 0));
    EXPECT_EQ(*loc_itr++, Location(1, 1));
    EXPECT_EQ(*loc_itr++, Location(1, 2));
    EXPECT_EQ(*loc_itr++, Location(2, 0));
    EXPECT_EQ(*loc_itr++, Location(2, 1));
    EXPECT_EQ(*loc_itr++, Location(2, 2));
  }
}

}
