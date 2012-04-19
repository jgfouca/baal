#ifndef Time_hpp
#define Time_hpp

#include <string>
#include <iosfwd>
#include <utility>
#include <libxml/parser.h>

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
class Time
{
 public:
  Time();

  void operator++();

  Season season() const { return m_curr_season; }

  unsigned year() const { return m_curr_year; }

  xmlNodePtr to_xml();

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
