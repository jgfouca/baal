#ifndef Spell_hpp
#define Spell_hpp

#include "SpellFactory.hpp"
#include "BaalCommon.hpp"
#include "BaalMath.hpp"
#include "PlayerAI.hpp"
#include "WorldTile.hpp"
#include "City.hpp"
#include "Engine.hpp"
#include "Weather.hpp"

#include <iosfwd>
#include <vector>
#include <utility>
#include <functional>

namespace baal {

// We use this file to define all the spells. This will avoid
// creation of lots of very small hpp/cpp files.

class City;
class Engine;

/**
 * Defines the prerequisits for a spell. The previous level of any
 * spell > level 1 is always a prereq.
 */
struct SpellPrereq
{
  unsigned min_player_level() const { return m_min_player_level; }

  vecstr_t::const_iterator begin() const { return std::begin(m_min_spell_prereqs); }

  vecstr_t::const_iterator end() const { return std::end(m_min_spell_prereqs); }

  unsigned m_min_player_level;
  vecstr_t m_min_spell_prereqs;
};

const float DOES_NOT_APPLY = -1.0;

typedef std::function<float(const WorldTile&)> factor_function_t;
typedef std::pair<std::string, factor_function_t> factor_t;
typedef std::vector<factor_t> factor_vector_t;
typedef std::function<float(const WorldTile&, float)> base_function_t;
typedef std::pair<base_function_t, factor_vector_t> base_factor_pair_t;

// Functors that describe how a spell works
struct SpellSpec
{
  factor_vector_t    m_destructiveness_spec;
  base_factor_pair_t m_kill_spec;
  base_factor_pair_t m_infra_dmg_spec;
  base_factor_pair_t m_defense_dmg_spec;
  base_function_t    m_tile_dmg_spec;
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
        const SpellPrereq&  prereq,
        Engine&             engine,
        const SpellSpec&    spec);

  virtual ~Spell() = default;

  Spell(const Spell&) = delete;
  Spell& operator=(const Spell&) = delete;

  //
  // API
  //

  // Verify that the user's attempt to cast this spell is sane.
  // This method should throw user errors so that apply doesn't
  // have to.
  virtual void verify_apply() const = 0;

  // Apply should NEVER throw a User exception
  unsigned apply() const;

  //
  // getters
  //

  virtual unsigned cost() const
  { return DEFAULT_COST_FUNC(m_base_cost, m_spell_level); }

  const std::string& name() const { return m_name; }

  virtual const char* info() const { return "TODO"; }

  const SpellPrereq& prereq() const { return m_prereq; }

  unsigned level() const { return m_spell_level; }

  const Location& location() const { return m_location; }

  friend std::ostream& operator<<(std::ostream& out, const Spell& spell);

 protected:

  virtual void apply_to_world(WorldTile& tile,
                              std::vector<WorldTile*>& affected_tiles,
                              std::vector<std::pair<std::string, unsigned>>& triggered) const = 0;

  float compute_destructiveness(const WorldTile& tile, bool report) const;

  void verify_no_repeat_cast() const;

 protected:

  // Members
  const std::string& m_name;
  unsigned           m_spell_level;
  Location           m_location;
  unsigned           m_base_cost;
  const SpellPrereq& m_prereq;
  Engine&            m_engine;
  SpellSpec          m_spec;

  // Constants

 private:

  static constexpr unsigned CITY_DESTROY_EXP_BONUS = 1000;
  static constexpr unsigned CHAIN_REACTION_BONUS = 2;

  static unsigned INFRA_EXP_FUNC(unsigned infra_destroyed)
  { return std::pow(2, infra_destroyed) * 200; }

  static unsigned DEFENSE_EXP_FUNC(unsigned levels_destroyed)
  { return std::pow(2, levels_destroyed) * 400; }

  static unsigned DEFAULT_COST_FUNC(unsigned base, unsigned level)
  { return base * (std::pow(1.3, level - 1)); }

  //
  // Internal methods
  //

  // Returns exp gained and if city was wiped
  std::pair<unsigned,bool> kill_base(WorldTile const& city,
                                     float destructiveness) const;
  std::pair<unsigned,bool> kill(City& city, float kill_pct) const;

  // Returns exp gained
  unsigned damage(LandTile& tile, float destructiveness, const base_factor_pair_t& spec, const std::string& name) const;

  unsigned destroy(LandTile& tile, unsigned max_destroyed, const std::string& name,
                   std::function<unsigned(LandTile&)> const&  getter,
                   std::function<void(LandTile&, unsigned)> const&  destroyer,
                   std::function<unsigned(unsigned)> const& exp) const;

  void damage_tile(LandTile& tile, float destructiveness) const;

