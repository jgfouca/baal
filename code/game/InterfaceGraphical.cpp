#include "InterfaceGraphical.hpp"
#include "Engine.hpp"

namespace baal {

///////////////////////////////////////////////////////////////////////////////
InterfaceGraphical::InterfaceGraphical(Engine& engine)
///////////////////////////////////////////////////////////////////////////////
  : Interface(),
    m_engine(engine)
{
  RequireUser(false, "Not implemented");
  (void) m_engine;
}

}
