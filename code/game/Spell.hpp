#ifndef Spell_hpp
#define Spell_hpp

#include "SpellFactory.hpp"
#include "BaalCommon.hpp"

#include <iosfwd>
#include <vector>

namespace baal {

// We use this file to define all the spells. This will avoid
// creation of lots of very small hpp/cpp files.


class WorldTile;
class City;

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
  Spell(const std::string&  name,
        unsigned            spell_level,
        const Location&     location,
        unsigned            base_cost,
        unsigned            cost_increment,
        const SpellPrereq&  prereq);

  virtual ~Spell() {}

  // Verify that the user's attempt to cast this spell is sane.
  // This method should throw user errors so that apply doesn't
  // have to.
  virtual void verify_apply() const = 0;

  // Apply should NEVER throw a User exception
  virtual unsigned apply() const = 0;

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

  // Constants

  static const unsigned CITY_DESTROY_EXP_BONUS = 1000;
  static const unsigned CHAIN_REACTION_BONUS = 2;

  // Internal methods

  // Returns exp gained
  unsigned kill(City& city,
                float pct_killed) const;

  // Returns exp gained
  unsigned destroy_infra(WorldTile& tile, unsigned max_destroyed) const;

  void damage_tile(WorldTile& tile, float damage) const;

  // Some disasters can spawn other disasters (chain reaction). This method
  // encompassed the implementation of this phenominon. The amount of exp
  // gained is returned.
  unsigned spawn(const std::string& spell_name, unsigned spell_level) const;
};

std::ostream& operator<<(std::ostream& out, const Spell& spell);

// TODO - Do we want spells for controlling all the basic properties
// of the atmosphere? Or do we want to leave some up to pure chance (pressure)?

/**
 * Increases the immediate temperature of a region. High temperatures can
 * kill people in cities or deplete soil moisture on farms. This spell is not
 * intended to be a primary damage dealer; instead, you should be using this
 * spell to enhance the more-powerful spells.
 *
 * Enhanced by high temps, high dewpoints. Decreased by AI tech level.
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

  virtual void verify_apply() const;
  virtual unsigned apply() const;

  static const unsigned BASE_COST = 50;
  static const unsigned COST_INC = BASE_COST / 3;
  static const unsigned DEGREES_PER_LEVEL = 7;
  static const float OCEAN_SURFACE_CHG_RATIO = .35;
  static const int KILL_THRESHOLD = 100;
  static const float EXPONENT = 1.5;
  static const unsigned DIVISOR = 5;
  static SpellPrereq PREREQ;
};

/**
 * Decreases the immediate temperature of a region. Cold temperatures can
 * kill people in cities or kill crops. This spell is not
 * intended to be a primary damage dealer; instead, you should be using this
 * spell to enhance the more-powerful spells.
 *
 * Enhanced by low temps, low dewpoints. Decreased by AI tech level.
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

  virtual void verify_apply() const;
  virtual unsigned apply() const;

  static const unsigned BASE_COST = 50;
  static const unsigned COST_INC = BASE_COST / 3;
  static const unsigned DEGREES_PER_LEVEL = 5;
  static const float OCEAN_SURFACE_CHG_RATIO = .35;
  static const int KILL_THRESHOLD = 0;
  static const float WIND_BONUS_PER_MPH = 0.01;
  static const float FAMINE_BONUS = 2.0;
  static const float EXPONENT = 1.5;
  static const unsigned DIVISOR = 5;
  static SpellPrereq PREREQ;
};

/**
 * A weak direct damage spell against cities, this spell causes
 * an infection to spread within a city. This spell should be very
 * useful for getting players to the higher level spells.
 *
 * Enhanced by extreme temperatures, recent famine, and size of city.
 * Decreased by AI tech level.
 *
 * This is a tier 1 spell
 */
class Infect : public Spell
{
 public:
  Infect(unsigned        spell_level,
         const Location& location)
    : Spell(SpellFactory::INFECT,
            spell_level,
            location,
            BASE_COST,
            COST_INC,
            PREREQ)
  {}

  virtual void verify_apply() const;
  virtual unsigned apply() const;

  static const unsigned BASE_COST = 50;
  static const unsigned COST_INC = BASE_COST / 3;
  static const float KILL_PCT_PER_LEVEL = 1.0;
  static const float BONUS_PER_CITY_RANK = 0.10;
  static const float FAMINE_BONUS = 2.0;
  static const int WARM_THRESHOLD = 90;
  static const int COLD_THRESHOLD = 30;
  static const float BONUS_PER_DEGREE_BEYOND_THRESHOLD = 0.1;
  static SpellPrereq PREREQ;
};

