#include "SpellFactory.hpp"
#include "BaalCommon.hpp"
#include "BaalExceptions.hpp"
#include "Spell.hpp"

using namespace baal;

const std::string SpellFactory::HOT  = "hot";
const std::string SpellFactory::COLD = "cold";
const std::string SpellFactory::WIND = "wind";

const std::string SpellFactory::FIRE   = "fire";
const std::string SpellFactory::TSTORM = "tstorm";
const std::string SpellFactory::SNOW   = "snow";

const std::string SpellFactory::AVALANCHE = "avalanche";
const std::string SpellFactory::FLOOD     = "flood";
const std::string SpellFactory::DRY       = "dry";
const std::string SpellFactory::BLIZZARD  = "blizzard";
const std::string SpellFactory::TORNADO   = "tornado";

const std::string SpellFactory::HEATWAVE = "heatwave";
const std::string SpellFactory::COLDWAVE = "coldwave";
const std::string SpellFactory::DROUGHT  = "drought";
const std::string SpellFactory::MONSOON  = "monsoon";

const std::string SpellFactory::DISEASE    = "disease";
const std::string SpellFactory::EARTHQUAKE = "earthquake";
const std::string SpellFactory::HURRICANE  = "hurricane";

const std::string SpellFactory::PLAGUE  = "plague";
const std::string SpellFactory::VOLCANO = "volcano";

const std::string SpellFactory::ASTEROID = "asteroid";

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
