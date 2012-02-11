#ifndef Geology_hpp
#define Geology_hpp

#include "Drawable.hpp"
#include "BaalCommon.hpp"

#include <string>
#include <iosfwd>
#include <libxml/parser.h>

// This file contains the classes having to do with Geology. The
// classes here are just data-holders for the most part. As usual, we define
// multiple classes here to avoid having a large number of header files for
// very small classes.

namespace baal {

/**
 * Contains geology (plate-tectonic) data.
 *
 * Plate boundaries always fall between tiles. Boundaries can be:
 * Divergent
 * Convergent(ocean/land)
 * Convergent(ocean/ocean)
 * Convergent(land/land)
 * Transform
 *
 * Every tile builds up plate tension and magma based on geology.
 */
class Geology : public Drawable
{
 public:
  Geology(float base_tension_buildup,
          float base_magma_buildup,
          float plate_movement); // cm/year

  void cycle_turn();

  float tension() const { return m_tension; }

  float magma() const { return m_magma; }

  virtual void draw_text(std::ostream& out) const;

  virtual void draw_graphics() const { /*TODO*/ }

  static bool is_geological(DrawMode mode);

  xmlNodePtr to_xml();

 protected:

  virtual const char* color() const = 0;

  virtual std::string symbol() const = 0;

  virtual const char* geology_type() const = 0;

  float m_tension;
  float m_magma;
  float m_plate_movement;
  float m_tension_buildup;
  float m_magma_buildup;
};

/**
 *
 */
class Divergent : public Geology
{
 public:
  Divergent(float plate_movement)
    : Geology(DIVERGENT_TENSION_BUILDUP,
              DIVERGENT_MAGMA_BUILDUP,
              plate_movement)
  {}

 protected:
  virtual const char* color() const { return BLUE; }

  virtual std::string symbol() const { return " <-->"; }

  virtual const char* geology_type() const { return "Divergent"; }

 private:
  static constexpr float DIVERGENT_MAGMA_BUILDUP   = 0.001;
  static constexpr float DIVERGENT_TENSION_BUILDUP = 0.000;
};

/**
 * There is no difference between convergent land-ocean and convergent
 * ocean-ocean. Both are subducting.
 */
class Subducting : public Geology
{
 public:
  Subducting(float plate_movement)
    : Geology(SUBDUCTING_TENSION_BUILDUP,
              SUBDUCTING_MAGMA_BUILDUP,
              plate_movement)
  {}

 protected:
  virtual const char* color() const { return RED; }

  virtual std::string symbol() const { return " -v<-"; }

  virtual const char* geology_type() const { return "Subducting"; }

 private:
  static constexpr float SUBDUCTING_MAGMA_BUILDUP   = 0.002;
  static constexpr float SUBDUCTING_TENSION_BUILDUP = 0.002;
};

/**
 * Coverging land-land => Orogenic boundary
 */
class Orogenic : public Geology
{
 public:
  Orogenic(float plate_movement)
    : Geology(OROGENIC_TENSION_BUILDUP,
              OROGENIC_MAGMA_BUILDUP,
              plate_movement)
  {}

 protected:
  virtual const char* color() const { return GREEN; }

  virtual std::string symbol() const { return " -><-"; }

  virtual const char* geology_type() const { return "Orogenic"; }

 private:
  static constexpr float OROGENIC_MAGMA_BUILDUP   = 0.002;
  static constexpr float OROGENIC_TENSION_BUILDUP = 0.002;
};

/**
 * Transform - Two plates sliding past each other.
 */
class Transform : public Geology
{
 public:
  Transform(float plate_movement)
    : Geology(TRANSFORM_TENSION_BUILDUP,
              TRANSFORM_MAGMA_BUILDUP,
              plate_movement)
  {}

 protected:
  virtual const char* color() const { return YELLOW; }

  virtual std::string symbol() const { return " vv^^"; }

  virtual const char* geology_type() const { return "Transform"; }

 private:
  static constexpr float TRANSFORM_MAGMA_BUILDUP   = 0.000;
  static constexpr float TRANSFORM_TENSION_BUILDUP = 0.003;
};

/**
 * Inactive - No nearby plate boundary
 */
class Inactive : public Geology
{
 public:
  Inactive()
    : Geology(INACTIVE_TENSION_BUILDUP,
              INACTIVE_MAGMA_BUILDUP,
              0.0)
  {}

 protected:
  virtual const char* color() const { return WHITE; }

  virtual std::string symbol() const { return "     "; }

  virtual const char* geology_type() const { return "Inactive"; }

 private:
  static constexpr float INACTIVE_MAGMA_BUILDUP   = 0.000;
  static constexpr float INACTIVE_TENSION_BUILDUP = 0.000;
};

}

#endif
