#ifndef SpellFactory_hpp
#define SpellFactory_hpp

#include <string>
#include <vector>

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

  typedef const char* SpellName;

  static bool is_in_all_names(SpellName spell_name);

  static unsigned num_spells();

  static SpellName HOT;
  static SpellName COLD;
  static SpellName WIND;
  static SpellName INFECT;

  static SpellName FIRE;
  static SpellName TSTORM;
  static SpellName SNOW;

  static SpellName AVALANCHE;
  static SpellName FLOOD;
  static SpellName DRY;
  static SpellName BLIZZARD;
  static SpellName TORNADO;

  static SpellName HEATWAVE;
  static SpellName COLDWAVE;
  static SpellName DROUGHT;
  static SpellName MONSOON;

  static SpellName DISEASE;
  static SpellName EARTHQUAKE;
  static SpellName HURRICANE;

  static SpellName PLAGUE;
  static SpellName VOLCANO;

  static SpellName ASTEROID;

  static SpellName ALL_SPELLS[];
};

}

#endif
