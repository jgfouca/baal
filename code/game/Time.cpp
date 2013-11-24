#include "Time.hpp"
#include "BaalExceptions.hpp"
#include "BaalCommon.hpp"

namespace baal {

const unsigned Time::STARTING_YEAR;

///////////////////////////////////////////////////////////////////////////////
Time::Time()
///////////////////////////////////////////////////////////////////////////////
  : m_curr_year(STARTING_YEAR),
    m_curr_season(get_first<Season>())
{}

///////////////////////////////////////////////////////////////////////////////
void Time::operator++()
///////////////////////////////////////////////////////////////////////////////
{
  if (m_curr_season == get_last<Season>()) {
    m_curr_season = get_first<Season>();
    ++m_curr_year;
  }
  else {
    ++m_curr_season;
  }
}

///////////////////////////////////////////////////////////////////////////////
xmlNodePtr Time::to_xml()
///////////////////////////////////////////////////////////////////////////////
{
  xmlNodePtr Time_node = xmlNewNode(nullptr, BAD_CAST "Time");

  std::ostringstream m_curr_year_oss, m_curr_season_oss;
  m_curr_year_oss << m_curr_year;
  xmlNewChild(Time_node, nullptr, BAD_CAST "m_curr_year", BAD_CAST m_curr_year_oss.str().c_str());
  m_curr_season_oss << m_curr_season;
  xmlNewChild(Time_node, nullptr, BAD_CAST "m_curr_season", BAD_CAST m_curr_season_oss.str().c_str());

  return Time_node;
}

}
