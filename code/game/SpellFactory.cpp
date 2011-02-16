#include "SpellFactory.hpp"
#include "BaalCommon.hpp"
#include "BaalExceptions.hpp"
#include "Spell.hpp"

using namespace baal;

const std::string SpellFactory::FIRE_SPELL = "fire";

///////////////////////////////////////////////////////////////////////////////
const Spell& SpellFactory::create_spell(const std::string& spell_name,
                                        unsigned           spell_level,
                                        const Location&    location)
///////////////////////////////////////////////////////////////////////////////
{
  if (spell_name == FIRE_SPELL) {
    return *(new FireSpell(spell_name, spell_level, location));
  }
  else {
    RequireUser(false, "Unknown spell: " << spell_name);
  }
}
