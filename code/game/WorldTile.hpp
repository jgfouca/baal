#ifndef WorldTile_hpp
#define WorldTile_hpp

#include "Drawable.hpp"
#include "Weather.hpp"
#include "BaalCommon.hpp"
#include "Time.hpp"

#include <vector>
#include <iosfwd>

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
 * tile changed via dynamic_cast? In other cases, the operation may not
 * make sense, but is a harmless query; in that case, a "NULL-like" value
 * is returned.
 *
 * TODO: There is probably a better way to design this... consider not
 * relying so heavily on inheritence.
 */
class WorldTile : public Drawable
{
 public:
  WorldTile(Yield yield, Climate& climate, Geology& geology);

  virtual ~WorldTile();

  virtual Yield yield() const = 0;

  virtual void draw_text(std::ostream& out) const;

  virtual void draw_graphics() const { /*TODO*/ }

  virtual void cycle_turn(const std::vector<const Anomaly*>& anomalies,
                          const Location& location,
                          Season season);

  // Land-related interface

  virtual unsigned infra_level() const { return 0; }

  virtual City* city() const { return NULL; }

  virtual void damage(float dmg) {}

  // Ocean-related interface

  virtual unsigned depth() const { return 0; }

  // Mountain-related interface

  virtual unsigned elevation() const { return 0; }

  virtual unsigned snowpack() const { return 0; }

  // Public constants

  static const unsigned TILE_TEXT_HEIGHT = 5;
  static const unsigned TILE_TEXT_WIDTH = 5;

 protected:

  // Internal methods relating to drawing

  virtual const char* color() const = 0;

  virtual char symbol() const = 0;

  void draw_land(std::ostream& out) const;

  // Members

  Yield        m_base_yield;
  Climate&     m_climate;
  Geology&     m_geology;
  Atmosphere   m_atmosphere;

 private:
  // forbidden methods
  WorldTile();
  WorldTile(const WorldTile&);
  WorldTile& operator=(const WorldTile&);
};

/**
 * Ocean tiles have fixed yields; they also have a
 * depth (as in water-depth) and a water surface temperature
 * that is semi-independent of air temperature.
 */
class OceanTile : public WorldTile
{
 public:
  OceanTile(unsigned depth, Climate& climate, Geology& geology);

  virtual Yield yield() const { return m_base_yield; }

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
};

/**
 * LandTile is the abstract base class of all land tiles
 */
class LandTile: public WorldTile
{
 public:
  LandTile(Yield yield, Climate& climate, Geology& geology);

  ~LandTile();

  virtual Yield yield() const;

  virtual void cycle_turn(const std::vector<const Anomaly*>& anomalies,
                          const Location& location,
                          Season season);

  virtual void damage(float dmg);

  virtual unsigned infra_level() const { return m_infra_level; }

  virtual City* city() const { return m_city; }

  void build_infra();

  void place_city(City& city);

 protected:
  void recover();

  float m_hp; // 0..1
  unsigned m_infra_level; // 0..MAX
  City* m_city; // valid to have no (NULL) city, so use ptr

  static const float LAND_TILE_RECOVERY_RATE = 0.10;
  static const unsigned LAND_TILE_MAX_INFRA  = 5;
};

/**
 *
 */
class MountainTile : public LandTile
{
 public:
  MountainTile(unsigned elevation, Climate& climate, Geology& geology)
    : LandTile(Yield(0, 2), climate, geology),
      m_elevation(elevation)
  {}

  virtual const char* color() const { return WHITE; }

  virtual char symbol() const { return '^'; }

  virtual void cycle_turn(const std::vector<const Anomaly*>& anomalies,
                          const Location& location,
                          Season season);

  virtual unsigned elevation() const { return m_elevation; }

  virtual unsigned snowpack() const { return m_snowpack; }

 protected:
  unsigned m_elevation;
  unsigned m_snowpack; // in inches
};

/**
 *
 */
class DesertTile : public LandTile
{
 public:
  DesertTile(Climate& climate, Geology& geology)
    : LandTile(Yield(0, 0.5), climate, geology)
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
  TundraTile(Climate& climate, Geology& geology)
    : LandTile(Yield(0, 0.5), climate, geology)
  {}

  virtual const char* color() const { return WHITE; }

  virtual char symbol() const { return '-'; }
};

/**
 *
 */
class HillsTile : public LandTile
{
 public:
  HillsTile(Climate& climate, Geology& geology)
    : LandTile(Yield(0, 1), climate, geology)
  {}

  virtual const char* color() const { return GREEN; }

  virtual char symbol() const { return '^'; }
};

/**
 *
 */
class TileWithPlantGrowth : public LandTile
{
 public:
  TileWithPlantGrowth(Yield yield, Climate& climate, Geology& geology)
    : LandTile(yield, climate, geology),
      m_soil_moisture(1.0)
  {}

  virtual Yield yield() const;

  float soil_moisture() const { return m_soil_moisture; }

  virtual void cycle_turn(const std::vector<const Anomaly*>& anomalies,
                          const Location& location,
                          Season season);

  static const float FLOODING_THRESHOLD = 1.5;
  static const float TOTALLY_FLOODED = 2.75;

 protected:
  float m_soil_moisture;
};

/**
 *
 */
class PlainsTile : public TileWithPlantGrowth
{
 public:
  PlainsTile(Climate& climate, Geology& geology)
    : TileWithPlantGrowth(Yield(1, 0), climate, geology)
  {}

  virtual const char* color() const { return GREEN; }

  virtual char symbol() const { return '_'; }
};

/**
 *
 */
class LushTile : public TileWithPlantGrowth
{
 public:
  LushTile(Climate& climate, Geology& geology)
    : TileWithPlantGrowth(Yield(2, 0), climate, geology)
  {}

  virtual const char* color() const { return GREEN; }

  virtual char symbol() const { return '='; }
};

}

#endif