  // Some disasters can trigger other disasters (chain reaction). This method
  // encompassed the implementation of this phenominon. The amount of exp
  // gained is returned.
  unsigned trigger(const std::string& spell_name, unsigned spell_level) const;
};

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
            PREREQ,
            engine,
            SpellSpec {
              // destructiveness
              { {"temperature", [](WorldTile const& tile) -> float{
                    return baal::poly_growth(tile.atmosphere().temperature(), 1.5, KILL_THRESHOLD, 8);
                  } },
                {"dewpoint", [](WorldTile const& tile) -> float{
                    return 1.0;  // TODO
                  } }
              },
                // kill spec
              { [](WorldTile const&, float destructiveness) -> float{ return destructiveness; },
                { {"tech level", [&engine](WorldTile const& tile) -> float {
                    return baal::sqrt(engine.ai_player().tech_level());
                    } } }
              },
                // infra dmg spec
              { [](WorldTile const&, float) -> float{ return DOES_NOT_APPLY; }, {} },
                // defense dmg spec
              { [](WorldTile const&, float) -> float{ return DOES_NOT_APPLY; }, {} },
                // tile dmg spec
                [](WorldTile const&, float) -> float{ return DOES_NOT_APPLY; } } ),
    m_degrees_heated_land(
      [](WorldTile const& tile, unsigned spell_level) -> unsigned{ return 7 * spell_level; }
                          ),
    m_degrees_heated_ocean(
      [](WorldTile const& tile, unsigned spell_level) -> unsigned{ return 2 * spell_level; }
                           )
  {}

  virtual void verify_apply() const;

  virtual void apply_to_world(WorldTile& tile,
                              std::vector<WorldTile*>& affected_tiles,
                              std::vector<std::pair<std::string, unsigned>>& triggered) const;

  std::function<unsigned(const WorldTile&, unsigned)> m_degrees_heated_land;
  std::function<unsigned(const WorldTile&, unsigned)> m_degrees_heated_ocean;

  static constexpr unsigned BASE_COST = 50;
  static constexpr int KILL_THRESHOLD = 100;
  static const SpellPrereq PREREQ;
  static const std::string NAME;
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
            PREREQ,
            engine,
            SpellSpec {
              // destructiveness
              { {"temperature", [](WorldTile const& tile) -> float{
                    return baal::poly_growth(tile.atmosphere().temperature(), -KILL_THRESHOLD, 1.5, 8) ;
                  } },
                {"wind", [](WorldTile const& tile) -> float{
                    return baal::exp_growth(1.02, tile.atmosphere().wind().m_speed, 40);
                  } },
                {"famine", [](WorldTile const& tile) -> float{
                    return tile.city() != nullptr && tile.city()->famine() ? FAMINE_BONUS : 1.0;
                  } }
              },
                // kill spec
              { [](WorldTile const&, float destructiveness) -> float{ return destructiveness; },
                { {"tech level", [&engine](WorldTile const& tile) -> float {
                    return engine.ai_player().tech_level();
                    } } }
              },
                // infra dmg spec
              { [](WorldTile const&, float) -> float{ return DOES_NOT_APPLY; }, {} },
                // defense dmg spec
              { [](WorldTile const&, float) -> float{ return DOES_NOT_APPLY; }, {} },
                // tile dmg spec
                [](WorldTile const&, float) -> float{ return DOES_NOT_APPLY; } } ),
    m_degrees_cooled_land(
      [](WorldTile const& tile, unsigned spell_level) -> unsigned{ return 7 * spell_level; }
                          ),
    m_degrees_cooled_ocean(
      [](WorldTile const& tile, unsigned spell_level) -> unsigned{ return 2 * spell_level; }
                           )
  {}

  virtual void verify_apply() const;

  virtual void apply_to_world(WorldTile& tile,
                              std::vector<WorldTile*>& affected_tiles,
                              std::vector<std::pair<std::string, unsigned>>& triggered) const;

  std::function<unsigned(const WorldTile&, unsigned)> m_degrees_cooled_land;
  std::function<unsigned(const WorldTile&, unsigned)> m_degrees_cooled_ocean;

  static constexpr unsigned BASE_COST = 50;
  static constexpr int KILL_THRESHOLD = 0;
  static constexpr float FAMINE_BONUS = 2.0;
  static const SpellPrereq PREREQ;
  static const std::string NAME;
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
            PREREQ,
            engine,
            SpellSpec {
              // destructiveness
              { {"spell power", [spell_level](WorldTile const& tile) -> float{
                    return baal::poly_growth(spell_level, 1.3) ;
                  } },
                {"city size", [](WorldTile const& tile) -> float{
                    return baal::exp_growth(1.05, tile.city()->rank()) ;
                  } },
                {"extreme temp", [](WorldTile const& tile) -> float{
                    const int curr_temp = tile.atmosphere().temperature();
                    if (curr_temp < COLD_THRESHOLD) {
                      return baal::exp_growth(1.03, COLD_THRESHOLD - curr_temp);
                    }
                    else if (curr_temp > WARM_THRESHOLD) {
                      return baal::exp_growth(1.03, curr_temp);
                    }
                    return 1.0;
                  } },
                {"famine", [](WorldTile const& tile) -> float{
                    return tile.city()->famine() ? FAMINE_BONUS : 1.0;
                  } }
              },
                // kill spec
              { [](WorldTile const&, float destructiveness) -> float{ return destructiveness; },
                { {"tech level", [&engine](WorldTile const& tile) -> float {
                    return engine.ai_player().tech_level();
                    } } }
              },
                // infra dmg spec
              { [](WorldTile const&, float) -> float{ return DOES_NOT_APPLY; }, {} },
                // defense dmg spec
              { [](WorldTile const&, float) -> float{ return DOES_NOT_APPLY; }, {} },
                // tile dmg spec
                [](WorldTile const&, float) -> float{ return DOES_NOT_APPLY; } } )
  {}

  virtual void verify_apply() const;
  virtual void apply_to_world(WorldTile& tile,
                              std::vector<WorldTile*>& affected_tiles,
                              std::vector<std::pair<std::string, unsigned>>& triggered) const;

  static constexpr unsigned BASE_COST = 50;
  static constexpr float FAMINE_BONUS = 2.0;
  static constexpr int WARM_THRESHOLD = 90;
  static constexpr int COLD_THRESHOLD = 30;
  static const SpellPrereq PREREQ;
  static const std::string NAME;
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
            PREREQ,
            engine,
            SpellSpec {
              // destructiveness
              { {"wind", [](WorldTile const& tile) -> float{
                    return baal::exp_growth(1.03, tile.atmosphere().wind().m_speed, KILL_THRESHOLD);
                  } }
              },
                // kill spec
              { [](WorldTile const&, float destructiveness) -> float{ return destructiveness; },
                { {"tech level", [&engine](WorldTile const& tile) -> float {
                    return baal::sqrt(engine.ai_player().tech_level());
                  } },
                {"defense", [](WorldTile const& tile) -> float {
                    return baal::sqrt(tile.city()->defense());
                  } } }
              },
                // infra dmg spec
              { [](WorldTile const& tile, float) -> float{
                  return baal::exp_growth(1.03, tile.atmosphere().wind().m_speed, DAMAGE_THRESHOLD);
                },
                { {"tech level", [&engine](WorldTile const& tile) -> float {
                    return baal::sqrt(engine.ai_player().tech_level());
                    } } }
              },
                // defense dmg spec
              { [](WorldTile const&, float) -> float{ return DOES_NOT_APPLY; }, {} },
                // tile dmg spec
                [](WorldTile const&, float) -> float{ return DOES_NOT_APPLY; } } ),
    m_wind_speedup(
      [](WorldTile const& tile, unsigned spell_level) -> unsigned{ return 20 * spell_level; }
                   )
  {}

  virtual void verify_apply() const;
  virtual void apply_to_world(WorldTile& tile,
                              std::vector<WorldTile*>& affected_tiles,
                              std::vector<std::pair<std::string, unsigned>>& triggered) const;

  std::function<unsigned(const WorldTile&, unsigned)> m_wind_speedup;

  static constexpr unsigned BASE_COST = 50;
  static constexpr unsigned DAMAGE_THRESHOLD = 60;
  static constexpr unsigned KILL_THRESHOLD = 80;
  static const SpellPrereq PREREQ;
  static const std::string NAME;
};

