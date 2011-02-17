#ifndef Spell_hpp
#define Spell_hpp

#include "BaalCommon.hpp"

#include <iosfwd>

namespace baal {

// We use this file to define all the spells. This will avoid
// creation of lots of very small hpp/cpp files.

class World;

/**
 * Abstract base class for all spells. Spells must know how to apply
 * themselves to the world and how much mana they cost.
 */
class Spell
{
 public:
  Spell(const std::string& name,
        unsigned           spell_level,
        const Location&    location);

  virtual void apply(World& world) const = 0;

  virtual unsigned cost() const = 0;

  const std::string& name() const { return m_name; }

  unsigned level() const { return m_spell_level; }

  std::ostream& operator<<(std::ostream& out) const;

 protected:
  std::string m_name;
  unsigned    m_spell_level;
  Location    m_location;
};

std::ostream& operator<<(std::ostream& out, const Spell& spell);

/**
 * Starts a fire at a location.
 */
class FireSpell : public Spell
{
 public:
  FireSpell(const std::string& name,
            unsigned           spell_level,
            const Location&    location)
    : Spell(name, spell_level, location)
  {}

  virtual void apply(World& world) const;

  virtual unsigned cost() const { return m_spell_level * BASE_COST; }

 private:
  static const unsigned BASE_COST = 50;
};

// TODO - Need many more disasters

}

#endif
