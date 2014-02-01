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
#include <algorithm>

using std::ostream;
namespace mpl = boost::mpl;

namespace baal {

constexpr int Hot::KILL_THRESHOLD;
constexpr int Fire::TEMP_TIPPING_POINT;
constexpr int Fire::WIND_TIPPING_POINT;
constexpr unsigned WindSpell::KILL_THRESHOLD;
constexpr unsigned WindSpell::DAMAGE_THRESHOLD;

const std::string Hot::NAME       = "hot";
const std::string Cold::NAME      = "cold";
const std::string WindSpell::NAME = "wind";
const std::string Infect::NAME    = "infect";

const std::string Fire::NAME   = "fire";
const std::string Tstorm::NAME = "tstorm";
const std::string Snow::NAME   = "snow";

const std::string Avalanche::NAME = "avalanche";
const std::string Flood::NAME     = "flood";
const std::string Dry::NAME       = "dry";
const std::string Blizzard::NAME  = "blizzard";
const std::string Tornado::NAME   = "tornado";

const std::string Heatwave::NAME = "heatwave";
const std::string Coldwave::NAME = "coldwave";
const std::string Drought::NAME  = "drought";
const std::string Monsoon::NAME  = "monsoon";

const std::string Disease::NAME    = "disease";
const std::string Earthquake::NAME = "earthquake";
const std::string Hurricane::NAME  = "hurricane";

const std::string Plague::NAME  = "plague";
const std::string Volcano::NAME = "volcano";

const std::string Asteroid::NAME = "asteroid";

const SpellPrereq Hot::PREREQ       = {1, vecstr_t()};
const SpellPrereq Cold::PREREQ      = {1, vecstr_t()};
const SpellPrereq WindSpell::PREREQ = {1, vecstr_t()};
const SpellPrereq Infect::PREREQ    = {1, vecstr_t()};

const SpellPrereq Fire::PREREQ   = {5, vecstr_t({Hot::NAME})};
const SpellPrereq Tstorm::PREREQ = {5, vecstr_t({WindSpell::NAME})};
const SpellPrereq Snow::PREREQ   = {5, vecstr_t({Cold::NAME})};

const SpellPrereq Avalanche::PREREQ = {10, vecstr_t({Snow::NAME})};
const SpellPrereq Flood::PREREQ     = {10, vecstr_t({Tstorm::NAME})};
const SpellPrereq Dry::PREREQ       = {10, vecstr_t({Fire::NAME})};
const SpellPrereq Blizzard::PREREQ  = {10, vecstr_t({Snow::NAME})};
const SpellPrereq Tornado::PREREQ   = {10, vecstr_t({Tstorm::NAME})};

const SpellPrereq Heatwave::PREREQ = {15, vecstr_t({Dry::NAME})};
const SpellPrereq Coldwave::PREREQ = {15, vecstr_t({Blizzard::NAME})};
const SpellPrereq Drought::PREREQ  = {15, vecstr_t({Dry::NAME})};
const SpellPrereq Monsoon::PREREQ  = {15, vecstr_t({Flood::NAME})};

const SpellPrereq Disease::PREREQ    = {20, vecstr_t({Infect::NAME})};
const SpellPrereq Earthquake::PREREQ = {20, vecstr_t()};
const SpellPrereq Hurricane::PREREQ  = {20, vecstr_t({Monsoon::NAME})};

const SpellPrereq Plague::PREREQ  = {25, vecstr_t({Disease::NAME})};
const SpellPrereq Volcano::PREREQ = {25, vecstr_t({Earthquake::NAME})};

const SpellPrereq Asteroid::PREREQ = {30, vecstr_t({Volcano::NAME})};

///////////////////////////////////////////////////////////////////////////////
Spell::Spell(const std::string& name,
             unsigned           spell_level,
             const Location&    location,
             unsigned           base_cost,
             const SpellPrereq& prereq,
             Engine&            engine,
             const SpellSpec&   spec)
///////////////////////////////////////////////////////////////////////////////
  : m_name(name),
    m_spell_level(spell_level),
    m_location(location),
    m_base_cost(base_cost),
    m_prereq(prereq),
    m_engine(engine),
    m_spec(spec)
{
  Assert(SpellFactory::is_in_all_names(name), name);
}

///////////////////////////////////////////////////////////////////////////////
std::pair<unsigned, bool> Spell::kill_base(WorldTile const& tile, float destructiveness) const
///////////////////////////////////////////////////////////////////////////////
{
  float base_pct = m_spec.m_kill_spec.first(tile, destructiveness);
  SPELL_REPORT("base kill %: " << base_pct);
  for (auto factor : m_spec.m_kill_spec.second) {
    const float mitigation_multiplier = factor.second(tile);
    base_pct /= mitigation_multiplier;
    SPELL_REPORT(factor.first << ": " << mitigation_multiplier);
  }

  SPELL_REPORT("final kill %: " << base_pct);

  return kill(*tile.city(), base_pct);
}

///////////////////////////////////////////////////////////////////////////////
std::pair<unsigned, bool> Spell::kill(City& city,
                                      float pct_killed) const
///////////////////////////////////////////////////////////////////////////////
{
  pct_killed = std::min(pct_killed, float(100.0));
  unsigned num_killed = city.population() * (pct_killed / 100);
  if (num_killed == 0) {
    return std::make_pair(0, false);
  }

  city.kill(num_killed);
  SPELL_REPORT("killed " << num_killed);

  if (city.population() < City::MIN_CITY_SIZE) {
    SPELL_REPORT("obliterated city '" << city.name() << "'");
    m_engine.world().remove_city(city);
    city.kill(city.population());
    num_killed += city.population();

    // TODO: Give bigger city-kill bonus based on maximum attained rank of
    // city.
    return std::make_pair(num_killed + CITY_DESTROY_EXP_BONUS, true);
  }
  else {
    return std::make_pair(num_killed, false);
  }
}

///////////////////////////////////////////////////////////////////////////////
unsigned Spell::damage(LandTile& tile, float destructiveness, const base_factor_pair_t& spec, const std::string& name) const
///////////////////////////////////////////////////////////////////////////////
{
  Require(name == "defense" || name == "infrastructure", "Unknown name " << name);

  float num_destroyed = spec.first(tile, destructiveness);
  if (num_destroyed != DOES_NOT_APPLY) {
    SPELL_REPORT("base " << name << " damage capacity: " << num_destroyed);
    for (auto factor : spec.second) {
      const float mitigation_multiplier = factor.second(tile);
      num_destroyed /= mitigation_multiplier;
      SPELL_REPORT(factor.first << ": " << mitigation_multiplier);
    }

    unsigned damage_capacity = std::round(num_destroyed);
    SPELL_REPORT("total " << name <<  " damage capacity: " << damage_capacity);

    if (name == "defense") {
      return destroy(tile, damage_capacity, name,
                     [](LandTile& tile) -> unsigned { return tile.city()->defense(); },
                     [](LandTile& tile, unsigned num_destroy) -> void { tile.city()->destroy_defense(num_destroy); },
                     [](unsigned num_destroyed) -> unsigned { return DEFENSE_EXP_FUNC(num_destroyed); } );
    }
    else {
      return destroy(tile, damage_capacity, name,
                     [](LandTile& tile) -> unsigned { return tile.infra_level(); },
                     [](LandTile& tile, unsigned num_destroy) -> void { tile.destroy_infra(num_destroy); },
                     [](unsigned num_destroyed) -> unsigned { return INFRA_EXP_FUNC(num_destroyed); } );
    }
  }
  return 0;
}

///////////////////////////////////////////////////////////////////////////////
unsigned Spell::destroy(LandTile& tile, unsigned max_destroyed, const std::string& name,
                        std::function<unsigned(LandTile&)> const&  getter,
                        std::function<void(LandTile&, unsigned)> const&  destroyer,
                        std::function<unsigned(unsigned)> const& exp) const
///////////////////////////////////////////////////////////////////////////////
{
  const unsigned num_destroyed = std::min(getter(tile), max_destroyed);
  if (num_destroyed > 0) {

    destroyer(tile, num_destroyed);
    if (getter(tile) > 0) {
      SPELL_REPORT("destroyed " << num_destroyed << " levels of " << name);
    }
    else {
      SPELL_REPORT("destroyed all " << name << " (" << num_destroyed << " levels)");
    }

    // Convert to exp
    return exp(num_destroyed);
  }
  else {
    if (getter(tile) == 0) {
      SPELL_REPORT("no " << name << " to destroy");
    }

    return 0;
  }
}

///////////////////////////////////////////////////////////////////////////////
void Spell::damage_tile(LandTile& tile, float destructiveness) const
///////////////////////////////////////////////////////////////////////////////
{
  float damage_pct = m_spec.m_tile_dmg_spec(tile, destructiveness);
  if (damage_pct != DOES_NOT_APPLY) {
    if (damage_pct > 0.0) {
      LandTile& land_tile = dynamic_cast<LandTile&>(tile);

      damage_pct = std::min(damage_pct, float(100.0));
      land_tile.damage(damage_pct);
    }
    SPELL_REPORT("caused " << damage_pct << "% damage to tile");
  }
}

///////////////////////////////////////////////////////////////////////////////
unsigned Spell::trigger(const std::string& spell_name, unsigned spell_level) const
///////////////////////////////////////////////////////////////////////////////
{
  auto spell = SpellFactory::create_spell(spell_name,
                                          m_engine,
                                          spell_level,
                                          m_location);

  // Check if this spell can be applied here
  try {
    spell->verify_apply();

    SPELL_REPORT("caused a level " << spell_level << " " << spell_name);

    unsigned exp = CHAIN_REACTION_BONUS * spell->apply();
    return exp;
  }
  catch (UserError& error) {
    return 0;
  }
}

///////////////////////////////////////////////////////////////////////////////
float Spell::compute_destructiveness(const WorldTile& tile, bool report) const
///////////////////////////////////////////////////////////////////////////////
{
  factor_vector_t const& factors = m_spec.m_destructiveness_spec;
  float rv = 1.0;
  for (auto factor : factors) {
    const float factor_multiplier = factor.second(tile);
    rv *= factor_multiplier;
    if (report) {
      SPELL_REPORT(factor.first << ": " << factor_multiplier);
    }
  }
  if (report) {
    SPELL_REPORT("total destructiveness: " << rv);
  }
  return rv;
}

///////////////////////////////////////////////////////////////////////////////
void Spell::verify_no_repeat_cast() const
///////////////////////////////////////////////////////////////////////////////
{
  const WorldTile& tile = m_engine.world().get_tile(m_location);
  RequireUser(!tile.already_casted(m_name), "Already cast " << m_name << " on this tile");
}

///////////////////////////////////////////////////////////////////////////////
unsigned Spell::apply() const
///////////////////////////////////////////////////////////////////////////////
{
  // Every spell has the same phases:
  // 1) Modifies the environment
  // 2) Potentially triggers other disasters
  // 3) Impacts civilization:
  //   3.a) Impacting cities
  //     3.a.1) Kills citizens
  //     3.a.2) Reduces city defense
  //   3.b) Destroys infrastructure

  World& world         = m_engine.world();
  WorldTile& tile      = world.get_tile(m_location);
  unsigned exp         = 0;

  std::vector<std::pair<std::string, unsigned>> triggered;
  std::vector<WorldTile*> affected_tiles;
  apply_to_world(tile, affected_tiles, triggered);
  Require( !affected_tiles.empty(), "No affected tiles?" );

  for (WorldTile* affected_tile : affected_tiles) {
    const float destructiveness = compute_destructiveness(*affected_tile, true /*report*/);

    if (affected_tile->infra_level() > 0) {
      exp += damage(dynamic_cast<LandTile&>(*affected_tile), destructiveness, m_spec.m_infra_dmg_spec, "infrastructure");
    }
    else if (affected_tile->city() != nullptr) {
      // order matters here!
      std::pair<unsigned, bool> result = kill_base(*affected_tile, destructiveness);
      exp += result.first;
      if (!result.second) {
        // city was not wiped, still exists
        exp += damage(dynamic_cast<LandTile&>(*affected_tile), destructiveness, m_spec.m_defense_dmg_spec, "defense");
      }
    }

    // Also, can damage land tiles
    if (dynamic_cast<LandTile*>(affected_tile) != nullptr) {
      damage_tile(dynamic_cast<LandTile&>(*affected_tile), destructiveness);
    }

    // Register that spell was cast on this tile
    affected_tile->cast(m_name);
  }

  for (auto trig : triggered) {
    exp += trigger(trig.first, trig.second);
  }

  return exp;
}

///////////////////////////////////////////////////////////////////////////////
ostream& operator<<(ostream& out, const Spell& spell)
///////////////////////////////////////////////////////////////////////////////
{
  return out << spell.name() << '[' << spell.level() << ']';
}

/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
void Hot::verify_apply() const
///////////////////////////////////////////////////////////////////////////////
{
  verify_no_repeat_cast();
}

///////////////////////////////////////////////////////////////////////////////
void Hot::apply_to_world(WorldTile& tile,
                         std::vector<WorldTile*>& affected_tiles,
                         std::vector<std::pair<std::string, unsigned>>& triggered) const
///////////////////////////////////////////////////////////////////////////////
{
  Atmosphere& atmos    = tile.atmosphere();

  // 3 cases: Ocean, Mountain, Land
  OceanTile* ocean_tile  = dynamic_cast<OceanTile*>(&tile);
  MountainTile* mtn_tile = dynamic_cast<MountainTile*>(&tile);

  // Regardless of tile type, atmosphere is warmed
  const int prior_temp = atmos.temperature();
  const unsigned warmup = m_degrees_heated_land(tile, m_spell_level);
  const int new_temp = prior_temp + warmup;
  atmos.set_temperature(new_temp);
  SPELL_REPORT("raised temperature from " << prior_temp << " to " << new_temp);

  if (ocean_tile != nullptr) {
    // Heat ocean surface up
    const int prior_ocean_temp = ocean_tile->surface_temp();
    const int ocean_warmup = m_degrees_heated_ocean(tile, m_spell_level);
    const int new_ocean_temp = prior_ocean_temp + ocean_warmup;
    ocean_tile->set_surface_temp(new_ocean_temp);
    SPELL_REPORT("raised ocean surface temperature from " <<
                 prior_ocean_temp << " to " << new_ocean_temp);
  }
  else if (mtn_tile != nullptr) {
    // Check snowpack. A sudden meltoff of snowpack could cause a flood.
    //unsigned snowpack = mtn_tile->snowpack();

    // This introduces the notion of a chain-reaction disaster. Players should
    // get double exp for induced disaster.

    // TODO
  }

  affected_tiles.push_back(&tile);
}

/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
void Cold::verify_apply() const
///////////////////////////////////////////////////////////////////////////////
{
  verify_no_repeat_cast();
}

///////////////////////////////////////////////////////////////////////////////
void Cold::apply_to_world(WorldTile& tile,
                          std::vector<WorldTile*>& affected_tiles,
                          std::vector<std::pair<std::string, unsigned>>& triggered) const
///////////////////////////////////////////////////////////////////////////////
{
  Atmosphere& atmos    = tile.atmosphere();

  // 2 cases: Ocean, Land
  OceanTile* ocean_tile  = dynamic_cast<OceanTile*>(&tile);

  // Regardless of tile type, atmosphere is cooled
  const int prior_temp = atmos.temperature();
  const unsigned cooldown = m_degrees_cooled_land(tile, m_spell_level);
  const int new_temp = prior_temp - cooldown;
  atmos.set_temperature(new_temp);
  SPELL_REPORT("reduced temperature from " << prior_temp << " to " << new_temp);

  if (ocean_tile != nullptr) {
    // Cool ocean surface down
    const int prior_ocean_temp = ocean_tile->surface_temp();
    const unsigned ocean_cooldown = m_degrees_cooled_ocean(tile, m_spell_level);
    int new_ocean_temp = prior_ocean_temp - ocean_cooldown;

    // Once frozen, ocean temps cannot go lower
    if (new_ocean_temp < 32) {
      new_ocean_temp = 32;
    }

    ocean_tile->set_surface_temp(new_ocean_temp);
    SPELL_REPORT("reduced ocean surface temperature from " <<
                 prior_ocean_temp << " to " << new_ocean_temp);
  }

  affected_tiles.push_back(&tile);
}

/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
void Infect::verify_apply() const
///////////////////////////////////////////////////////////////////////////////
{
  // This spell can only be cast on cities

  WorldTile& tile = m_engine.world().get_tile(m_location);

  // Check for city
  City* city = tile.city();
  RequireUser(city != nullptr, "Must cast " << m_name << " on a city.");

  verify_no_repeat_cast();
}

///////////////////////////////////////////////////////////////////////////////
void Infect::apply_to_world(WorldTile& tile,
                            std::vector<WorldTile*>& affected_tiles,
                            std::vector<std::pair<std::string, unsigned>>& triggered) const
///////////////////////////////////////////////////////////////////////////////
{
  // No affect on world properties
  affected_tiles.push_back(&tile);
}

/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
void WindSpell::verify_apply() const
///////////////////////////////////////////////////////////////////////////////
{
  verify_no_repeat_cast();
}

///////////////////////////////////////////////////////////////////////////////
void WindSpell::apply_to_world(WorldTile& tile,
                               std::vector<WorldTile*>& affected_tiles,
                               std::vector<std::pair<std::string, unsigned>>& triggered) const
///////////////////////////////////////////////////////////////////////////////
{
  Atmosphere& atmos = tile.atmosphere();

  // Compute and apply new wind speed
  const Wind prior_wind = atmos.wind();
  const unsigned speedup = m_wind_speedup(tile, m_spell_level);
  const Wind new_wind = prior_wind + speedup;
  const unsigned new_wind_speed = new_wind.m_speed;
  atmos.set_wind(new_wind);
  SPELL_REPORT("increased wind from " << prior_wind.m_speed <<
               " to " << new_wind_speed);

  affected_tiles.push_back(&tile);
}

/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
void Fire::verify_apply() const
///////////////////////////////////////////////////////////////////////////////
{
  // This spell can only be cast on tiles with plant growth

  WorldTile& tile = m_engine.world().get_tile(m_location);
  FoodTile* food_tile = dynamic_cast<FoodTile*>(&tile);
  RequireUser(food_tile != nullptr,
              "Fire can only be cast on tiles with plant growth");

  verify_no_repeat_cast();
}

///////////////////////////////////////////////////////////////////////////////
void Fire::apply_to_world(WorldTile& tile,
                          std::vector<WorldTile*>& affected_tiles,
                          std::vector<std::pair<std::string, unsigned>>& triggered) const
///////////////////////////////////////////////////////////////////////////////
{
  // No extra effects on world.
  affected_tiles.push_back(&tile);
}

/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
void Tstorm::verify_apply() const
///////////////////////////////////////////////////////////////////////////////
{
  // This spell can only be cast on plains and lush tiles (food tiles).

  WorldTile& tile = m_engine.world().get_tile(m_location);
  FoodTile* food_tile = dynamic_cast<FoodTile*>(&tile);
  RequireUser(food_tile != nullptr,
              "Tstorm can only be cast on tiles with plant growth");

  verify_no_repeat_cast();
}

///////////////////////////////////////////////////////////////////////////////
void Tstorm::apply_to_world(WorldTile& tile,
                            std::vector<WorldTile*>& affected_tiles,
                            std::vector<std::pair<std::string, unsigned>>& triggered) const
///////////////////////////////////////////////////////////////////////////////
{
  const float destructiveness = compute_destructiveness(tile, false);

  const unsigned wind_spawn_level     = m_wind_spawn_func(destructiveness);
  const unsigned flood_spawn_level    = m_flood_spawn_func(destructiveness);
  const unsigned tornado_spawn_level  = m_tornado_spawn_func(destructiveness);

  if (wind_spawn_level > 0) {
    triggered.push_back(std::make_pair(WindSpell::NAME, wind_spawn_level));
  }

  if (flood_spawn_level > 0) {
    triggered.push_back(std::make_pair(Flood::NAME, flood_spawn_level));
  }
  else {
    // Some minimal impact on soil moisture, but this tstorm was not
    // a big rain producer
    const float new_moisture = tile.soil_moisture() + DRY_STORM_MOISTURE_ADD;
    SPELL_REPORT("Raised soil moisture from " << tile.soil_moisture() << " to " << new_moisture);
    tile.set_soil_moisture(new_moisture);
  }

  if (tornado_spawn_level > 0) {
    triggered.push_back(std::make_pair(Tornado::NAME, tornado_spawn_level));
  }

  // TODO: Need a better system for this (computing affected area)
  affected_tiles.push_back(&tile);
}

///////////////////////////////////////////////////////////////////////////////
void Snow::verify_apply() const
///////////////////////////////////////////////////////////////////////////////
{
  // Must be cast on a land tile
  WorldTile& tile = m_engine.world().get_tile(m_location);
  LandTile* land_tile = dynamic_cast<LandTile*>(&tile);
  RequireUser(land_tile != nullptr,
              "Snow can only be cast on land tiles");

  // Must be cold
  RequireUser(tile.atmosphere().temperature() <= MAX_TEMP,
              "It is not cold enough on this tile for it to snow, maximum temp for this spell is " << MAX_TEMP);

  verify_no_repeat_cast();
}

///////////////////////////////////////////////////////////////////////////////
void Snow::apply_to_world(WorldTile& tile,
                          std::vector<WorldTile*>& affected_tiles,
                          std::vector<std::pair<std::string, unsigned>>& triggered) const
///////////////////////////////////////////////////////////////////////////////
{
  const float destructiveness = compute_destructiveness(tile, false);
  const unsigned snowfall = m_snowfall_func(destructiveness);
  const unsigned new_snowpack = tile.snowpack() + snowfall;
  SPELL_REPORT("With " << snowfall << " inches of snowfall, snowpack raised to " << new_snowpack);
  tile.set_snowpack(new_snowpack);

  affected_tiles.push_back(&tile);
}

///////////////////////////////////////////////////////////////////////////////
void Avalanche::verify_apply() const
///////////////////////////////////////////////////////////////////////////////
{
  // Must be cast on a hill or mountain tile
  WorldTile& tile = m_engine.world().get_tile(m_location);
  HillsTile* hills_tile = dynamic_cast<HillsTile*>(&tile);
  MountainTile* mtn_tile = dynamic_cast<MountainTile*>(&tile);
  RequireUser(hills_tile != nullptr || mtn_tile != nullptr,
              "Avalanche can only be cast on hill or mountain tiles");

  // Must have snow
  RequireUser(tile.snowpack() > 0, "There is no snow on this tile");

  verify_no_repeat_cast();
}

///////////////////////////////////////////////////////////////////////////////
void Avalanche::apply_to_world(WorldTile& tile,
                          std::vector<WorldTile*>& affected_tiles,
                          std::vector<std::pair<std::string, unsigned>>& triggered) const
///////////////////////////////////////////////////////////////////////////////
{
  // No extra effects on world.
  affected_tiles.push_back(&tile);
}

///////////////////////////////////////////////////////////////////////////////
void Flood::verify_apply() const
///////////////////////////////////////////////////////////////////////////////
{
  // Must be cast on a land tile
  WorldTile& tile = m_engine.world().get_tile(m_location);
  LandTile* land_tile = dynamic_cast<LandTile*>(&tile);
  RequireUser(land_tile != nullptr,
              "Flood can only be cast on land tiles");

  // Must have snow
  RequireUser(tile.atmosphere().temperature() > MIN_TEMP,
              "Too cold to flood, temp must be avove " << MIN_TEMP);

  verify_no_repeat_cast();
}

///////////////////////////////////////////////////////////////////////////////
void Flood::apply_to_world(WorldTile& tile,
                          std::vector<WorldTile*>& affected_tiles,
                          std::vector<std::pair<std::string, unsigned>>& triggered) const
///////////////////////////////////////////////////////////////////////////////
{
  World& world = m_engine.world();

  factor_vector_t const& factors = m_spec.m_destructiveness_spec;
  float destructiveness_without_land_factors = 1.0;
  for (auto factor : factors) {
    if (factor.first != "moisture" && factor.first != "elevation") {
      const float factor_multiplier = factor.second(tile);
      destructiveness_without_land_factors *= factor_multiplier;
    }
  }

  const float rainfall = m_rainfall_func(destructiveness_without_land_factors);
  const float average_precip = tile.climate().precip(world.time().season());
  const float orig_moisture = tile.soil_moisture();
  const float added_moisture = rainfall / average_precip;
  const float new_moisture = orig_moisture + added_moisture;
  tile.set_soil_moisture(new_moisture);

  affected_tiles.push_back(&tile);

  SPELL_REPORT("With " << rainfall << " inches of rainfall, soil moisture raised to " << new_moisture);
}

///////////////////////////////////////////////////////////////////////////////
void Dry::verify_apply() const
///////////////////////////////////////////////////////////////////////////////
{
  // This spell can only be cast on tiles with soil moisture

  WorldTile& tile = m_engine.world().get_tile(m_location);
  TileWithSoil* soil_tile = dynamic_cast<TileWithSoil*>(&tile);
  RequireUser(soil_tile != nullptr,
              "Dry can only be cast on tiles with soil moisture");

  verify_no_repeat_cast();
}

///////////////////////////////////////////////////////////////////////////////
void Dry::apply_to_world(WorldTile& tile,
                         std::vector<WorldTile*>& affected_tiles,
                         std::vector<std::pair<std::string, unsigned>>& triggered) const
///////////////////////////////////////////////////////////////////////////////
{
  factor_vector_t const& factors = m_spec.m_destructiveness_spec;
  float destructiveness_without_land_factors = 1.0;
  for (auto factor : factors) {
    if (factor.first != "moisture") {
      const float factor_multiplier = factor.second(tile);
      destructiveness_without_land_factors *= factor_multiplier;
    }
  }

  const float orig_moisture = tile.soil_moisture();
  const float lost_moisture = m_moisture_reduction_fraction_func(destructiveness_without_land_factors);
  const float new_moisture  = orig_moisture - lost_moisture;
  tile.set_soil_moisture(new_moisture);

  // TODO: reduce dewpoint?

  SPELL_REPORT("Soil moisture reduced to " << new_moisture);

  affected_tiles.push_back(&tile);
}

///////////////////////////////////////////////////////////////////////////////
void Tornado::verify_apply() const
///////////////////////////////////////////////////////////////////////////////
{
  // This spell can only be cast on plains and lush tiles (food tiles).

  WorldTile& tile = m_engine.world().get_tile(m_location);
  FoodTile* food_tile = dynamic_cast<FoodTile*>(&tile);
  RequireUser(food_tile != nullptr,
              "Tornado can only be cast on tiles with plant growth");

  verify_no_repeat_cast();
}

///////////////////////////////////////////////////////////////////////////////
void Tornado::apply_to_world(WorldTile& tile,
                             std::vector<WorldTile*>& affected_tiles,
                             std::vector<std::pair<std::string, unsigned>>& triggered) const
///////////////////////////////////////////////////////////////////////////////
{
  World& world = m_engine.world();

  for (Location location : world.valid_nearby_tile_range(tile.location())) {
    WorldTile& affected_tile = world.get_tile(location);
    affected_tiles.push_back(&affected_tile);

    // Some minimal impact on soil moisture, but this tstorm was not
    // a big rain producer
    const float new_moisture = affected_tile.soil_moisture() + DRY_STORM_MOISTURE_ADD;
    SPELL_REPORT("Raised soil moisture for tile " << affected_tile.location() << " from "
                 << affected_tile.soil_moisture() << " to " << new_moisture);
    affected_tile.set_soil_moisture(new_moisture);
  }
}

///////////////////////////////////////////////////////////////////////////////
void Blizzard::verify_apply() const
///////////////////////////////////////////////////////////////////////////////
{
  // Must be cast on a land tile
  WorldTile& tile = m_engine.world().get_tile(m_location);
  LandTile* land_tile = dynamic_cast<LandTile*>(&tile);
  RequireUser(land_tile != nullptr,
              "Blizzard can only be cast on land tiles");

  // Must be cold
  RequireUser(tile.atmosphere().temperature() <= MAX_TEMP,
              "It is not cold enough on this tile for a blizzard, maximum temp for this spell is " << MAX_TEMP);

  verify_no_repeat_cast();
}

///////////////////////////////////////////////////////////////////////////////
void Blizzard::apply_to_world(WorldTile& tile,
                              std::vector<WorldTile*>& affected_tiles,
                              std::vector<std::pair<std::string, unsigned>>& triggered) const
///////////////////////////////////////////////////////////////////////////////
{
  World& world = m_engine.world();

  for (Location location : world.valid_nearby_tile_range(tile.location())) {
    WorldTile& affected_tile = world.get_tile(location);
    affected_tiles.push_back(&affected_tile);

    const float destructiveness = compute_destructiveness(affected_tile, false);
    const unsigned snowfall = m_snowfall_func(destructiveness);
    const unsigned new_snowpack = affected_tile.snowpack() + snowfall;
    SPELL_REPORT("With " << snowfall << " inches of snowfall, snowpack raised to " << new_snowpack);
    affected_tile.set_snowpack(new_snowpack);
  }
}

}
