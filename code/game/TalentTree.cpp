#include "TalentTree.hpp"
#include "Spell.hpp"
#include "BaalExceptions.hpp"
#include "Player.hpp"
#include "SpellFactory.hpp"

using namespace baal;

///////////////////////////////////////////////////////////////////////////////
void TalentTree::add(const Spell& spell, const Player& player)
///////////////////////////////////////////////////////////////////////////////
{
  RequireUser(player.level() > m_num_learned,
              "You cannot learn any more spells until you level-up");

  check_prereqs(spell, player);

  std::string spell_name  = spell.name();
  unsigned    spell_level = spell.level();

  std::map<std::string, unsigned>::iterator
    find_iter = m_spell_level_map.find(spell_name);

  if (find_iter == m_spell_level_map.end()) {
    // If learning for first time, it had better be level 1
    RequireUser(spell_level == 1, "You must learn level 1 first.");
    m_spell_level_map[spell_name] = 1;
  }
  else {
    RequireUser(!has(spell), "You already know " << spell);
    RequireUser(find_iter->second == spell_level - 1,
      "You must learn level " << spell_level - 1 << " of that spell first");
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
  std::map<std::string, unsigned>::const_iterator
    find_iter = m_spell_level_map.find(spell_name);
  if (find_iter == m_spell_level_map.end()) {
    return false;
  }
  else {
    return find_iter->second >= spell_level;
  }
}

///////////////////////////////////////////////////////////////////////////////
void TalentTree::query_all_castable_spells(std::vector<std::pair<std::string, unsigned> >& rv) const
///////////////////////////////////////////////////////////////////////////////
{
  rv.clear();
  rv.reserve(m_num_learned);

  for (std::map<std::string, unsigned>::const_iterator
       itr = m_spell_level_map.begin();
       itr != m_spell_level_map.end();
       ++itr) {
    std::string spell_name = itr->first;
    unsigned max_level = itr->second;
    for (unsigned l = 1; l <= max_level; ++l) {
      rv.push_back(std::pair<std::string, unsigned>(spell_name, l));
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
void TalentTree::query_all_learnable_spells(std::vector<std::pair<std::string, unsigned> >& rv,
                                            const Player& player) const
///////////////////////////////////////////////////////////////////////////////
{
  const unsigned num_spells = SpellFactory::num_spells();

  rv.clear();
  rv.reserve(num_spells);

  for (unsigned i = 0; i < num_spells; ++i) {
    std::string spell_name(SpellFactory::ALL_SPELLS[i]);
    std::map<std::string, unsigned>::const_iterator
      fitr= m_spell_level_map.find(spell_name);
    if (fitr != m_spell_level_map.end()) {
      rv.push_back(std::pair<std::string, unsigned>(spell_name,
                                                    fitr->second + 1));
    }
    else {
      const Spell& spell = SpellFactory::create_spell(spell_name,
                                                      1 /*level*/,
                                                      Location());
      bool prereqs_ok = true;
      try {
        check_prereqs(spell, player);
      }
      catch (...) {
        prereqs_ok = false;
      }
      if (prereqs_ok) {
        rv.push_back(std::pair<std::string, unsigned>(spell_name,
                                                      1 /*level*/));
      }
      delete &spell;
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
void TalentTree::validate_invariants() const
///////////////////////////////////////////////////////////////////////////////
{
  unsigned computed_num_learned = 0;
  for (std::map<std::string, unsigned>::const_iterator
       itr = m_spell_level_map.begin();
       itr != m_spell_level_map.end();
       ++itr) {
    computed_num_learned += itr->second;
  }

  Require(m_num_learned == computed_num_learned,
          m_num_learned << " != " << computed_num_learned);
}

///////////////////////////////////////////////////////////////////////////////
void TalentTree::check_prereqs(const Spell& spell, const Player& player) const
///////////////////////////////////////////////////////////////////////////////
{
  const SpellPrereq& prereq = spell.prereq();

  RequireUser(player.level() >= prereq.m_min_player_level,
              "You are not high-enough level to learn that spell");

  for (std::vector<std::pair<std::string, unsigned> >::const_iterator
       itr =  prereq.m_min_spell_prereqs.begin();
       itr != prereq.m_min_spell_prereqs.end();
       ++itr) {
    std::string spell_name  = itr->first;
    unsigned    spell_level = itr->second;
    RequireUser(has(spell_name, spell_level),
      "Missing required prereq " << spell_level << '[' << spell_level << ']');
  }
}

