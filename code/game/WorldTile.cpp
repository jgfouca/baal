#include "WorldTile.hpp"
#include "City.hpp"
#include "BaalExceptions.hpp"
#include "Geology.hpp"

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
  switch (m_draw_mode) {
  case NORMAL:
    out << "\033[1;" << color() << "m" // set color and bold text
        << symbol()                    // print symbol
        << "\033[0m"                   // clear color and boldness
        << " ";                        // separator
    break;
  case GEOLOGY:
  case MAGMA:
  case TENSION:
    m_geology.draw_text(out);
    break;
  case WIND:
  case TEMPERATURE:
  case PRESSURE:
  case DEWPOINT:
    m_atmosphere.draw_text(out);
    break;
  default:
    Require(false, "Unrecognized mode");
  }
}

///////////////////////////////////////////////////////////////////////////////
void WorldTile::place_city(City& city)
///////////////////////////////////////////////////////////////////////////////
{
  RequireUser(false, "Can only place cities on land tiles");
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
