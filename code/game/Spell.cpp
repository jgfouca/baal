#include "Spell.hpp"
#include "World.hpp"
#include "Engine.hpp"
#include "Interface.hpp"
#include "Player.hpp"
#include "PlayerAI.hpp"
#include "City.hpp"

#include <iostream>
#include <cmath>

using std::ostream;
using namespace baal;

SpellPrereq Hot::PREREQ;
SpellPrereq Cold::PREREQ;
SpellPrereq WindSpell::PREREQ;
SpellPrereq Infect::PREREQ;

SpellPrereq Fire::PREREQ;
SpellPrereq Tstorm::PREREQ;
SpellPrereq Snow::PREREQ;

SpellPrereq Avalanche::PREREQ;
SpellPrereq Flood::PREREQ;
SpellPrereq Dry::PREREQ;
SpellPrereq Blizzard::PREREQ;
SpellPrereq Tornado::PREREQ;

SpellPrereq Heatwave::PREREQ;
SpellPrereq Coldwave::PREREQ;
SpellPrereq Drought::PREREQ;
SpellPrereq Monsoon::PREREQ;

SpellPrereq Disease::PREREQ;
SpellPrereq Earthquake::PREREQ;
SpellPrereq Hurricane::PREREQ;

SpellPrereq Plague::PREREQ;
SpellPrereq Volcano::PREREQ;

SpellPrereq Asteroid::PREREQ;

namespace {

SpellPrereqStaticInitializer static_prereq_init;

}

///////////////////////////////////////////////////////////////////////////////
SpellPrereqStaticInitializer::SpellPrereqStaticInitializer()
///////////////////////////////////////////////////////////////////////////////
{
  // Hot, Cold, Wind, Infect have no prereqs

  Fire::PREREQ.m_min_player_level = 5;
  Fire::PREREQ.m_min_spell_prereqs.push_back(Prereq(SpellFactory::HOT, 1));

  Tstorm::PREREQ.m_min_player_level = 5;
  Tstorm::PREREQ.m_min_spell_prereqs.push_back(Prereq(SpellFactory::WIND, 1));

  Snow::PREREQ.m_min_player_level = 5;
  Snow::PREREQ.m_min_spell_prereqs.push_back(Prereq(SpellFactory::COLD, 1));

  Avalanche::PREREQ.m_min_player_level = 10;
  Avalanche::PREREQ.m_min_spell_prereqs.push_back(Prereq(SpellFactory::SNOW, 1));

  Blizzard::PREREQ.m_min_player_level = 10;
  Blizzard::PREREQ.m_min_spell_prereqs.push_back(Prereq(SpellFactory::SNOW, 1));

  Flood::PREREQ.m_min_player_level = 10;
  Flood::PREREQ.m_min_spell_prereqs.push_back(Prereq(SpellFactory::TSTORM, 1));

  Dry::PREREQ.m_min_player_level = 10;
  Dry::PREREQ.m_min_spell_prereqs.push_back(Prereq(SpellFactory::FIRE, 1));

  Tornado::PREREQ.m_min_player_level = 10;
  Tornado::PREREQ.m_min_spell_prereqs.push_back(Prereq(SpellFactory::TSTORM, 1));

  Heatwave::PREREQ.m_min_player_level = 15;
  Heatwave::PREREQ.m_min_spell_prereqs.push_back(Prereq(SpellFactory::DRY, 1));

  Coldwave::PREREQ.m_min_player_level = 15;
  Coldwave::PREREQ.m_min_spell_prereqs.push_back(Prereq(SpellFactory::BLIZZARD, 1));

  Drought::PREREQ.m_min_player_level = 15;
  Drought::PREREQ.m_min_spell_prereqs.push_back(Prereq(SpellFactory::DRY, 1));

  Monsoon::PREREQ.m_min_player_level = 15;
  Monsoon::PREREQ.m_min_spell_prereqs.push_back(Prereq(SpellFactory::FLOOD, 1));

  Disease::PREREQ.m_min_player_level = 20;
  Disease::PREREQ.m_min_spell_prereqs.push_back(Prereq(SpellFactory::INFECT, 1));

  Earthquake::PREREQ.m_min_player_level = 20;

  Hurricane::PREREQ.m_min_player_level = 20;
  Hurricane::PREREQ.m_min_spell_prereqs.push_back(Prereq(SpellFactory::MONSOON, 1));

  Plague::PREREQ.m_min_player_level = 25;
  Plague::PREREQ.m_min_spell_prereqs.push_back(Prereq(SpellFactory::DISEASE, 1));

  Volcano::PREREQ.m_min_player_level = 25;
  Volcano::PREREQ.m_min_spell_prereqs.push_back(Prereq(SpellFactory::EARTHQUAKE, 1));

  Asteroid::PREREQ.m_min_player_level = 30;
  Asteroid::PREREQ.m_min_spell_prereqs.push_back(Prereq(SpellFactory::VOLCANO, 1));
}

