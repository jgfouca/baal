#ifndef WorldTile_hpp
#define WorldTile_hpp

#include "Drawable.hpp"
#include "Weather.hpp"

#include <vector>
#include <iosfwd>

#ifndef NO_GRAPHICS
#include <SDL/sge.h>
#endif

// We put all WorldTile classes in this header to avoid
// generating a ton of header files.

namespace baal {

class City;
class Geology;

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
 * TODO: Every tile has an atmosphere, climate, geology, and yield.
 */
class WorldTile : public Drawable
{
 public:
  WorldTile(Yield yield, Climate& climate, Geology& geology);

  ~WorldTile();

  virtual Yield yield() const = 0;

  virtual void draw_text(std::ostream& out) const;

  virtual void draw_graphics() const { /*TODO*/ }

#ifndef NO_GRAPHICS
  void setSprite(SGESPRITE *sprite) { m_sprite = sprite; }

  SGESPRITE * sprite() const { return m_sprite; }
#endif

 protected:

  virtual int color() const = 0;

  virtual char symbol() const = 0;

  Yield        m_base_yield;
  Climate&     m_climate;
  Geology&     m_geology;
  Atmosphere   m_atmosphere;

#ifndef NO_GRAPHICS
  SGESPRITE* m_sprite;
#endif

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
  OceanTile(unsigned depth, Climate& climate, Geology& geology)
    : WorldTile(Yield(3,0), climate, geology),
      m_depth(depth)
  {}

  virtual Yield yield() const { return m_base_yield; }

  virtual int color() const { return 34; } // blue

  virtual char symbol() const { return '~'; }

 protected:
  unsigned m_depth;
  // TODO: Surface temp
};

/**
 * LandTile is the abstract base class of all land tiles
 */
class LandTile: public WorldTile
{
 public:
  LandTile(Yield yield, Climate& climate, Geology& geology);

  void damage(float dmg);

  void recover();

  void build_infra();

  unsigned get_infra() const { return m_infra_level; }

  virtual Yield yield() const;

  City* city() const { return m_city; }

  void place_city(City* city);

  // By default, tiles have no moisture
  virtual float soil_moisture() const { return 0.0; }

 protected:
  float m_hp; // 0..1
  unsigned m_infra_level; // 0..MAX
  City* m_city; // valid to have no (NULL) city, so use ptr

  static const float LAND_TILE_RECOVERY_RATE = 0.30;
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

  virtual int color() const { return 37; } // white

  virtual char symbol() const { return '^'; }

 protected:
  unsigned m_elevation;
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

  virtual int color() const { return 33; } // yellow

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

  virtual int color() const { return 37; } // white

  virtual char symbol() const { return '-'; }
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

  virtual float soil_moisture() const { return m_soil_moisture; }

 protected:
  float m_soil_moisture;
};

/**
 *
 */
class PlainsTile : public LandTile
{
 public:
  PlainsTile(Climate& climate, Geology& geology)
    : LandTile(Yield(1, 0), climate, geology)
  {}

  virtual int color() const { return 32; } // green

  virtual char symbol() const { return '_'; }
};

/**
 *
 */
class LushTile : public LandTile
{
 public:
  LushTile(Climate& climate, Geology& geology)
    : LandTile(Yield(2, 0), climate, geology)
  {}

  virtual int color() const { return 32; } // green

  virtual char symbol() const { return '='; }
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

  virtual int color() const { return 32; } // green

  virtual char symbol() const { return '^'; }
};

}

#endif
