#include "Weather.hpp"
#include "BaalExceptions.hpp"
#include "WorldTile.hpp"
#include "World.hpp"

#include <cstdlib>
#include <iomanip>

using namespace baal;

namespace {

///////////////////////////////////////////////////////////////////////////////
std::string direction_str(Direction direction)
///////////////////////////////////////////////////////////////////////////////
{
  switch(direction) {
  case N:   return "N";
  case NNE: return "NNE";
  case NE:  return "NE";
  case ENE: return "ENE";
  case E:   return "E";
  case ESE: return "ESE";
  case SE:  return "SE";
  case SSE: return "SSE";
  case S:   return "S";
  case SSW: return "SSW";
  case SW:  return "SW";
  case WSW: return "WSW";
  case W:   return "W";
  case WNW: return "WNW";
  case NW:  return "NW";
  case NNW: return "NNW";
  default:
    Require(false, "Should never make it here");
  }
}

///////////////////////////////////////////////////////////////////////////////
void draw_wind(std::ostream& out, const Wind& wind)
///////////////////////////////////////////////////////////////////////////////
{
  const unsigned max_direction_len = 3;
  out << std::setw(max_direction_len) << std::left
      << direction_str(wind.m_direction);
  out << std::right;

  unsigned speed = wind.m_speed;
  const char* color = "";
  if (speed < 10) {
    color = GREEN;
  }
  else if (speed < 20) {
    color = YELLOW;
  }
  else {
    color = RED;
  }

  out << BOLD_COLOR << color       // set color and bold text
      << std::setw(WorldTile::TILE_TEXT_WIDTH - max_direction_len) << speed
      << CLEAR_ALL;                // clear color and boldness
}

///////////////////////////////////////////////////////////////////////////////
void draw_dewpoint(std::ostream& out, int dewpoint)
///////////////////////////////////////////////////////////////////////////////
{
  const char* color = "";
  if (dewpoint < 32) {
    color = RED;
  }
  else if (dewpoint < 55) {
    color = YELLOW;
  }
  else {
    color = GREEN;
  }

  out << BOLD_COLOR << color       // set color and bold text
      << std::setw(WorldTile::TILE_TEXT_WIDTH) << dewpoint //value
      << CLEAR_ALL;                // clear color and boldness
}

///////////////////////////////////////////////////////////////////////////////
void draw_temperature(std::ostream& out, int temperature)
///////////////////////////////////////////////////////////////////////////////
{
  const char* color = "";
  if (temperature < 32) {
    color = BLUE;
  }
  else if (temperature < 80) {
    color = YELLOW;
  }
  else {
    color = RED;
  }

  out << BOLD_COLOR << color       // set color and bold text
      << std::setw(WorldTile::TILE_TEXT_WIDTH) << temperature  // print value
      << CLEAR_ALL;                // clear color and boldness
}

///////////////////////////////////////////////////////////////////////////////
void draw_pressure(std::ostream& out, unsigned pressure)
///////////////////////////////////////////////////////////////////////////////
{
  const char* color = "";
  if (pressure < 975) {
    color = GREEN;
  }
  else if (pressure < 1025) {
    color = YELLOW;
  }
  else {
    color = RED;
  }

  out << BOLD_COLOR << color   // set color and bold text
      << std::setw(WorldTile::TILE_TEXT_WIDTH) << pressure  // print value
      << CLEAR_ALL;            // clear color and boldness
}

///////////////////////////////////////////////////////////////////////////////
void draw_rainfall(std::ostream& out, float rainfall)
///////////////////////////////////////////////////////////////////////////////
{
  const char* color = "";
  if (rainfall > 10) {
    color = GREEN;
  }
  else if (rainfall > 2) {
    color = YELLOW;
  }
  else {
    color = RED;
  }

  out << BOLD_COLOR << color   // set color and bold text
      << std::setw(WorldTile::TILE_TEXT_WIDTH)
      << std::setprecision(3) << rainfall  // print value
      << CLEAR_ALL;            // clear color and boldness
}

} // empty namespace

