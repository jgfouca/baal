#ifndef WorldTile_hpp
#define WorldTile_hpp

#include "Weather.hpp"
#include "BaalCommon.hpp"
#include "Time.hpp"

#include <vector>
#include <iosfwd>
#include <libxml/parser.h>

// We put all WorldTile classes in this header to avoid
// generating a ton of header files.

namespace baal {

class City;
class Geology;
class Anomaly;

/**
 * A simple structure that specifies yields for tiles.
 */
struct Yield
{
  Yield(float food, float prod);

  Yield operator*(float multiplier) const
  {
    return Yield(m_food * multiplier, m_prod * multiplier);
  }

  float m_food;
  float m_prod;

  xmlNodePtr to_xml();

 private:
  Yield();
};

/**
 * WorldTile is the abstract base class of all tiles. Tile classes
 * are generally passive containers of data.
 *
 * Every tile has an atmosphere, climate, geology, and yield.
 *
 * This class also serves as an aggregate of all the interfaces of the
 * Tile subclasses. Anything you can possibly do to any WorldTile must
 * be associated with some method in the WorldTile class. In some cases,
 * operations on certain tiles are invalid, such as asking for the
 * sea surface temperature of a land tile. In that case, the type of the
 * tile checked via dynamic_cast? In other cases, the operation may not
 * make sense, but is a harmless query; in that case, a "nullptr-like" value
 * is returned.
 *
 * TODO: There is probably a better way to design this... consider not
 * relying so heavily on inheritence.
 */
class WorldTile
{
 public:
  WorldTile(Location location, Yield yield, Climate& climate, Geology& geology);

  virtual ~WorldTile();

  // forbidden methods
  WorldTile() = delete;
  WorldTile(const WorldTile&) = delete;
  WorldTile& operator=(const WorldTile&) = delete;

  // Basic tile interface

  virtual Yield yield() const { return m_base_yield; }

  virtual void cycle_turn(const std::vector<std::shared_ptr<const Anomaly>>& anomalies,
                          const Location& location,
                          Season season);

  void work();

  virtual bool supports_city() const { return false; }

  void cast(const std::string& spell);

  // Land-related interface

  virtual float soil_moisture() const {
    Require(false, "Should never be called");
    return 0.0;
  }

  virtual unsigned elevation() const {
    Require(false, "Should never be called");
    return 0;
  }

  virtual void set_soil_moisture(float moisture) {
    Require(false, "Should never be called");
  }

  virtual unsigned infra_level() const { return 0; }

  virtual City* city() const { return nullptr; }

  virtual void damage(float dmg) {
    Require(false, "Should never be called");
  }

  // Ocean-related interface

  virtual unsigned depth() const {
    Require(false, "Should never be called");
    return 0;
  }

  // Mountain-related interface

  virtual unsigned snowpack() const {
    Require(false, "Should never be called");
    return 0;
  }

  virtual void set_snowpack(unsigned snowpack) {
    Require(false, "Should never be called");
  }

  // Getters

  bool already_casted(const std::string& spell) const;

  bool worked() const { return m_worked; }

  Location location() const { return m_location; }

  const Atmosphere& atmosphere() const { return m_atmosphere; }

  const Geology& geology() const { return m_geology; }

  const Climate& climate() const { return m_climate; }

  Atmosphere& atmosphere() { return m_atmosphere; }

  xmlNodePtr to_xml();

 protected:

  // Members

  Location   m_location;
  Yield      m_base_yield;
  Climate&   m_climate;
  Geology&   m_geology;
  Atmosphere m_atmosphere;
  bool       m_worked;
  vecstr_t   m_casted_spells;
};

/**
 * Ocean tiles have fixed yields; they also have a
 * depth (as in water-depth) and a water surface temperature
 * that is semi-independent of air temperature.
 */
class OceanTile : public WorldTile
{
 public:
  OceanTile(Location location, unsigned depth, Climate& climate, Geology& geology);

  virtual void cycle_turn(const std::vector<std::shared_ptr<const Anomaly>>& anomalies,
                          const Location& location,
                          Season season);

  virtual unsigned depth() const { return m_depth; }

  int surface_temp() const { return m_surface_temp; }

  void set_surface_temp(int new_temp) { m_surface_temp = new_temp; }

 protected:

  unsigned m_depth;
  int m_surface_temp; // in farenheit

