#ifndef Spell_hpp
#define Spell_hpp

#include "SpellFactory.hpp"
#include "BaalCommon.hpp"

#include <iosfwd>
#include <vector>

namespace baal {

// We use this file to define all the spells. This will avoid
// creation of lots of very small hpp/cpp files.

class Engine;
class LandTile;
class City;
class Interface;

/**
 * Constructor will initialize all the static prereq members in
 * all the spell classes.
 */
class SpellPrereqStaticInitializer
{
 public:
  SpellPrereqStaticInitializer();
};

typedef std::pair<std::string, unsigned> Prereq;

/**
 * Defines the prerequisits for a spell. The previous level of any
 * spell > level 1 is always a prereq.
 */
struct SpellPrereq
{
  SpellPrereq()
    : m_min_player_level(1),
      m_min_spell_prereqs()
  {}

  unsigned m_min_player_level;
  std::vector<Prereq> m_min_spell_prereqs;
};

/**
 * Abstract base class for all spells. The base class will take
 * care of everything except how the spell affects the world.
 */
class Spell
{
 public:
  Spell(SpellFactory::SpellName name,
        unsigned                spell_level,
        const Location&         location,
        unsigned                base_cost,
        unsigned                cost_increment,
        const SpellPrereq&      prereq);

  virtual ~Spell() {}

  // Verify that the user's attempt to cast this spell is sane.
  // This method should throw user errors so that apply doesn't
  // have to.
  virtual void verify_apply(Engine& engine) const = 0;

  // Apply should NEVER throw a User exception
  virtual unsigned apply(Engine& engine) const = 0;

  virtual unsigned cost() const
  { return m_base_cost + (m_spell_level - 1) * m_cost_increment; }

  const std::string& name() const { return m_name; }

  const SpellPrereq& prereq() const { return m_prereq; }

  unsigned level() const { return m_spell_level; }

  std::ostream& operator<<(std::ostream& out) const;

 protected:

  // Members
  std::string        m_name;
  unsigned           m_spell_level;
  Location           m_location;
  unsigned           m_base_cost;
  unsigned           m_cost_increment;
  const SpellPrereq& m_prereq;

  // Internal methods

  void kill(Interface& interface,
            LandTile& tile,
            City& city,
            unsigned num_killed) const;
};

std::ostream& operator<<(std::ostream& out, const Spell& spell);

/**
 * Increases the immediate temperature of a region. High temperatures can
 * kill people in cities or deplete soil moisture on farms.
 *
 * Enhanced by high temps, high dewpoints
 *
 * This is a tier 1 spell
 */
class Hot : public Spell
{
 public:
  Hot(unsigned        spell_level,
      const Location& location)
    : Spell(SpellFactory::HOT,
            spell_level,
            location,
            BASE_COST,
            COST_INC,
            PREREQ)
  {}

  virtual void verify_apply(Engine& engine) const;
  virtual unsigned apply(Engine& engine) const;

  static const unsigned BASE_COST = 50;
  static const unsigned COST_INC = BASE_COST / 3;
  static const unsigned DEGREES_PER_LEVEL = 5;
  static const float OCEAN_SURFACE_CHG_RATIO = .35;
  static const int KILL_THRESHOLD = 100;
  static SpellPrereq PREREQ;
};

/**
 * Decreases the immediate temperature of a region. Cold temperatures can
 * kill people in cities or kill crops.
 *
 * Enhanced by low temps, low dewpoints
 *
 * This is a tier 1 spell
 */
class Cold : public Spell
{
 public:
  Cold(unsigned        spell_level,
       const Location& location)
    : Spell(SpellFactory::COLD,
            spell_level,
            location,
            BASE_COST,
            COST_INC,
            PREREQ)
  {}

  virtual void verify_apply(Engine& engine) const;
  virtual unsigned apply(Engine& engine) const;