/*****************************************************************************/
///////////////////////////////////////////////////////////////////////////////
xmlNodePtr Climate::to_xml()
///////////////////////////////////////////////////////////////////////////////
{
  xmlNodePtr Climate_node = xmlNewNode(NULL, BAD_CAST "Climate");

  std::ostringstream m_temperature_oss, m_rainfall_oss;
  m_temperature_oss << m_temperature;
  m_rainfall_oss << m_rainfall;
  xmlNewChild(Climate_node, NULL, BAD_CAST "m_temperature", BAD_CAST m_temperature_oss.str().c_str());
  xmlNewChild(Climate_node, NULL, BAD_CAST "m_rainfall", BAD_CAST m_rainfall_oss.str().c_str());

  xmlNodePtr Wind_node;
  Wind_node = xmlNewNode(NULL, BAD_CAST "Wind");
  std::ostringstream m_speed_oss;
  m_speed_oss << m_wind.m_speed;
  xmlNewChild(Wind_node, NULL, BAD_CAST "m_speed", BAD_CAST m_speed_oss.str().c_str());
  xmlNewChild(Wind_node, NULL, BAD_CAST "m_direction", BAD_CAST direction_str(m_wind.m_direction).c_str());

  xmlAddChild(Climate_node, Wind_node);

  return Climate_node;
}

/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
Atmosphere::Atmosphere(const Climate& climate)
///////////////////////////////////////////////////////////////////////////////
  : m_temperature(climate.temperature(Time::LAST_SEASON_OF_YEAR)),
    m_rainfall(climate.rainfall(Time::LAST_SEASON_OF_YEAR)),
    m_pressure(NORMAL_PRESSURE),
    m_wind(climate.wind(Time::LAST_SEASON_OF_YEAR)),
    m_climate(climate)
{
  m_dewpoint = compute_dewpoint();
}

///////////////////////////////////////////////////////////////////////////////
void Atmosphere::draw_text(std::ostream& out) const
///////////////////////////////////////////////////////////////////////////////
{
  switch (s_draw_mode) {
  case WIND:
    draw_wind(out, m_wind);
    break;
  case DEWPOINT:
    draw_dewpoint(out, m_dewpoint);
    break;
  case TEMPERATURE:
    draw_temperature(out, m_temperature);
    break;
  case PRESSURE:
    draw_pressure(out, m_pressure);
    break;
  case RAINFALL:
    draw_rainfall(out, m_rainfall);
    break;
  default:
    Require(false, "Should not draw atmosphere in mode: " << s_draw_mode);
  }
}

///////////////////////////////////////////////////////////////////////////////
int Atmosphere::compute_dewpoint() const
///////////////////////////////////////////////////////////////////////////////
{
  // TODO - Function of temp and rainfall probably
  return m_temperature - 20;
}

///////////////////////////////////////////////////////////////////////////////
bool Atmosphere::is_atmospheric(DrawMode mode)
///////////////////////////////////////////////////////////////////////////////
{
  return mode == WIND        ||
         mode == DEWPOINT    ||
         mode == TEMPERATURE ||
         mode == RAINFALL    ||
         mode == PRESSURE;
}

///////////////////////////////////////////////////////////////////////////////
void Atmosphere::cycle_turn(const std::vector<const Anomaly*>& anomalies,
                            const Location& location,
                            Season season)
///////////////////////////////////////////////////////////////////////////////
{
  // Gather all modifiers from all anomalies
  float precip_modifier = 1.0;
  int temp_modifier = 0;
  int pressure_modifier = 0;
  for (std::vector<const Anomaly*>::const_iterator itr = anomalies.begin();
       itr != anomalies.end();
       ++itr) {
    const Anomaly* anomaly = *itr;
    precip_modifier *= anomaly->precip_effect(location);
    temp_modifier += anomaly->temp_effect(location);
    pressure_modifier += anomaly->pressure_effect(location);
  }

  m_temperature = m_climate.temperature(season) + temp_modifier;
  m_pressure    = NORMAL_PRESSURE + pressure_modifier;
  m_rainfall    = m_climate.rainfall(season) * precip_modifier;

  m_dewpoint = compute_dewpoint();

  // TODO: Need to compute wind speed changes due to pressure
  m_wind = m_climate.wind(season);
}