  // Just average, putting higher weight on ocean temp
  static int new_surface_temp_func(int last_season_water_temp, int new_season_air_temp)
  { return (last_season_water_temp * 4 + new_season_air_temp) / 5; }

 private:

  static constexpr float OCEAN_FOOD = 3.0;
  static constexpr float OCEAN_PROD = 0.0;
};

/**
 * LandTile is the abstract base class of all land tiles
 */
class LandTile: public WorldTile
{
 public:
  LandTile(Location location, unsigned elevation, Yield yield, Climate& climate, Geology& geology);

  ~LandTile();

  virtual Yield yield() const;

  virtual void cycle_turn(const std::vector<std::shared_ptr<const Anomaly>>& anomalies,
                          const Location& location,
                          Season season);

  virtual void damage(float dmg);

  virtual unsigned infra_level() const { return m_infra_level; }

  virtual City* city() const { return m_city; }

  virtual bool supports_city() const { return true; }

  virtual unsigned elevation() const { return m_elevation; }

  virtual unsigned snowpack() const { return m_snowpack; }

  virtual void set_snowpack(unsigned snowpack) { m_snowpack = snowpack; }

  void build_infra();

  static constexpr unsigned LAND_TILE_MAX_INFRA  = 5;
  static constexpr float LAND_TILE_RECOVERY_RATE = 0.10;

 protected:

  // Internal methods

  static float land_tile_recovery_func(float prior)
  {
    // 10% per turn
    float new_dmg = prior + LAND_TILE_RECOVERY_RATE;
    return new_dmg > 1.0 ? 1.0 : new_dmg;
  }

  static Yield compute_yield_func(Yield const& base_yield, unsigned infra_level, float tile_hp)
  {
    // TODO: Should high snowpack effect production yields?
    return base_yield * (1 + infra_level) * tile_hp;
  }

  static float portion_of_precip_that_falls_as_snow_func(int season_avg_temp)
  {
    if (season_avg_temp < 30) {
      return 1.0;
    }
    else if (season_avg_temp < 60) {
      return float(60 - season_avg_temp) / 30;
    }
    else {
      return 0.0;
    }
  }

  static float portion_of_snowpack_that_melted(int season_avg_temp)
  {
    if (season_avg_temp < 15) {
      return 0.0;
    }
    else if (season_avg_temp < 75) {
      return float(season_avg_temp - 15) / 60;
    }
    else {
      return 1.0;
    }
  }

  // Members

  float m_hp; // 0..1
  unsigned m_infra_level; // 0..MAX
  unsigned m_elevation;
  unsigned m_snowpack; // in inches
  City* m_city; // valid to have no (nullptr) city, so use ptr

  // Friends interface

  virtual void place_city(City& city);

  void remove_city();

  void destroy_infra(unsigned num_destroyed);

  // Friends

  friend class World;
  friend class Spell;
};

/**
 * Represents mountain tiles. Mountains don't add any new concepts. Cities
 * can't be built on mountains.
 */
class MountainTile : public LandTile
{
 public:
  MountainTile(Location location, unsigned elevation, Climate& climate, Geology& geology)
    : LandTile(location, elevation, Yield(MOUNTAIN_FOOD, MOUNTAIN_PROD), climate, geology)
  {}

  virtual bool supports_city() const { return false; }

 protected:
  virtual void place_city(City& city);

 private:
  static constexpr float MOUNTAIN_FOOD = 0.0;
  static constexpr float MOUNTAIN_PROD = 2.0;
};


/**
 * Represents land tiles that have soil and retain
 * moisture. Introduces the concept of soil moisture.

 * This is an abstract tile type and should not be created directly.
 */
class TileWithSoil : public LandTile
{
 public:
  TileWithSoil(Location location, unsigned elevation, Yield yield, Climate& climate, Geology& geology)
    : LandTile(location, elevation, yield, climate, geology),
      m_soil_moisture(1.0)
  {}

  virtual float soil_moisture() const { return m_soil_moisture; }

  virtual void set_soil_moisture(float moisture) { m_soil_moisture = moisture; }

  virtual void cycle_turn(const std::vector<std::shared_ptr<const Anomaly>>& anomalies,
                          const Location& location,
                          Season season);

 private:
  float m_soil_moisture;

