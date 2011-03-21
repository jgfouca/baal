#ifndef Weather_hpp
#define Weather_hpp

#include "Drawable.hpp"
#include "BaalCommon.hpp"

#include <iosfwd>
#include <vector>

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
  Climate(int temperature, unsigned rainfall, Wind wind)
    : m_temperature(temperature),
      m_rainfall(rainfall),
      m_wind(wind)
  {}

  int temperature() const { return m_temperature; }

  unsigned rainfall() const { return m_rainfall; }

  Wind wind() const { return m_wind; }

 private:
  int      m_temperature; // in farenheit
  unsigned m_rainfall;    // in inches/year
  Wind     m_wind;        // prevailing wind
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

  unsigned pressure() const { return m_pressure; }

  Wind wind() const { return m_wind; }

  virtual void draw_text(std::ostream& out) const;

  virtual void draw_graphics() const { /*TODO*/ }

  static bool is_atmospheric(DrawMode mode);

  // TODO: Based on season and anomalies, initialize self
  void cycle_turn(const std::vector<const Anomaly*>& anomalies) {}

 private:
  int compute_dewpoint() const;

  int      m_temperature; // in farenheit
  int      m_dewpoint;    // in farenheit
  unsigned m_pressure;    // in millibars
  Wind     m_wind;

  static const unsigned NORMAL_PRESSURE = 1000;
};

/**
 * An Anomaly represents a deviation from normal weather patterns
 * over a certain area and of a certain intensity. Each level of
 * intensity is linearly more intense than the prior level but
 * exponentially less likely. The area affected by the anomaly
 * will depend on the size of the map.
 */
class Anomaly
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
  float precip_effect(const Location& location) const
  {
    //TODO
    return 1.0;
  }

  /**
   * Return this anomaly's effect on a location's temperature
   * in terms of degrees of deviation from norm.
   */
  int temp_effect(const Location& location) const
  {
    // TODO
    return 0;
  }

  /**
   * Return this anomaly's effect on a location's pressure in
   * terms of millibars of deviation from norm.
   */
  int pressure_effect(const Location& location) const
  {
    // TODO
    return 0;
  }

  static const unsigned MIN_INTENSITY = 1;
  static const unsigned MAX_INTENSITY = 3;

  static const AnomalyCategory FIRST = TEMPERATURE;
  static const AnomalyCategory LAST  = PRESSURE;

 private:
  Anomaly(AnomalyCategory category,
          Type type,
          unsigned intensity,
          const Location& location,
          unsigned world_area);


  AnomalyCategory m_category;
  Type            m_type;
  unsigned        m_intensity;
  const Location& m_location;
  unsigned        m_world_area;
};

Anomaly::AnomalyCategory& operator++(Anomaly::AnomalyCategory& category);

}

#endif
