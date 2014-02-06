#include "InterfaceGraphical.hpp"
#include "Engine.hpp"

namespace baal {

///////////////////////////////////////////////////////////////////////////////
InterfaceGraphical::InterfaceGraphical(Engine& engine)
///////////////////////////////////////////////////////////////////////////////
  : Interface(TILE_DISPLAY_WIDTH, TILE_DISPLAY_HEIGHT),
    m_engine(engine)
{
  RequireUser(false, "Not implemented");
  (void) m_engine;
}

}
