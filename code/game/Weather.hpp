#ifndef Weather_hpp
#define Weather_hpp

#include "Drawable.hpp"
#include "BaalCommon.hpp"
#include "Time.hpp"

#include <iosfwd>
#include <vector>
#include <libxml/parser.h>

// This file contains the classes having to do with Weather. The
// classes here are just data-holders for the most part. As usual, we define
// multiple classes here to avoid having a large number of header files for
// very small classes.

namespace baal {

class World;
class Anomaly;

enum Direction
{
  N,
  NNE,
  NE,
  ENE,
  E,
  ESE,
  SE,
  SSE,
  S,
  SSW,
  SW,
  WSW,
  W,
  WNW,
  NW,
  NNW
};

struct Wind
{
  Wind(unsigned speed, Direction direction) :
    m_speed(speed), m_direction(direction)
  {}

  Wind operator+(unsigned mph)
  {
    return Wind(m_speed + mph, m_direction);
  }

  unsigned  m_speed; // mph
  Direction m_direction;
};

/**
 * Every tile has a climate. Averate temp, average rainfall, and
 * prevailing wind.
 *
 * TODO: This needs to be different from season to season.
 */
class Climate
{
 public:
  Climate(int temperature, float rainfall, Wind wind)
    : m_temperature(temperature),
      m_rainfall(rainfall),
      m_wind(wind)
  {}

  int temperature(Season season) const { return m_temperature; }

  float rainfall(Season season) const { return m_rainfall / 4; }

  Wind wind(Season season) const { return m_wind; }

  xmlNodePtr to_xml();

 private:
  int   m_temperature; // in farenheit
  float m_rainfall;    // in inches/year
  Wind  m_wind;        // prevailing wind
};

/**
 * Every tile has atmosphere above it. Atmosphere has dewpoint,
 * temperature, wind vector, and pressure.
 */
class Atmosphere : public Drawable
{
 public:
  Atmosphere(const Climate& climate);

  int temperature() const { return m_temperature; }

  int dewpoint() const { return m_dewpoint; }

  float rainfall() const { return m_rainfall; }

  unsigned pressure() const { return m_pressure; }

  Wind wind() const { return m_wind; }

  virtual void draw_text(std::ostream& out) const;

  virtual void draw_graphics() const { /*TODO*/ }

  static bool is_atmospheric(DrawMode mode);

  // Based on season and anomalies, initialize self
  void cycle_turn(const std::vector<const Anomaly*>& anomalies,
                  const Location& location,
                  Season season);

  xmlNodePtr to_xml();

 private:
  int compute_dewpoint() const;

  // Interface for friend spells

  void set_temperature(int new_temp) { m_temperature = new_temp; }

  void set_wind(const Wind& wind) { m_wind = wind; }

  // Friends

  friend class Hot;
  friend class Cold;
  friend class WindSpell;

  // Members

  int            m_temperature; // in farenheit
  int            m_dewpoint;    // in farenheit
  float          m_rainfall;    // in inches, most recent season
  unsigned       m_pressure;    // in millibars
  Wind           m_wind;
  const Climate& m_climate;

  static const unsigned NORMAL_PRESSURE = 1000;
};

/**
 * An Anomaly represents a deviation from normal weather patterns
 * over a certain area and of a certain intensity. Each level of
 * intensity is linearly more intense than the prior level but
 * exponentially less likely. The area affected by the anomaly
 * will depend on the size of the map.
 */
class Anomaly : public Drawable
{
 public:
  enum Type
  {
    ABOVE,
    BELOW
  };

  // Do not change FIRST or LAST values without changing FIRST/LAST
  // constants in Anomaly. Do NOT assign custom values to enums.
  enum AnomalyCategory
  {
    TEMPERATURE, // Coupled with Anomaly::FIRST
    RAINFALL,
    PRESSURE     // Coupled with Anomaly::LAST
  };

  /**
   * Generates an anomaly. Returns NULL if the dice roll did not
   * merit the creation of an anomaly.
   */
  static const Anomaly* generate_anomaly(AnomalyCategory category,
                                         const Location& location,
                                         const World& world);

  /**
   * Return this anomaly's effect on a location as a % of
   * normal value of precip.
   */
  float precip_effect(const Location& location) const;

  /**
   * Return this anomaly's effect on a location's temperature
   * in terms of degrees of deviation from norm.
   */
  int temp_effect(const Location& location) const;

  /**
   * Return this anomaly's effect on a location's pressure in
   * terms of millibars of deviation from norm.
   */
  int pressure_effect(const Location& location) const;

  virtual void draw_text(std::ostream& out) const;

  virtual void draw_graphics() const { /*TODO*/ }

  static std::string type_to_str(Type type);

  static std::string category_to_str(AnomalyCategory category);

  static const unsigned MIN_INTENSITY = 1;
  static const unsigned MAX_INTENSITY = 3;

  static const AnomalyCategory FIRST = TEMPERATURE;
  static const AnomalyCategory LAST  = PRESSURE;

  xmlNodePtr to_xml() const;

  static const float PRECIP_CHANGE_PER_LEVEL = 0.25;
  static const int TEMP_CHANGE_PER_LEVEL = 7;
  static const int PRESSURE_CHANGE_PER_LEVEL = 15;

 private:
  Anomaly(AnomalyCategory category,
          Type type,
          unsigned intensity,
          const Location& location,
          unsigned world_area);

  // Members

  AnomalyCategory m_category;
  Type            m_type;
  unsigned        m_intensity;
  Location        m_location;
  unsigned        m_world_area;
};

Anomaly::AnomalyCategory& operator++(Anomaly::AnomalyCategory& category);

}

#endif