  static const unsigned BASE_COST = 50;
  static const unsigned COST_INC = BASE_COST / 3;
  static const unsigned DEGREES_PER_LEVEL = 5;
  static const float OCEAN_SURFACE_CHG_RATIO = .35;
  static const int KILL_THRESHOLD = 0;
  static SpellPrereq PREREQ;
};

/**
 * A weak direct damage spell against cities, this spell causes
 * generic misfortune within a city.
 *
 * Enhanced by nothing. A civ's technology acts as a resistance to
 * this spell.
 *
 * This is a tier 1 spell
 */
class Harm : public Spell
{
 public:
  Harm(unsigned        spell_level,
       const Location& location)
    : Spell(SpellFactory::HARM,
            spell_level,
            location,
            BASE_COST,
            COST_INC,
            PREREQ)
  {}

  virtual void verify_apply(Engine& engine) const;
  virtual unsigned apply(Engine& engine) const;

  static const unsigned BASE_COST = 50;
  static const unsigned COST_INC = BASE_COST / 3;
  static const float KILL_PCT_PER_LEVEL = 0.01;
  static SpellPrereq PREREQ;
};

/**
 * Increases the immediate wind speed of a region.
 *
 * Enhanced by high winds.
 *
 * This is a tier 1 spell
 */
class WindSpell : public Spell
{
 public:
  WindSpell(unsigned        spell_level,
            const Location& location)
    : Spell(SpellFactory::WIND,
            spell_level,
            location,
            BASE_COST,
            COST_INC,
            PREREQ)
  {}

  virtual void verify_apply(Engine& engine) const { /*TODO*/ }
  virtual unsigned apply(Engine& engine) const { return 0; /*TODO*/ }

  static const unsigned BASE_COST = 50;
  static const unsigned COST_INC = BASE_COST / 3;
  static SpellPrereq PREREQ;
};

/**
 * Starts a fire at a location. Fires will kill people in cities and
 * destroy infrastructure.
 *
 * Enhanced by high wind, low dewpoint, high temperature, and low soil
 * moisture.
 *
 * This is a tier 2 spell
 */
class Fire : public Spell
{
 public:
  Fire(unsigned        spell_level,
       const Location& location)
    : Spell(SpellFactory::FIRE,
            spell_level,
            location,
            BASE_COST,
            COST_INC,
            PREREQ)
  {}

  virtual void verify_apply(Engine& engine) const { /*TODO*/ }
  virtual unsigned apply(Engine& engine) const;

  static const unsigned BASE_COST = 100;
  static const unsigned COST_INC = BASE_COST / 3;
  static SpellPrereq PREREQ;
};

/**
 * Spawn severe thunderstorms. These storms have a chance to cause floods,
 * tornadoes, and high winds. Lightning can kill city dwellers.
 *
 * Enhanced by high wind, high dewpoint, high temperature, low pressure, and
 * high temperature differentials.
 *
 * This is a tier 2 spell
 */
class Tstorm : public Spell
{
 public:
  Tstorm(unsigned        spell_level,
         const Location& location)
    : Spell(SpellFactory::TSTORM,
            spell_level,
            location,
            BASE_COST,
            COST_INC,
            PREREQ)
  {}

  virtual void verify_apply(Engine& engine) const { /*TODO*/ }
  virtual unsigned apply(Engine& engine) const { return 0; /*TODO*/ }

  static const unsigned BASE_COST = 100;
  static const unsigned COST_INC = BASE_COST / 3;
  static SpellPrereq PREREQ;
};

/**
 * Spawn a large snow storm. Temporarily drastically reduces yields on tiles.
 *
 * Enhanced by high dewpoint, low temperature, low pressure.
 *
 * This is a tier 2 spell
 */
class Snow : public Spell
{
 public:
  Snow(unsigned        spell_level,
       const Location& location)
    : Spell(SpellFactory::SNOW,
            spell_level,
            location,
            BASE_COST,
            COST_INC,
            PREREQ)
  {}

  virtual void verify_apply(Engine& engine) const { /*TODO*/ }
  virtual unsigned apply(Engine& engine) const { return 0; /*TODO*/ }