/**
 * Starts a fire at a location. Fires will kill people in cities and
 * destroy infrastructure.
 * TODO: has a chance to spread?
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
            PREREQ,
            engine,
            SpellSpec {
              // destructiveness
              { {"spell power", [spell_level](WorldTile const& tile) -> float{
                    return baal::poly_growth(spell_level, 1.3);
                  } },
                {"wind", [](WorldTile const& tile) -> float{
                    return baal::exp_growth(1.05, tile.atmosphere().wind().m_speed, WIND_TIPPING_POINT, 30);
                  } },
                {"temperature", [](WorldTile const& tile) -> float{
                    return baal::exp_growth(1.03, tile.atmosphere().temperature(), TEMP_TIPPING_POINT);
                  } },
                {"moisture", [](WorldTile const& tile) -> float{
                    const float pct_beyond_dry = (MOISTURE_TIPPING_POINT - dynamic_cast<FoodTile const&>(tile).soil_moisture()) * 100;
                    return baal::exp_growth(1.05, pct_beyond_dry, 40);
                  } },
                {"dewpoint", [](WorldTile const& tile) -> float{
                    return 1.0; // TODO
                  } },
                {"snowpack", [](WorldTile const& tile) -> float{
                    return 1 / baal::exp_growth(1.1, tile.snowpack());
                  } }
              },
                // kill spec
              { [](WorldTile const&, float destructiveness) -> float{
                  return baal::linear_growth(destructiveness, 0, 1.0);
                },
                { {"tech level", [&engine](WorldTile const& tile) -> float {
                    return baal::sqrt(engine.ai_player().tech_level());
                  } },
                {"defense", [](WorldTile const& tile) -> float {
                    return baal::sqrt(tile.city()->defense());
                  } } }
              },
                // infra dmg spec
              { [](WorldTile const&, float destructiveness) -> float{
                  return baal::exp_growth(1.05, destructiveness);
                },
                { {"tech level", [&engine](WorldTile const& tile) -> float {
                    return baal::sqrt(engine.ai_player().tech_level());
                    } } }
              },
                // defense dmg spec
              { [](WorldTile const&, float destructiveness) -> float{
                  return baal::exp_growth(1.03, destructiveness);
                },
                { {"tech level", [&engine](WorldTile const& tile) -> float {
                    return baal::sqrt(engine.ai_player().tech_level());
                    } } }
              },
                // tile dmg spec
                [](WorldTile const&, float destructiveness) -> float{ return destructiveness; } } )
  {}

  virtual void verify_apply() const;
  virtual void apply_to_world(WorldTile& tile,
                              std::vector<WorldTile*>& affected_tiles,
                              std::vector<std::pair<std::string, unsigned>>& triggered) const;

  static constexpr unsigned BASE_COST = 100;
  static constexpr int TEMP_TIPPING_POINT = 75;
  static constexpr float TEMP_EXP_BASE = 1.03;
  static constexpr int WIND_TIPPING_POINT = 20;
  static constexpr float WIND_EXP_BASE = 1.05;
  static constexpr float MOISTURE_TIPPING_POINT = 0.75;
  static constexpr float MOISTURE_EXP_BASE = 1.05; // per moisture pct
  static constexpr float DESTRUCTIVENESS_PER_INFRA = 5.0;

  static const SpellPrereq PREREQ;
  static const std::string NAME;
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
            PREREQ,
            engine,
            SpellSpec {
              // destructiveness
              { {"spell power", [spell_level](WorldTile const& tile) -> float{
                    return spell_level;
                  } },
                {"wind", [](WorldTile const& tile) -> float{
                    return baal::exp_growth(1.03, tile.atmosphere().wind().m_speed, WIND_TIPPING_POINT);
                  } },
                {"temperature", [](WorldTile const& tile) -> float{
                    return baal::exp_growth(1.03, tile.atmosphere().temperature(), TEMP_TIPPING_POINT);
                  } },
                {"pressure", [](WorldTile const& tile) -> float{
                    return baal::exp_growth(1.05, tile.atmosphere().pressure(), PRESSURE_TIPPING_POINT);
                  } },
                {"dewpoint", [](WorldTile const& tile) -> float{
                    return 1.0; // TODO
                  } }
              },
                // kill spec
              { [](WorldTile const&, float destructiveness) -> float{
                  return destructiveness / 5.0;
                },
                { {"tech level", [&engine](WorldTile const& tile) -> float {
                    return baal::sqrt(engine.ai_player().tech_level());
                  } },
                {"defense", [](WorldTile const& tile) -> float {
                    return baal::sqrt(tile.city()->defense());
                  } } }
              },
                // infra dmg spec
              { [](WorldTile const&, float) -> float{ return DOES_NOT_APPLY; }, {} },
                // defense dmg spec
              { [](WorldTile const&, float) -> float{ return DOES_NOT_APPLY; }, {} },
                // tile dmg spec
                [](WorldTile const&, float) -> float{ return DOES_NOT_APPLY; } } ),
    m_wind_spawn_func( [](float destructiveness) -> unsigned{
        return baal::fibonacci_div(destructiveness, WIND_DESTRUCTIVENESS_THRESHOLD);
      }),
    m_flood_spawn_func( [](float destructiveness) -> unsigned{
        return baal::fibonacci_div(destructiveness, FLOOD_DESTRUCTIVENESS_THRESHOLD);
      }),
    m_tornado_spawn_func( [](float destructiveness) -> unsigned{
        return baal::fibonacci_div(destructiveness, TORNADO_DESTRUCTIVENESS_THRESHOLD);
      })
  {}

  virtual void verify_apply() const;
  virtual void apply_to_world(WorldTile& tile,
                              std::vector<WorldTile*>& affected_tiles,
                              std::vector<std::pair<std::string, unsigned>>& triggered) const;

  static constexpr unsigned BASE_COST = 100;
  static constexpr int TEMP_TIPPING_POINT = 85;
  static constexpr int WIND_TIPPING_POINT = 15;
  static constexpr int PRESSURE_TIPPING_POINT = Atmosphere::NORMAL_PRESSURE;
  static constexpr float DRY_STORM_MOISTURE_ADD = .1;

  static constexpr float WIND_DESTRUCTIVENESS_THRESHOLD = 10.0;
  static constexpr float FLOOD_DESTRUCTIVENESS_THRESHOLD = 15.0;
  static constexpr float TORNADO_DESTRUCTIVENESS_THRESHOLD = 20.0;

  static constexpr float LIGHTING_PCT_KILL_PER_DESTRUCTIVENESS = 0.2;

  static const SpellPrereq PREREQ;
  static const std::string NAME;

  std::function<unsigned(float destructiveness)> m_wind_spawn_func;
  std::function<unsigned(float destructiveness)> m_flood_spawn_func;
  std::function<unsigned(float destructiveness)> m_tornado_spawn_func;
};

/**
 * Spawn a large snow storm. Temporarily drastically reduces yields on tiles,
 * especially food tiles. Snow storms can cause deaths, but they are not
 * particularly effective at killing people directly.
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
            PREREQ,
            engine,
            SpellSpec {
              // destructiveness
              { {"spell power", [spell_level](WorldTile const& tile) -> float{
                    return spell_level * 4;
                  } },
                {"pressure", [](WorldTile const& tile) -> float{
                    return baal::exp_growth(1.05, Atmosphere::NORMAL_PRESSURE - tile.atmosphere().pressure());
                  } },
                {"temperature", [](WorldTile const& tile) -> float{
                    return baal::exp_growth(1.03, MAX_TEMP - tile.atmosphere().temperature(), 0, 15);
                  } },
                {"dewpoint", [](WorldTile const& tile) -> float{
                    return baal::exp_growth(1.05, tile.atmosphere().dewpoint(), 20);
                  } }
              },
                // kill spec
              { [](WorldTile const&, float destructiveness) -> float{ return destructiveness / 4; },
                { {"tech level", [&engine](WorldTile const& tile) -> float {
                      return baal::poly_growth(engine.ai_player().tech_level(), 0.5);
                    } } }
              },
                // infra dmg spec
              { [](WorldTile const&, float) -> float{ return DOES_NOT_APPLY; }, {} },
                // defense dmg spec
              { [](WorldTile const&, float) -> float{ return DOES_NOT_APPLY; }, {} },
                // tile dmg spec
                [](WorldTile const&, float) -> float{ return DOES_NOT_APPLY; } } ),
    m_snowfall_func( [](float destructiveness) -> unsigned{
        return destructiveness * 4;
      })
  {}

  virtual void verify_apply() const;
  virtual void apply_to_world(WorldTile& tile,
                              std::vector<WorldTile*>& affected_tiles,
                              std::vector<std::pair<std::string, unsigned>>& triggered) const;

  static constexpr unsigned BASE_COST = 100;
  static constexpr int MAX_TEMP = 35;
  static const SpellPrereq PREREQ;
  static const std::string NAME;

  std::function<unsigned(float destructiveness)> m_snowfall_func;
};

/**
 * Cause a flooding rainstorm. Can kill people in cities and destroy
 * infrastructure.
 *
 * Enhanced by high soil moisture, high dewpoints, low pressure. On elevated
 * tiles, flood destructiveness is increased due to flash-flooding.
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
            PREREQ,
            engine,
            SpellSpec {
              // destructiveness
              { {"spell power", [spell_level](WorldTile const& tile) -> float{
                    return spell_level ;
                  } },
                {"dewpoint", [](WorldTile const& tile) -> float{
                    return baal::exp_growth(1.03, tile.atmosphere().dewpoint(), 55);
                  } },
                {"pressure", [](WorldTile const& tile) -> float{
                    return baal::exp_growth(1.03, tile.atmosphere().pressure(), Atmosphere::NORMAL_PRESSURE);
                  } },
                {"moisture", [](WorldTile const& tile) -> float{
                    return baal::exp_growth(1.05, tile.soil_moisture() * 10, 10);
                  } },
                {"elevation", [](WorldTile const& tile) -> float{
                    return baal::exp_growth(1.1, tile.elevation() / 500.0);
                  } }
              },
                // kill spec
              { [](WorldTile const&, float destructiveness) -> float{ return destructiveness; },
                { {"tech level", [&engine](WorldTile const& tile) -> float {
                      return baal::sqrt(engine.ai_player().tech_level());
                    } },
                {"defense", [](WorldTile const& tile) -> float {
                    return tile.city()->defense();
                  } } }
              },
                // infra dmg spec
              { [](WorldTile const&, float destructiveness) -> float{
                  return baal::exp_growth(1.05, destructiveness);
                },
                { {"tech level", [&engine](WorldTile const& tile) -> float {
                    return baal::sqrt(engine.ai_player().tech_level());
                    } } }
              },
                // defense dmg spec
              { [](WorldTile const&, float destructiveness) -> float{
                  return baal::exp_growth(1.03, destructiveness);
                },
                { {"tech level", [&engine](WorldTile const& tile) -> float {
                    return baal::sqrt(engine.ai_player().tech_level());
                    } } }
              },
                // tile dmg spec
                [](WorldTile const&, float) -> float{ return DOES_NOT_APPLY; } } ),
    m_rainfall_func( [](float destructiveness) -> float{
        return destructiveness;
      })
  {}

  virtual void verify_apply() const;
  virtual void apply_to_world(WorldTile& tile,
                              std::vector<WorldTile*>& affected_tiles,
                              std::vector<std::pair<std::string, unsigned>>& triggered) const;

  static constexpr unsigned BASE_COST = 200;
  static constexpr int MIN_TEMP = 40;
  static const SpellPrereq PREREQ;
  static const std::string NAME;

  std::function<float(float destructiveness)> m_rainfall_func;
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
            PREREQ,
            engine,
                        SpellSpec {
              // destructiveness
              { {"spell power", [spell_level](WorldTile const& tile) -> float{
                    return baal::poly_growth(spell_level, 0.0, 1.3) ;
                  } },
                {"city size", [](WorldTile const& tile) -> float{
                    return baal::exp_growth(tile.city()->rank(), 0.0, 1.05) ;
                  } },
                {"famine", [](WorldTile const& tile) -> float{
                    return tile.city()->famine() ? 0.0 : 1.0;
                  } }
              },
                // kill spec
              { [](WorldTile const&, float destructiveness) -> float{ return destructiveness; },
                { {"tech level", [&engine](WorldTile const& tile) -> float {
                    return engine.ai_player().tech_level();
                    } } }
              },
                // infra dmg spec
              { [](WorldTile const&, float) -> float{ return DOES_NOT_APPLY; }, {} },
                // defense dmg spec
              { [](WorldTile const&, float) -> float{ return DOES_NOT_APPLY; }, {} },
                // tile dmg spec
                [](WorldTile const&, float) -> float{ return DOES_NOT_APPLY; } } )
  {}

  virtual void verify_apply() const { /*TODO*/ }
  virtual void apply_to_world(WorldTile& tile,
                              std::vector<WorldTile*>& affected_tiles,
                              std::vector<std::pair<std::string, unsigned>>& triggered) const {}

  static constexpr unsigned BASE_COST = 200;
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
            PREREQ,
            engine,
                        SpellSpec {
              // destructiveness
              { {"spell power", [spell_level](WorldTile const& tile) -> float{
                    return baal::poly_growth(spell_level, 0.0, 1.3) ;
                  } },
                {"city size", [](WorldTile const& tile) -> float{
                    return baal::exp_growth(tile.city()->rank(), 0.0, 1.05) ;
                  } },
                {"famine", [](WorldTile const& tile) -> float{
                    return tile.city()->famine() ? 0.0 : 1.0;
                  } }
              },
                // kill spec
              { [](WorldTile const&, float destructiveness) -> float{ return destructiveness; },
                { {"tech level", [&engine](WorldTile const& tile) -> float {
                    return engine.ai_player().tech_level();
                    } } }
              },
                // infra dmg spec
              { [](WorldTile const&, float) -> float{ return DOES_NOT_APPLY; }, {} },
                // defense dmg spec
              { [](WorldTile const&, float) -> float{ return DOES_NOT_APPLY; }, {} },
                // tile dmg spec
                [](WorldTile const&, float) -> float{ return DOES_NOT_APPLY; } } )
  {}

  virtual void verify_apply() const { /*TODO*/ }
  virtual void apply_to_world(WorldTile& tile,
                              std::vector<WorldTile*>& affected_tiles,
                              std::vector<std::pair<std::string, unsigned>>& triggered) const {}

  static constexpr unsigned BASE_COST = 200;
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
            PREREQ,
            engine,
            SpellSpec {
              // destructiveness
              { {"spell power", [spell_level](WorldTile const& tile) -> float{
                    return baal::poly_growth(spell_level, 1.3) ;
                  } },
                {"ongoing snowstorm", [](WorldTile const& tile) -> float{
                    return tile.already_casted(Snow::NAME) ? 1.5 : 1;
                  } },
                {"ongoing blizzard", [](WorldTile const& tile) -> float{
                    return tile.already_casted(Blizzard::NAME) ? 2 : 1;
                  } },
                {"elevation", [](WorldTile const& tile) -> float{
                    return baal::exp_growth(1.1, tile.elevation() / 1000.0, 2.0);
                  } },
                {"snowpack", [](WorldTile const& tile) -> float{
                    return baal::exp_growth(1.002, tile.snowpack(), 100);
                  } }
              },
                // kill spec
              { [](WorldTile const&, float destructiveness) -> float{ return destructiveness; },
                { {"tech level", [&engine](WorldTile const& tile) -> float {
                      return baal::sqrt(engine.ai_player().tech_level());
                    } },
                {"defense", [](WorldTile const& tile) -> float {
                    return baal::sqrt(tile.city()->defense());
                  } } }
              },
                // infra dmg spec
              { [](WorldTile const&, float destructiveness) -> float{
                  return baal::exp_growth(1.05, destructiveness);
                },
                { {"tech level", [&engine](WorldTile const& tile) -> float {
                    return baal::sqrt(engine.ai_player().tech_level());
                    } } }
              },
                // defense dmg spec
              { [](WorldTile const&, float destructiveness) -> float{
                  return baal::exp_growth(1.03, destructiveness);
                },
                { {"tech level", [&engine](WorldTile const& tile) -> float {
                    return baal::sqrt(engine.ai_player().tech_level());
                    } } }
              },
                // tile dmg spec
                [](WorldTile const&, float) -> float{ return DOES_NOT_APPLY; } } )
  {}

  virtual void verify_apply() const;
  virtual void apply_to_world(WorldTile& tile,
                              std::vector<WorldTile*>& affected_tiles,
                              std::vector<std::pair<std::string, unsigned>>& triggered) const;

  static constexpr unsigned BASE_COST = 200;
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
            PREREQ,
            engine,
                        SpellSpec {
              // destructiveness
              { {"spell power", [spell_level](WorldTile const& tile) -> float{
                    return baal::poly_growth(spell_level, 0.0, 1.3) ;
                  } },
                {"city size", [](WorldTile const& tile) -> float{
                    return baal::exp_growth(tile.city()->rank(), 0.0, 1.05) ;
                  } },
                {"famine", [](WorldTile const& tile) -> float{
                    return tile.city()->famine() ? 0.0 : 1.0;
                  } }
              },
                // kill spec
              { [](WorldTile const&, float destructiveness) -> float{ return destructiveness; },
                { {"tech level", [&engine](WorldTile const& tile) -> float {
                    return engine.ai_player().tech_level();
                    } } }
              },
                // infra dmg spec
              { [](WorldTile const&, float) -> float{ return DOES_NOT_APPLY; }, {} },
                // defense dmg spec
              { [](WorldTile const&, float) -> float{ return DOES_NOT_APPLY; }, {} },
                // tile dmg spec
                [](WorldTile const&, float) -> float{ return DOES_NOT_APPLY; } } )
  {}

  virtual void verify_apply() const { /*TODO*/ }
  virtual void apply_to_world(WorldTile& tile,
                              std::vector<WorldTile*>& affected_tiles,
                              std::vector<std::pair<std::string, unsigned>>& triggered) const {}

  static constexpr unsigned BASE_COST = 200;
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
            PREREQ,
            engine,
                        SpellSpec {
              // destructiveness
              { {"spell power", [spell_level](WorldTile const& tile) -> float{
                    return baal::poly_growth(spell_level, 0.0, 1.3) ;
                  } },
                {"city size", [](WorldTile const& tile) -> float{
                    return baal::exp_growth(tile.city()->rank(), 0.0, 1.05) ;
                  } },
                {"famine", [](WorldTile const& tile) -> float{
                    return tile.city()->famine() ? 0.0 : 1.0;
                  } }
              },
                // kill spec
              { [](WorldTile const&, float destructiveness) -> float{ return destructiveness; },
                { {"tech level", [&engine](WorldTile const& tile) -> float {
                    return engine.ai_player().tech_level();
                    } } }
              },
                // infra dmg spec
              { [](WorldTile const&, float) -> float{ return DOES_NOT_APPLY; }, {} },
                // defense dmg spec
              { [](WorldTile const&, float) -> float{ return DOES_NOT_APPLY; }, {} },
                // tile dmg spec
                [](WorldTile const&, float) -> float{ return DOES_NOT_APPLY; } } )
  {}

  virtual void verify_apply() const { /*TODO*/ }
  virtual void apply_to_world(WorldTile& tile,
                              std::vector<WorldTile*>& affected_tiles,
                              std::vector<std::pair<std::string, unsigned>>& triggered) const {}

  static constexpr unsigned BASE_COST = 400;
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
            PREREQ,
            engine,
                        SpellSpec {
              // destructiveness
              { {"spell power", [spell_level](WorldTile const& tile) -> float{
                    return baal::poly_growth(spell_level, 0.0, 1.3) ;
                  } },
                {"city size", [](WorldTile const& tile) -> float{
                    return baal::exp_growth(tile.city()->rank(), 0.0, 1.05) ;
                  } },
                {"famine", [](WorldTile const& tile) -> float{
                    return tile.city()->famine() ? 0.0 : 1.0;
                  } }
              },
                // kill spec
              { [](WorldTile const&, float destructiveness) -> float{ return destructiveness; },
                { {"tech level", [&engine](WorldTile const& tile) -> float {
                    return engine.ai_player().tech_level();
                    } } }
              },
                // infra dmg spec
              { [](WorldTile const&, float) -> float{ return DOES_NOT_APPLY; }, {} },
                // defense dmg spec
              { [](WorldTile const&, float) -> float{ return DOES_NOT_APPLY; }, {} },
                // tile dmg spec
                [](WorldTile const&, float) -> float{ return DOES_NOT_APPLY; } } )
  {}

  virtual void verify_apply() const { /*TODO*/ }
  virtual void apply_to_world(WorldTile& tile,
                              std::vector<WorldTile*>& affected_tiles,
                              std::vector<std::pair<std::string, unsigned>>& triggered) const {}

  static constexpr unsigned BASE_COST = 400;
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
            PREREQ,
            engine,
                        SpellSpec {
              // destructiveness
              { {"spell power", [spell_level](WorldTile const& tile) -> float{
                    return baal::poly_growth(spell_level, 0.0, 1.3) ;
                  } },
                {"city size", [](WorldTile const& tile) -> float{
                    return baal::exp_growth(tile.city()->rank(), 0.0, 1.05) ;
                  } },
                {"famine", [](WorldTile const& tile) -> float{
                    return tile.city()->famine() ? 0.0 : 1.0;
                  } }
              },
                // kill spec
              { [](WorldTile const&, float destructiveness) -> float{ return destructiveness; },
                { {"tech level", [&engine](WorldTile const& tile) -> float {
                    return engine.ai_player().tech_level();
                    } } }
              },
                // infra dmg spec
              { [](WorldTile const&, float) -> float{ return DOES_NOT_APPLY; }, {} },
                // defense dmg spec
              { [](WorldTile const&, float) -> float{ return DOES_NOT_APPLY; }, {} },
                // tile dmg spec
                [](WorldTile const&, float) -> float{ return DOES_NOT_APPLY; } } )
  {}

  virtual void verify_apply() const { /*TODO*/ }
  virtual void apply_to_world(WorldTile& tile,
                              std::vector<WorldTile*>& affected_tiles,
                              std::vector<std::pair<std::string, unsigned>>& triggered) const {}

  static constexpr unsigned BASE_COST = 400;
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
            PREREQ,
            engine,
                        SpellSpec {
              // destructiveness
              { {"spell power", [spell_level](WorldTile const& tile) -> float{
                    return baal::poly_growth(spell_level, 0.0, 1.3) ;
                  } },
                {"city size", [](WorldTile const& tile) -> float{
                    return baal::exp_growth(tile.city()->rank(), 0.0, 1.05) ;
                  } },
                {"famine", [](WorldTile const& tile) -> float{
                    return tile.city()->famine() ? 0.0 : 1.0;
                  } }
              },
                // kill spec
              { [](WorldTile const&, float destructiveness) -> float{ return destructiveness; },
                { {"tech level", [&engine](WorldTile const& tile) -> float {
                    return engine.ai_player().tech_level();
                    } } }
              },
                // infra dmg spec
              { [](WorldTile const&, float) -> float{ return DOES_NOT_APPLY; }, {} },
                // defense dmg spec
              { [](WorldTile const&, float) -> float{ return DOES_NOT_APPLY; }, {} },
                // tile dmg spec
                [](WorldTile const&, float) -> float{ return DOES_NOT_APPLY; } } )
  {}

  virtual void verify_apply() const { /*TODO*/ }
  virtual void apply_to_world(WorldTile& tile,
                              std::vector<WorldTile*>& affected_tiles,
                              std::vector<std::pair<std::string, unsigned>>& triggered) const {}

  static constexpr unsigned BASE_COST = 400;
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
            PREREQ,
            engine,
                        SpellSpec {
              // destructiveness
              { {"spell power", [spell_level](WorldTile const& tile) -> float{
                    return baal::poly_growth(spell_level, 0.0, 1.3) ;
                  } },
                {"city size", [](WorldTile const& tile) -> float{
                    return baal::exp_growth(tile.city()->rank(), 0.0, 1.05) ;
                  } },
                {"extreme temp", [](WorldTile const& tile) -> float{
                    const int curr_temp = tile.atmosphere().temperature();
                    if (curr_temp < 0) {
                      return baal::exp_growth(-curr_temp, 0, 1.03);
                    }
                    else if (curr_temp > 90) {
                      return baal::exp_growth(curr_temp - 90, 0, 1.03);
                    }
                    return 1.0;
                  } },
                {"famine", [](WorldTile const& tile) -> float{
                    return tile.city()->famine() ? 0.0 : 1.0;
                  } }
              },
                // kill spec
              { [](WorldTile const&, float destructiveness) -> float{ return destructiveness; },
                { {"tech level", [&engine](WorldTile const& tile) -> float {
                    return engine.ai_player().tech_level();
                    } } }
              },
                // infra dmg spec
              { [](WorldTile const&, float) -> float{ return DOES_NOT_APPLY; }, {} },
                // defense dmg spec
              { [](WorldTile const&, float) -> float{ return DOES_NOT_APPLY; }, {} },
                // tile dmg spec
                [](WorldTile const&, float) -> float{ return DOES_NOT_APPLY; } } )
  {}

  virtual void verify_apply() const { /*TODO*/ }
  virtual void apply_to_world(WorldTile& tile,
                              std::vector<WorldTile*>& affected_tiles,
                              std::vector<std::pair<std::string, unsigned>>& triggered) const {}

  static constexpr unsigned BASE_COST = 800;
  static const SpellPrereq PREREQ;
  static const std::string NAME;
};

