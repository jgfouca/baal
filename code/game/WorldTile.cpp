#include "WorldTile.hpp"
#include "City.hpp"
#include "BaalExceptions.hpp"
#include "Geology.hpp"

#include <iomanip>

using namespace baal;

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

/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
WorldTile::WorldTile(Yield yield, Climate& climate, Geology& geology)
///////////////////////////////////////////////////////////////////////////////
  : m_base_yield(yield),
    m_climate(climate),
    m_geology(geology),
    m_atmosphere(climate)
{}

///////////////////////////////////////////////////////////////////////////////
WorldTile::~WorldTile()
///////////////////////////////////////////////////////////////////////////////
{
  delete &m_climate;
  delete &m_geology;
}

///////////////////////////////////////////////////////////////////////////////
void WorldTile::draw_text(std::ostream& out) const
///////////////////////////////////////////////////////////////////////////////
{
  if (s_draw_mode == LAND) {
    out << BOLD_COLOR << color(); // bold text and set color
    for (unsigned w = 0; w < TILE_TEXT_WIDTH; ++w) {
      out << symbol(); // print symbol
    }
    out << CLEAR_ALL; // clear color and boldness
  }
  else if (s_draw_mode == CIV) {
    out << BOLD_COLOR; // bold text
    if (city() != NULL) {
      out << RED << " C:" << std::setw(TILE_TEXT_WIDTH - 3) << city()->rank();
    }
    else if (infra_level() > 0) {
      out << YELLOW << " I:" << std::setw(TILE_TEXT_WIDTH- 3) << infra_level();
    }
    else {
      out << color();
      for (unsigned w = 0; w < TILE_TEXT_WIDTH; ++w) {
        out << symbol(); // print symbol
      }
    }
    out << CLEAR_ALL; // clear color and boldness
  }
  else if (Geology::is_geological(s_draw_mode)) {
    m_geology.draw_text(out);
  }
  else if (Atmosphere::is_atmospheric(s_draw_mode)) {
    m_atmosphere.draw_text(out);
  }
  else {
    Require(false, "Unhandled mode: " << s_draw_mode);
  }
}

///////////////////////////////////////////////////////////////////////////////
void WorldTile::place_city(City& city)
///////////////////////////////////////////////////////////////////////////////
{
  RequireUser(false, "Can only place cities on land tiles");
}

///////////////////////////////////////////////////////////////////////////////
void WorldTile::cycle_turn()
///////////////////////////////////////////////////////////////////////////////
{
  m_geology.cycle_turn();
  m_atmosphere.cycle_turn();
}

/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
OceanTile::OceanTile(unsigned depth, Climate& climate, Geology& geology)
///////////////////////////////////////////////////////////////////////////////
  : WorldTile(Yield(3,0), climate, geology),
    m_depth(depth),
    m_surface_temp(climate.temperature())
{}

/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
LandTile::LandTile(Yield yield, Climate& climate, Geology& geology)
///////////////////////////////////////////////////////////////////////////////
  : WorldTile(yield, climate, geology),
    m_hp(1.0),
    m_infra_level(0),
    m_city(NULL)
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
  Require(dmg > 0.0 && dmg < 1.0, "Invalid value for damage: " << dmg);

  m_hp *= dmg;

  Require(m_hp > 0.0 && m_hp < 1.0, "Invariant for hp failed: " << m_hp);
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
void LandTile::cycle_turn()
///////////////////////////////////////////////////////////////////////////////
{
  WorldTile::cycle_turn();
  recover();
}

///////////////////////////////////////////////////////////////////////////////
void LandTile::build_infra()
///////////////////////////////////////////////////////////////////////////////
{
  RequireUser(m_infra_level < LAND_TILE_MAX_INFRA, "Infra is maxed");

  m_infra_level++;
}

///////////////////////////////////////////////////////////////////////////////
Yield LandTile::yield() const
///////////////////////////////////////////////////////////////////////////////
{
  // Important equation that may need balancing

  // Food yielding tiles need to take soil moisture into account
  float moisture_multiplier = 1.0;
  if (m_base_yield.m_food > 0) {
    float moisture = soil_moisture();
    if (moisture < 1.5) {
      // Up until 1.5 times average moisture, yields benefit from more moisture
      moisture_multiplier = moisture;
    }
    else if (moisture < 2.75) {
      // Yields drop quickly as soil becomes over-saturated
      moisture_multiplier = 1.5 - (moisture - 1.5);
    }
    else {
      // Things are flooded and can't get any worse. Farmers are able to
      // salvage some fixed portion of their crops.
      moisture_multiplier = 0.25;
    }
  }
  return
    m_base_yield * // base
    ( m_infra_level ? (2 * m_infra_level) : 1 ) * // infra multiplier
    m_hp * // damaged tiles yield less
    moisture_multiplier; //tiles with favorable moisture levels yield more food
}

///////////////////////////////////////////////////////////////////////////////
void LandTile::place_city(City& city)
///////////////////////////////////////////////////////////////////////////////
{
  RequireUser(m_city == NULL, "Tile already had city: " << city.name());
  m_city = &city;
}
