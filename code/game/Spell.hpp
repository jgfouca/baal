#ifndef Spell_hpp
#define Spell_hpp

#include "SpellFactory.hpp"
#include "BaalCommon.hpp"
#include "BaalMath.hpp"

#include <iosfwd>
#include <vector>
#include <utility>
#include <functional>

#include <boost/mpl/for_each.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/mpl/transform.hpp>
#include <boost/mpl/placeholders.hpp>

#include <boost/type_traits/detail/wrap.hpp>

namespace baal {

// We use this file to define all the spells. This will avoid
// creation of lots of very small hpp/cpp files.

class WorldTile;
class City;
class Engine;

/**
 * Defines the prerequisits for a spell. The previous level of any
 * spell > level 1 is always a prereq.
 */
class SpellPrereq
{
  typedef std::vector<std::string> spell_list;
  typedef spell_list::const_iterator const_iterator;

  struct add_to_list
  {
    add_to_list(spell_list& list) : m_list(list) {}

    template <typename Spell>
    void operator()(boost::type_traits::wrap<Spell>) const
    {
      m_list.push_back(Spell::NAME);
    }

    spell_list& m_list;
  };

  SpellPrereq() = default;

 public:
  SpellPrereq(SpellPrereq&& rhs)
    : m_min_player_level(rhs.m_min_player_level),
      m_min_spell_prereqs(std::move(rhs.m_min_spell_prereqs))
  {}

  SpellPrereq& operator=(SpellPrereq&& rhs)
  {
    m_min_player_level  = rhs.m_min_player_level;
    m_min_spell_prereqs = std::move(rhs.m_min_spell_prereqs);
    return *this;
  }

  template <typename SpellList, unsigned MinLevel>
  static SpellPrereq spell_prereq_factory()
  {
    SpellPrereq rv;
    rv.m_min_player_level = MinLevel;

    // Spells are not default constructable, so we must transform
    // the list of spell types into pointer types so that they can
    // be instantiated with a default constructor and given to for_each.
    typedef typename boost::mpl::transform
      <SpellList,
       boost::type_traits::wrap<boost::mpl::placeholders::_1> >::type
      wrapped_spell_list;
    boost::mpl::for_each<wrapped_spell_list>(add_to_list(rv.m_min_spell_prereqs));
    return rv;
  }

  unsigned min_player_level() const { return m_min_player_level; }

  const_iterator begin() const { return std::begin(m_min_spell_prereqs); }

  const_iterator end() const { return std::end(m_min_spell_prereqs); }

 private:
  unsigned m_min_player_level;
  spell_list m_min_spell_prereqs;
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
        const SpellPrereq&  prereq,
        Engine&             engine);

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

  const char* info() const { return "TODO"; }

  const SpellPrereq& prereq() const { return m_prereq; }

  unsigned level() const { return m_spell_level; }

  std::ostream& operator<<(std::ostream& out) const;

 protected:

  // Members
  const std::string& m_name;
  unsigned           m_spell_level;
  Location           m_location;
  unsigned           m_base_cost;
  unsigned           m_cost_increment;
  const SpellPrereq& m_prereq;
  Engine&            m_engine;

  // Constants

  static constexpr unsigned CITY_DESTROY_EXP_BONUS = 1000;
  static constexpr unsigned CHAIN_REACTION_BONUS = 2;

  // Internal methods

  // Returns exp gained
  unsigned kill(City& city,
                float pct_killed) const;

  // Returns exp gained
  unsigned destroy_infra(WorldTile& tile, unsigned max_destroyed) const;

  void damage_tile(WorldTile& tile, float damage_pct) const;

  // Some disasters can spawn other disasters (chain reaction). This method
  // encompassed the implementation of this phenominon. The amount of exp
  // gained is returned.
  unsigned spawn(const std::string& spell_name, unsigned spell_level) const;
};

std::ostream& operator<<(std::ostream& out, const Spell& spell);

// TODO - Do we want spells for controlling all the basic properties
// of the atmosphere? Or do we want to leave some up to pure chance (pressure)?

