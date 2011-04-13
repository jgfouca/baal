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
void Spell::kill(City& city,
                 unsigned num_killed) const
///////////////////////////////////////////////////////////////////////////////
{
  Engine& engine = Engine::instance();
  Interface& interface = engine.interface();

  city.kill(num_killed);
  SPELL_REPORT(interface, m_name << " has killed: " << num_killed);

  if (city.population() == 0) {
    SPELL_REPORT(interface, m_name << " has obliterated city: " << city.name());
    engine.world().remove_city(city);
  }
}

///////////////////////////////////////////////////////////////////////////////
unsigned Spell::destroy_infra(WorldTile& tile, unsigned max_destroyed) const
///////////////////////////////////////////////////////////////////////////////
{
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
  return num_destroyed;
}

///////////////////////////////////////////////////////////////////////////////
ostream& Spell::operator<<(ostream& out) const
///////////////////////////////////////////////////////////////////////////////
{
  return out << m_name << '[' << m_spell_level << ']';
}

///////////////////////////////////////////////////////////////////////////////
unsigned Spell::exp_for_destroyed_infra(unsigned num_destroyed)
///////////////////////////////////////////////////////////////////////////////
{
  if (num_destroyed == 0) {
    return 0;
  }
  else {
    static const unsigned base_exp = 200;
    return std::pow(2.0, num_destroyed) * base_exp;
  }
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
  Interface& interface = engine.interface();
  WorldTile& tile      = world.get_tile(m_location);
  Atmosphere& atmos    = tile.atmosphere();
  PlayerAI& ai_player  = engine.ai_player();

  unsigned num_killed = 0;

  // 3 cases: Ocean, Mountain, Land
  OceanTile* ocean_tile  = dynamic_cast<OceanTile*>(&tile);
  MountainTile* mtn_tile = dynamic_cast<MountainTile*>(&tile);
  FoodTile* food_tile    = dynamic_cast<FoodTile*>(&tile);

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
  else if (food_tile != NULL && new_temp > KILL_THRESHOLD) {
    // Damage tile (if temps are high enough)
    const float damage =
      std::sqrt(static_cast<float>(new_temp - KILL_THRESHOLD));
    food_tile->damage(damage);

    SPELL_REPORT(interface, m_name << " has inflicted damage: " << damage);
  }

  // This spell can kill if cast on a city and temps get high enough.
  City* city = tile.city();
  if (city != NULL && new_temp > KILL_THRESHOLD) {
    unsigned city_pop = city->population();
    const float pct_killed =
      std::sqrt(static_cast<float>(new_temp - KILL_THRESHOLD)) / // base
      ai_player.tech_level(); // tech penalty
    num_killed = pct_killed * city_pop;

    // Reduce city pop by kill-count
    kill(*city, num_killed);
  }

  return num_killed;
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
  Interface& interface = engine.interface();
  WorldTile& tile      = world.get_tile(m_location);
  Atmosphere& atmos    = tile.atmosphere();
  PlayerAI& ai_player  = engine.ai_player();

  unsigned num_killed = 0;

  // 2 cases: Ocean, Land
  OceanTile* ocean_tile  = dynamic_cast<OceanTile*>(&tile);
  FoodTile* food_tile    = dynamic_cast<FoodTile*>(&tile);

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
  else if (food_tile != NULL && new_temp < KILL_THRESHOLD) {
    // Damage tile (if temps are low enough)
    const float damage =
      std::sqrt(static_cast<float>(KILL_THRESHOLD - new_temp));
    food_tile->damage(damage);

    SPELL_REPORT(interface, m_name << " has inflicted damage: " << damage);
  }

  // This spell can kill if cast on a city and temps get low enough.
  City* city = tile.city();
  if (city != NULL && new_temp < KILL_THRESHOLD) {
    unsigned city_pop = city->population();
    const float pct_killed =
      std::sqrt(static_cast<float>(KILL_THRESHOLD - new_temp)) / // base
      ai_player.tech_level(); // tech penalty
    num_killed = pct_killed * city_pop;

    // Reduce city pop by kill-count
    kill(*city, num_killed);
  }

  return num_killed;
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

  unsigned num_killed = 0;

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
  unsigned city_pop = city->population();
  const float pct_killed =
    (KILL_PCT_PER_LEVEL * m_spell_level) * // base kill %
    (1.0 + city->rank() * BONUS_PER_CITY_RANK) * // city rank bonus
    (city->famine() ? FAMINE_BONUS : 1.0) * // famine bonus
    extreme_temp_bonus / // extreme temp bonus (calculated above)
    ai_player.tech_level(); // tech penalty
  num_killed = city_pop * pct_killed;

  // Reduce city pop by kill-count
  kill(*city, num_killed);

  return num_killed;
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
  Interface& interface = engine.interface();
  WorldTile& tile      = world.get_tile(m_location);
  Atmosphere& atmos    = tile.atmosphere();
  PlayerAI& ai_player  = engine.ai_player();

  unsigned num_killed = 0;
  unsigned infra_destroyed = 0;

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
      infra_destroyed += destroy_infra(tile, max_infra_destroyed);
    }
  }
  else {
    // Check for the existance of a city
    // This spell can kill if cast on a city and winds get high enough
    City* city = tile.city();
    if (city != NULL && new_wind.m_speed > KILL_THRESHOLD) {
      unsigned city_pop = city->population();
      const float pct_killed =
        std::sqrt(static_cast<float>(new_wind.m_speed - KILL_THRESHOLD)) / // base
        std::sqrt(ai_player.tech_level()) / // tech penalty
        std::sqrt(city->defense()); // city-defense penalty
      num_killed = pct_killed * city_pop;

      // Reduce city pop by kill-count
      kill(*city, num_killed);
    }
  }

  return num_killed + exp_for_destroyed_infra(infra_destroyed);
}

/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
unsigned Fire::apply() const
///////////////////////////////////////////////////////////////////////////////
{
  // TODO Fill in
  // Does not apply to oceans

  World& world = Engine::instance().world();
  WorldTile& tile = world.get_tile(m_location);
  LandTile* tile_ptr = dynamic_cast<LandTile*>(&tile);
  Require(tile_ptr != NULL, "Can only cast fire on land tiles");
  LandTile& affected_tile = *tile_ptr;

  // TODO: check for city, soil moisture, wind, temp, etc
  affected_tile.damage(.5);

  return 0;
}
