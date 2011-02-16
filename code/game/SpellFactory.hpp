#ifndef SpellFactory_hpp
#define SpellFactory_hpp

#include <string>

namespace baal {

struct Location;
class Spell;

/**
 * Factory class used to create spells. This class encapsulates
 * the knowledge of the set of available spells.
 */
class SpellFactory
{
 public:
  // client responsible for deletion
  static const Spell& create_spell(const std::string& spell_name,
                                   unsigned           spell_level,
                                   const Location&    location);

  static const std::string FIRE_SPELL;
};

}

#endif
