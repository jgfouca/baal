#include "Spell.hpp"
#include "World.hpp"
#include "Engine.hpp"
#include "Interface.hpp"
#include "Player.hpp"
#include "PlayerAI.hpp"
#include "City.hpp"

#include <iostream>
#include <cmath>
#include <limits>

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

// Given a tstorm's destructiveness, compute the level of the spawned disaster
unsigned tstorm_spawn_helper(float destructiveness, float base_cost)
{
  float destructiveness_unspent = destructiveness;
  unsigned level = 0;
  for ( ; ; ++level) {
    float next_level_cost = (level+1) * base_cost;
    if (destructiveness_unspent >= next_level_cost) {
      destructiveness_unspent -= next_level_cost;
    }
    else {
      return level;
    }
  }
}

}

///////////////////////////////////////////////////////////////////////////////
SpellPrereqStaticInitializer::SpellPrereqStaticInitializer()
///////////////////////////////////////////////////////////////////////////////
{
  // Tier 1

  // Hot, Cold, Wind, Infect have no prereqs

  // Tier 2

  Fire::PREREQ.m_min_player_level = 5;
  Fire::PREREQ.m_min_spell_prereqs.push_back(Prereq(SpellFactory::HOT, 1));

  Tstorm::PREREQ.m_min_player_level = 5;
  Tstorm::PREREQ.m_min_spell_prereqs.push_back(Prereq(SpellFactory::WIND, 1));

  Snow::PREREQ.m_min_player_level = 5;
  Snow::PREREQ.m_min_spell_prereqs.push_back(Prereq(SpellFactory::COLD, 1));

  // Tier 3

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

  // Tier 4

  Heatwave::PREREQ.m_min_player_level = 15;
  Heatwave::PREREQ.m_min_spell_prereqs.push_back(Prereq(SpellFactory::DRY, 1));

  Coldwave::PREREQ.m_min_player_level = 15;
  Coldwave::PREREQ.m_min_spell_prereqs.push_back(Prereq(SpellFactory::BLIZZARD, 1));

  Drought::PREREQ.m_min_player_level = 15;
  Drought::PREREQ.m_min_spell_prereqs.push_back(Prereq(SpellFactory::DRY, 1));

  Monsoon::PREREQ.m_min_player_level = 15;
  Monsoon::PREREQ.m_min_spell_prereqs.push_back(Prereq(SpellFactory::FLOOD, 1));

  // Tier 5

  Disease::PREREQ.m_min_player_level = 20;
  Disease::PREREQ.m_min_spell_prereqs.push_back(Prereq(SpellFactory::INFECT, 1));

  Earthquake::PREREQ.m_min_player_level = 20;

  Hurricane::PREREQ.m_min_player_level = 20;
  Hurricane::PREREQ.m_min_spell_prereqs.push_back(Prereq(SpellFactory::MONSOON, 1));

  // Tier 6

  Plague::PREREQ.m_min_player_level = 25;
  Plague::PREREQ.m_min_spell_prereqs.push_back(Prereq(SpellFactory::DISEASE, 1));

  Volcano::PREREQ.m_min_player_level = 25;
  Volcano::PREREQ.m_min_spell_prereqs.push_back(Prereq(SpellFactory::EARTHQUAKE, 1));

  // Tier 7

  Asteroid::PREREQ.m_min_player_level = 30;
  Asteroid::PREREQ.m_min_spell_prereqs.push_back(Prereq(SpellFactory::VOLCANO, 1));
}

///////////////////////////////////////////////////////////////////////////////
Spell::Spell(const std::string& name,
             unsigned           spell_level,
             const Location&    location,
             unsigned           base_cost,
             unsigned           cost_increment,
             const SpellPrereq& prereq)
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

  if (pct_killed > 100.0) {
    pct_killed = 100.0;
  }
  unsigned num_killed = city.population() * (pct_killed / 100);

  city.kill(num_killed);
  SPELL_REPORT("killed " << num_killed);

  if (city.population() < City::MIN_CITY_SIZE) {
    SPELL_REPORT("obliterated city '" << city.name() << "'");
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

  LandTile& land_tile = dynamic_cast<LandTile&>(tile);
  unsigned orig_infra = land_tile.infra_level();
  unsigned num_destroyed = // Cannot destroy infra that's not there
    (orig_infra < max_destroyed) ? orig_infra : max_destroyed;
  land_tile.destroy_infra(num_destroyed);

  SPELL_REPORT("destroyed " << num_destroyed << " levels of infrastructure");

  // Convert to exp
  static const unsigned base_exp = 200;
  return std::pow(2.0, num_destroyed) * base_exp;
}