///////////////////////////////////////////////////////////////////////////////
Spell::Spell(SpellFactory::SpellName name,
             unsigned                spell_level,
             const Location&         location,
             unsigned                base_cost,
             unsigned                cost_increment,
             const SpellPrereq&      prereq)
///////////////////////////////////////////////////////////////////////////////
  : m_name(name),
    m_spell_level(spell_level),
    m_location(location),
    m_base_cost(base_cost),
    m_cost_increment(cost_increment),
    m_prereq(prereq)
{
  Assert(SpellFactory::is_in_all_names(name), name);
}

///////////////////////////////////////////////////////////////////////////////
unsigned Spell::kill(City& city,
                     float pct_killed) const
///////////////////////////////////////////////////////////////////////////////
{
  Require(pct_killed > 0.0, "Do not call this if no one killed");

  Engine& engine = Engine::instance();
  Interface& interface = engine.interface();

  if (pct_killed > 1.0) {
    pct_killed = 1.0;
  }
  unsigned num_killed = city.population() * pct_killed;

  city.kill(num_killed);
  SPELL_REPORT(interface, m_name << " has killed: " << num_killed);

  if (city.population() < City::MIN_CITY_SIZE) {
    SPELL_REPORT(interface, m_name << " has obliterated city: " << city.name());
    engine.world().remove_city(city);
    city.kill(city.population());
    num_killed += city.population();

    // TODO: Give bigger city-kill bonus based on maximum attained rank of
    // city.
    return num_killed + CITY_DESTROY_EXP_BONUS;
  }
  else {
    return num_killed;
  }
}

///////////////////////////////////////////////////////////////////////////////
unsigned Spell::destroy_infra(WorldTile& tile, unsigned max_destroyed) const
///////////////////////////////////////////////////////////////////////////////
{
  Require(max_destroyed > 0, "Do not call this if nothing destroyed");

  Engine& engine = Engine::instance();
  Interface& interface = engine.interface();

  LandTile& land_tile = dynamic_cast<LandTile&>(tile);
  unsigned orig_infra = land_tile.infra_level();
  unsigned num_destroyed = // Cannot destroy infra that's not there
    (orig_infra < max_destroyed) ? orig_infra : max_destroyed;
  land_tile.destroy_infra(num_destroyed);

  SPELL_REPORT(interface,
               m_name << " has destroyed " <<
               num_destroyed << " levels of infrastructure");

  // Convert to exp
  static const unsigned base_exp = 200;
  return std::pow(2.0, num_destroyed) * base_exp;
}

///////////////////////////////////////////////////////////////////////////////
void Spell::damage_tile(WorldTile& tile, float damage) const
///////////////////////////////////////////////////////////////////////////////
{
  Require(damage > 0.0, "Do not call this if nothing damaged");

  Engine& engine = Engine::instance();
  Interface& interface = engine.interface();
  LandTile& land_tile = dynamic_cast<LandTile&>(tile);

  if (damage > 1.0) {
    damage = 1.0;
  }
  land_tile.damage(damage);

  SPELL_REPORT(interface,
               m_name << " has caused " << damage*100 << "% damage to tile");
}

///////////////////////////////////////////////////////////////////////////////
ostream& Spell::operator<<(ostream& out) const
///////////////////////////////////////////////////////////////////////////////
{
  return out << m_name << '[' << m_spell_level << ']';
}

///////////////////////////////////////////////////////////////////////////////
ostream& baal::operator<<(ostream& out, const Spell& spell)
///////////////////////////////////////////////////////////////////////////////
{
  return spell.operator<<(out);
}

