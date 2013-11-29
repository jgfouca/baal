#include "WorldTile.hpp"
#include "World.hpp"
#include "City.hpp"
#include "BaalExceptions.hpp"
#include "Geology.hpp"
#include "Weather.hpp"
#include "Engine.hpp"
#include "PlayerAI.hpp"

namespace baal {

///////////////////////////////////////////////////////////////////////////////
Yield::Yield(float food, float prod)
///////////////////////////////////////////////////////////////////////////////
  : m_food(food),
    m_prod(prod)
{
  // Tiles should either be food-yielding or production-yielding, not both
  Require(food == 0 || prod == 0, "Tile cannot yield food and production");
  Require(food >= 0, "Cannot have negative yields");
  Require(prod >= 0, "Cannot have negative yields");
}

///////////////////////////////////////////////////////////////////////////////
xmlNodePtr Yield::to_xml()
///////////////////////////////////////////////////////////////////////////////
{
  xmlNodePtr Yield_node = xmlNewNode(nullptr, BAD_CAST "Yield");
  std::ostringstream m_food_oss, m_prod_oss;
  m_food_oss << m_food;
  m_prod_oss << m_prod;
  xmlNewChild(Yield_node, nullptr, BAD_CAST "m_food", BAD_CAST m_food_oss.str().c_str());
  xmlNewChild(Yield_node, nullptr, BAD_CAST "m_prod", BAD_CAST m_prod_oss.str().c_str());

  return Yield_node;
}
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
WorldTile::WorldTile(Location location, Yield yield, Climate& climate, Geology& geology)
///////////////////////////////////////////////////////////////////////////////
  : m_location(location),
    m_base_yield(yield),
    m_climate(climate),
    m_geology(geology),
    m_atmosphere(climate),
    m_worked(false),
    m_casted_spells()
{}

///////////////////////////////////////////////////////////////////////////////
WorldTile::~WorldTile()
///////////////////////////////////////////////////////////////////////////////
{
  delete &m_climate;
  delete &m_geology;
}

///////////////////////////////////////////////////////////////////////////////
void WorldTile::cycle_turn(const std::vector<std::shared_ptr<const Anomaly>>& anomalies,
                           const Location& location,
                           Season season)
///////////////////////////////////////////////////////////////////////////////
{
  m_geology.cycle_turn();
  m_atmosphere.cycle_turn(anomalies, location, season);
  m_worked = false;
  m_casted_spells.clear();
}

///////////////////////////////////////////////////////////////////////////////
void WorldTile::work()
///////////////////////////////////////////////////////////////////////////////
{
  Require(!m_worked, "Tile already being worked");
  m_worked = true;
}

///////////////////////////////////////////////////////////////////////////////
void WorldTile::cast(const std::string& spell)
///////////////////////////////////////////////////////////////////////////////
{
  Require(!contains(m_casted_spells, spell), "Duplicate: " << spell);
  m_casted_spells.push_back(spell);
}

///////////////////////////////////////////////////////////////////////////////
bool WorldTile::already_casted(const std::string& spell) const
///////////////////////////////////////////////////////////////////////////////
{
  return contains(m_casted_spells, spell);
}

///////////////////////////////////////////////////////////////////////////////
xmlNodePtr WorldTile::to_xml()
///////////////////////////////////////////////////////////////////////////////
{
  xmlNodePtr WorldTile_node = xmlNewNode(nullptr, BAD_CAST "Tile");

  xmlAddChild(WorldTile_node, m_base_yield.to_xml());
  xmlAddChild(WorldTile_node, m_climate.to_xml());
  xmlAddChild(WorldTile_node, m_geology.to_xml());
  xmlAddChild(WorldTile_node, m_atmosphere.to_xml());

  std::ostringstream worked_oss;
  worked_oss << m_worked;
  xmlNewChild(WorldTile_node, nullptr, BAD_CAST "m_worked", BAD_CAST worked_oss.str().c_str());

  return WorldTile_node;
}

/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
OceanTile::OceanTile(Location location, unsigned depth, Climate& climate, Geology& geology)
///////////////////////////////////////////////////////////////////////////////
  : WorldTile(location, Yield(3,0), climate, geology),
    m_depth(depth),
    m_surface_temp(m_atmosphere.temperature())
{}

///////////////////////////////////////////////////////////////////////////////
void OceanTile::cycle_turn(const std::vector<std::shared_ptr<const Anomaly>>& anomalies,
                           const Location& location,
                           Season season)
///////////////////////////////////////////////////////////////////////////////
{
  WorldTile::cycle_turn(anomalies, location, season);

  // Sea temperatures retain some heat, so new sea temps have to take old
  // sea temps into account. Here, we just average season temp and prior
  // sea temp together as a very simple model.
  m_surface_temp = (m_atmosphere.temperature() + m_surface_temp) / 2;
}

///////////////////////////////////////////////////////////////////////////////
Yield OceanTile::yield() const
///////////////////////////////////////////////////////////////////////////////
{
  return m_base_yield;
}

/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
LandTile::LandTile(Location location, Yield yield, Climate& climate, Geology& geology)
///////////////////////////////////////////////////////////////////////////////
  : WorldTile(location, yield, climate, geology),
    m_hp(1.0),
    m_infra_level(0),
    m_city(nullptr)
{}

///////////////////////////////////////////////////////////////////////////////
LandTile::~LandTile()
///////////////////////////////////////////////////////////////////////////////
{
  if (m_city) {
    delete m_city;
  }
}

///////////////////////////////////////////////////////////////////////////////
void LandTile::damage(float dmg)
///////////////////////////////////////////////////////////////////////////////
{
  Require(dmg >= 0.0 && dmg <= 1.0, "Invalid value for damage: " << dmg);

  m_hp *= (1.0 - dmg);

  Require(m_hp >= 0.0 && m_hp <= 1.0, "Invariant for hp failed: " << m_hp);
}

///////////////////////////////////////////////////////////////////////////////
void LandTile::recover()
///////////////////////////////////////////////////////////////////////////////
{
  m_hp += LAND_TILE_RECOVERY_RATE;
  if (m_hp > 1.0) {
    m_hp = 1.0;
  }

  Require(m_hp > 0.0 && m_hp <= 1.0, "Invariant for hp failed: " << m_hp);
}

///////////////////////////////////////////////////////////////////////////////
void LandTile::cycle_turn(const std::vector<std::shared_ptr<const Anomaly>>& anomalies,
                          const Location& location,
                          Season season)
///////////////////////////////////////////////////////////////////////////////
{
  WorldTile::cycle_turn(anomalies, location, season);
  recover();
}

///////////////////////////////////////////////////////////////////////////////
void LandTile::build_infra()
///////////////////////////////////////////////////////////////////////////////
{
  Require(m_infra_level < LAND_TILE_MAX_INFRA, "Infra is maxed");
  Require(city() == nullptr, "Cannot build infra if there is city here");

  m_infra_level++;
}

///////////////////////////////////////////////////////////////////////////////
void LandTile::destroy_infra(unsigned num_destroyed)
///////////////////////////////////////////////////////////////////////////////
{
  Require(m_infra_level >= num_destroyed, "num_destroyed too high");

  m_infra_level -= num_destroyed;
}

///////////////////////////////////////////////////////////////////////////////
Yield LandTile::yield() const
///////////////////////////////////////////////////////////////////////////////
{
  // MODEL: Important equation that may need balancing

  return
    m_base_yield * // base
    (1 + m_infra_level) * // infra multiplier
    m_hp; // damaged tiles yield less
}

///////////////////////////////////////////////////////////////////////////////
void LandTile::place_city(City& city)
///////////////////////////////////////////////////////////////////////////////
{
  Require(infra_level() == 0, "Cannot put city on tile with infra");
  Require(supports_city(), "Tile does not support cities");
  Require(m_city == nullptr, "Tile already had city: " << city.name());
  m_city = &city;
}

///////////////////////////////////////////////////////////////////////////////
void LandTile::remove_city()
///////////////////////////////////////////////////////////////////////////////
{
  Require(m_city != nullptr, "Erroneous call to remove_city");
  m_city = nullptr;
}

/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
void MountainTile::cycle_turn(const std::vector<std::shared_ptr<const Anomaly>>& anomalies,
                              const Location& location,
                              Season season)
///////////////////////////////////////////////////////////////////////////////
{
  LandTile::cycle_turn(anomalies, location, season);

  // MODEL: Model how snowpack changes

  float precip = m_atmosphere.precip();
  int temp = m_atmosphere.temperature();

  // How much precip fell as snow?
  unsigned multiplier = 12; // 12'' of snow per 1'' of precip
  if (temp > 30) {
    if (temp < 60) {
      multiplier *= (60 - temp) / 30;
    }
    else {
      multiplier = 0;
    }
  }
  unsigned new_snow = precip * multiplier;

  float melt_pct = 0.0;
  if (temp > 30) {
    if (temp < 70) {
      melt_pct = static_cast<float>(temp - 30) / static_cast<float>(40);
    }
    else {
      melt_pct = 1.0;
    }
  }

  m_snowpack = (new_snow + m_snowpack) * melt_pct;
}

///////////////////////////////////////////////////////////////////////////////
void MountainTile::place_city(City& city)
///////////////////////////////////////////////////////////////////////////////
{
  Require(false, "Mountain tiles cannot support cities");
}

/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
Yield FoodTile::yield() const
///////////////////////////////////////////////////////////////////////////////
{
  Require (m_base_yield.m_food > 0, "Tiles with growth should yield food");

  // MODEL: Important equation that may need balancing

  // Food yielding tiles need to take soil moisture into account

  float moisture_multiplier = 1.0;

  float moisture = soil_moisture();
  if (moisture < FLOODING_THRESHOLD) {
    // Up until FLOODING_THRESHOLD times average moisture, yields benefit from
    // more moisture
    moisture_multiplier = moisture;
  }
  else if (moisture < TOTALLY_FLOODED) {
    // Yields drop quickly as soil becomes over-saturated
    moisture_multiplier = FLOODING_THRESHOLD - (moisture - FLOODING_THRESHOLD);
  }
  else {
    // Things are flooded and can't get any worse. Farmers are able to
    // salvage some fixed portion of their crops.
    moisture_multiplier = 0.25;
  }

  return LandTile::yield() * moisture_multiplier;
}

///////////////////////////////////////////////////////////////////////////////
void FoodTile::cycle_turn(const std::vector<std::shared_ptr<const Anomaly>>& anomalies,
                                     const Location& location,
                                     Season season)
///////////////////////////////////////////////////////////////////////////////
{
  LandTile::cycle_turn(anomalies, location, season);

  // MODEL: Model how precip and temp changes soil moisture

  // Get the parameters we need to make the calculation
  float precip         = m_atmosphere.precip();
  int temp             = m_atmosphere.temperature();
  float av_precip      = m_climate.precip(season);
  int av_temp          = m_climate.temperature(season);
  float prior_moisture = m_soil_moisture;

  // Precip's effect on moisture
  float seasonal_moisture = 1.0;
  seasonal_moisture *= precip / av_precip;

  // Temperature's effect on moisture. This is trickier; for now only a very
  // basic model is in place.
  const float pct_per_degree = 0.01;
  const int temp_diff = av_temp - temp;
  seasonal_moisture *= 1.0 + (pct_per_degree * temp_diff);

  // Take past moisture into account but weight current moisture more heavily
  m_soil_moisture = ((seasonal_moisture * 2) + prior_moisture) / 3;

  Require(m_soil_moisture >= 0.0 && m_soil_moisture < 100,
          "Moisture " << m_soil_moisture << " not valid");
}

}