///////////////////////////////////////////////////////////////////////////////
void Spell::damage_tile(WorldTile& tile, float damage_pct) const
///////////////////////////////////////////////////////////////////////////////
{
  if (damage_pct > 0.0) {
    LandTile& land_tile = dynamic_cast<LandTile&>(tile);

    if (damage_pct > 100.0) {
      damage_pct = 100.0;
    }
    land_tile.damage(damage_pct);

    SPELL_REPORT("caused " << damage_pct << "% damage to tile");
  }
}

///////////////////////////////////////////////////////////////////////////////
unsigned Spell::spawn(const std::string& spell_name, unsigned spell_level) const
///////////////////////////////////////////////////////////////////////////////
{
  const Spell& spell = SpellFactory::create_spell(spell_name,
                                                  spell_level,
                                                  m_location);

  // Check if this spell can be applied here
  bool verify_ok = true;
  try {
    spell.verify_apply();
  }
  catch (UserError& error) {
    verify_ok = false;
  }

  if (verify_ok) {
    SPELL_REPORT("caused a level " << spell_level << " " << spell_name);

    unsigned exp = CHAIN_REACTION_BONUS * spell.apply();
    delete &spell;
    return exp;
  }

  return 0;
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

  // TODO: Dewpoint should enhance spell kill count

  unsigned exp = 0;

  // 3 cases: Ocean, Mountain, Land
  OceanTile* ocean_tile  = dynamic_cast<OceanTile*>(&tile);
  MountainTile* mtn_tile = dynamic_cast<MountainTile*>(&tile);

  // Regardless of tile type, atmosphere is warmed
  int prior_temp = atmos.temperature();
  unsigned warmup = m_spell_level * DEGREES_PER_LEVEL;
  int new_temp = prior_temp + warmup;
  atmos.set_temperature(new_temp);
  SPELL_REPORT("raised temperature from " << prior_temp << " to " << new_temp);

  if (ocean_tile != NULL) {
    // Heat ocean surface up
    int prior_ocean_temp = ocean_tile->surface_temp();
    int new_ocean_temp = prior_ocean_temp + warmup * OCEAN_SURFACE_CHG_RATIO;
    ocean_tile->set_surface_temp(new_ocean_temp);
    SPELL_REPORT("raised ocean surface temperature from " <<
                 prior_ocean_temp << " to " << new_ocean_temp);
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
  if (city != NULL) {
    if (new_temp > KILL_THRESHOLD) {
      float base_kill_pct = m_base_kill_func(new_temp);
      float tech_penalty  = m_tech_penalty_func(ai_player.tech_level());

      const float pct_killed = base_kill_pct / tech_penalty;

      SPELL_REPORT("base kill % is " << base_kill_pct);
      SPELL_REPORT("tech penalty (divisor) is " << tech_penalty);
      SPELL_REPORT("final kill % is " << pct_killed);

      // Reduce city pop by fraction
      exp += kill(*city, pct_killed);
    }
    else {
      SPELL_REPORT("fell below kill threshold " << KILL_THRESHOLD);
    }
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
  SPELL_REPORT("reduced temperature from " << prior_temp << " to " << new_temp);

  if (ocean_tile != NULL) {
    // Cool ocean surface down
    int prior_ocean_temp = ocean_tile->surface_temp();
    int new_ocean_temp = prior_ocean_temp - cooldown * OCEAN_SURFACE_CHG_RATIO;

    // Once frozen, ocean temps cannot go lower
    if (new_ocean_temp < 32) {
      new_ocean_temp = 32;
    }

    ocean_tile->set_surface_temp(new_ocean_temp);
    SPELL_REPORT("reduced ocean surface temperature from " <<
                 prior_ocean_temp << " to " << new_ocean_temp);
  }

  // This spell can kill if cast on a city and temps get low enough.
  City* city = tile.city();
  if (city != NULL) {
    if (new_temp < KILL_THRESHOLD) {
      float base_kill_pct = m_base_kill_func(-new_temp);
      float wind_bonus    = m_wind_bonus_func(atmos.wind().m_speed);
      float famine_bonus  = city->famine() ? FAMINE_BONUS : 1.0;
      float tech_penalty  = m_tech_penalty_func(ai_player.tech_level());

      const float pct_killed =
        base_kill_pct * wind_bonus * famine_bonus / tech_penalty;

      SPELL_REPORT("base kill % is " << base_kill_pct);
      SPELL_REPORT("wind bonus (multiplier) is " << wind_bonus);
      SPELL_REPORT("famine bonus (multiplier) is " << famine_bonus);
      SPELL_REPORT("tech penalty (divisor) is " << tech_penalty);
      SPELL_REPORT("final kill % is " << pct_killed);

      // Reduce city pop by kill-count
      exp = kill(*city, pct_killed);
    }
    else {
      SPELL_REPORT("was above kill threshold " << KILL_THRESHOLD);
    }
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
  int degrees_extreme = 0;
  int curr_temp = atmos.temperature();
  if (curr_temp < COLD_THRESHOLD) {
    degrees_extreme = COLD_THRESHOLD - curr_temp;
  }
  else if (curr_temp > WARM_THRESHOLD) {
    degrees_extreme = curr_temp - WARM_THRESHOLD;
  }

  // Calculate num killed
  float base_kill_pct      = m_base_kill_func(m_spell_level);
  float city_size_bonus    = m_city_size_bonus_func(city->rank());
  float extreme_temp_bonus = m_extreme_temp_bonus_func(degrees_extreme);
  float famine_bonus       = city->famine() ? FAMINE_BONUS : 1.0;
  float tech_penalty       = m_tech_penalty_func(ai_player.tech_level());

  const float pct_killed = base_kill_pct *
                           city_size_bonus *
                           extreme_temp_bonus *
                           famine_bonus /
                           tech_penalty;

  SPELL_REPORT("base kill % is " << base_kill_pct);
  SPELL_REPORT("city-size bonus (multiplier) is " << city_size_bonus);
  SPELL_REPORT("extreme-temperature bonus (multiplier) is " << extreme_temp_bonus);
  SPELL_REPORT("famine bonus (multiplier) is " << famine_bonus);
  SPELL_REPORT("tech penalty (divisor) is " << tech_penalty);
  SPELL_REPORT("final kill % is " << pct_killed);

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
  SPELL_REPORT("increased wind from " << prior_wind.m_speed <<
               " to " << new_wind.m_speed);

  // Wind can destroy infrastructure
  if (tile.infra_level() > 0) {
    unsigned damage_threshold = BASE_DAMAGE_THRESHOLD + ai_player.tech_level();
    if (new_wind.m_speed > damage_threshold) {
      unsigned max_infra_destroyed = 1 + ( (new_wind.m_speed - damage_threshold) /
                                           MPH_PER_ADDITIONAL_INFRA_DEVASTATION );
      exp += destroy_infra(tile, max_infra_destroyed);
    }
    else {
      SPELL_REPORT("fell below the threshold of " << damage_threshold <<
                   " required to destroy infrastructure");
    }
  }

  // Check for the existence of a city
  // This spell can kill if cast on a city and winds get high enough
  City* city = tile.city();
  if (city != NULL) {
    if (new_wind.m_speed > KILL_THRESHOLD) {
      float base_kill_pct = std::sqrt(new_wind.m_speed - KILL_THRESHOLD);
      float tech_penalty = std::sqrt(ai_player.tech_level());
      float defense_penalty = std::sqrt(city->defense());
      float pct_killed = base_kill_pct / tech_penalty / defense_penalty;

      SPELL_REPORT("base kill % is " << base_kill_pct);
      SPELL_REPORT("tech penalty (divisor) is " << tech_penalty);
      SPELL_REPORT("city defense penalty (divisor) is " << defense_penalty);
      SPELL_REPORT("final kill % is " << pct_killed);

      // Reduce city pop by kill-count
      exp += kill(*city, pct_killed);
    }
    else {
      SPELL_REPORT("fell below the kill theshold " << KILL_THRESHOLD);
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
  float base_destructiveness = m_spell_level;
  float temp_multiplier = std::pow(TEMP_EXP_BASE, temperature - TEMP_TIPPING_POINT);
  float wind_multiplier = std::pow(WIND_EXP_BASE, wind_speed - WIND_TIPPING_POINT);
  float moisture_multiplier =
    std::pow(MOISTURE_EXP_BASE, (MOISTURE_TIPPING_POINT - soil_moisture) * 100);

  float destructiveness = base_destructiveness *
                          temp_multiplier *
                          wind_multiplier *
                          moisture_multiplier;

  SPELL_REPORT("base destructiveness " << base_destructiveness);
  SPELL_REPORT("temperature multiplier " << temp_multiplier);
  SPELL_REPORT("wind multiplier " << wind_multiplier);
  SPELL_REPORT("moisture multiplier " << moisture_multiplier);
  SPELL_REPORT("total destructiveness " << destructiveness);

  // Fire can destroy infrastructure
  if (tile.infra_level() > 0) {
    float damage_threshold = DESTRUCTIVENESS_PER_INFRA + std::sqrt(ai_player.tech_level());
    unsigned max_infra_destroyed = destructiveness / damage_threshold;
    if (max_infra_destroyed > 0) {
      exp += destroy_infra(tile, max_infra_destroyed);
    }
    else {
      SPELL_REPORT("fell below destructive threshold " << damage_threshold
                   << " required to destroy infrastructure");
    }
    damage_tile(tile, destructiveness);
  }

  // Check for the existence of a city
  // This spell can kill if cast on a city and winds get high enough
  City* city = tile.city();
  if (city != NULL) {
    float base_kill_pct = destructiveness;
    float tech_penalty = std::sqrt(ai_player.tech_level());
    float defense_penalty = std::sqrt(city->defense());
    float pct_killed = base_kill_pct / tech_penalty / defense_penalty;

    SPELL_REPORT("base kill % is " << base_kill_pct);
    SPELL_REPORT("tech penalty (divisor) is " << tech_penalty);
    SPELL_REPORT("city defense penalty (divisor) is " << defense_penalty);
    SPELL_REPORT("final kill % is " << pct_killed);

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
  float base_destructiveness = m_spell_level;
  float temp_multiplier = std::pow(TEMP_EXP_BASE, temperature - TEMP_TIPPING_POINT);
  float wind_multiplier = std::pow(WIND_EXP_BASE, wind_speed - WIND_TIPPING_POINT);
  float pressure_multiplier = std::pow(PRESSURE_EXP_BASE, pressure - PRESSURE_TIPPING_POINT);

  float destructiveness = base_destructiveness *
                          temp_multiplier *
                          wind_multiplier *
                          pressure_multiplier;

  SPELL_REPORT("base destructiveness " << base_destructiveness);
  SPELL_REPORT("temperature multiplier " << temp_multiplier);
  SPELL_REPORT("wind multiplier " << wind_multiplier);
  SPELL_REPORT("pressure multiplier " << pressure_multiplier);
  SPELL_REPORT("total destructiveness " << destructiveness);

  if (destructiveness > WIND_DESTRUCTIVENESS_THRESHOLD) {
    unsigned wind_level =
      tstorm_spawn_helper(destructiveness, WIND_DESTRUCTIVENESS_THRESHOLD);
    Require(wind_level > 0, "Problem in helper");

    exp += spawn(SpellFactory::WIND, wind_level);
  }

  if (destructiveness > FLOOD_DESTRUCTIVENESS_THRESHOLD) {
    unsigned flood_level =
      tstorm_spawn_helper(destructiveness, FLOOD_DESTRUCTIVENESS_THRESHOLD);
    Require(flood_level > 0, "Problem in helper");

    exp += spawn(SpellFactory::FLOOD, flood_level);
  }

  if (destructiveness > TORNADO_DESTRUCTIVENESS_THRESHOLD) {
    unsigned tornado_level =
      tstorm_spawn_helper(destructiveness, TORNADO_DESTRUCTIVENESS_THRESHOLD);
    Require(tornado_level > 0, "Problem in helper");

    exp += spawn(SpellFactory::TORNADO, tornado_level);
  }

  // Check for the existence of a city
  // This spell can kill via lightning on a city
  City* city = tile.city();
  if (city != NULL) {
    float base_kill_pct = destructiveness * LIGHTING_PCT_KILL_PER_DESTRUCTIVENESS;
    float tech_penalty = std::sqrt(ai_player.tech_level());
    float defense_penalty = std::sqrt(city->defense());
    float pct_killed = base_kill_pct / tech_penalty / defense_penalty;

    SPELL_REPORT("base kill % is " << base_kill_pct);
    SPELL_REPORT("tech penalty (divisor) is " << tech_penalty);
    SPELL_REPORT("city defense penalty (divisor) is " << defense_penalty);
    SPELL_REPORT("final kill % is " << pct_killed);

    // Reduce city pop by kill-count
    exp += kill(*city, pct_killed);
  }

  return exp;
}
