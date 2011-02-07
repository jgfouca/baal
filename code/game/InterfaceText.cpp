#include "InterfaceText.hpp"
#include "World.hpp"
#include "BaalExceptions.hpp"

#include <iostream>

using namespace baal;

///////////////////////////////////////////////////////////////////////////////
void InterfaceText::draw(const World& world) const
///////////////////////////////////////////////////////////////////////////////
{
  for (unsigned row = 0; row < world.height(); ++row) {
    for (unsigned col = 0; col < world.width(); ++col) {
      switch (world.get_tile(row, col).type()) {
      case MTN:
        m_stream << "M ";
        break;
      case PLAIN:
        m_stream << "P ";
        break;
      case OCEAN:
        m_stream << "O ";
        break;
      case UNDEFINED:
        Require(false, "World[" << row << "][" << col << "] is undefined");
        break;
      default:
        Require(false, "Should never make it here");
      }
    }
    m_stream << "\n";
  }
  m_stream.flush();
}