///////////////////////////////////////////////////////////////////////////////
xmlNodePtr Atmosphere::to_xml()
///////////////////////////////////////////////////////////////////////////////
{
  xmlNodePtr Atmosphere_node = xmlNewNode(NULL, BAD_CAST "Atmosphere");

  std::ostringstream m_temperature_oss, m_dewpoint_oss, m_rainfall_oss, m_pressure_oss;
  m_temperature_oss << m_temperature;
  m_dewpoint_oss << m_dewpoint;
  m_rainfall_oss << m_rainfall;
  m_pressure_oss << m_pressure;
  xmlNewChild(Atmosphere_node, NULL, BAD_CAST "m_temperature", BAD_CAST m_temperature_oss.str().c_str());
  xmlNewChild(Atmosphere_node, NULL, BAD_CAST "m_dewpoint", BAD_CAST m_dewpoint_oss.str().c_str());
  xmlNewChild(Atmosphere_node, NULL, BAD_CAST "m_rainfall", BAD_CAST m_rainfall_oss.str().c_str());
  xmlNewChild(Atmosphere_node, NULL, BAD_CAST "m_pressure", BAD_CAST m_pressure_oss.str().c_str());

  xmlNodePtr Wind_node;
  Wind_node = xmlNewNode(NULL, BAD_CAST "Wind");
  std::ostringstream m_speed_oss;
  m_speed_oss << m_wind.m_speed;
  xmlNewChild(Wind_node, NULL, BAD_CAST "m_speed", BAD_CAST m_speed_oss.str().c_str());
  xmlNewChild(Wind_node, NULL, BAD_CAST "m_direction", BAD_CAST direction_str(m_wind.m_direction).c_str());

  xmlAddChild(Atmosphere_node, Wind_node);

  return Atmosphere_node;
}

/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
Anomaly::Anomaly(AnomalyCategory category,
                 Type type,
                 unsigned intensity,
                 const Location& location,
                 unsigned world_area)
///////////////////////////////////////////////////////////////////////////////
  : m_category(category),
    m_type(type),
    m_intensity(intensity),
    m_location(location),
    m_world_area(world_area)
{}

///////////////////////////////////////////////////////////////////////////////
const Anomaly* Anomaly::generate_anomaly(AnomalyCategory category,
                                         const Location& location,
                                         const World& world)
///////////////////////////////////////////////////////////////////////////////
{
  // Generate random float 0.0 -> 100.0
  float roll = (static_cast<float>(std::rand()) /
                static_cast<float>(RAND_MAX)) * 100;

  unsigned area = world.height() * world.width();

  // Translate roll into an anomaly
  if (roll <= .10) {
    return new Anomaly(category, BELOW, 3, location, area);
  }
  else if (roll <= 1.0) {
    return new Anomaly(category, BELOW, 2, location, area);
  }
  else if (roll <= 3.0) {
    return new Anomaly(category, BELOW, 1, location, area);
  }
  else if (roll >= 99.9) {
    return new Anomaly(category, ABOVE, 3, location, area);
  }
  else if (roll >= 99.0) {
    return new Anomaly(category, ABOVE, 2, location, area);
  }
  else if (roll >= 97.0) {
    return new Anomaly(category, ABOVE, 1, location, area);
  }
  else {
    return NULL;
  }
}

///////////////////////////////////////////////////////////////////////////////
float Anomaly::precip_effect(const Location& location) const
///////////////////////////////////////////////////////////////////////////////
{
  // TODO - In all effect methods, need a more sophisticated model for
  // determining an anomaly's effect on a location. For the moment, anomalies
  // only affect the immediate location.

  if (m_category != RAINFALL || m_location != location) {
    return 1.0; // no effect
  }
  else {
    int multiplier = (m_type == ABOVE) ? 1 : -1;
    return 1.0 + (PRECIP_CHANGE_PER_LEVEL * multiplier * m_intensity);
  }
}

