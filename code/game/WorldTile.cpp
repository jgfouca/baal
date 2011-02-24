#include "WorldTile.hpp"
#include "City.hpp"
#include "BaalExceptions.hpp"

using namespace baal;

///////////////////////////////////////////////////////////////////////////////
Yield::Yield(float food, float prod)
///////////////////////////////////////////////////////////////////////////////
  : m_food(food),
    m_prod(prod)
{
  // Tiles should either be food-yielding or production-yielding, not both
  Require(food == 0 || prod == 0, "Tile cannot yield food and production");
}

///////////////////////////////////////////////////////////////////////////////
void WorldTile::draw_text(std::ostream& out) const
///////////////////////////////////////////////////////////////////////////////
{
  out << "\033[1;" << color() << "m" // set color and bold text
      << symbol()                    // print symbol
      << "\033[0m"                   // clear color and boldness
      << " ";                        // separator
}

///////////////////////////////////////////////////////////////////////////////
LandTile::LandTile(Yield yield)
///////////////////////////////////////////////////////////////////////////////
  : WorldTile(yield),
    m_hp(1.0),
    m_infra_level(0),
    m_city(NULL)
{}

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
void LandTile::build_infra()
///////////////////////////////////////////////////////////////////////////////
{
  Require(m_infra_level < LAND_TILE_MAX_INFRA, "Infra is maxed");

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
    m_base_yield *
    ( m_infra_level ? (2 * m_infra_level) : 1 ) *
    m_hp *
    moisture_multiplier;
}

///////////////////////////////////////////////////////////////////////////////
void LandTile::place_city(City* city)
///////////////////////////////////////////////////////////////////////////////
{
  Require(m_city == NULL, "Tile already had city: " << city->name());
  m_city = city;
}