// Spell header components are designed to maximize tweakability from
// the header file. TODO: should things like degrees-heated for Hot also be
// represented by a function?

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
      const Location& location,
      Engine&         engine)
    : Spell(NAME,
            spell_level,
            location,
            BASE_COST,
            COST_INC,
            PREREQ,
            engine)
  {
    // base_kill(temp) = (temp - KILL_THRESHOLD)^1.5 / 8;
    m_base_kill_func = std::bind(baal::poly_growth,
                                      std::placeholders::_1,
                                      KILL_THRESHOLD,
                                      1.5,
                                      8);

    // tech_penalty(tech_level) = sqrt(tech_level)
    m_tech_penalty_func = std::bind(baal::sqrt,
                                         std::placeholders::_1,
                                         0.0); // no threshold
  }

  virtual void verify_apply() const;
  virtual unsigned apply() const;

  static constexpr unsigned BASE_COST = 50;
  static constexpr unsigned COST_INC = BASE_COST / 3;
  static constexpr unsigned DEGREES_PER_LEVEL = 7;
  static constexpr float OCEAN_SURFACE_CHG_RATIO = .35;
  static constexpr int KILL_THRESHOLD = 100;
  static const SpellPrereq PREREQ;
  static const std::string NAME;

 private:
  std::function<float(float)> m_base_kill_func;
  std::function<float(float)> m_tech_penalty_func;
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
       const Location& location,
       Engine&         engine)
    : Spell(NAME,
            spell_level,
            location,
            BASE_COST,
            COST_INC,
            PREREQ,
            engine)
  {
    // base_kill(temp) = (KILL_THRESHOLD - temp)^1.5 / 8;
    m_base_kill_func = std::bind(baal::poly_growth,
                                      std::placeholders::_1,
                                      -KILL_THRESHOLD,
                                      1.5,
                                      8);

    // wind_bonus(speed) = 1.02^speed , diminishing returns at 40
    m_wind_bonus_func = std::bind(baal::exp_growth,
                                       std::placeholders::_1,
                                       0.0,  // no threshold
                                       1.02, // slow growth
                                       40.0);

    // tech_penalty(tech_level) = tech_level
    m_tech_penalty_func = std::bind(baal::linear_growth,
                                         std::placeholders::_1,
                                         0.0, // no threshold
                                         1.0); // no multiplier
  }

  virtual void verify_apply() const;
  virtual unsigned apply() const;

  static constexpr unsigned BASE_COST = 50;
  static constexpr unsigned COST_INC = BASE_COST / 3;
  static constexpr unsigned DEGREES_PER_LEVEL = 7;
  static constexpr float OCEAN_SURFACE_CHG_RATIO = .35;
  static constexpr int KILL_THRESHOLD = 0;
  static constexpr float FAMINE_BONUS = 2.0;
  static const SpellPrereq PREREQ;
  static const std::string NAME;

 private:
  std::function<float(float)> m_base_kill_func;
  std::function<float(float)> m_wind_bonus_func;
  std::function<float(float)> m_tech_penalty_func;
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
         const Location& location,
         Engine&         engine)
    : Spell(NAME,
            spell_level,
            location,
            BASE_COST,
            COST_INC,
            PREREQ,
            engine)
  {
    // base_kill(spell_level) = spell_level^1.3
    m_base_kill_func = std::bind(baal::poly_growth,
                                      std::placeholders::_1,
                                      0.0, // no threshold
                                      1.3,
                                      1.0); // no divisor

    // city_bonus(size) = 1.05^size
    m_city_size_bonus_func = std::bind(baal::exp_growth,
                                            std::placeholders::_1,
                                            0.0,  // no threshold
                                            1.05, // fast growth
                                            MAX_FLOAT); // never diminishes

    // extreme_temp_bonus(degrees_extreme) = 1.03^degrees_extreme
    m_extreme_temp_bonus_func = std::bind(baal::exp_growth,
                                               std::placeholders::_1,
                                               0.0,  // no threshold
                                               1.03, // medium growth
                                               MAX_FLOAT); // never diminishes

    // tech_penalty(tech_level) = tech_level
    m_tech_penalty_func = std::bind(baal::linear_growth,
                                         std::placeholders::_1,
                                         0.0, // no threshold
                                         1.0); // no multiplier
  }

  virtual void verify_apply() const;
  virtual unsigned apply() const;

  static constexpr unsigned BASE_COST = 50;
  static constexpr unsigned COST_INC = BASE_COST / 3;
  static constexpr float FAMINE_BONUS = 2.0;
  static constexpr int WARM_THRESHOLD = 90;
  static constexpr int COLD_THRESHOLD = 30;
  static const SpellPrereq PREREQ;
  static const std::string NAME;

 private:
  std::function<float(float)> m_base_kill_func;
  std::function<float(float)> m_city_size_bonus_func;
  std::function<float(float)> m_extreme_temp_bonus_func;
  std::function<float(float)> m_tech_penalty_func;
};

