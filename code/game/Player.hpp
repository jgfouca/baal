#ifndef Player_hpp
#define Player_hpp

#include "TalentTree.hpp"

#include <string>
#include <cmath>
#include <libxml/parser.h>

namespace baal {

class Spell;
class Engine;

/**
 * Encapsulates player state.
 */
class Player
{
 public:
  Player(const Engine& engine);

  // Have this player learn a spell by name. If the player already
  // knows the spell, the player's skill in that spell will be
  // increased by one.  This method will throw a user error if the
  // player cannot learn the spell.
  void learn(const std::string& spell_name);

  // Check if this player can cast a spell. Throws a user error if the
  // answer is no.
  void verify_cast(const Spell& spell) const;

  // Adjust player state appropriately assuming that spell was cast. This
  // method should never throw a user error. verify_cast should have been
  // called prior to ensure that the casting of this spell is valid from
  // this player's point-of-view.
  void cast(const Spell& spell);

  // Inform the player that they have gained some exp.
  void gain_exp(unsigned exp);

  // Inform this player that the turn has cycled.
  void cycle_turn();

  // Getters

  unsigned level() const { return m_level; }

  const TalentTree& talents() const { return m_talents; }

  xmlNodePtr to_xml();

  const Engine& engine() const { return m_engine; }

  unsigned exp() const { return m_exp; }

  unsigned next_level_cost() const { return EXP_LEVEL_COST_FUNC(m_level); }

  const std::string& name() const { return m_name; }

  unsigned mana() const { return m_mana; }

  unsigned max_mana() const { return m_max_mana; }

private:
  // Forbidden
  Player(const Player&);
  Player& operator=(const Player&);

  // Instance members
  std::string m_name;
  unsigned    m_mana;
  unsigned    m_max_mana;
  unsigned    m_exp;
  unsigned    m_level;
  unsigned    m_next_level_cost;
  TalentTree  m_talents;
  const Engine& m_engine;

  // Class members
  static constexpr unsigned STARTING_MANA            = 100;
  static constexpr unsigned FIRST_LEVELUP_EXP_COST   = 100;
  static constexpr float    MANA_REGEN_RATE          = 1.0 / 20.0; // 5%

  static unsigned MANA_POOL_FUNC(unsigned level)
  {
    return STARTING_MANA * std::pow(1.4, level - 1); // 40% per level
  }

  static unsigned EXP_LEVEL_COST_FUNC(unsigned level)
  {
    return FIRST_LEVELUP_EXP_COST * std::pow(1.4, level - 1); // 40% per level
  }

  static const std::string DEFAULT_PLAYER_NAME;
};

}

#endif