  static const unsigned BASE_COST = 100;
  static const unsigned COST_INC = BASE_COST / 3;
  static SpellPrereq PREREQ;
};

/**
 * Cause an avalanche. This can devastate mountain infrasture and mountain
 * cities.
 *
 * Enhanced by high snowpack, ongoing snowstorm/blizzard.
 *
 * This is a tier 3 spell
 */
class Avalanche : public Spell
{
 public:
  Avalanche(unsigned        spell_level,
            const Location& location)
    : Spell(SpellFactory::AVALANCHE,
            spell_level,
            location,
            BASE_COST,
            COST_INC,
            PREREQ)
  {}

  virtual void verify_apply(Engine& engine) const { /*TODO*/ }
  virtual unsigned apply(Engine& engine) const { return 0; /*TODO*/ }

  static const unsigned BASE_COST = 200;
  static const unsigned COST_INC = BASE_COST / 3;
  static SpellPrereq PREREQ;
};

/**
 * Cause a flooding rainstorm. Can kill people in cities and destroy
 * infrastructure.
 *
 * Enhanced by high soil moisture, high dewpoints, low pressure.
 *
 * This is a tier 3 spell
 */
class Flood : public Spell
{
 public:
  Flood(unsigned        spell_level,
        const Location& location)
    : Spell(SpellFactory::FLOOD,
            spell_level,
            location,
            BASE_COST,
            COST_INC,
            PREREQ)
  {}

  virtual void verify_apply(Engine& engine) const { /*TODO*/ }
  virtual unsigned apply(Engine& engine) const { return 0; /*TODO*/ }

  static const unsigned BASE_COST = 200;
  static const unsigned COST_INC = BASE_COST / 3;
  static SpellPrereq PREREQ;
};

/**
 * Causes abnormally dry weather. Hurts food production by reducing soil
 * moisture.
 *
 * Enhanced by high temperatures, low dewpoints, high pressure.
 *
 * This is a tier 3 spell
 */
class Dry : public Spell
{
 public:
  Dry(unsigned        spell_level,
      const Location& location)
    : Spell(SpellFactory::DRY,
            spell_level,
            location,
            BASE_COST,
            COST_INC,
            PREREQ)
  {}

  virtual void verify_apply(Engine& engine) const { /*TODO*/ }
  virtual unsigned apply(Engine& engine) const { return 0; /*TODO*/ }

  static const unsigned BASE_COST = 200;
  static const unsigned COST_INC = BASE_COST / 3;
  static SpellPrereq PREREQ;
};

/**
 * Causes a massive snow/wind storm. Drastically reduces temperatures. Will
 * kill people in cities and drastically reduce tile yields temporarily.
 *
 * Enhanced by low temperatures, high dewpoints, low pressure, high winds.
 *
 * This is a tier 3 spell
 */
class Blizzard : public Spell
{
 public:
  Blizzard(unsigned        spell_level,
           const Location& location)
    : Spell(SpellFactory::BLIZZARD,
            spell_level,
            location,
            BASE_COST,
            COST_INC,
            PREREQ)
  {}

  virtual void verify_apply(Engine& engine) const { /*TODO*/ }
  virtual unsigned apply(Engine& engine) const { return 0; /*TODO*/ }

  static const unsigned BASE_COST = 200;
  static const unsigned COST_INC = BASE_COST / 3;
  static SpellPrereq PREREQ;
};

/**
 * Causes a large severe thunderstorm outbreak with tornadoes. Severe storms
 * impact a large area. Each tornado has a chance of scoring a "direct hit".
 * Infrastructure/cities that get a direct hit will take serious damage.
 *
 * Enhanced by same things that enhance thunderstorms.
 *
 * This is a tier 3 spell
 */
class Tornado : public Spell
{
 public:
  Tornado(unsigned        spell_level,
          const Location& location)
    : Spell(SpellFactory::TORNADO,
            spell_level,
            location,
            BASE_COST,
            COST_INC,
            PREREQ)
  {}

  virtual void verify_apply(Engine& engine) const { /*TODO*/ }
  virtual unsigned apply(Engine& engine) const { return 0; /*TODO*/ }

