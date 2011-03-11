#ifndef Spell_hpp
#define Spell_hpp

#include "BaalCommon.hpp"

#include <iosfwd>
#include <vector>

namespace baal {

// We use this file to define all the spells. This will avoid
// creation of lots of very small hpp/cpp files.

class World;
class Player;

/**
 * Constructor will initialize all the static prereq members in
 * all the spell classes.
 */
class SpellPrereqStaticInitializer
{
 public:
  SpellPrereqStaticInitializer();
};

/**
 * Defines the prerequisits for a spell. The previous level of any
 * spell > level 1 is always a prereq.
 */
struct SpellPrereq
{
  unsigned m_min_player_level;
  std::vector<std::pair<std::string, unsigned> > m_min_spell_prereqs;
};

/**
 * Abstract base class for all spells. The base class will take
 * care of everything except how the spell affects the world.
 */
class Spell
{
 public:
  Spell(const std::string& name,
        unsigned           spell_level,
        const Location&    location,
        unsigned           base_cost,
        const SpellPrereq& prereq);

  // pure virtual
  virtual void apply(World& world) const = 0;

  unsigned cost() const { return m_spell_level * m_base_cost; }

  const std::string& name() const { return m_name; }

  void verify_prereqs(const Player& player) const;

  unsigned level() const { return m_spell_level; }

  std::ostream& operator<<(std::ostream& out) const;

 protected:
  std::string        m_name;
  unsigned           m_spell_level;
  Location           m_location;
  unsigned           m_base_cost;
  const SpellPrereq& m_prereq;

 private:
  static SpellPrereqStaticInitializer s_static_prereq_init;
};

std::ostream& operator<<(std::ostream& out, const Spell& spell);

/**
 * Starts a fire at a location.
 *
 * Enhanced by high wind, low dewpoint, high temperature, and low soil
 * moisture.
 */
class FireSpell : public Spell
{
 public:
  FireSpell(const std::string& name,
            unsigned           spell_level,
            const Location&    location)
    : Spell(name, spell_level, location, BASE_COST, PREREQ)
  {}

  virtual void apply(World& world) const;

 private:
  static const unsigned BASE_COST = 50;
  static SpellPrereq PREREQ;
};

// TODO - Need many more disasters

// /**
//  * Starts a fire at a location.
//  */
// class FireSpell : public Spell
// {
//  public:
//   FireSpell(const std::string& name,
//             unsigned           spell_level,
//             const Location&    location)
//     : Spell(name, spell_level, location, BASE_COST)
//   {}

//   virtual void apply(World& world) const;

//  private:
//   static const unsigned BASE_COST = 50;
// };

}

#endif
