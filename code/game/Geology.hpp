#ifndef Geology_hpp
#define Geology_hpp

#include "BaalCommon.hpp"
#include "DrawMode.hpp"

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
class Geology
{
 public:
  Geology(float base_tension_buildup,
          float base_magma_buildup,
          float plate_movement); // cm/year

  virtual ~Geology() {}

  void cycle_turn();

  float tension() const { return m_tension; }

  float magma() const { return m_magma; }

  float plate_movement() const { return m_plate_movement; }

  float tension_buildup() const { return m_tension_buildup; }

  float magma_buildup() const { return m_magma_buildup; }

  static bool is_geological(DrawMode mode);

  xmlNodePtr to_xml();

 protected:
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
    : Geology(TENSION_BUILDUP,
              MAGMA_BUILDUP,
              plate_movement)
  {}

  static constexpr float MAGMA_BUILDUP   = 0.001;
  static constexpr float TENSION_BUILDUP = 0.000;

 protected:
  virtual const char* geology_type() const { return "Divergent"; }
};

/**
 * There is no difference between convergent land-ocean and convergent
 * ocean-ocean. Both are subducting.
 */
class Subducting : public Geology
{
 public:
  Subducting(float plate_movement)
    : Geology(TENSION_BUILDUP,
              MAGMA_BUILDUP,
              plate_movement)
  {}

  static constexpr float MAGMA_BUILDUP   = 0.002;
  static constexpr float TENSION_BUILDUP = 0.002;

 protected:
  virtual const char* geology_type() const { return "Subducting"; }
};

/**
 * Coverging land-land => Orogenic boundary
 */
class Orogenic : public Geology
{
 public:
  Orogenic(float plate_movement)
    : Geology(TENSION_BUILDUP,
              MAGMA_BUILDUP,
              plate_movement)
  {}

  static constexpr float MAGMA_BUILDUP   = 0.002;
  static constexpr float TENSION_BUILDUP = 0.002;

 protected:
  virtual const char* geology_type() const { return "Orogenic"; }
};

/**
 * Transform - Two plates sliding past each other.
 */
class Transform : public Geology
{
 public:
  Transform(float plate_movement)
    : Geology(TENSION_BUILDUP,
              MAGMA_BUILDUP,
              plate_movement)
  {}

  static constexpr float MAGMA_BUILDUP   = 0.000;
  static constexpr float TENSION_BUILDUP = 0.003;

 protected:
  virtual const char* geology_type() const { return "Transform"; }
};

/**
 * Inactive - No nearby plate boundary
 */
class Inactive : public Geology
{
 public:
  Inactive()
    : Geology(TENSION_BUILDUP,
              MAGMA_BUILDUP,
              0.0)
  {}

  static constexpr float MAGMA_BUILDUP   = 0.000;
  static constexpr float TENSION_BUILDUP = 0.000;

 protected:
  virtual const char* geology_type() const { return "Inactive"; }
};

}

#endif
