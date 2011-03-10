#include "Time.hpp"
#include "BaalExceptions.hpp"
#include "BaalCommon.hpp"

using namespace baal;

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
void baal::operator++(Season& season)
///////////////////////////////////////////////////////////////////////////////
{
  Require(season != Time::LAST_SEASON_OF_YEAR, "Cannot increment: " << season);

  unsigned temp = static_cast<unsigned>(season);
  ++temp;
  season = static_cast<Season>(temp);
}
