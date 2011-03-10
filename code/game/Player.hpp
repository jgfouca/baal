#ifndef Player_hpp
#define Player_hpp

#include "TalentTree.hpp"
#include "Drawable.hpp"

#include <string>

namespace baal {

class Spell;

/**
 * Encapsulates player state.
 */
class Player : public Drawable
{
 public:
  Player();

  void learn(const Spell& spell);

  void verify_cast(const Spell& spell);

  void cast(const Spell& spell);

  void gain_exp(unsigned exp);

  void cycle_turn();

  // Drawing methods

  virtual void draw_text(std::ostream& out) const;

  virtual void draw_graphics() const { /*TODO*/ }

  // Getters

  unsigned level() const { return m_level; }

  const TalentTree& talents() const { return m_talents; }

private:
  // Forbidden
  Player(const Player&);
  Player& operator=(const Player&);

  // Instance members
  std::string m_name;
  unsigned    m_mana;
  unsigned    m_max_mana;
  unsigned    m_mana_regen_rate;
  unsigned    m_exp;
  unsigned    m_level;
  unsigned    m_next_level_cost;
  TalentTree  m_talents;

  // Class members
  static const unsigned STARTING_MANA            = 100;
  static const unsigned FIRST_LEVELUP_EXP_COST   = 100;
  static const unsigned STARTING_MANA_REGEN_RATE = STARTING_MANA / 20;
  static const float    MANA_INCREASE_PER_LEVEL  = 1.2;
  static const float    EXP_LEVEL_COST_INCREASE  = 2.0;

  static const std::string DEFAULT_PLAYER_NAME;
};

}

#endif