/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
void Hot::verify_apply() const
///////////////////////////////////////////////////////////////////////////////
{
  // no-op. This spell can be cast anywhere, anytime.
}

///////////////////////////////////////////////////////////////////////////////
unsigned Hot::apply() const
///////////////////////////////////////////////////////////////////////////////
{
  Engine& engine       = Engine::instance();
  World& world         = engine.world();
  WorldTile& tile      = world.get_tile(m_location);
  Atmosphere& atmos    = tile.atmosphere();
  PlayerAI& ai_player  = engine.ai_player();

  unsigned exp = 0;

  // 3 cases: Ocean, Mountain, Land
  OceanTile* ocean_tile  = dynamic_cast<OceanTile*>(&tile);
  MountainTile* mtn_tile = dynamic_cast<MountainTile*>(&tile);

  // Regardless of tile type, atmosphere is warmed
  int prior_temp = atmos.temperature();
  unsigned warmup = m_spell_level * DEGREES_PER_LEVEL;
  int new_temp = prior_temp + warmup;
  atmos.set_temperature(new_temp);

  if (ocean_tile != NULL) {
    // Heat ocean surface up
    int prior_ocean_temp = ocean_tile->surface_temp();
    int new_ocean_temp = prior_ocean_temp + warmup * OCEAN_SURFACE_CHG_RATIO;
    ocean_tile->set_surface_temp(new_ocean_temp);
  }
  else if (mtn_tile != NULL) {
    // Check snowpack. A sudden meltoff of snowpack could cause a flood.
    //unsigned snowpack = mtn_tile->snowpack();

    // This introduces the notion of a chain-reaction disaster. Players should
    // get double exp for induced disaster.

    // TODO
  }

  // This spell can kill if cast on a city and temps get high enough.
  City* city = tile.city();
  if (city != NULL && new_temp > KILL_THRESHOLD) {
    const float pct_killed =
      std::pow(new_temp - KILL_THRESHOLD, EXPONENT) / DIVISOR / // base
      std::sqrt(ai_player.tech_level()) /  // tech penalty
      100; // convert to percent

    // Reduce city pop by fraction
    exp += kill(*city, pct_killed);
  }

  return exp;
}

/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
void Cold::verify_apply() const
///////////////////////////////////////////////////////////////////////////////
{
  // no-op. This spell can be cast anywhere, anytime.
}

///////////////////////////////////////////////////////////////////////////////
unsigned Cold::apply() const
///////////////////////////////////////////////////////////////////////////////
{
  Engine& engine       = Engine::instance();
  World& world         = engine.world();
  WorldTile& tile      = world.get_tile(m_location);
  Atmosphere& atmos    = tile.atmosphere();
  PlayerAI& ai_player  = engine.ai_player();

  unsigned exp = 0;

  // 2 cases: Ocean, Land
  OceanTile* ocean_tile  = dynamic_cast<OceanTile*>(&tile);

  // Regardless of tile type, atmosphere is cooled
  int prior_temp = atmos.temperature();
  unsigned cooldown = m_spell_level * DEGREES_PER_LEVEL;
  int new_temp = prior_temp - cooldown;
  atmos.set_temperature(new_temp);

  if (ocean_tile != NULL) {
    // Cool ocean surface down
    int prior_ocean_temp = ocean_tile->surface_temp();
    int new_ocean_temp = prior_ocean_temp - cooldown * OCEAN_SURFACE_CHG_RATIO;

    // Once frozen, ocean temps cannot go lower
    if (new_ocean_temp < 32) {
      new_ocean_temp = 32;
    }

    ocean_tile->set_surface_temp(new_ocean_temp);
  }

  // This spell can kill if cast on a city and temps get low enough.
  City* city = tile.city();
  if (city != NULL && new_temp < KILL_THRESHOLD) {
    const float pct_killed =
      (std::pow(KILL_THRESHOLD - new_temp, EXPONENT) / DIVISOR) * // base
      (city->famine() ? FAMINE_BONUS : 1.0) / // famine bonus
      ai_player.tech_level() /  // tech penalty
      100; // convert to percent

    // Reduce city pop by kill-count
    exp = kill(*city, pct_killed);
  }

  return exp;
}

