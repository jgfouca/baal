#ifndef Weather_hpp
#define Weather_hpp

#include "Drawable.hpp"

#include <iosfwd>

// This file contains the classes having to do with Weather. The
// classes here are just data-holders for the most part. As usual, we define
// multiple classes here to avoid having a large number of header files for
// very small classes.

namespace baal {

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

  unsigned m_speed; // mph
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
  unsigned m_rainfall;  // in inches/year
  Wind     m_wind; // prevailing wind
};

/**
 * Every tile has atmosphere above it. Atmosphere has dewpoint,
 * temperature, wind vector, and pressure.
 */
class Atmosphere : public Drawable
{
 public:
  Atmosphere(const Climate& climate)
    : m_temperature(climate.temperature()),
      m_pressure(NORMAL_PRESSURE),
      m_wind(climate.wind())
  {}

  int temperature() const { return m_temperature; }

  // TODO: dewpoint modelling is significantly more complex, save
  // this for later
  //int dewpoint() const { return m_dewpoint; }

  unsigned pressure() const { return m_pressure; }

  Wind wind() const { return m_wind; }

  virtual void draw_text(std::ostream& out) const;

  virtual void draw_graphics() const { /*TODO*/ }

 private:
  int m_temperature; // in farenheit
  //int m_dewpoint; // in farenheit
  unsigned m_pressure; // in millibars
  Wind m_wind;

  static const unsigned NORMAL_PRESSURE = 1000;
};

}

#endif
