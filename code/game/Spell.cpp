#include "Spell.hpp"
#include "World.hpp"
#include "Player.hpp"

#include <iostream>

using std::ostream;
using namespace baal;

SpellPrereq Hot::PREREQ;
SpellPrereq Cold::PREREQ;
SpellPrereq WindSpell::PREREQ;

SpellPrereq Fire::PREREQ;
SpellPrereq Tstorm::PREREQ;
SpellPrereq Snow::PREREQ;

SpellPrereq Avalanche::PREREQ;
SpellPrereq Flood::PREREQ;
SpellPrereq Dry::PREREQ;
SpellPrereq Blizzard::PREREQ;
SpellPrereq Tornado::PREREQ;

SpellPrereq Heatwave::PREREQ;
SpellPrereq Coldwave::PREREQ;
SpellPrereq Drought::PREREQ;
SpellPrereq Monsoon::PREREQ;

SpellPrereq Disease::PREREQ;
SpellPrereq Earthquake::PREREQ;
SpellPrereq Hurricane::PREREQ;

SpellPrereq Plague::PREREQ;
SpellPrereq Volcano::PREREQ;

SpellPrereq Asteroid::PREREQ;

namespace {

SpellPrereqStaticInitializer static_prereq_init;

}

///////////////////////////////////////////////////////////////////////////////
SpellPrereqStaticInitializer::SpellPrereqStaticInitializer()
///////////////////////////////////////////////////////////////////////////////
{
  // Hot, Cold, Wind have no prereqs

  Fire::PREREQ.m_min_player_level = 5;
  Fire::PREREQ.m_min_spell_prereqs.push_back(Prereq(SpellFactory::HOT, 1));

  Tstorm::PREREQ.m_min_player_level = 5;
  Tstorm::PREREQ.m_min_spell_prereqs.push_back(Prereq(SpellFactory::WIND, 1));

  Snow::PREREQ.m_min_player_level = 5;
  Snow::PREREQ.m_min_spell_prereqs.push_back(Prereq(SpellFactory::COLD, 1));

  Avalanche::PREREQ.m_min_player_level = 10;
  Avalanche::PREREQ.m_min_spell_prereqs.push_back(Prereq(SpellFactory::SNOW, 1));

  Blizzard::PREREQ.m_min_player_level = 10;
  Blizzard::PREREQ.m_min_spell_prereqs.push_back(Prereq(SpellFactory::SNOW, 1));

  Flood::PREREQ.m_min_player_level = 10;
  Flood::PREREQ.m_min_spell_prereqs.push_back(Prereq(SpellFactory::TSTORM, 1));

  Dry::PREREQ.m_min_player_level = 10;
  Dry::PREREQ.m_min_spell_prereqs.push_back(Prereq(SpellFactory::FIRE, 1));

  Tornado::PREREQ.m_min_player_level = 10;
  Tornado::PREREQ.m_min_spell_prereqs.push_back(Prereq(SpellFactory::TSTORM, 1));

  Heatwave::PREREQ.m_min_player_level = 15;
  Heatwave::PREREQ.m_min_spell_prereqs.push_back(Prereq(SpellFactory::DRY, 1));

  Coldwave::PREREQ.m_min_player_level = 15;
  Coldwave::PREREQ.m_min_spell_prereqs.push_back(Prereq(SpellFactory::BLIZZARD, 1));

  Drought::PREREQ.m_min_player_level = 15;
  Drought::PREREQ.m_min_spell_prereqs.push_back(Prereq(SpellFactory::DRY, 1));

  Monsoon::PREREQ.m_min_player_level = 15;
  Monsoon::PREREQ.m_min_spell_prereqs.push_back(Prereq(SpellFactory::FLOOD, 1));

  Disease::PREREQ.m_min_player_level = 20;
  Disease::PREREQ.m_min_spell_prereqs.push_back(Prereq(SpellFactory::HEATWAVE, 1));
  Disease::PREREQ.m_min_spell_prereqs.push_back(Prereq(SpellFactory::COLDWAVE, 1));

  Earthquake::PREREQ.m_min_player_level = 20;

  Hurricane::PREREQ.m_min_player_level = 20;
  Hurricane::PREREQ.m_min_spell_prereqs.push_back(Prereq(SpellFactory::MONSOON, 1));

  Plague::PREREQ.m_min_player_level = 25;
  Plague::PREREQ.m_min_spell_prereqs.push_back(Prereq(SpellFactory::DISEASE, 1));

  Volcano::PREREQ.m_min_player_level = 25;
  Volcano::PREREQ.m_min_spell_prereqs.push_back(Prereq(SpellFactory::EARTHQUAKE, 1));

  Asteroid::PREREQ.m_min_player_level = 30;
  Asteroid::PREREQ.m_min_spell_prereqs.push_back(Prereq(SpellFactory::VOLCANO, 1));
}

///////////////////////////////////////////////////////////////////////////////
Spell::Spell(SpellFactory::SpellName name,
             unsigned                spell_level,
             const Location&         location,
             unsigned                base_cost,
             const SpellPrereq&      prereq)
///////////////////////////////////////////////////////////////////////////////
  : m_name(name),
    m_spell_level(spell_level),
    m_location(location),
    m_base_cost(base_cost),
    m_prereq(prereq)
{
  Assert(SpellFactory::is_in_all_names(name), name);
}

///////////////////////////////////////////////////////////////////////////////
ostream& Spell::operator<<(ostream& out) const
///////////////////////////////////////////////////////////////////////////////
{
  return out << m_name << '[' << m_spell_level << ']';
}

/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
void Fire::apply(World& world) const
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
