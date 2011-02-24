#include "Spell.hpp"
#include "World.hpp"

#include <iostream>

using std::ostream;

namespace baal {

///////////////////////////////////////////////////////////////////////////////
Spell::Spell(const std::string& name,
             unsigned           spell_level,
             const Location&    location)
///////////////////////////////////////////////////////////////////////////////
  : m_name(name),
    m_spell_level(spell_level),
    m_location(location)
{
}

///////////////////////////////////////////////////////////////////////////////
ostream& Spell::operator<<(ostream& out) const
///////////////////////////////////////////////////////////////////////////////
{
  return out << m_name << '[' << m_spell_level << ']';
}

/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
void FireSpell::apply(World& world) const
///////////////////////////////////////////////////////////////////////////////
{
  // TODO Fill in
  // Does not apply to oceans

  WorldTile& tile = world.get_tile(m_location);
  LandTile* tile_ptr = dynamic_cast<LandTile*>(&tile);
  RequireUser(tile_ptr != NULL, "Can only cast fire on land tiles");
  LandTile& affected_tile = *tile_ptr;

  // TODO: check for city, soil moisture, wind, temp, etc
  affected_tile.damage(.5);
}

}

///////////////////////////////////////////////////////////////////////////////
ostream& baal::operator<<(ostream& out, const Spell& spell)
///////////////////////////////////////////////////////////////////////////////
{
  return spell.operator<<(out);
}
