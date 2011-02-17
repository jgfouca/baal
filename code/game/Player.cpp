#include "Player.hpp"
#include "BaalExceptions.hpp"
#include "Spell.hpp"
#include "Configuration.hpp"

using namespace baal;

#include <iostream>

const std::string Player::DEFAULT_PLAYER_NAME = "human";

///////////////////////////////////////////////////////////////////////////////
Player::Player()
///////////////////////////////////////////////////////////////////////////////
  : m_name(Configuration::instance().get_player_name()),
    m_mana(STARTING_MANA),
    m_max_mana(STARTING_MANA),
    m_mana_regen_rate(STARTING_MANA_REGEN_RATE),
    m_exp(0),
    m_level(1),
    m_next_level_cost(FIRST_LEVELUP_EXP_COST),
    m_talent_tree()
{
  if (m_name == Configuration::UNSET) {
    m_name = DEFAULT_PLAYER_NAME;
  }
}

///////////////////////////////////////////////////////////////////////////////
void Player::learn(const Spell& spell)
///////////////////////////////////////////////////////////////////////////////
{
  RequireUser(m_level > m_talent_tree.num_learned(),
              "You cannot learn any more spells until you level-up");
  m_talent_tree.add(spell);
}

///////////////////////////////////////////////////////////////////////////////
void Player::verify_cast(const Spell& spell)
///////////////////////////////////////////////////////////////////////////////
{
  RequireUser(spell.cost() <= m_mana,
              "Spell requires " << spell.cost() <<
              " mana, player only has " << m_mana << " mana");
  RequireUser(m_talent_tree.has(spell), "Player cannot cast spell " << spell);
}

///////////////////////////////////////////////////////////////////////////////
void Player::cast(const Spell& spell)
///////////////////////////////////////////////////////////////////////////////
{
  m_mana -= spell.cost();

  // Maintain mana invariant
  Require(m_mana <= m_max_mana,
          "m_mana(" << m_mana << ") > m_max_mana(" << m_max_mana << ")");
}

///////////////////////////////////////////////////////////////////////////////
void Player::gain_exp(unsigned exp)
///////////////////////////////////////////////////////////////////////////////
{
  m_exp += exp;

  // Check for level-up
  if (m_exp >= m_next_level_cost) {
    m_level++;
    m_max_mana *= MANA_INCREASE_PER_LEVEL;
    m_mana_regen_rate *= MANA_INCREASE_PER_LEVEL;
    m_exp -= m_next_level_cost;
    m_next_level_cost *= EXP_LEVEL_COST_INCREASE;
  }

  // Check exp invariant
  Require(m_exp < m_next_level_cost,
    "m_exp(" << m_exp << ") > m_next_level_cost(" << m_next_level_cost << ")");
}

///////////////////////////////////////////////////////////////////////////////
void Player::cycle_turn()
///////////////////////////////////////////////////////////////////////////////
{
  m_mana += m_mana_regen_rate;
  if (m_mana > m_max_mana) {
    m_mana = m_max_mana;
  }

  // Maintain mana invariant
  Require(m_mana <= m_max_mana,
          "m_mana(" << m_mana << ") > m_max_mana(" << m_max_mana << ")");
}
