#include "SpellFactory.hpp"
#include "BaalExceptions.hpp"
#include "Spell.hpp"

namespace baal {

std::string SpellFactory::ALL_SPELLS[] = {
  Hot::NAME,
  Cold::NAME,
  WindSpell::NAME,
  Infect::NAME,
  Fire::NAME,
  Tstorm::NAME,
  Snow::NAME,
  Avalanche::NAME,
  Flood::NAME,
  Dry::NAME,
  Blizzard::NAME,
  Tornado::NAME,
  Heatwave::NAME,
  Coldwave::NAME,
  Drought::NAME,
  Monsoon::NAME,
  Disease::NAME,
  Earthquake::NAME,
  Hurricane::NAME,
  Plague::NAME,
  Volcano::NAME,
  Asteroid::NAME
};

///////////////////////////////////////////////////////////////////////////////
std::shared_ptr<const Spell>
SpellFactory::create_spell(const std::string& spell_name,
                           Engine&            engine,
                           unsigned           spell_level,
                           const Location&    location)
///////////////////////////////////////////////////////////////////////////////
{
  Spell* new_spell = nullptr;
  if (spell_name == Hot::NAME) {
    new_spell = new Hot(spell_level, location, engine);
  }
  else if (spell_name == Cold::NAME) {
    new_spell = new Cold(spell_level, location, engine);
  }
  else if (spell_name == WindSpell::NAME) {
    new_spell = new WindSpell(spell_level, location, engine);
  }
  else if (spell_name == Infect::NAME) {
    new_spell = new Infect(spell_level, location, engine);
  }
  else if (spell_name == Fire::NAME) {
    new_spell = new Fire(spell_level, location, engine);
  }
  else if (spell_name == Tstorm::NAME) {
    new_spell = new Tstorm(spell_level, location, engine);
  }
  else if (spell_name == Snow::NAME) {
    new_spell = new Snow(spell_level, location, engine);
  }
  else if (spell_name == Avalanche::NAME) {
    new_spell = new Avalanche(spell_level, location, engine);
  }
  else if (spell_name == Flood::NAME) {
    new_spell = new Flood(spell_level, location, engine);
  }
  else if (spell_name == Dry::NAME) {
    new_spell = new Dry(spell_level, location, engine);
  }
  else if (spell_name == Blizzard::NAME) {
    new_spell = new Blizzard(spell_level, location, engine);
  }
  else if (spell_name == Tornado::NAME) {
    new_spell = new Tornado(spell_level, location, engine);
  }
  else if (spell_name == Heatwave::NAME) {
    new_spell = new Heatwave(spell_level, location, engine);
  }
  else if (spell_name == Coldwave::NAME) {
    new_spell = new Coldwave(spell_level, location, engine);
  }
  else if (spell_name == Drought::NAME) {
    new_spell = new Drought(spell_level, location, engine);
  }
  else if (spell_name == Monsoon::NAME) {
    new_spell = new Monsoon(spell_level, location, engine);
  }
  else if (spell_name == Disease::NAME) {
    new_spell = new Disease(spell_level, location, engine);
  }
  else if (spell_name == Earthquake::NAME) {
    new_spell = new Earthquake(spell_level, location, engine);
  }
  else if (spell_name == Hurricane::NAME) {
    new_spell = new Hurricane(spell_level, location, engine);
  }
  else if (spell_name == Plague::NAME) {
    new_spell = new Plague(spell_level, location, engine);
  }
  else if (spell_name == Volcano::NAME) {
    new_spell = new Volcano(spell_level, location, engine);
  }
  else if (spell_name == Asteroid::NAME) {
    new_spell = new Asteroid(spell_level, location, engine);
  }
  else {
    RequireUser(false, "Unknown spell: " << spell_name);
  }
  return std::shared_ptr<const Spell>(new_spell);
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

}
