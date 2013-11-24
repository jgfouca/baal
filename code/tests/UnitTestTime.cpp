#include "Time.hpp"

#include <gtest/gtest.h>

namespace {

TEST(Time, Basic)
{
  using namespace baal;

  Time time;

  EXPECT_EQ(time.season(), get_first<Season>());
  EXPECT_EQ(time.year(), Time::STARTING_YEAR);

  for (Season s : iterate<Season>()) {
    ++time;
    (void) s;
  }

  EXPECT_EQ(time.season(), get_first<Season>());
  EXPECT_EQ(time.year(), Time::STARTING_YEAR + 1);

  ++time;
  Season next_season = get_first<Season>();
  ++next_season;
  EXPECT_EQ(time.season(), next_season);
}


}