///////////////////////////////////////////////////////////////////////////////
int Anomaly::temp_effect(const Location& location) const
///////////////////////////////////////////////////////////////////////////////
{
  if (m_category != TEMPERATURE || m_location != location) {
    return 0; // no effect
  }
  else {
    int multiplier = (m_type == ABOVE) ? 1 : -1;
    return TEMP_CHANGE_PER_LEVEL * multiplier * m_intensity;
  }
}

///////////////////////////////////////////////////////////////////////////////
int Anomaly::pressure_effect(const Location& location) const
///////////////////////////////////////////////////////////////////////////////
{
  if (m_category != PRESSURE || m_location != location) {
    return 0; // no effect
  }
  else {
    int multiplier = (m_type == ABOVE) ? 1 : -1;
    return PRESSURE_CHANGE_PER_LEVEL * multiplier * m_intensity;
  }
}

///////////////////////////////////////////////////////////////////////////////
void Anomaly::draw_text(std::ostream& out) const
///////////////////////////////////////////////////////////////////////////////
{
  out << "Level: " << type_to_str(m_type) << m_intensity << " "
      << category_to_str(m_category) << " anomaly at location: " << m_location;
}

///////////////////////////////////////////////////////////////////////////////
std::string Anomaly::type_to_str(Type type)
///////////////////////////////////////////////////////////////////////////////
{
  switch (type) {
  case ABOVE:
    return "+";
  case BELOW:
    return "-";
  default:
    Require(false, "Unhandled anomaly type: " << type);
  }
}

///////////////////////////////////////////////////////////////////////////////
std::string Anomaly::category_to_str(AnomalyCategory category)
///////////////////////////////////////////////////////////////////////////////
{
  switch (category) {
  case TEMPERATURE:
    return "temperature";
  case RAINFALL:
    return "rainfall";
  case PRESSURE:
    return "pressure";
  default:
    Require(false, "Unhandled anomaly category: " << category);
  }
}


///////////////////////////////////////////////////////////////////////////////
Anomaly::AnomalyCategory& baal::operator++(Anomaly::AnomalyCategory& category)
///////////////////////////////////////////////////////////////////////////////
{
  Require(category != Anomaly::LAST, "Iterating off end of anomalies");

  int i = static_cast<int>(category);
  ++i;
  return category = static_cast<Anomaly::AnomalyCategory>(i);
}

///////////////////////////////////////////////////////////////////////////////
xmlNodePtr Anomaly::to_xml() const
///////////////////////////////////////////////////////////////////////////////
{
  xmlNodePtr Anomaly_node = xmlNewNode(NULL, BAD_CAST "Anomaly");

  // AnomalyCategory m_category;
  // Type            m_type;
  // unsigned        m_intensity;
  // Location        m_location;
  // unsigned        m_world_area;

  xmlNewChild(Anomaly_node, NULL, BAD_CAST "m_category", BAD_CAST category_to_str(m_category).c_str());

  xmlNewChild(Anomaly_node, NULL, BAD_CAST "m_type", BAD_CAST type_to_str(m_type).c_str());

  std::ostringstream m_intensity_oss;
  m_intensity_oss << m_intensity;
  xmlNewChild(Anomaly_node, NULL, BAD_CAST "m_intensity", BAD_CAST m_intensity_oss.str().c_str());

  xmlNodePtr Location_node = xmlNewNode(NULL, BAD_CAST "Location");
  std::ostringstream row_oss, col_oss;
  row_oss << m_location.row;
  col_oss << m_location.col;
  xmlNewChild(Location_node, NULL, BAD_CAST "row", BAD_CAST row_oss.str().c_str());
  xmlNewChild(Location_node, NULL, BAD_CAST "col", BAD_CAST col_oss.str().c_str());

  std::ostringstream m_world_area_oss;
  m_world_area_oss << m_world_area;
  xmlNewChild(Anomaly_node, NULL, BAD_CAST "m_world_area", BAD_CAST m_world_area_oss.str().c_str());

  return Anomaly_node;
}
