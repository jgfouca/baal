#ifndef TalentTree_hpp
#define TalentTree_hpp

#include <map>
#include <string>
#include <vector>
#include <utility>
#include <libxml/parser.h>

namespace baal {

class Spell;
class Player;

/**
 * Keeps track of a Player's talent tree and enforces spell prereqs.
 */
class TalentTree
{
 public:
  typedef std::vector<std::pair<std::string, unsigned> > query_return_type;
  typedef std::map<std::string, unsigned> map_type;

  TalentTree(const Player& player) :
    m_num_learned(0),
    m_player(player)
  {}

  void add(const std::string& spell_name);

  bool has(const Spell& spell) const;

  bool has(const std::string& spell_name, unsigned spell_level = 1) const;

  unsigned num_learned() const { return m_num_learned; }

  query_return_type query_all_castable_spells() const;

  query_return_type query_all_learnable_spells() const;

  unsigned spell_skill(const std::string& spell_name) const;

  xmlNodePtr to_xml();

 private:
  void check_prereqs(const std::string& spell_name,
                     unsigned spell_level,
                     unsigned player_level) const;
  void validate_invariants() const;

  map_type      m_spell_level_map;
  unsigned      m_num_learned;
  const Player& m_player;

  static const unsigned MAX_SPELL_LEVEL = 5;
};

}

#endif
