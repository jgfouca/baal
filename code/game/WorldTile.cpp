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
OceanTile::OceanTile(Location location, unsigned depth, Climate& climate, Geology& geology) :
///////////////////////////////////////////////////////////////////////////////
  WorldTile(location, Yield(OCEAN_FOOD, OCEAN_PROD), climate, geology),
  m_depth(depth),
  m_surface_temp(0)
{
  // Current temp will be avg over all seasons
  for (Season s : iterate<Season>()) {
    m_surface_temp += climate.temperature(s);
  }
  m_surface_temp /= size<Season>();
}

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
  m_surface_temp = new_surface_temp_func(m_surface_temp, m_atmosphere.temperature());
}

/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
LandTile::LandTile(Location location, unsigned elevation, Yield yield, Climate& climate, Geology& geology)
///////////////////////////////////////////////////////////////////////////////
  : WorldTile(location, yield, climate, geology),
    m_hp(1.0),
    m_infra_level(0),
    m_elevation(elevation),
    m_snowpack(0), // TODO: Should have prexisting snowpack
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
void LandTile::cycle_turn(const std::vector<std::shared_ptr<const Anomaly>>& anomalies,
                          const Location& location,
                          Season season)
///////////////////////////////////////////////////////////////////////////////
{
  WorldTile::cycle_turn(anomalies, location, season);

  // Compute HP recovery
  m_hp = land_tile_recovery_func(m_hp);

  // Compute change in snowpack
  const float precip = atmosphere().precip();
  const int   temp   = atmosphere().temperature();

  // TODO - Take elevation into account

  const float snowfall_portion = portion_of_precip_that_falls_as_snow_func(temp);
  const float snowpack_melt_portion = portion_of_snowpack_that_melted(temp);
  const unsigned snowfall = (precip * 12) * snowfall_portion; // 12 inches of snow per liquid inch

  m_snowpack = (snowfall + m_snowpack) * (1 - snowpack_melt_portion);
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
  return compute_yield_func(m_base_yield, m_infra_level, m_hp);
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
void MountainTile::place_city(City& city)
///////////////////////////////////////////////////////////////////////////////
{
  Require(false, "Mountain tiles cannot support cities");
}

/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
void TileWithSoil::cycle_turn(const std::vector<std::shared_ptr<const Anomaly>>& anomalies,
                              const Location& location,
                              Season season)
///////////////////////////////////////////////////////////////////////////////
{
  LandTile::cycle_turn(anomalies, location, season);

  // MODEL: Model how precip and temp changes soil moisture

  // Get the parameters we need to make the calculation
  const float precip         = m_atmosphere.precip();
  const int temp             = m_atmosphere.temperature();
  const float avg_precip     = m_climate.precip(season);
  const int avg_temp         = m_climate.temperature(season);
  const float prior_moisture = m_soil_moisture;

  // Precip's effect on moisture
  const float precip_effect = precip_effect_on_moisture_func(avg_precip, precip);

  // Temperature's effect on moisture. This is trickier; for now only a very
  // basic model is in place.
  const float temp_effect = temp_effect_on_moisture_func(avg_temp, temp);

  // Compute overall seasonal forcing on soil moisture
  const float current_forcing = precip_effect * temp_effect;

  m_soil_moisture = compute_moisture_func(prior_moisture, current_forcing);

  Require(m_soil_moisture >= 0.0, "Moisture " << m_soil_moisture << " not valid");
}

/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
Yield FoodTile::yield() const
///////////////////////////////////////////////////////////////////////////////
{
  Require (m_base_yield.m_food > 0, "Tiles with growth should yield food");

  // MODEL: Important equation that may need balancing

  // Food yielding tiles need to take soil moisture into account
  const float moisture_effect = moisture_yield_effect_func(m_soil_moisture);
  const float snowpack_effect = snowpack_yield_effect_func(m_snowpack);
  return LandTile::yield() * moisture_effect * snowpack_effect;
}

}
