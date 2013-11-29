#include "Weather.hpp"
#include "BaalExceptions.hpp"
#include "WorldTile.hpp"
#include "World.hpp"

#include <cstdlib>
#include <iomanip>

namespace baal {

const unsigned Anomaly::MAX_INTENSITY;

///////////////////////////////////////////////////////////////////////////////
xmlNodePtr Climate::to_xml()
///////////////////////////////////////////////////////////////////////////////
{
  xmlNodePtr Climate_node = xmlNewNode(nullptr, BAD_CAST "Climate");

  std::ostringstream m_temperature_oss, m_precip_oss, m_wind_oss;
  for (Season s : iterate<Season>()) {
    m_temperature_oss << m_temperature[s] << " ";
    m_precip_oss << m_precip[s] << " ";
    m_wind_oss << m_wind[s] << " ";
  }

  xmlNewChild(Climate_node, nullptr, BAD_CAST "m_temperature", BAD_CAST m_temperature_oss.str().c_str());
  xmlNewChild(Climate_node, nullptr, BAD_CAST "m_precip", BAD_CAST m_precip_oss.str().c_str());
  xmlNewChild(Climate_node, nullptr, BAD_CAST "m_wind", BAD_CAST m_wind_oss.str().c_str());

  return Climate_node;
}

///////////////////////////////////////////////////////////////////////////////
Atmosphere::Atmosphere(const Climate& climate)
///////////////////////////////////////////////////////////////////////////////
  : m_temperature(climate.temperature(get_first<Season>())),
    m_dewpoint(-1u),
    m_precip(climate.precip(get_first<Season>())),
    m_pressure(NORMAL_PRESSURE),
    m_wind(climate.wind(get_first<Season>())),
    m_climate(climate)
{
  m_dewpoint = compute_dewpoint();
}

///////////////////////////////////////////////////////////////////////////////
int Atmosphere::compute_dewpoint() const
///////////////////////////////////////////////////////////////////////////////
{
  // TODO - Function of temp and precip probably
  return m_temperature - 20;
}

///////////////////////////////////////////////////////////////////////////////
bool Atmosphere::is_atmospheric(DrawMode mode)
///////////////////////////////////////////////////////////////////////////////
{
  return mode == WIND        ||
         mode == DEWPOINT    ||
         mode == TEMPERATURE ||
         mode == PRECIP    ||
         mode == PRESSURE;
}

///////////////////////////////////////////////////////////////////////////////
void Atmosphere::cycle_turn(const std::vector<std::shared_ptr<const Anomaly>>& anomalies,
                            const Location& location,
                            Season season)
///////////////////////////////////////////////////////////////////////////////
{
  // Gather all modifiers from all anomalies
  float precip_modifier = 1.0;
  int temp_modifier = 0;
  int pressure_modifier = 0;
  for (auto anomaly : anomalies) {
    precip_modifier *= anomaly->precip_effect(location);
    temp_modifier += anomaly->temp_effect(location);
    pressure_modifier += anomaly->pressure_effect(location);
  }

  m_temperature = m_climate.temperature(season) + temp_modifier;
  m_pressure    = NORMAL_PRESSURE + pressure_modifier;
  m_precip    = m_climate.precip(season) * precip_modifier;

  m_dewpoint = compute_dewpoint();

  // TODO: Need to compute wind speed changes due to pressure
  m_wind = m_climate.wind(season);
}

///////////////////////////////////////////////////////////////////////////////
xmlNodePtr Atmosphere::to_xml()
///////////////////////////////////////////////////////////////////////////////
{
  xmlNodePtr Atmosphere_node = xmlNewNode(nullptr, BAD_CAST "Atmosphere");

  std::ostringstream m_temperature_oss, m_dewpoint_oss, m_precip_oss, m_pressure_oss, m_wind_oss;
  m_temperature_oss << m_temperature;
  m_dewpoint_oss << m_dewpoint;
  m_precip_oss << m_precip;
  m_pressure_oss << m_pressure;
  m_wind_oss << m_wind;
  xmlNewChild(Atmosphere_node, nullptr, BAD_CAST "m_temperature", BAD_CAST m_temperature_oss.str().c_str());
  xmlNewChild(Atmosphere_node, nullptr, BAD_CAST "m_dewpoint", BAD_CAST m_dewpoint_oss.str().c_str());
  xmlNewChild(Atmosphere_node, nullptr, BAD_CAST "m_precip", BAD_CAST m_precip_oss.str().c_str());
  xmlNewChild(Atmosphere_node, nullptr, BAD_CAST "m_pressure", BAD_CAST m_pressure_oss.str().c_str());
  xmlNewChild(Atmosphere_node, nullptr, BAD_CAST "m_wind", BAD_CAST m_wind_oss.str().c_str());

  return Atmosphere_node;
}

/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
Anomaly::Anomaly(AnomalyCategory category,
                 int intensity,
                 const Location& location,
                 unsigned world_area)
///////////////////////////////////////////////////////////////////////////////
  : m_category(category),
    m_intensity(intensity),
    m_location(location),
    m_world_area(world_area)
{}

///////////////////////////////////////////////////////////////////////////////
std::shared_ptr<const Anomaly> Anomaly::generate_anomaly(AnomalyCategory category,
                                                         const Location& location,
                                                         const World& world)
///////////////////////////////////////////////////////////////////////////////
{
  const int intensity = GENERATE_ANOMALY_INTENSITY_FUNC();
  const unsigned area = world.height() * world.width();

  if (intensity != 0) {
    return std::shared_ptr<const Anomaly>(new Anomaly(category, intensity, location, area));
  }
  else {
    return nullptr;
  }
}

///////////////////////////////////////////////////////////////////////////////
float Anomaly::precip_effect(const Location& location) const
///////////////////////////////////////////////////////////////////////////////
{
  // TODO - In all effect methods, need a more sophisticated model for
  // determining an anomaly's effect on a location. For the moment, anomalies
  // only affect the immediate location.

  if (m_category != PRECIP_ANOMALY || m_location != location) {
    return 1.0; // no effect
  }
  else {
    return PRECIP_CHANGE_FUNC(m_intensity);
  }
}

///////////////////////////////////////////////////////////////////////////////
int Anomaly::temp_effect(const Location& location) const
///////////////////////////////////////////////////////////////////////////////
{
  if (m_category != TEMPERATURE_ANOMALY || m_location != location) {
    return 0; // no effect
  }
  else {
    return TEMPERATURE_CHANGE_FUNC(m_intensity);
  }
}

///////////////////////////////////////////////////////////////////////////////
int Anomaly::pressure_effect(const Location& location) const
///////////////////////////////////////////////////////////////////////////////
{
  if (m_category != PRESSURE_ANOMALY || m_location != location) {
    return 0; // no effect
  }
  else {
    return PRESSURE_CHANGE_FUNC(m_intensity);
  }
}

///////////////////////////////////////////////////////////////////////////////
xmlNodePtr Anomaly::to_xml() const
///////////////////////////////////////////////////////////////////////////////
{
  xmlNodePtr Anomaly_node = xmlNewNode(nullptr, BAD_CAST "Anomaly");

  // AnomalyCategory m_category;
  // unsigned        m_intensity;
  // Location        m_location;
  // unsigned        m_world_area;

  std::ostringstream m_category_oss, m_intensity_oss;
  m_category_oss << m_category;
  xmlNewChild(Anomaly_node, nullptr, BAD_CAST "m_category", BAD_CAST m_category_oss.str().c_str());

  m_intensity_oss << m_intensity;
  xmlNewChild(Anomaly_node, nullptr, BAD_CAST "m_intensity", BAD_CAST m_intensity_oss.str().c_str());

  xmlNodePtr Location_node = xmlNewNode(nullptr, BAD_CAST "Location");
  std::ostringstream row_oss, col_oss;
  row_oss << m_location.row;
  col_oss << m_location.col;
  xmlNewChild(Location_node, nullptr, BAD_CAST "row", BAD_CAST row_oss.str().c_str());
  xmlNewChild(Location_node, nullptr, BAD_CAST "col", BAD_CAST col_oss.str().c_str());

  std::ostringstream m_world_area_oss;
  m_world_area_oss << m_world_area;
  xmlNewChild(Anomaly_node, nullptr, BAD_CAST "m_world_area", BAD_CAST m_world_area_oss.str().c_str());

  return Anomaly_node;
}

///////////////////////////////////////////////////////////////////////////////
std::ostream& operator<<(std::ostream& out, Wind const& wind)
///////////////////////////////////////////////////////////////////////////////
{
  out << std::setw(3) << std::left << wind.m_direction;
  out << std::setw(3) << std::right << wind.m_speed;
  return out;
}

///////////////////////////////////////////////////////////////////////////////
std::istream& operator>>(std::istream& in, Wind& wind)
///////////////////////////////////////////////////////////////////////////////
{
  in >> std::setw(3) >> std::left >> wind.m_direction;
  in >> std::setw(3) >> std::right >> wind.m_speed;
  return in;
}

}
