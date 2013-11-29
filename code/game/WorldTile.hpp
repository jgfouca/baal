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

  virtual Yield yield() const = 0;

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

  virtual unsigned elevation() const {
    Require(false, "Should never be called");
    return 0;
  }

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

  virtual Yield yield() const;

  virtual void cycle_turn(const std::vector<std::shared_ptr<const Anomaly>>& anomalies,
                          const Location& location,
                          Season season);

  virtual unsigned depth() const { return m_depth; }

  int surface_temp() const { return m_surface_temp; }

  void set_surface_temp(int new_temp) { m_surface_temp = new_temp; }

 protected:

  unsigned m_depth;
  int m_surface_temp; // in farenheit
};

/**
 * LandTile is the abstract base class of all land tiles
 */
class LandTile: public WorldTile
{
 public:
  LandTile(Location location, Yield yield, Climate& climate, Geology& geology);

  ~LandTile();

  virtual Yield yield() const;

  virtual void cycle_turn(const std::vector<std::shared_ptr<const Anomaly>>& anomalies,
                          const Location& location,
                          Season season);

  virtual void damage(float dmg);

  virtual unsigned infra_level() const { return m_infra_level; }

  virtual City* city() const { return m_city; }

  virtual bool supports_city() const { return true; }

  void build_infra();

  static constexpr unsigned LAND_TILE_MAX_INFRA  = 5;
  static constexpr float LAND_TILE_RECOVERY_RATE = 0.10;

 protected:

  // Internal methods

  void recover();

  // Members

  float m_hp; // 0..1
  unsigned m_infra_level; // 0..MAX
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
 *
 */
class MountainTile : public LandTile
{
 public:
  MountainTile(Location location, unsigned elevation, Climate& climate, Geology& geology)
    : LandTile(location, Yield(0, 2), climate, geology),
      m_elevation(elevation)
  {}

  virtual void cycle_turn(const std::vector<std::shared_ptr<const Anomaly>>& anomalies,
                          const Location& location,
                          Season season);

  virtual unsigned elevation() const { return m_elevation; }

  virtual unsigned snowpack() const { return m_snowpack; }

  virtual void set_snowpack(unsigned snowpack) { m_snowpack = snowpack; }

  virtual bool supports_city() const { return false; }

 protected:
  virtual void place_city(City& city);

  unsigned m_elevation;
  unsigned m_snowpack; // in inches
};

/**
 *
 */
class DesertTile : public LandTile
{
 public:
  DesertTile(Location location, Climate& climate, Geology& geology)
    : LandTile(location, Yield(0, 0.5), climate, geology)
  {}
};

/**
 *
 */
class TundraTile : public LandTile
{
 public:
  TundraTile(Location location, Climate& climate, Geology& geology)
    : LandTile(location, Yield(0, 0.5), climate, geology)
  {}
};

/**
 * TODO: Hill-tile lack of moisture make it awkward to handle
 * for many disasters. Resolve this issue.
 */
class HillsTile : public LandTile
{
 public:
  HillsTile(Location location, Climate& climate, Geology& geology)
    : LandTile(location, Yield(0, 1), climate, geology)
  {}
};

/**
 *
 */
class FoodTile : public LandTile
{
 public:
  FoodTile(Location location, Yield yield, Climate& climate, Geology& geology)
    : LandTile(location, yield, climate, geology),
      m_soil_moisture(1.0)
  {}

  virtual Yield yield() const;

  float soil_moisture() const { return m_soil_moisture; }

  void set_soil_moisture(float moisture) { m_soil_moisture = moisture; }

  virtual void cycle_turn(const std::vector<std::shared_ptr<const Anomaly>>& anomalies,
                          const Location& location,
                          Season season);

  static constexpr float FLOODING_THRESHOLD = 1.5;
  static constexpr float TOTALLY_FLOODED = 2.75;

 protected:
  float m_soil_moisture;
};

/**
 *
 */
class PlainsTile : public FoodTile
{
 public:
  PlainsTile(Location location, Climate& climate, Geology& geology)
    : FoodTile(location, Yield(1, 0), climate, geology)
  {}
};

/**
 *
 */
class LushTile : public FoodTile
{
 public:
  LushTile(Location location, Climate& climate, Geology& geology)
    : FoodTile(location, Yield(2, 0), climate, geology)
  {}
};

}

#endif