/**
 * Increases the immediate wind speed of a region. High wind speeds can
 * kill, but this spell is generally more useful in combinations rather
 * than a direct damage spell. High wind speeds can damage farm
 * infrastructure.
 *
 * Enhanced by high winds. Decreased by AI tech level. Decreased by city
 * defense.
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

  virtual void verify_apply() const;
  virtual unsigned apply() const;

  static const unsigned BASE_COST = 50;
  static const unsigned COST_INC = BASE_COST / 3;
  static const unsigned MPH_PER_LEVEL = 20;
  static const unsigned BASE_DAMAGE_THRESHOLD = 60;
  static const unsigned MPH_PER_ADDITIONAL_INFRA_DEVASTATION = 30;
  static const unsigned KILL_THRESHOLD = 80;
  static SpellPrereq PREREQ;
};

/**
 * Starts a fire at a location. Fires will kill people in cities and
 * destroy infrastructure.
 * TODO: has a chance to spead?
 *
 * Enhanced by high wind, low dewpoint, high temperature, and low soil
 * moisture. Reduced by city defense and tech level.
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

  virtual void verify_apply() const;
  virtual unsigned apply() const;

  static const unsigned BASE_COST = 100;
  static const unsigned COST_INC = BASE_COST / 3;
  static const int TEMP_TIPPING_POINT = 75;
  static const float TEMP_EXP_BASE = 1.03;
  static const int WIND_TIPPING_POINT = 10;
  static const float WIND_EXP_BASE = 1.05;
  static const float MOISTURE_TIPPING_POINT = 0.75;
  static const float MOISTURE_EXP_BASE = 1.05; // per moisture pct
  static const float DESTRUCTIVENESS_PER_INFRA = 5.0;

  static SpellPrereq PREREQ;
};

/**
 * Spawn severe thunderstorms. These storms have a chance to cause
 * weak floods, tornadoes, and high winds, making this a good spell for
 * causing chain-reactions. Lightning can kill city dwellers and is
 * the only way for a tstorm to directly get kills.
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

  virtual void verify_apply() const;
  virtual unsigned apply() const;

  static const unsigned BASE_COST = 100;
  static const unsigned COST_INC = BASE_COST / 3;
  static const int TEMP_TIPPING_POINT = 85;
  static const float TEMP_EXP_BASE = 1.03;
  static const int WIND_TIPPING_POINT = 15;
  static const float WIND_EXP_BASE = 1.03;
  static const int PRESSURE_TIPPING_POINT = 990;
  static const float PRESSURE_EXP_BASE = 1.05;

  static const float WIND_DESTRUCTIVENESS_THRESHOLD = 10.0;
  static const float FLOOD_DESTRUCTIVENESS_THRESHOLD = 20.0;
  static const float TORNADO_DESTRUCTIVENESS_THRESHOLD = 40.0;

  static const float LIGHTING_PCT_KILL_PER_DESTRUCTIVENESS = 2.0;

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

  virtual void verify_apply() const { /*TODO*/ }
  virtual unsigned apply() const { return 0; /*TODO*/ }

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

  virtual void verify_apply() const { /*TODO*/ }
  virtual unsigned apply() const { return 0; /*TODO*/ }

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

  virtual void verify_apply() const { /*TODO*/ }
  virtual unsigned apply() const { return 0; /*TODO*/ }

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

  virtual void verify_apply() const { /*TODO*/ }
  virtual unsigned apply() const { return 0; /*TODO*/ }

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

  virtual void verify_apply() const { /*TODO*/ }
  virtual unsigned apply() const { return 0; /*TODO*/ }

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

  virtual void verify_apply() const { /*TODO*/ }
  virtual unsigned apply() const { return 0; /*TODO*/ }

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

  virtual void verify_apply() const { /*TODO*/ }
  virtual unsigned apply() const { return 0; /*TODO*/ }

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

  virtual void verify_apply() const { /*TODO*/ }
  virtual unsigned apply() const { return 0; /*TODO*/ }

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

  virtual void verify_apply() const { /*TODO*/ }
  virtual unsigned apply() const { return 0; /*TODO*/ }

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

  virtual void verify_apply() const { /*TODO*/ }
  virtual unsigned apply() const { return 0; /*TODO*/ }

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

  virtual void verify_apply() const { /*TODO*/ }
  virtual unsigned apply() const { return 0; /*TODO*/ }

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

  virtual void verify_apply() const { /*TODO*/ }
  virtual unsigned apply() const { return 0; /*TODO*/ }

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

  virtual void verify_apply() const { /*TODO*/ }
  virtual unsigned apply() const { return 0; /*TODO*/ }

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

  virtual void verify_apply() const { /*TODO*/ }
  virtual unsigned apply() const { return 0; /*TODO*/ }

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

  virtual void verify_apply() const { /*TODO*/ }
  virtual unsigned apply() const { return 0; /*TODO*/ }

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

  virtual void verify_apply() const { /*TODO*/ }
  virtual unsigned apply() const { return 0; /*TODO*/ }

  static const unsigned BASE_COST = 3200;
  static const unsigned COST_INC = BASE_COST / 3;
  static SpellPrereq PREREQ;
};

}

#endif
