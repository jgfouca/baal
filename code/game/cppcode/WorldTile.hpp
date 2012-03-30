#ifndef WorldTile_hpp
#define WorldTile_hpp

#include "Drawable.hpp"
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
 * are generally passive containers of data but they do know
 * how to draw themselves.
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
class WorldTile : public Drawable
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

  virtual void draw_text(std::ostream& out) const;

  virtual void draw_graphics() const { /*TODO*/ }

  virtual void cycle_turn(const std::vector<const Anomaly*>& anomalies,
                          const Location& location,
                          Season season);

  void work();

  virtual bool supports_city() const { return false; }

  // Land-related interface

  virtual unsigned infra_level() const { return 0; }

  virtual City* city() const { return nullptr; }

  virtual void damage(float dmg) {}

  // Ocean-related interface

  virtual unsigned depth() const { return 0; }

  // Mountain-related interface

  virtual unsigned elevation() const { return 0; }

  virtual unsigned snowpack() const { return 0; }

  // Getters

  bool worked() const { return m_worked; }

  Location location() const { return m_location; }

  const Atmosphere& atmosphere() const { return m_atmosphere; }

  Atmosphere& atmosphere() { return m_atmosphere; }

  // Public constants

  static const unsigned TILE_TEXT_HEIGHT = 5;
  static const unsigned TILE_TEXT_WIDTH = 5;

  xmlNodePtr to_xml();

 protected:

  // Internal methods relating to drawing

  virtual const char* color() const = 0;

  virtual char symbol() const = 0;

  void draw_land(std::ostream& out) const;

  // Members

  Location   m_location;
  Yield      m_base_yield;
  Climate&   m_climate;
  Geology&   m_geology;
  Atmosphere m_atmosphere;
  bool       m_worked;
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

  virtual const char* color() const { return BLUE; }

  virtual char symbol() const { return '~'; }

  virtual void cycle_turn(const std::vector<const Anomaly*>& anomalies,
                          const Location& location,
                          Season season);

  virtual unsigned depth() const { return m_depth; }

  int surface_temp() const { return m_surface_temp; }

 protected:

  unsigned m_depth;
  int m_surface_temp; // in farenheit

 private:

  // Friend interface

  void set_surface_temp(int new_temp) { m_surface_temp = new_temp; }

  // Friends

  friend class Hot;
  friend class Cold;
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

  virtual void cycle_turn(const std::vector<const Anomaly*>& anomalies,
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

  virtual const char* color() const { return WHITE; }

  virtual char symbol() const { return '^'; }

  virtual void cycle_turn(const std::vector<const Anomaly*>& anomalies,
                          const Location& location,
                          Season season);

  virtual unsigned elevation() const { return m_elevation; }

  virtual unsigned snowpack() const { return m_snowpack; }

  virtual bool supports_city() const { return false; }

 protected:
  virtual void place_city(City& city);

  unsigned m_elevation;
  unsigned m_snowpack; // in inches

 private:

  // Friend interface

  void set_snowpack(unsigned new_snowpack) { m_snowpack = new_snowpack; }

  // Friends

  friend class Hot;
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

  virtual const char* color() const { return YELLOW; }

  virtual char symbol() const { return '-'; }
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

  virtual const char* color() const { return WHITE; }

  virtual char symbol() const { return '-'; }
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

  virtual const char* color() const { return GREEN; }

  virtual char symbol() const { return '^'; }
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

  virtual void cycle_turn(const std::vector<const Anomaly*>& anomalies,
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

  virtual const char* color() const { return GREEN; }

  virtual char symbol() const { return '_'; }
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

  virtual const char* color() const { return GREEN; }

  virtual char symbol() const { return '='; }
};

}

#endif