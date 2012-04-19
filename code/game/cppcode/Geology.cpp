#include "Geology.hpp"
#include "BaalExceptions.hpp"
#include "WorldTile.hpp"

#include <iomanip>

namespace baal {

///////////////////////////////////////////////////////////////////////////////
Geology::Geology(float base_tension_buildup,
                 float base_magma_buildup,
                 float plate_movement)
///////////////////////////////////////////////////////////////////////////////
  : m_tension(0.0),
    m_magma(0.0),
    m_plate_movement(plate_movement),
    m_tension_buildup(base_tension_buildup * plate_movement),
    m_magma_buildup(base_magma_buildup * plate_movement)
{
  Require(plate_movement >= 0.0,       "Broken precondition");
  Require(base_tension_buildup >= 0.0, "Broken precondition");
  Require(base_magma_buildup >= 0.0,   "Broken precondition");
}

///////////////////////////////////////////////////////////////////////////////
void Geology::cycle_turn()
///////////////////////////////////////////////////////////////////////////////
{
  // Tension/magma build up more slowly as they reach 100%
  m_tension += (1 - m_tension) * m_tension_buildup;
  m_magma   += (1 - m_magma)   * m_magma_buildup;

  Require(m_tension < 1.0, "Invariant violated: " << m_tension);
  Require(m_magma   < 1.0, "Invariant violated: " << m_magma);
}

///////////////////////////////////////////////////////////////////////////////
bool Geology::is_geological(DrawMode mode)
///////////////////////////////////////////////////////////////////////////////
{
  return mode == GEOLOGY ||
         mode == TENSION ||
         mode == MAGMA;
}

///////////////////////////////////////////////////////////////////////////////
xmlNodePtr Geology::to_xml()
///////////////////////////////////////////////////////////////////////////////
{
  xmlNodePtr Geology_node = xmlNewNode(nullptr, BAD_CAST "Geology");

  std::ostringstream m_plate_movement_oss;
  m_plate_movement_oss << m_plate_movement;
  xmlNewChild(Geology_node, nullptr, BAD_CAST "m_plate_movement", BAD_CAST m_plate_movement_oss.str().c_str());
  xmlNewChild(Geology_node, nullptr, BAD_CAST "type", BAD_CAST this->geology_type());

  return Geology_node;
}

}
