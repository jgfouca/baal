#include "Time.hpp"
#include "BaalExceptions.hpp"
#include "BaalCommon.hpp"

namespace baal {

///////////////////////////////////////////////////////////////////////////////
Time::Time()
///////////////////////////////////////////////////////////////////////////////
  : m_curr_year(STARTING_YEAR),
    m_curr_season(FIRST_SEASON_OF_YEAR)
{}

///////////////////////////////////////////////////////////////////////////////
void Time::operator++()
///////////////////////////////////////////////////////////////////////////////
{
  if (m_curr_season == LAST_SEASON_OF_YEAR) {
    m_curr_season = FIRST_SEASON_OF_YEAR;
    ++m_curr_year;
  }
  else {
    ++m_curr_season;
  }
}

///////////////////////////////////////////////////////////////////////////////
void operator++(Season& season)
///////////////////////////////////////////////////////////////////////////////
{
  Require(season != Time::LAST_SEASON_OF_YEAR, "Cannot increment: " << season);

  unsigned temp = static_cast<unsigned>(season);
  ++temp;
  season = static_cast<Season>(temp);
}

///////////////////////////////////////////////////////////////////////////////
xmlNodePtr Time::to_xml()
///////////////////////////////////////////////////////////////////////////////
{
  xmlNodePtr Time_node = xmlNewNode(nullptr, BAD_CAST "Time");

  std::ostringstream m_curr_year_oss, m_curr_season_oss;
  m_curr_year_oss << m_curr_year;
  xmlNewChild(Time_node, nullptr, BAD_CAST "m_curr_year", BAD_CAST m_curr_year_oss.str().c_str());
  switch (m_curr_season) {
    case WINTER: m_curr_season_oss << "WINTER"; break;
    case SPRING: m_curr_season_oss << "SPRING"; break;
    case SUMMER: m_curr_season_oss << "SUMMER"; break;
    case FALL:   m_curr_season_oss << "FALL";   break;
  }
  xmlNewChild(Time_node, nullptr, BAD_CAST "m_curr_season", BAD_CAST m_curr_season_oss.str().c_str());

  return Time_node;
}

}
