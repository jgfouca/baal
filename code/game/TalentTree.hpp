#ifndef TalentTree_hpp
#define TalentTree_hpp

#include <map>
#include <string>
#include <vector>
#include <utility>

namespace baal {

class Spell;
class Player;

/**
 * Keeps track of a Player's talent tree and enforces spell prereqs.
 */
class TalentTree
{
 public:
  TalentTree() : m_num_learned(0) {}

  void add(const Spell& spell, const Player& player);

  bool has(const Spell& spell) const;

  bool has(const std::string& spell_name, unsigned spell_level) const;

  unsigned num_learned() const { return m_num_learned; }

  void query_all_castable_spells(std::vector<std::pair<std::string, unsigned> >& rv) const;

  void query_all_learnable_spells(std::vector<std::pair<std::string, unsigned> >& rv,
                                  const Player& player) const;

 private:
  void check_prereqs(const Spell& spell, const Player& player) const;
  void validate_invariants() const;

  std::map<std::string, unsigned> m_spell_level_map;
  unsigned m_num_learned;
};

}

#endif