  static const unsigned BASE_COST = 200;
  static const unsigned COST_INC = BASE_COST / 3;
  static SpellPrereq PREREQ;
};

/**
 * Will cause the next season to be abnormally warm for the surrounding region.
 * Note that this spell affects long-term weather.
 *
 * Enhanced by low soil moisture.
 *
 * This is a tier 4 spell
 */
class Heatwave : public Spell
{
 public:
  Heatwave(unsigned        spell_level,
           const Location& location)
    : Spell(SpellFactory::HEATWAVE,
            spell_level,
            location,
            BASE_COST,
            COST_INC,
            PREREQ)
  {}

  virtual void verify_apply(Engine& engine) const { /*TODO*/ }
  virtual unsigned apply(Engine& engine) const { return 0; /*TODO*/ }

  static const unsigned BASE_COST = 400;
  static const unsigned COST_INC = BASE_COST / 3;
  static SpellPrereq PREREQ;
};

/**
 * Will cause the next season to be abnormally cold for the surrounding region.
 * Note that this spell affects long-term weather.
 *
 * Enhanced by high snowpack?
 *
 * This is a tier 4 spell
 */
class Coldwave : public Spell
{
 public:
  Coldwave(unsigned        spell_level,
           const Location& location)
    : Spell(SpellFactory::COLDWAVE,
            spell_level,
            location,
            BASE_COST,
            COST_INC,
            PREREQ)
  {}

  virtual void verify_apply(Engine& engine) const { /*TODO*/ }
  virtual unsigned apply(Engine& engine) const { return 0; /*TODO*/ }

  static const unsigned BASE_COST = 400;
  static const unsigned COST_INC = BASE_COST / 3;
  static SpellPrereq PREREQ;
};

/**
 * Will cause the next season to be abnormally dry for the surrounding region.
 * Note that this spell affects long-term weather.
 *
 * Enhanced by low soil moisture.
 *
 * This is a tier 4 spell
 */
class Drought : public Spell
{
 public:
  Drought(unsigned        spell_level,
          const Location& location)
    : Spell(SpellFactory::DROUGHT,
            spell_level,
            location,
            BASE_COST,
            COST_INC,
            PREREQ)
  {}

  virtual void verify_apply(Engine& engine) const { /*TODO*/ }
  virtual unsigned apply(Engine& engine) const { return 0; /*TODO*/ }

  static const unsigned BASE_COST = 400;
  static const unsigned COST_INC = BASE_COST / 3;
  static SpellPrereq PREREQ;
};

/**
 * Will cause the next season to be abnormally moist for the surrounding region.
 * Note that this spell affects long-term weather.
 *
 * Enhanced by high soil moisture.
 *
 * This is a tier 4 spell
 */
class Monsoon : public Spell
{
 public:
  Monsoon(unsigned        spell_level,
          const Location& location)
    : Spell(SpellFactory::MONSOON,
            spell_level,
            location,
            BASE_COST,
            COST_INC,
            PREREQ)
  {}

  virtual void verify_apply(Engine& engine) const { /*TODO*/ }
  virtual unsigned apply(Engine& engine) const { return 0; /*TODO*/ }

  static const unsigned BASE_COST = 400;
  static const unsigned COST_INC = BASE_COST / 3;
  static SpellPrereq PREREQ;
};

/**
 * Will cause a disease to breakout in the targetted city.
 *
 * Enhanced by extreme temperatures, famine, and large cities.
 *
 * This is a tier 5 spell
 */
class Disease : public Spell
{
 public:
  Disease(unsigned        spell_level,
          const Location& location)
    : Spell(SpellFactory::DISEASE,
            spell_level,
            location,
            BASE_COST,
            COST_INC,
            PREREQ)
  {}

  virtual void verify_apply(Engine& engine) const { /*TODO*/ }
  virtual unsigned apply(Engine& engine) const { return 0; /*TODO*/ }

