#ifndef Time_hpp
#define Time_hpp

#include <string>
#include <iosfwd>
#include <utility>
#include <libxml/parser.h>

#include "BaalCommon.hpp"

SMART_ENUM(Season, WINTER, SPRING, SUMMER, FALL);

namespace baal {

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
  static const unsigned STARTING_YEAR = 0;

private:
  unsigned m_curr_year;
  Season   m_curr_season;
};

}

#endif
