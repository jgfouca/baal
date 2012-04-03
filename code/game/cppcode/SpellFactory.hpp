#ifndef SpellFactory_hpp
#define SpellFactory_hpp

#include "BaalCommon.hpp"

#include <string>
#include <vector>

namespace baal {

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
                                   Engine&            engine,
                                   unsigned           spell_level = 1,
                                   const Location&    location = Location());

  static bool is_in_all_names(const std::string& spell_name);

  static unsigned num_spells();

  static std::string ALL_SPELLS[];
};

}

#endif
