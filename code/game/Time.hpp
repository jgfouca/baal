#ifndef Time_hpp
#define Time_hpp

#include "Drawable.hpp"

#include <string>
#include <iosfwd>
#include <utility>

namespace baal {

enum Season
{
  WINTER = 0,
  SPRING = 1,
  SUMMER = 2,
  FALL   = 3
};

/**
 * Encapsulates how time elapses in the system.
 */
class Time : public Drawable
{
 public:
  Time();

  void operator++();

  virtual void draw_text(std::ostream& out) const;

  virtual void draw_graphics() const { /* TODO */ }

  static std::pair<std::string, const char*> season_info(Season season);

  // Constants
  static const unsigned STARTING_YEAR         = 0;
  static const Season   FIRST_SEASON_OF_YEAR  = WINTER;
  static const Season   LAST_SEASON_OF_YEAR   = FALL;

private:
  unsigned m_curr_year;
  Season   m_curr_season;
};

void operator++(Season& season);

}

#endif