  static float precip_effect_on_moisture_func(float avg_precip, float precip)
  { return precip / avg_precip; }

  static float temp_effect_on_moisture_func(int avg_temp, int temp)
  { return 1.0 + ( 0.01 * (avg_temp - temp) ); }

  static float compute_moisture_func(float prior, float current_forcing)
  { return ((current_forcing * 2) + prior) / 3; }
};

/**
 * Represents desert tiles. Deserts add no concepts, so this class is simple.
 */
class DesertTile : public TileWithSoil
{
 public:
  DesertTile(Location location, unsigned elevation, Climate& climate, Geology& geology)
    : TileWithSoil(location, elevation, Yield(DESERT_FOOD, DESERT_PROD), climate, geology)
  {}

 private:
  static constexpr float DESERT_FOOD = 0.0;
  static constexpr float DESERT_PROD = 0.5;
};

/**
 * Represents tundra tiles. Tundra add no concepts, so this class is simple.
 */
class TundraTile : public TileWithSoil
{
 public:
  TundraTile(Location location, unsigned elevation, Climate& climate, Geology& geology)
    : TileWithSoil(location, elevation, Yield(TUNDRA_FOOD, TUNDRA_PROD), climate, geology)
  {}

 private:
  static constexpr float TUNDRA_FOOD = 0.0;
  static constexpr float TUNDRA_PROD = 0.5;
};

/**
 * Represents hill tiles. Hills add no concepts, so this class is simple.
 */
class HillsTile : public TileWithSoil
{
 public:
  HillsTile(Location location, unsigned elevation, Climate& climate, Geology& geology)
    : TileWithSoil(location, elevation, Yield(HILLS_FOOD, HILLS_PROD), climate, geology)
  {}

private:
  static constexpr float HILLS_FOOD = 0.0;
  static constexpr float HILLS_PROD = 1.0;
};

/**
 * Represents land tiles that yield food. This is an abstract tile type.
 */
class FoodTile : public TileWithSoil
{
 public:
  FoodTile(Location location, unsigned elevation, Yield yield, Climate& climate, Geology& geology)
    : TileWithSoil(location, elevation, yield, climate, geology),
      m_soil_moisture(1.0)
  {}

  virtual Yield yield() const;

  float soil_moisture() const { return m_soil_moisture; }

  void set_soil_moisture(float moisture) { m_soil_moisture = moisture; }

  static constexpr float FLOODING_THRESHOLD = 1.5;
  static constexpr float TOTALLY_FLOODED    = 2.75;

 protected:
  float m_soil_moisture;

 private:

  static float moisture_yield_effect_func(float moisture)
  {
    if (moisture < FLOODING_THRESHOLD) {
      // Up to the flooding threshold, yields improve as moisture increases
      return moisture;
    }
    else if (moisture < TOTALLY_FLOODED) {
      // Yields drop quickly as soil becomes over-saturated
      return FLOODING_THRESHOLD - (moisture - FLOODING_THRESHOLD);
    }
    else {
      // Things are flooded and can't get any worse. Farmers are able to
      // salvage some fixed portion of their crops.
      return 0.25;
    }
  }

  static float snowpack_yield_effect_func(unsigned snowpack)
  {
    if (snowpack > 100) {
      return 0.0;
    }
    else {
      return float(100 - snowpack) / 100.0;
    }
  }
};

/**
 * Represents plains tiles. Plains add no concepts, so this class is simple.
 */
class PlainsTile : public FoodTile
{
 public:
  PlainsTile(Location location, unsigned elevation, Climate& climate, Geology& geology)
    : FoodTile(location, elevation, Yield(PLAINS_FOOD, PLAINS_PROD), climate, geology)
  {}

private:
  static constexpr float PLAINS_FOOD = 1.0;
  static constexpr float PLAINS_PROD = 0.0;
};

/**
 * Represents lush tiles. Lush add no concepts, so this class is simple. Lush
 * tiles are like plains tiles except they have higher food yields.
 */
class LushTile : public FoodTile
{
 public:
  LushTile(Location location, unsigned elevation, Climate& climate, Geology& geology)
    : FoodTile(location, elevation, Yield(LUSH_FOOD, LUSH_PROD), climate, geology)
  {}

private:
  static constexpr float LUSH_FOOD = 2.0;
  static constexpr float LUSH_PROD = 0.0;
};

}

#endif
