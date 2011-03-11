#include "TalentTree.hpp"
#include "Spell.hpp"
#include "BaalExceptions.hpp"

using namespace baal;

///////////////////////////////////////////////////////////////////////////////
void TalentTree::add(const Spell& spell)
///////////////////////////////////////////////////////////////////////////////
{
  std::string spell_name  = spell.name();
  unsigned    spell_level = spell.level();

  std::map<std::string, unsigned>::iterator
    find_iter = m_spell_level_map.find(spell_name);
  if (find_iter == m_spell_level_map.end()) {
    Require(spell_level == 1, "Prereq checking failed for " << spell);
    m_spell_level_map[spell_name] = 1;
  }
  else {
    RequireUser(!has(spell), "You already know " << spell);
    Require(find_iter->second == spell_level - 1,
            "Prereq checking failed for " << spell);
    m_spell_level_map[spell_name] += 1;
  }

  ++m_num_learned;
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
