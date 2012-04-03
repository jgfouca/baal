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
const Spell& SpellFactory::create_spell(const std::string& spell_name,
                                        Engine&            engine,
                                        unsigned           spell_level,
                                        const Location&    location)
///////////////////////////////////////////////////////////////////////////////
{
  if (spell_name == Hot::NAME) {
    return *(new Hot(spell_level, location, engine));
  }
  else if (spell_name == Cold::NAME) {
    return *(new Cold(spell_level, location, engine));
  }
  else if (spell_name == WindSpell::NAME) {
    return *(new WindSpell(spell_level, location, engine));
  }
  else if (spell_name == Infect::NAME) {
    return *(new Infect(spell_level, location, engine));
  }
  else if (spell_name == Fire::NAME) {
    return *(new Fire(spell_level, location, engine));
  }
  else if (spell_name == Tstorm::NAME) {
    return *(new Tstorm(spell_level, location, engine));
  }
  else if (spell_name == Snow::NAME) {
    return *(new Snow(spell_level, location, engine));
  }
  else if (spell_name == Avalanche::NAME) {
    return *(new Avalanche(spell_level, location, engine));
  }
  else if (spell_name == Flood::NAME) {
    return *(new Flood(spell_level, location, engine));
  }
  else if (spell_name == Dry::NAME) {
    return *(new Dry(spell_level, location, engine));
  }
  else if (spell_name == Blizzard::NAME) {
    return *(new Blizzard(spell_level, location, engine));
  }
  else if (spell_name == Tornado::NAME) {
    return *(new Tornado(spell_level, location, engine));
  }
  else if (spell_name == Heatwave::NAME) {
    return *(new Heatwave(spell_level, location, engine));
  }
  else if (spell_name == Coldwave::NAME) {
    return *(new Coldwave(spell_level, location, engine));
  }
  else if (spell_name == Drought::NAME) {
    return *(new Drought(spell_level, location, engine));
  }
  else if (spell_name == Monsoon::NAME) {
    return *(new Monsoon(spell_level, location, engine));
  }
  else if (spell_name == Disease::NAME) {
    return *(new Disease(spell_level, location, engine));
  }
  else if (spell_name == Earthquake::NAME) {
    return *(new Earthquake(spell_level, location, engine));
  }
  else if (spell_name == Hurricane::NAME) {
    return *(new Hurricane(spell_level, location, engine));
  }
  else if (spell_name == Plague::NAME) {
    return *(new Plague(spell_level, location, engine));
  }
  else if (spell_name == Volcano::NAME) {
    return *(new Volcano(spell_level, location, engine));
  }
  else if (spell_name == Asteroid::NAME) {
    return *(new Asteroid(spell_level, location, engine));
  }
  else {
    RequireUser(false, "Unknown spell: " << spell_name);
    return *static_cast<Spell*>(nullptr);
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

}
