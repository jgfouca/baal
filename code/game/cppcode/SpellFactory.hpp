#ifndef SpellFactory_hpp
#define SpellFactory_hpp

#include <string>
#include <vector>

namespace baal {

struct Location;
class Spell;
class Engine;

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
                                   const Location&    location,
                                   Engine&            engine);

  static bool is_in_all_names(const std::string& spell_name);

  static unsigned num_spells();

  static std::string ALL_SPELLS[];
};

}

#endif