/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
void Infect::verify_apply() const
///////////////////////////////////////////////////////////////////////////////
{
  // This spell can only be cast on cities

  WorldTile& tile = Engine::instance().world().get_tile(m_location);

  // Check for city
  City* city = tile.city();
  RequireUser(city != NULL, "Must cast " << m_name << " on a city.");
}

///////////////////////////////////////////////////////////////////////////////
unsigned Infect::apply() const
///////////////////////////////////////////////////////////////////////////////
{
  Engine& engine       = Engine::instance();
  World& world         = engine.world();
  PlayerAI& ai_player  = engine.ai_player();
  WorldTile& tile      = world.get_tile(m_location);
  Atmosphere& atmos    = tile.atmosphere();

  unsigned exp = 0;

  // This verification should only have passed if we have a city
  City* city = tile.city();
  Require(city != NULL, "Verification did not catch NULL city");

  // Calculate extreme temperature bonus
  float extreme_temp_bonus = 1.0;
  int curr_temp = atmos.temperature();
  if (curr_temp < COLD_THRESHOLD) {
    extreme_temp_bonus +=
      (BONUS_PER_DEGREE_BEYOND_THRESHOLD * (COLD_THRESHOLD - curr_temp));
  }
  else if (curr_temp > WARM_THRESHOLD) {
    extreme_temp_bonus +=
      (BONUS_PER_DEGREE_BEYOND_THRESHOLD * (curr_temp - WARM_THRESHOLD));
  }

  // Calculate num killed
  const float pct_killed =
    (KILL_PCT_PER_LEVEL * m_spell_level) * // base kill %
    (1.0 + city->rank() * BONUS_PER_CITY_RANK) * // city rank bonus
    (city->famine() ? FAMINE_BONUS : 1.0) * // famine bonus
    extreme_temp_bonus / // extreme temp bonus (calculated above)
    ai_player.tech_level(); // tech penalty

  // Reduce city pop by kill-count
  exp += kill(*city, pct_killed);

  return exp;
}

/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
void WindSpell::verify_apply() const
///////////////////////////////////////////////////////////////////////////////
{
  // no-op. This spell can be cast anywhere, anytime.
}

///////////////////////////////////////////////////////////////////////////////
unsigned WindSpell::apply() const
///////////////////////////////////////////////////////////////////////////////
{
  Engine& engine       = Engine::instance();
  World& world         = engine.world();
  WorldTile& tile      = world.get_tile(m_location);
  Atmosphere& atmos    = tile.atmosphere();
  PlayerAI& ai_player  = engine.ai_player();

  unsigned exp = 0;

  // Compute and apply new wind speed
  Wind prior_wind = atmos.wind();
  unsigned speedup = m_spell_level * MPH_PER_LEVEL;
  Wind new_wind = prior_wind + speedup;
  atmos.set_wind(new_wind);

  // Wind can destroy infrastructure
  if (tile.infra_level() > 0) {
    unsigned damage_threshold = BASE_DAMAGE_THRESHOLD + ai_player.tech_level();
    if (new_wind.m_speed > damage_threshold) {
      unsigned max_infra_destroyed = 1 + ( (new_wind.m_speed - damage_threshold) /
                                           MPH_PER_ADDITIONAL_INFRA_DEVASTATION );
      exp += destroy_infra(tile, max_infra_destroyed);
    }
  }
  else {
    // Check for the existence of a city
    // This spell can kill if cast on a city and winds get high enough
    City* city = tile.city();
    if (city != NULL && new_wind.m_speed > KILL_THRESHOLD) {
      const float pct_killed =
        std::sqrt(static_cast<float>(new_wind.m_speed - KILL_THRESHOLD)) / // base
        std::sqrt(ai_player.tech_level()) / // tech penalty
        std::sqrt(city->defense()) / // city-defense penalty
        100; // convert to percent

      // Reduce city pop by kill-count
      exp += kill(*city, pct_killed);
    }
  }

  return exp;
}

/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
void Fire::verify_apply() const
///////////////////////////////////////////////////////////////////////////////
{
  // This spell can only be cast on tiles with plant growth

  WorldTile& tile = Engine::instance().world().get_tile(m_location);
  FoodTile* food_tile = dynamic_cast<FoodTile*>(&tile);
  RequireUser(food_tile != NULL,
              "Fire can only be cast on tiles with plant growth");
}

