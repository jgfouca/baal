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
  TalentTree() : m_num_learned(0) {}

  void add(const Spell& spell);

  bool has(const Spell& spell) const;

  unsigned num_learned() const { return m_num_learned; }

 private:
  std::map<std::string, unsigned> m_spell_level_map;
  unsigned m_num_learned;
};

}

#endif