/**
 * Will cause an earthquake to occur. Devastates nearby cities and
 * infrastructure. Can cause avalanches?
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
            PREREQ,
            engine,
                        SpellSpec {
              // destructiveness
              { {"spell power", [spell_level](WorldTile const& tile) -> float{
                    return baal::poly_growth(spell_level, 0.0, 1.3) ;
                  } },
                {"city size", [](WorldTile const& tile) -> float{
                    return baal::exp_growth(tile.city()->rank(), 0.0, 1.05) ;
                  } },
                {"famine", [](WorldTile const& tile) -> float{
                    return tile.city()->famine() ? 0.0 : 1.0;
                  } }
              },
                // kill spec
              { [](WorldTile const&, float destructiveness) -> float{ return destructiveness; },
                { {"tech level", [&engine](WorldTile const& tile) -> float {
                    return engine.ai_player().tech_level();
                    } } }
              },
                // infra dmg spec
              { [](WorldTile const&, float) -> float{ return DOES_NOT_APPLY; }, {} },
                // defense dmg spec
              { [](WorldTile const&, float) -> float{ return DOES_NOT_APPLY; }, {} },
                // tile dmg spec
                [](WorldTile const&, float) -> float{ return DOES_NOT_APPLY; } } )
  {}

  virtual void verify_apply() const { /*TODO*/ }
  virtual void apply_to_world(WorldTile& tile,
                              std::vector<WorldTile*>& affected_tiles,
                              std::vector<std::pair<std::string, unsigned>>& triggered) const {}

  static constexpr unsigned BASE_COST = 800;
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
            PREREQ,
            engine,
                        SpellSpec {
              // destructiveness
              { {"spell power", [spell_level](WorldTile const& tile) -> float{
                    return baal::poly_growth(spell_level, 0.0, 1.3) ;
                  } },
                {"city size", [](WorldTile const& tile) -> float{
                    return baal::exp_growth(tile.city()->rank(), 0.0, 1.05) ;
                  } },
                {"famine", [](WorldTile const& tile) -> float{
                    return tile.city()->famine() ? 0.0 : 1.0;
                  } }
              },
                // kill spec
              { [](WorldTile const&, float destructiveness) -> float{ return destructiveness; },
                { {"tech level", [&engine](WorldTile const& tile) -> float {
                    return engine.ai_player().tech_level();
                    } } }
              },
                // infra dmg spec
              { [](WorldTile const&, float) -> float{ return DOES_NOT_APPLY; }, {} },
                // defense dmg spec
              { [](WorldTile const&, float) -> float{ return DOES_NOT_APPLY; }, {} },
                // tile dmg spec
                [](WorldTile const&, float) -> float{ return DOES_NOT_APPLY; } } )
  {}

  virtual void verify_apply() const { /*TODO*/ }
  virtual void apply_to_world(WorldTile& tile,
                              std::vector<WorldTile*>& affected_tiles,
                              std::vector<std::pair<std::string, unsigned>>& triggered) const {}

  static constexpr unsigned BASE_COST = 800;
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
            PREREQ,
            engine,
                        SpellSpec {
              // destructiveness
              { {"spell power", [spell_level](WorldTile const& tile) -> float{
                    return baal::poly_growth(spell_level, 0.0, 1.3) ;
                  } },
                {"city size", [](WorldTile const& tile) -> float{
                    return baal::exp_growth(tile.city()->rank(), 0.0, 1.05) ;
                  } },
                {"famine", [](WorldTile const& tile) -> float{
                    return tile.city()->famine() ? 0.0 : 1.0;
                  } }
              },
                // kill spec
              { [](WorldTile const&, float destructiveness) -> float{ return destructiveness; },
                { {"tech level", [&engine](WorldTile const& tile) -> float {
                    return engine.ai_player().tech_level();
                    } } }
              },
                // infra dmg spec
              { [](WorldTile const&, float) -> float{ return DOES_NOT_APPLY; }, {} },
                // defense dmg spec
              { [](WorldTile const&, float) -> float{ return DOES_NOT_APPLY; }, {} },
                // tile dmg spec
                [](WorldTile const&, float) -> float{ return DOES_NOT_APPLY; } } )
  {}

  virtual void verify_apply() const { /*TODO*/ }
  virtual void apply_to_world(WorldTile& tile,
                              std::vector<WorldTile*>& affected_tiles,
                              std::vector<std::pair<std::string, unsigned>>& triggered) const {}

  static constexpr unsigned BASE_COST = 1600;
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
            PREREQ,
            engine,
                        SpellSpec {
              // destructiveness
              { {"spell power", [spell_level](WorldTile const& tile) -> float{
                    return baal::poly_growth(spell_level, 0.0, 1.3) ;
                  } },
                {"city size", [](WorldTile const& tile) -> float{
                    return baal::exp_growth(tile.city()->rank(), 0.0, 1.05) ;
                  } },
                {"famine", [](WorldTile const& tile) -> float{
                    return tile.city()->famine() ? 0.0 : 1.0;
                  } }
              },
                // kill spec
              { [](WorldTile const&, float destructiveness) -> float{ return destructiveness; },
                { {"tech level", [&engine](WorldTile const& tile) -> float {
                    return engine.ai_player().tech_level();
                    } } }
              },
                // infra dmg spec
              { [](WorldTile const&, float) -> float{ return DOES_NOT_APPLY; }, {} },
                // defense dmg spec
              { [](WorldTile const&, float) -> float{ return DOES_NOT_APPLY; }, {} },
                // tile dmg spec
                [](WorldTile const&, float) -> float{ return DOES_NOT_APPLY; } } )
  {}

  virtual void verify_apply() const { /*TODO*/ }
  virtual void apply_to_world(WorldTile& tile,
                              std::vector<WorldTile*>& affected_tiles,
                              std::vector<std::pair<std::string, unsigned>>& triggered) const {}

  static constexpr unsigned BASE_COST = 1600;
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
            PREREQ,
            engine,
                        SpellSpec {
              // destructiveness
              { {"spell power", [spell_level](WorldTile const& tile) -> float{
                    return baal::poly_growth(spell_level, 0.0, 1.3) ;
                  } },
                {"city size", [](WorldTile const& tile) -> float{
                    return baal::exp_growth(tile.city()->rank(), 0.0, 1.05) ;
                  } },
                {"famine", [](WorldTile const& tile) -> float{
                    return tile.city()->famine() ? 0.0 : 1.0;
                  } }
              },
                // kill spec
              { [](WorldTile const&, float destructiveness) -> float{ return destructiveness; },
                { {"tech level", [&engine](WorldTile const& tile) -> float {
                    return engine.ai_player().tech_level();
                    } } }
              },
                // infra dmg spec
              { [](WorldTile const&, float) -> float{ return DOES_NOT_APPLY; }, {} },
                // defense dmg spec
              { [](WorldTile const&, float) -> float{ return DOES_NOT_APPLY; }, {} },
                // tile dmg spec
                [](WorldTile const&, float) -> float{ return DOES_NOT_APPLY; } } )
  {}

  virtual void verify_apply() const { /*TODO*/ }
  virtual void apply_to_world(WorldTile& tile,
                              std::vector<WorldTile*>& affected_tiles,
                              std::vector<std::pair<std::string, unsigned>>& triggered) const {}

  static constexpr unsigned BASE_COST = 3200;
  static const SpellPrereq PREREQ;
  static const std::string NAME;
};

}

#endif