///////////////////////////////////////////////////////////////////////////////
unsigned Fire::apply() const
///////////////////////////////////////////////////////////////////////////////
{
  Engine& engine       = Engine::instance();
  World& world         = engine.world();
  FoodTile& tile       = dynamic_cast<FoodTile&>(world.get_tile(m_location));
  Atmosphere& atmos    = tile.atmosphere();
  PlayerAI& ai_player  = engine.ai_player();

  unsigned exp = 0;

  // Query properties relevant to fire destructiveness
  int wind_speed      = atmos.wind().m_speed;
  int temperature     = atmos.temperature();
  //int dewpoint        = atmos.dewpoint(); // TODO
  float soil_moisture = tile.soil_moisture();

  // Compute destructiveness of fire
  float destructiveness =
    m_spell_level * // base
    std::pow(TEMP_EXP_BASE, temperature - TEMP_TIPPING_POINT) * // temperature influence
    std::pow(WIND_EXP_BASE, wind_speed - WIND_TIPPING_POINT) * // wind infuence
    std::pow(MOISTURE_EXP_BASE, (MOISTURE_TIPPING_POINT - soil_moisture) * 100); // moisture influence

  // Fire can destroy infrastructure
  if (tile.infra_level() > 0) {
    unsigned max_infra_destroyed =
      destructiveness /
      (DESTRUCTIVENESS_PER_INFRA + std::sqrt(ai_player.tech_level()));
    if (max_infra_destroyed > 0) {
      exp += destroy_infra(tile, max_infra_destroyed);
    }
    damage_tile(tile, destructiveness / 100);
  }
  else {
    // Check for the existence of a city
    // This spell can kill if cast on a city and winds get high enough
    City* city = tile.city();
    const float pct_killed =
      destructiveness / // base
      std::sqrt(ai_player.tech_level()) / // tech penalty
      std::sqrt(city->defense()); // city-defense penalty

    // Reduce city pop by kill-count
    exp += kill(*city, pct_killed);
  }

  return exp;
}

/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
void Tstorm::verify_apply() const
///////////////////////////////////////////////////////////////////////////////
{
  // This spell can only be cast on plains and lush tiles (food tiles).

  WorldTile& tile = Engine::instance().world().get_tile(m_location);
  FoodTile* food_tile = dynamic_cast<FoodTile*>(&tile);
  RequireUser(food_tile != NULL,
              "Tstorm can only be cast on tiles with plant growth");
}

///////////////////////////////////////////////////////////////////////////////
unsigned Tstorm::apply() const
///////////////////////////////////////////////////////////////////////////////
{
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

  Engine& engine       = Engine::instance();
  World& world         = engine.world();
  FoodTile& tile       = dynamic_cast<FoodTile&>(world.get_tile(m_location));
  Atmosphere& atmos    = tile.atmosphere();
  PlayerAI& ai_player  = engine.ai_player();

  unsigned exp = 0;

  // Query properties relevant to tstorm destructiveness
  int wind_speed = atmos.wind().m_speed;
  int temperature     = atmos.temperature();
  //int dewpoint        = atmos.dewpoint(); // TODO
  int pressure   = atmos.pressure();

  // Compute destructiveness of tstorm
  float destructiveness =
    m_spell_level * // base
    std::pow(TEMP_EXP_BASE, temperature - TEMP_TIPPING_POINT) * // temperature influence
    std::pow(WIND_EXP_BASE, wind_speed - WIND_TIPPING_POINT) * // wind infuence
    std::pow(PRESSURE_EXP_BASE, pressure - PRESSURE_TIPPING_POINT); // moisture influence

  // TODO: What levels should chain reaction spells be?
  // TODO: In general, spells need to be much more verbose about
  // what they're doing and why.
  if (destructiveness > 10.0) {

  }

  // Check for the existence of a city
  // This spell can kill via lightning on a city
  City* city = tile.city();
  const float pct_killed =
    destructiveness * LIGHTING_PCT_KILL_PER_DESTRUCTIVENESS / // base

    std::sqrt(ai_player.tech_level()) / // tech penalty
    std::sqrt(city->defense()); // city-defense penalty

  // Reduce city pop by kill-count
  exp += kill(*city, pct_killed);

  return exp;
}
