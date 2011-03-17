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

  static const std::string HOT;
  static const std::string COLD;
  static const std::string WIND;

  static const std::string FIRE;
  static const std::string TSTORM;
  static const std::string SNOW;

  static const std::string AVALANCHE;
  static const std::string FLOOD;
  static const std::string DRY;
  static const std::string BLIZZARD;
  static const std::string TORNADO;

  static const std::string HEATWAVE;
  static const std::string COLDWAVE;
  static const std::string DROUGHT;
  static const std::string MONSOON;

  static const std::string DISEASE;
  static const std::string EARTHQUAKE;
  static const std::string HURRICANE;

  static const std::string PLAGUE;
  static const std::string VOLCANO;

  static const std::string ASTEROID;
};

}

#endif
