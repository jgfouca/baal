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
void Time::draw_text(std::ostream& out) const
///////////////////////////////////////////////////////////////////////////////
{
  std::pair<std::string, const char*> info = season_info(m_curr_season);
  std::string name = info.first;
  const char* color = info.second;
  out << BOLD_COLOR << color << name << ", Year " << m_curr_year << CLEAR_ALL << "\n";
}

///////////////////////////////////////////////////////////////////////////////
std::pair<std::string, const char*> Time::season_info(Season season)
///////////////////////////////////////////////////////////////////////////////
{
  switch(season) {
  case WINTER:
    return std::pair<std::string, const char*>("Winter", BLUE);
    break;
  case SPRING:
    return std::pair<std::string, const char*>("Spring", GREEN);
    break;
  case SUMMER:
    return std::pair<std::string, const char*>("Summer", RED);
    break;
  case FALL:
    return std::pair<std::string, const char*>("Fall", YELLOW);
    break;
  default:
    Require(false, "Unknown season: " << season);
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
  xmlNodePtr Time_node = xmlNewNode(NULL, BAD_CAST "Time");

  std::ostringstream m_curr_year_oss, m_curr_season_oss;
  m_curr_year_oss << m_curr_year;
  xmlNewChild(Time_node, NULL, BAD_CAST "m_curr_year", BAD_CAST m_curr_year_oss.str().c_str());
  switch (m_curr_season) {
    case WINTER: m_curr_season_oss << "WINTER"; break;
    case SPRING: m_curr_season_oss << "SPRING"; break;
    case SUMMER: m_curr_season_oss << "SUMMER"; break;
    case FALL:   m_curr_season_oss << "FALL";   break;
  }
  xmlNewChild(Time_node, NULL, BAD_CAST "m_curr_season", BAD_CAST m_curr_season_oss.str().c_str());

  return Time_node;
}

}
