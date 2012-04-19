#include "Player.hpp"
#include "BaalExceptions.hpp"
#include "Spell.hpp"
#include "Configuration.hpp"
#include "Engine.hpp"

#include <iostream>

namespace baal {

const std::string Player::DEFAULT_PLAYER_NAME = "human";

///////////////////////////////////////////////////////////////////////////////
Player::Player(const Engine& engine)
///////////////////////////////////////////////////////////////////////////////
  : m_name(engine.config().get_player_name()),
    m_mana(STARTING_MANA),
    m_max_mana(STARTING_MANA),
    m_mana_regen_rate(STARTING_MANA_REGEN_RATE),
    m_exp(0),
    m_level(1),
    m_next_level_cost(FIRST_LEVELUP_EXP_COST),
    m_talents(*this),
    m_engine(engine)
{
  if (m_name == Configuration::UNSET) {
    m_name = DEFAULT_PLAYER_NAME;
  }
}

///////////////////////////////////////////////////////////////////////////////
void Player::learn(const std::string& spell_name)
///////////////////////////////////////////////////////////////////////////////
{
  m_talents.add(spell_name);
}

///////////////////////////////////////////////////////////////////////////////
void Player::verify_cast(const Spell& spell) const
///////////////////////////////////////////////////////////////////////////////
{
  RequireUser(spell.cost() <= m_mana,
              "Spell requires " << spell.cost() <<
              " mana, player only has " << m_mana << " mana");
  RequireUser(m_talents.has(spell), "Player cannot cast spell " << spell);
}

///////////////////////////////////////////////////////////////////////////////
void Player::cast(const Spell& spell)
///////////////////////////////////////////////////////////////////////////////
{
  m_mana -= spell.cost();

  // Maintain mana invariant (m_mana is unsigned, so going below zero will
  // cause it to become enormous).
  Require(m_mana <= m_max_mana,
          "m_mana(" << m_mana << ") > m_max_mana(" << m_max_mana << ")");
}

///////////////////////////////////////////////////////////////////////////////
void Player::gain_exp(unsigned exp)
///////////////////////////////////////////////////////////////////////////////
{
  m_exp += exp;

  // Check for level-up
  while (m_exp >= m_next_level_cost) {
    m_level++;

    unsigned mana_pool_increase = m_max_mana * MANA_INCREASE_PER_LEVEL;
    m_max_mana += mana_pool_increase;
    m_mana += mana_pool_increase;
    m_mana_regen_rate *= (1 + MANA_INCREASE_PER_LEVEL);

    m_exp -= m_next_level_cost;
    m_next_level_cost *= EXP_LEVEL_COST_INCREASE;
  }

  // Check exp invariant
  Require(m_exp < m_next_level_cost,
    "m_exp(" << m_exp << ") > m_next_level_cost(" << m_next_level_cost << ")");

  // Check mana invariant
  Require(m_mana <= m_max_mana,
          "m_mana(" << m_mana << ") > m_max_mana(" << m_max_mana << ")");
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

///////////////////////////////////////////////////////////////////////////////
xmlNodePtr Player::to_xml()
///////////////////////////////////////////////////////////////////////////////
{
  xmlNodePtr Player_node = xmlNewNode(nullptr, BAD_CAST "Player");

  // std::string m_name;
  // unsigned    m_mana;
  // unsigned    m_max_mana;
  // unsigned    m_mana_regen_rate;
  // unsigned    m_exp;
  // unsigned    m_level;
  // unsigned    m_next_level_cost;
  // TalentTree  m_talents;

  xmlNewChild(Player_node, nullptr, BAD_CAST "m_name", BAD_CAST m_name.c_str());

  std::ostringstream m_mana_oss, m_max_mana_oss, m_mana_regen_rate_oss, m_exp_oss, m_level_oss, m_next_level_cost_oss;

  m_mana_oss << m_mana;
  xmlNewChild(Player_node, nullptr, BAD_CAST "m_mana", BAD_CAST m_mana_oss.str().c_str());

  m_max_mana_oss << m_max_mana;
  xmlNewChild(Player_node, nullptr, BAD_CAST "m_max_mana", BAD_CAST m_max_mana_oss.str().c_str());

  m_mana_regen_rate_oss << m_mana_regen_rate;
  xmlNewChild(Player_node, nullptr, BAD_CAST "m_mana_regen_rate", BAD_CAST m_mana_regen_rate_oss.str().c_str());

  m_exp_oss << m_exp;
  xmlNewChild(Player_node, nullptr, BAD_CAST "m_exp", BAD_CAST m_exp_oss.str().c_str());

  m_level_oss << m_level;
  xmlNewChild(Player_node, nullptr, BAD_CAST "m_level", BAD_CAST m_level_oss.str().c_str());

  m_next_level_cost_oss << m_next_level_cost;
  xmlNewChild(Player_node, nullptr, BAD_CAST "m_next_level_cost", BAD_CAST m_next_level_cost_oss.str().c_str());

  xmlAddChild(Player_node, m_talents.to_xml());

  return Player_node;
}

}
