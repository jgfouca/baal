#include "SpellFactory.hpp"
#include "BaalCommon.hpp"
#include "BaalExceptions.hpp"
#include "Spell.hpp"

using namespace baal;

std::string SpellFactory::HOT    = "hot";
std::string SpellFactory::COLD   = "cold";
std::string SpellFactory::WIND   = "wind";
std::string SpellFactory::INFECT = "infect";

std::string SpellFactory::FIRE   = "fire";
std::string SpellFactory::TSTORM = "tstorm";
std::string SpellFactory::SNOW   = "snow";

std::string SpellFactory::AVALANCHE = "avalanche";
std::string SpellFactory::FLOOD     = "flood";
std::string SpellFactory::DRY       = "dry";
std::string SpellFactory::BLIZZARD  = "blizzard";
std::string SpellFactory::TORNADO   = "tornado";

std::string SpellFactory::HEATWAVE = "heatwave";
std::string SpellFactory::COLDWAVE = "coldwave";
std::string SpellFactory::DROUGHT  = "drought";
std::string SpellFactory::MONSOON  = "monsoon";

std::string SpellFactory::DISEASE    = "disease";
std::string SpellFactory::EARTHQUAKE = "earthquake";
std::string SpellFactory::HURRICANE  = "hurricane";

std::string SpellFactory::PLAGUE  = "plague";
std::string SpellFactory::VOLCANO = "volcano";

std::string SpellFactory::ASTEROID = "asteroid";

std::string SpellFactory::ALL_SPELLS[] = {
  SpellFactory::HOT,
  SpellFactory::COLD,
  SpellFactory::WIND,
  SpellFactory::INFECT,
  SpellFactory::FIRE,
  SpellFactory::TSTORM,
  SpellFactory::SNOW,
  SpellFactory::AVALANCHE,
  SpellFactory::FLOOD,
  SpellFactory::DRY,
  SpellFactory::BLIZZARD,
  SpellFactory::TORNADO,
  SpellFactory::HEATWAVE,
  SpellFactory::COLDWAVE,
  SpellFactory::DROUGHT,
  SpellFactory::MONSOON,
  SpellFactory::DISEASE,
  SpellFactory::EARTHQUAKE,
  SpellFactory::HURRICANE,
  SpellFactory::PLAGUE,
  SpellFactory::VOLCANO,
  SpellFactory::ASTEROID
};

///////////////////////////////////////////////////////////////////////////////
const Spell& SpellFactory::create_spell(const std::string& spell_name,
                                        unsigned           spell_level,
                                        const Location&    location)
///////////////////////////////////////////////////////////////////////////////
{
  if (spell_name == HOT) {
    return *(new Hot(spell_level, location));
  }
  else if (spell_name == COLD) {
    return *(new Cold(spell_level, location));
  }
  else if (spell_name == WIND) {
    return *(new WindSpell(spell_level, location));
  }
  else if (spell_name == INFECT) {
    return *(new Infect(spell_level, location));
  }
  else if (spell_name == FIRE) {
    return *(new Fire(spell_level, location));
  }
  else if (spell_name == TSTORM) {
    return *(new Tstorm(spell_level, location));
  }
  else if (spell_name == SNOW) {
    return *(new Snow(spell_level, location));
  }
  else if (spell_name == AVALANCHE) {
    return *(new Avalanche(spell_level, location));
  }
  else if (spell_name == FLOOD) {
    return *(new Flood(spell_level, location));
  }
  else if (spell_name == DRY) {
    return *(new Dry(spell_level, location));
  }
  else if (spell_name == BLIZZARD) {
    return *(new Blizzard(spell_level, location));
  }
  else if (spell_name == TORNADO) {
    return *(new Tornado(spell_level, location));
  }
  else if (spell_name == HEATWAVE) {
    return *(new Heatwave(spell_level, location));
  }
  else if (spell_name == COLDWAVE) {
    return *(new Coldwave(spell_level, location));
  }
  else if (spell_name == DROUGHT) {
    return *(new Drought(spell_level, location));
  }
  else if (spell_name == MONSOON) {
    return *(new Monsoon(spell_level, location));
  }
  else if (spell_name == DISEASE) {
    return *(new Disease(spell_level, location));
  }
  else if (spell_name == EARTHQUAKE) {
    return *(new Earthquake(spell_level, location));
  }
  else if (spell_name == HURRICANE) {
    return *(new Hurricane(spell_level, location));
  }
  else if (spell_name == PLAGUE) {
    return *(new Plague(spell_level, location));
  }
  else if (spell_name == VOLCANO) {
    return *(new Volcano(spell_level, location));
  }
  else if (spell_name == ASTEROID) {
    return *(new Asteroid(spell_level, location));
  }
  else {
    RequireUser(false, "Unknown spell: " << spell_name);
  }
}

///////////////////////////////////////////////////////////////////////////////
bool SpellFactory::is_in_all_names(const std::string& spell_name)
///////////////////////////////////////////////////////////////////////////////
{
  for (unsigned i = 0; i < SpellFactory::num_spells(); ++i) {
    if (ALL_SPELLS[i] == spell_name) {
      return true;
    }
  }
  return false;
}

///////////////////////////////////////////////////////////////////////////////
unsigned SpellFactory::num_spells()
///////////////////////////////////////////////////////////////////////////////
{
  return sizeof(ALL_SPELLS)/sizeof(std::string);
}
