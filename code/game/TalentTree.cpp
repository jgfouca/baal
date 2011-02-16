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
    RequireUser(spell_level == 1, "Must learn level 1 first");
    m_spell_level_map[spell_name] = 1;
  }
  else {
    RequireUser(find_iter->second == spell_level - 1,
                "Must learn level " << spell_level - 1 << " first");
    m_spell_level_map[spell_name] += 1;
  }
}

///////////////////////////////////////////////////////////////////////////////
bool TalentTree::has(const Spell& spell) const
///////////////////////////////////////////////////////////////////////////////
{
  std::string spell_name  = spell.name();
  unsigned    spell_level = spell.level();

  std::map<std::string, unsigned>::const_iterator
    find_iter = m_spell_level_map.find(spell_name);
  if (find_iter == m_spell_level_map.end()) {
    return false;
  }
  else {
    return find_iter->second >= spell_level;
  }
}
