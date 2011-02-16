#ifndef TalentTree_hpp
#define TalentTree_hpp

#include <map>
#include <string>

namespace baal {

class Spell;

/**
 * TODO - Class needs major upgrades
 */
class TalentTree
{
 public:
  TalentTree() {}

  void add(const Spell& spell);

  bool has(const Spell& spell) const;

 private:
  std::map<std::string, unsigned> m_spell_level_map;
};

}

#endif
