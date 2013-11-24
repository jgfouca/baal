#include "TalentTree.hpp"
#include "Spell.hpp"
#include "BaalExceptions.hpp"
#include "Player.hpp"
#include "SpellFactory.hpp"

namespace baal {

///////////////////////////////////////////////////////////////////////////////
void TalentTree::add(const std::string& spell_name)
///////////////////////////////////////////////////////////////////////////////
{
  map_type::iterator find_iter = m_spell_level_map.find(spell_name);
  unsigned spell_level;

  // Compute implied spell-level
  if (find_iter != m_spell_level_map.end()) {
    spell_level = find_iter->second + 1;
  }
  else {
    spell_level = 1;
  }

  // Check if it is OK for them to learn this spell
  check_prereqs(spell_name, spell_level, m_player.level());

  // Add spell
  if (find_iter == m_spell_level_map.end()) {
    m_spell_level_map[spell_name] = 1;
  }
  else {
    m_spell_level_map[spell_name] += 1;
  }

  ++m_num_learned;

  validate_invariants();
}

///////////////////////////////////////////////////////////////////////////////
bool TalentTree::has(const Spell& spell) const
///////////////////////////////////////////////////////////////////////////////
{
  return has(spell.name(), spell.level());
}

///////////////////////////////////////////////////////////////////////////////
bool TalentTree::has(const std::string& spell_name, unsigned spell_level) const
///////////////////////////////////////////////////////////////////////////////
{
  map_type::const_iterator find_iter = m_spell_level_map.find(spell_name);
  if (find_iter == m_spell_level_map.end()) {
    return false;
  }
  else {
    return find_iter->second >= spell_level;
  }
}

///////////////////////////////////////////////////////////////////////////////
TalentTree::query_return_type TalentTree::query_all_castable_spells() const
///////////////////////////////////////////////////////////////////////////////
{
  query_return_type rv;
  rv.reserve(m_num_learned);

  for (map_type::const_iterator
       itr = m_spell_level_map.begin();
       itr != m_spell_level_map.end();
       ++itr) {
    std::string spell_name = itr->first;
    unsigned max_level = itr->second;
    rv.push_back(std::make_pair(spell_name, max_level));
  }

  return rv;
}

///////////////////////////////////////////////////////////////////////////////
TalentTree::query_return_type TalentTree::query_all_learnable_spells() const
///////////////////////////////////////////////////////////////////////////////
{
  const unsigned num_spells = SpellFactory::num_spells();

  query_return_type rv;
  rv.reserve(num_spells);

  for (unsigned i = 0; i < num_spells; ++i) {
    std::string spell_name(SpellFactory::ALL_SPELLS[i]);
    map_type::const_iterator
      fitr= m_spell_level_map.find(spell_name);
    if (fitr != m_spell_level_map.end()) {
      if (fitr->second != MAX_SPELL_LEVEL) {
        rv.push_back(std::make_pair(spell_name, fitr->second + 1));
      }
    }
    else {
      bool prereqs_ok = true;
      try {
        check_prereqs(spell_name, 1 /*spell-level*/, m_player.level() + 1);
      }
      catch (...) {
        prereqs_ok = false;
      }
      if (prereqs_ok) {
        rv.push_back(std::make_pair(spell_name, 1 /*level*/));
      }
    }
  }
  return rv;
}

///////////////////////////////////////////////////////////////////////////////
void TalentTree::validate_invariants() const
///////////////////////////////////////////////////////////////////////////////
{
  unsigned computed_num_learned = 0;
  for (map_type::const_iterator
       itr = m_spell_level_map.begin();
       itr != m_spell_level_map.end();
       ++itr) {
    computed_num_learned += itr->second;
  }

  Require(m_num_learned == computed_num_learned,
          m_num_learned << " != " << computed_num_learned);
}

///////////////////////////////////////////////////////////////////////////////
void TalentTree::check_prereqs(const std::string& spell_name,
                               unsigned spell_level,
                               unsigned player_level) const
///////////////////////////////////////////////////////////////////////////////
{
  auto spell = SpellFactory::create_spell(spell_name,
                                      const_cast<Engine&>(m_player.engine()));
  const SpellPrereq& prereq = spell->prereq();

  RequireUser(player_level > m_num_learned,
              "You cannot learn any more spells until you level-up");

  RequireUser(spell_level <= MAX_SPELL_LEVEL,
              "You've hit the maximum level for that spell");

  RequireUser(player_level >= prereq.min_player_level(),
              "You are not high-enough level to learn that spell");

  for (const std::string& spell_name : prereq) {
    RequireUser(has(spell_name), "Missing required prereq " << spell_name);
  }
}

///////////////////////////////////////////////////////////////////////////////
xmlNodePtr TalentTree::to_xml()
///////////////////////////////////////////////////////////////////////////////
{
  xmlNodePtr TalentTree_node = xmlNewNode(nullptr, BAD_CAST "TalentTree");

  auto castable_spells = query_all_castable_spells();

  for (const std::pair<std::string, unsigned>& spec : castable_spells) {
    std::string spell_name = spec.first;
    unsigned spell_lvl     = spec.second;

    std::ostringstream spell_name_oss, spell_lvl_oss;
    spell_name_oss << spell_name;
    spell_lvl_oss << spell_lvl;

    xmlNodePtr spell_node = xmlNewNode(nullptr, BAD_CAST "spell");
    xmlNewChild(spell_node, nullptr, BAD_CAST "spell_name", BAD_CAST spell_name_oss.str().c_str());
    xmlNewChild(spell_node, nullptr, BAD_CAST "spell_lvl", BAD_CAST spell_lvl_oss.str().c_str());

    xmlAddChild(TalentTree_node, spell_node);
  }

   return TalentTree_node;
}

///////////////////////////////////////////////////////////////////////////////
unsigned TalentTree::spell_skill(const std::string& spell_name) const
///////////////////////////////////////////////////////////////////////////////
{
  map_type::const_iterator find_iter = m_spell_level_map.find(spell_name);
  if (find_iter != m_spell_level_map.end()) {
    return find_iter->second;
  }
  else {
    return 0;
  }
}

}
