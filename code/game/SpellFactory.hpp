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

  static bool is_in_all_names(const std::string& spell_name);

  static unsigned num_spells();

  static std::string HOT;
  static std::string COLD;
  static std::string WIND;
  static std::string INFECT;

  static std::string FIRE;
  static std::string TSTORM;
  static std::string SNOW;

  static std::string AVALANCHE;
  static std::string FLOOD;
  static std::string DRY;
  static std::string BLIZZARD;
  static std::string TORNADO;

  static std::string HEATWAVE;
  static std::string COLDWAVE;
  static std::string DROUGHT;
  static std::string MONSOON;

  static std::string DISEASE;
  static std::string EARTHQUAKE;
  static std::string HURRICANE;

  static std::string PLAGUE;
  static std::string VOLCANO;

  static std::string ASTEROID;

  static std::string ALL_SPELLS[];
};

}

#endif