  static const unsigned BASE_COST = 800;
  static const unsigned COST_INC = BASE_COST / 3;
  static SpellPrereq PREREQ;
};

/**
 * Will cause an earthquake to occur. Devastates nearby cities and
 * infrastructure.
 *
 * Enhanced by plate tension.
 *
 * This is a tier 5 spell
 */
class Earthquake : public Spell
{
 public:
  Earthquake(unsigned        spell_level,
             const Location& location)
    : Spell(SpellFactory::EARTHQUAKE,
            spell_level,
            location,
            BASE_COST,
            COST_INC,
            PREREQ)
  {}

  virtual void verify_apply(Engine& engine) const { /*TODO*/ }
  virtual unsigned apply(Engine& engine) const { return 0; /*TODO*/ }

  static const unsigned BASE_COST = 800;
  static const unsigned COST_INC = BASE_COST / 3;
  static SpellPrereq PREREQ;
};

/**
 * Will spawn a hurricane. Causes floods, high winds, tornadoes, and tstorms
 * over a large region.
 *
 * Enhanced by warm sea temperatures, high dewpoints, high pressure and the
 * farther away from land they start, the larger they get, but the higher
 * chance they have to miss land.
 *
 * This is a tier 5 spell
 */
class Hurricane : public Spell
{
 public:
  Hurricane(unsigned        spell_level,
            const Location& location)
    : Spell(SpellFactory::HURRICANE,
            spell_level,
            location,
            BASE_COST,
            COST_INC,
            PREREQ)
  {}

  virtual void verify_apply(Engine& engine) const { /*TODO*/ }
  virtual unsigned apply(Engine& engine) const { return 0; /*TODO*/ }

  static const unsigned BASE_COST = 800;
  static const unsigned COST_INC = BASE_COST / 3;
  static SpellPrereq PREREQ;
};

/**
 * Will cause a large outbreak affecting all nearby cities.
 *
 * Same enhancements as disease.
 *
 * This is a tier 6 spell
 */
class Plague : public Spell
{
 public:
  Plague(unsigned        spell_level,
         const Location& location)
    : Spell(SpellFactory::PLAGUE,
            spell_level,
            location,
            BASE_COST,
            COST_INC,
            PREREQ)
  {}

  virtual void verify_apply(Engine& engine) const { /*TODO*/ }
  virtual unsigned apply(Engine& engine) const { return 0; /*TODO*/ }

  static const unsigned BASE_COST = 1600;
  static const unsigned COST_INC = BASE_COST / 3;
  static SpellPrereq PREREQ;
};

/**
 * Will cause a large volcanic eruption; everything nearby will be
 * eradicated.
 *
 * Enhanced by magma build-up.
 *
 * This is a tier 6 spell
 */
class Volcano : public Spell
{
 public:
  Volcano(unsigned        spell_level,
          const Location& location)
    : Spell(SpellFactory::VOLCANO,
            spell_level,
            location,
            BASE_COST,
            COST_INC,
            PREREQ)
  {}

  virtual void verify_apply(Engine& engine) const { /*TODO*/ }
  virtual unsigned apply(Engine& engine) const { return 0; /*TODO*/ }

  static const unsigned BASE_COST = 1600;
  static const unsigned COST_INC = BASE_COST / 3;
  static SpellPrereq PREREQ;
};

/**
 * Hurls an asteriod at the planet. There is a chance the asteroid will miss or
 * hit somewhere other than the targetted area. Will eradicate everything over
 * a large area.
 *
 * This is a tier 7 spell
 */
class Asteroid : public Spell
{
 public:
  Asteroid(unsigned        spell_level,
           const Location& location)
    : Spell(SpellFactory::ASTEROID,
            spell_level,
            location,
            BASE_COST,
            COST_INC,
            PREREQ)
  {}

  virtual void verify_apply(Engine& engine) const { /*TODO*/ }
  virtual unsigned apply(Engine& engine) const { return 0; /*TODO*/ }

  static const unsigned BASE_COST = 3200;
  static const unsigned COST_INC = BASE_COST / 3;
  static SpellPrereq PREREQ;
};

}

#endif