/**
 * Increases the immediate wind speed of a region. High wind speeds can
 * kill, but this spell is generally more useful in combinations rather
 * than a direct damage spell. High wind speeds can damage
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
            const Location& location,
            Engine&         engine)
    : Spell(NAME,
            spell_level,
            location,
            BASE_COST,
            COST_INC,
            PREREQ,
            engine)
  {
    // base_kill(speed) = 1.03^(speed - KILL_THRESHOLD)
    m_base_kill_func = std::bind(baal::exp_growth,
                                      std::placeholders::_1,
                                      KILL_THRESHOLD,
                                      1.03, // medium growth
                                      MAX_FLOAT); // never dimishes

    // base_infra_destroyed(speed) = 1.03^(speed - DAMAGE_THRESHOLD)
    m_base_infra_destroy_func = std::bind(baal::exp_growth,
                                               std::placeholders::_1,
                                               DAMAGE_THRESHOLD, // threshold
                                               1.03, // medium growth
                                               MAX_FLOAT); // never diminishes

    // defense_penalty(defense_level) = sqrt(tech_level)
    m_defense_penalty_func = std::bind(baal::sqrt,
                                            std::placeholders::_1,
                                            0.0); // no threshold

    // tech_penalty(tech_level) = sqrt(tech_level)
    m_tech_penalty_func = std::bind(baal::sqrt,
                                         std::placeholders::_1,
                                         0.0); // no threshold
  }

  virtual void verify_apply() const;
  virtual unsigned apply() const;

  static constexpr unsigned BASE_COST = 50;
  static constexpr unsigned COST_INC = BASE_COST / 3;
  static constexpr unsigned MPH_PER_LEVEL = 20;
  static constexpr unsigned DAMAGE_THRESHOLD = 60;
  static constexpr unsigned KILL_THRESHOLD = 80;
  static const SpellPrereq PREREQ;
  static const std::string NAME;

 private:
  std::function<float(float)> m_base_kill_func;
  std::function<float(float)> m_base_infra_destroy_func;
  std::function<float(float)> m_defense_penalty_func;
  std::function<float(float)> m_tech_penalty_func;
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
       const Location& location,
       Engine&         engine)
    : Spell(NAME,
            spell_level,
            location,
            BASE_COST,
            COST_INC,
            PREREQ,
            engine)
  {
    // base_destructiveness(spell_level) = spell_level^1.3
    m_base_destructiveness_func = std::bind(baal::poly_growth,
                                                 std::placeholders::_1,
                                                 0.0, // no threshold
                                                 1.3,
                                                 1.0); // no divisor

    // wind_effect(speed) = 1.05^(speed - tipping_pt)
    m_wind_effect_func =
      std::bind(baal::exp_growth,
                     std::placeholders::_1,
                     WIND_TIPPING_POINT,
                     1.05, // fast growth
                     30.0); // diminishes at 30 mph beyond the tipping pint

    // temp_effect(temp) = 1.03^(temp - tipping_pt)
    m_temp_effect_func = std::bind(baal::exp_growth,
                                        std::placeholders::_1,
                                        TEMP_TIPPING_POINT,
                                        1.03, // medium growth
                                        MAX_FLOAT); // never diminishes

    // moisture_effect(moisture_deficit%) = 1.03^(moisture_deficit%)
    m_moisture_effect_func =
      std::bind(baal::exp_growth,
                     std::placeholders::_1,
                     0.0, // no threshold
                     1.05, // fast growth
                     40.0); // diminishes at 30% below dry


    // base_infra_destroyed(destructiveness) = 1.05^(destructiveness)
    m_base_infra_destroy_func = std::bind(baal::exp_growth,
                                               std::placeholders::_1,
                                               0.0, // no threshold
                                               1.05, // fast growth
                                               MAX_FLOAT); // never diminishes

    // base_kill(destructiveness) = destructiveness
    m_base_kill_func = std::bind(baal::linear_growth,
                                      std::placeholders::_1,
                                      0.0, // no threshold
                                      1.0); // no multiplier

    // defense_penalty(defense_level) = sqrt(tech_level)
    m_defense_penalty_func = std::bind(baal::sqrt,
                                            std::placeholders::_1,
                                            0.0); // no threshold

    // tech_penalty(tech_level) = sqrt(tech_level)
    m_tech_penalty_func = std::bind(baal::sqrt,
                                         std::placeholders::_1,
                                         0.0); // no threshold
  }

  virtual void verify_apply() const;
  virtual unsigned apply() const;

  static constexpr unsigned BASE_COST = 100;
  static constexpr unsigned COST_INC = BASE_COST / 3;
  static constexpr int TEMP_TIPPING_POINT = 75;
  static constexpr float TEMP_EXP_BASE = 1.03;
  static constexpr int WIND_TIPPING_POINT = 20;
  static constexpr float WIND_EXP_BASE = 1.05;
  static constexpr float MOISTURE_TIPPING_POINT = 0.75;
  static constexpr float MOISTURE_EXP_BASE = 1.05; // per moisture pct
  static constexpr float DESTRUCTIVENESS_PER_INFRA = 5.0;

  static const SpellPrereq PREREQ;
  static const std::string NAME;

 private:
  std::function<float(float)> m_base_destructiveness_func;
  std::function<float(float)> m_temp_effect_func;
  std::function<float(float)> m_wind_effect_func;
  std::function<float(float)> m_moisture_effect_func;
  std::function<float(float)> m_base_kill_func;
  std::function<float(float)> m_base_infra_destroy_func;
  std::function<float(float)> m_defense_penalty_func;
  std::function<float(float)> m_tech_penalty_func;
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
         const Location& location,
         Engine&         engine)
    : Spell(NAME,
            spell_level,
            location,
            BASE_COST,
            COST_INC,
            PREREQ,
            engine)
  {}

  virtual void verify_apply() const;
  virtual unsigned apply() const;

  static constexpr unsigned BASE_COST = 100;
  static constexpr unsigned COST_INC = BASE_COST / 3;
  static constexpr int TEMP_TIPPING_POINT = 85;
  static constexpr float TEMP_EXP_BASE = 1.03;
  static constexpr int WIND_TIPPING_POINT = 15;
  static constexpr float WIND_EXP_BASE = 1.03;
  static constexpr int PRESSURE_TIPPING_POINT = 990;
  static constexpr float PRESSURE_EXP_BASE = 1.05;

  static constexpr float WIND_DESTRUCTIVENESS_THRESHOLD = 10.0;
  static constexpr float FLOOD_DESTRUCTIVENESS_THRESHOLD = 20.0;
  static constexpr float TORNADO_DESTRUCTIVENESS_THRESHOLD = 40.0;

  static constexpr float LIGHTING_PCT_KILL_PER_DESTRUCTIVENESS = 0.2;

  static const SpellPrereq PREREQ;
  static const std::string NAME;
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
       const Location& location,
       Engine&         engine)
    : Spell(NAME,
            spell_level,
            location,
            BASE_COST,
            COST_INC,
            PREREQ,
            engine)
  {}

  virtual void verify_apply() const { /*TODO*/ }
  virtual unsigned apply() const { return 0; /*TODO*/ }

  static constexpr unsigned BASE_COST = 100;
  static constexpr unsigned COST_INC = BASE_COST / 3;
  static const SpellPrereq PREREQ;
  static const std::string NAME;
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
            const Location& location,
            Engine&         engine)
    : Spell(NAME,
            spell_level,
            location,
            BASE_COST,
            COST_INC,
            PREREQ,
            engine)
  {}

  virtual void verify_apply() const { /*TODO*/ }
  virtual unsigned apply() const { return 0; /*TODO*/ }

  static constexpr unsigned BASE_COST = 200;
  static constexpr unsigned COST_INC = BASE_COST / 3;
  static const SpellPrereq PREREQ;
  static const std::string NAME;
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
        const Location& location,
        Engine&         engine)
    : Spell(NAME,
            spell_level,
            location,
            BASE_COST,
            COST_INC,
            PREREQ,
            engine)
  {}

  virtual void verify_apply() const { /*TODO*/ }
  virtual unsigned apply() const { return 0; /*TODO*/ }

  static constexpr unsigned BASE_COST = 200;
  static constexpr unsigned COST_INC = BASE_COST / 3;
  static const SpellPrereq PREREQ;
  static const std::string NAME;
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
      const Location& location,
      Engine&         engine)
    : Spell(NAME,
            spell_level,
            location,
            BASE_COST,
            COST_INC,
            PREREQ,
            engine)
  {}

  virtual void verify_apply() const { /*TODO*/ }
  virtual unsigned apply() const { return 0; /*TODO*/ }

  static constexpr unsigned BASE_COST = 200;
  static constexpr unsigned COST_INC = BASE_COST / 3;
  static const SpellPrereq PREREQ;
  static const std::string NAME;
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
           const Location& location,
           Engine&         engine)
    : Spell(NAME,
            spell_level,
            location,
            BASE_COST,
            COST_INC,
            PREREQ,
            engine)
  {}

  virtual void verify_apply() const { /*TODO*/ }
  virtual unsigned apply() const { return 0; /*TODO*/ }

  static constexpr unsigned BASE_COST = 200;
  static constexpr unsigned COST_INC = BASE_COST / 3;
  static const SpellPrereq PREREQ;
  static const std::string NAME;
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
          const Location& location,
          Engine&         engine)
    : Spell(NAME,
            spell_level,
            location,
            BASE_COST,
            COST_INC,
            PREREQ,
            engine)
  {}

  virtual void verify_apply() const { /*TODO*/ }
  virtual unsigned apply() const { return 0; /*TODO*/ }

  static constexpr unsigned BASE_COST = 200;
  static constexpr unsigned COST_INC = BASE_COST / 3;
  static const SpellPrereq PREREQ;
  static const std::string NAME;
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
           const Location& location,
           Engine&         engine)
    : Spell(NAME,
            spell_level,
            location,
            BASE_COST,
            COST_INC,
            PREREQ,
            engine)
  {}

  virtual void verify_apply() const { /*TODO*/ }
  virtual unsigned apply() const { return 0; /*TODO*/ }

  static constexpr unsigned BASE_COST = 400;
  static constexpr unsigned COST_INC = BASE_COST / 3;
  static const SpellPrereq PREREQ;
  static const std::string NAME;
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
           const Location& location,
           Engine&         engine)
    : Spell(NAME,
            spell_level,
            location,
            BASE_COST,
            COST_INC,
            PREREQ,
            engine)
  {}

  virtual void verify_apply() const { /*TODO*/ }
  virtual unsigned apply() const { return 0; /*TODO*/ }

  static constexpr unsigned BASE_COST = 400;
  static constexpr unsigned COST_INC = BASE_COST / 3;
  static const SpellPrereq PREREQ;
  static const std::string NAME;
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
          const Location& location,
          Engine&         engine)
    : Spell(NAME,
            spell_level,
            location,
            BASE_COST,
            COST_INC,
            PREREQ,
            engine)
  {}

  virtual void verify_apply() const { /*TODO*/ }
  virtual unsigned apply() const { return 0; /*TODO*/ }

  static constexpr unsigned BASE_COST = 400;
  static constexpr unsigned COST_INC = BASE_COST / 3;
  static const SpellPrereq PREREQ;
  static const std::string NAME;
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
          const Location& location,
          Engine&         engine)
    : Spell(NAME,
            spell_level,
            location,
            BASE_COST,
            COST_INC,
            PREREQ,
            engine)
  {}

  virtual void verify_apply() const { /*TODO*/ }
  virtual unsigned apply() const { return 0; /*TODO*/ }

  static constexpr unsigned BASE_COST = 400;
  static constexpr unsigned COST_INC = BASE_COST / 3;
  static const SpellPrereq PREREQ;
  static const std::string NAME;
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
          const Location& location,
          Engine&         engine)
    : Spell(NAME,
            spell_level,
            location,
            BASE_COST,
            COST_INC,
            PREREQ,
            engine)
  {}

  virtual void verify_apply() const { /*TODO*/ }
  virtual unsigned apply() const { return 0; /*TODO*/ }

  static constexpr unsigned BASE_COST = 800;
  static constexpr unsigned COST_INC = BASE_COST / 3;
  static const SpellPrereq PREREQ;
  static const std::string NAME;
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
             const Location& location,
             Engine&         engine)
    : Spell(NAME,
            spell_level,
            location,
            BASE_COST,
            COST_INC,
            PREREQ,
            engine)
  {}

  virtual void verify_apply() const { /*TODO*/ }
  virtual unsigned apply() const { return 0; /*TODO*/ }

  static constexpr unsigned BASE_COST = 800;
  static constexpr unsigned COST_INC = BASE_COST / 3;
  static const SpellPrereq PREREQ;
  static const std::string NAME;
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
            const Location& location,
            Engine&         engine)
    : Spell(NAME,
            spell_level,
            location,
            BASE_COST,
            COST_INC,
            PREREQ,
            engine)
  {}

  virtual void verify_apply() const { /*TODO*/ }
  virtual unsigned apply() const { return 0; /*TODO*/ }

  static constexpr unsigned BASE_COST = 800;
  static constexpr unsigned COST_INC = BASE_COST / 3;
  static const SpellPrereq PREREQ;
  static const std::string NAME;
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
         const Location& location,
         Engine&         engine)
    : Spell(NAME,
            spell_level,
            location,
            BASE_COST,
            COST_INC,
            PREREQ,
            engine)
  {}

  virtual void verify_apply() const { /*TODO*/ }
  virtual unsigned apply() const { return 0; /*TODO*/ }

  static constexpr unsigned BASE_COST = 1600;
  static constexpr unsigned COST_INC = BASE_COST / 3;
  static const SpellPrereq PREREQ;
  static const std::string NAME;
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
          const Location& location,
          Engine&         engine)
    : Spell(NAME,
            spell_level,
            location,
            BASE_COST,
            COST_INC,
            PREREQ,
            engine)
  {}

  virtual void verify_apply() const { /*TODO*/ }
  virtual unsigned apply() const { return 0; /*TODO*/ }

  static constexpr unsigned BASE_COST = 1600;
  static constexpr unsigned COST_INC = BASE_COST / 3;
  static const SpellPrereq PREREQ;
  static const std::string NAME;
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
           const Location& location,
           Engine&         engine)
    : Spell(NAME,
            spell_level,
            location,
            BASE_COST,
            COST_INC,
            PREREQ,
            engine)
  {}

  virtual void verify_apply() const { /*TODO*/ }
  virtual unsigned apply() const { return 0; /*TODO*/ }

  static constexpr unsigned BASE_COST = 3200;
  static constexpr unsigned COST_INC = BASE_COST / 3;
  static const SpellPrereq PREREQ;
  static const std::string NAME;
};

}

#endif
