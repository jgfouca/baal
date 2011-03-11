#include "Spell.hpp"
#include "World.hpp"
#include "Player.hpp"

#include <iostream>

using std::ostream;
using namespace baal;

SpellPrereq FireSpell::PREREQ;

///////////////////////////////////////////////////////////////////////////////
SpellPrereqStaticInitializer::SpellPrereqStaticInitializer()
///////////////////////////////////////////////////////////////////////////////
{
  // TODO
}

///////////////////////////////////////////////////////////////////////////////
Spell::Spell(const std::string& name,
             unsigned           spell_level,
             const Location&    location,
             unsigned           base_cost,
             const SpellPrereq& prereq)
///////////////////////////////////////////////////////////////////////////////
  : m_name(name),
    m_spell_level(spell_level),
    m_location(location),
    m_base_cost(base_cost),
    m_prereq(prereq)
{
}

///////////////////////////////////////////////////////////////////////////////
void Spell::verify_prereqs(const Player& player) const
///////////////////////////////////////////////////////////////////////////////
{
  RequireUser(player.level() >= m_prereq.m_min_player_level,
              "You are not high-enough level to learn that spell");

  const TalentTree& talents = player.talents();

  for (std::vector<std::pair<std::string, unsigned> >::const_iterator
       itr =  m_prereq.m_min_spell_prereqs.begin();
       itr != m_prereq.m_min_spell_prereqs.end();
       ++itr) {
    std::string spell_name  = itr->first;
    unsigned    spell_level = itr->second;
    RequireUser(talents.has(spell_name, spell_level),
      "Missing required prereq " << spell_level << '[' << spell_level << ']');
  }

  if (m_spell_level > 1) {
    RequireUser(talents.has(m_name, m_spell_level - 1),
     "Missing required prereq " << m_name << '[' << m_spell_level - 1 << ']');
  }
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
  Require(tile_ptr != NULL, "Can only cast fire on land tiles");
  LandTile& affected_tile = *tile_ptr;

  // TODO: check for city, soil moisture, wind, temp, etc
  affected_tile.damage(.5);
}

///////////////////////////////////////////////////////////////////////////////
ostream& baal::operator<<(ostream& out, const Spell& spell)
///////////////////////////////////////////////////////////////////////////////
{
  return spell.operator<<(out);
}
