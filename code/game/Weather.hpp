#ifndef Weather_hpp
#define Weather_hpp

#include "DrawMode.hpp"
#include "BaalCommon.hpp"
#include "Time.hpp"

#include <iosfwd>
#include <vector>
#include <libxml/parser.h>

// This file contains the classes having to do with Weather. The
// classes here are just data-holders for the most part. As usual, we define
// multiple classes here to avoid having a large number of header files for
// very small classes.

SMART_ENUM(Direction,
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
           NNW);

SMART_ENUM(AnomalyCategory,
           TEMPERATURE_ANOMALY,
           PRECIP_ANOMALY,
           PRESSURE_ANOMALY);

namespace baal {

class World;
class Anomaly;

struct Wind
{
  Wind() :
    m_speed(-1u), m_direction(N) {}

  Wind(unsigned speed, Direction direction) :
    m_speed(speed), m_direction(direction)
  {}

  Wind operator+(unsigned mph) const
  {
    return Wind(m_speed + mph, m_direction);
  }

  Wind operator+(Wind const& rhs) const
  {
    // TODO
    Require(false, "not implemented");
    return rhs;
  }

  template <typename T>
  Wind& operator+=(T const& rhs)
  {
    Wind temp = *this + rhs;
    *this = temp;
    return *this;
  }

  bool operator==(Wind const& rhs) const
  {
    return m_speed == rhs.m_speed && m_direction == rhs.m_direction;
  }

  friend std::ostream& operator<<(std::ostream& out, Wind const& wind);

  friend std::istream& operator>>(std::istream& in, Wind& wind);

  unsigned  m_speed; // mph
  Direction m_direction;
};

/**
 * Every tile has a climate. Averate temp, average precip, and
 * prevailing wind.
 *
 * This is different from season to season.
 */
class Climate
{
 public:
  Climate(std::vector<int> const& temperature,
          std::vector<float> const& precip,
          std::vector<Wind> const& wind)
    : m_temperature(temperature),
      m_precip(precip),
      m_wind(wind)
  {
    const size_t num_seasons = baal::size<Season>();

    Require(m_temperature.size() == num_seasons, "Wrong number of temperatures " << m_temperature.size());
    Require(m_precip.size()      == num_seasons, "Wrong number of precip "       << m_precip.size());
    Require(m_wind.size()        == num_seasons, "Wrong number of wind "         << m_wind.size());
  }

  ~Climate() = default;

  Climate(const Climate&) = delete;
  Climate& operator=(const Climate&) = delete;

  int temperature(Season season) const { return m_temperature[season]; }

  float precip(Season season) const { return m_precip[season]; }

  Wind wind(Season season) const { return m_wind[season]; }

  xmlNodePtr to_xml();

 private:
  std::vector<int>   m_temperature; // in farenheit
  std::vector<float> m_precip;      // in inches/season
  std::vector<Wind>  m_wind;        // prevailing wind
};

/**
 * Every tile has atmosphere above it. Atmosphere has dewpoint,
 * temperature, wind vector, and pressure.
 */
class Atmosphere
{
 public:
  Atmosphere(const Climate& climate);

  ~Atmosphere() = default;

  Atmosphere(const Atmosphere&) = delete;
  Atmosphere& operator=(const Atmosphere&) = delete;

  int temperature() const { return m_temperature; }

  int dewpoint() const { return m_dewpoint; }

  float precip() const { return m_precip; }

  unsigned pressure() const { return m_pressure; }

  Wind wind() const { return m_wind; }

  static bool is_atmospheric(DrawMode mode);

  // Based on season and anomalies, initialize self
  void cycle_turn(const std::vector<std::shared_ptr<const Anomaly>>& anomalies,
                  const Location& location,
                  Season season);

  xmlNodePtr to_xml();

  static const unsigned NORMAL_PRESSURE = 1000;

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
  float          m_precip;    // in inches, most recent season
  unsigned       m_pressure;    // in millibars
  Wind           m_wind;
  const Climate& m_climate;
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

  /**
   * Generates an anomaly. Returns nullptr if the dice roll did not
   * merit the creation of an anomaly.
   */
  static std::shared_ptr<const Anomaly> generate_anomaly(AnomalyCategory category,
                                                         const Location& location,
                                                         const World& world);

  ~Anomaly() = default;

  Anomaly(const Anomaly&) = delete;
  Anomaly& operator=(const Anomaly&) = delete;

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

  xmlNodePtr to_xml() const;

  // Getters

  AnomalyCategory category() const { return m_category; }

  int intensity() const { return m_intensity; }

  Location location() const { return m_location; }

  static const unsigned MAX_INTENSITY = 3; // anomalies are on a scale from +/- 1 -> MAX_INTENSITY

 private:
  // Members
  Anomaly(AnomalyCategory category,
          int intensity,
          const Location& location,
          unsigned world_area);

  AnomalyCategory m_category;
  int             m_intensity;
  Location        m_location;
  unsigned        m_world_area;

  static float PRECIP_CHANGE_FUNC(int intensity)
  {
    // Returns a multiplier on average precip
    // (max - 1 / max)^(-intensity)
    return std::pow(float(MAX_INTENSITY - 1) / MAX_INTENSITY, -intensity);
  }

  static int TEMPERATURE_CHANGE_FUNC(int intensity)
  { return 7 * intensity; }

  static int PRESSURE_CHANGE_FUNC(int intensity)
  { return 15 * intensity; }

  static int GENERATE_ANOMALY_INTENSITY_FUNC()
  {
    // Generate random float 0.0 -> 100.0
    float roll = (float(std::rand()) /
                  float(RAND_MAX)) * 100.0;

    const float negative_anom = MAX_INTENSITY / 100.0;
    const float positive_anom = (100 - MAX_INTENSITY) / 100.0;
    int intensity = 0;
    int modifier  = 0;

    // Normalize so that negative/positive rolls look the same, the only
    // difference is the modifier
    if (roll > positive_anom) {
      roll -= positive_anom;
      modifier = 1;
    }
    else {
      modifier = -1;
    }

    // Compute intensity of anomaly. Each level of anomaly is a factor
    // of two more unlikely than the previous level
    while (roll < negative_anom && std::abs(intensity) < MAX_INTENSITY) {
      intensity += modifier;
      roll *= 2;
    }

    Require(std::abs(intensity) <= MAX_INTENSITY, "Bad value: " << intensity);

    return intensity;
  }
};

}

#endif
