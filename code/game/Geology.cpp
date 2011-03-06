#include "Geology.hpp"
#include "BaalExceptions.hpp"

#include <iomanip>

using namespace baal;

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
void Geology::draw_text(std::ostream& out) const
///////////////////////////////////////////////////////////////////////////////
{
  int invalid_color = -1;
  int color = invalid_color;
  std::string invalid_symbol = "????";
  std::string symbol = invalid_symbol;
  float property;
  std::ostringstream oss;

  switch (s_draw_mode) {
  case GEOLOGY:
    color = this->color();
    symbol = this->symbol();
    break;
  case TENSION:
  case MAGMA:
    property = s_draw_mode == TENSION ? m_tension : m_magma;
    if (property < .333) {
      color = 32; // green
    }
    else if (property < .666) {
      color = 33; // yellow
    }
    else {
      color = 31; //red
    }
    oss << std::setprecision(3) << std::fixed << property;
    symbol = oss.str();
    break;
  default:
    Require(false, "Should not draw geology in mode: " << s_draw_mode);
  }

  Require(color  != invalid_color , "Color was not set");
  Require(symbol != invalid_symbol, "Symbol was not set");

  out << "\033[1;" << color << "m" // set color and bold text
      << symbol                    // print symbol
      << "\033[0m";                // clear color and boldness
}

///////////////////////////////////////////////////////////////////////////////
bool Geology::is_geological(DrawMode mode)
///////////////////////////////////////////////////////////////////////////////
{
  return mode == GEOLOGY ||
         mode == TENSION ||
         mode == MAGMA;
}
