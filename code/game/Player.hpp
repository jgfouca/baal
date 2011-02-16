#ifndef Player_hpp
#define Player_hpp

#include "TalentTree.hpp"

namespace baal {

class Spell;

/**
 * Encapsulates player state.
 */
class Player
{
 public:
  Player()
    : m_mana(10),
      m_talent_tree()
  {}

  void learn(const Spell& spell)
  {
    m_talent_tree.add(spell);
  }

  void verify_cast(const Spell& spell);

  void cast(const Spell& spell) throw();

 private:
  // Forbidden
  Player(const Player&);
  Player& operator=(const Player&);

  unsigned   m_mana;
  TalentTree m_talent_tree;
};

}

#endif
