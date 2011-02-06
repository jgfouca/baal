#include "InterfaceFactory.hpp"
#include "InterfaceText.hpp"
#include "InterfaceGraphical.hpp"
#include "Configuration.hpp"
#include "BaalExceptions.hpp"

using namespace baal;

const std::string InterfaceFactory::TEXT_INTERFACE      = "t";
const std::string InterfaceFactory::GRAPHICAL_INTERFACE = "g";
const std::string InterfaceFactory::DEFAULT_INTERFACE   = TEXT_INTERFACE;

///////////////////////////////////////////////////////////////////////////////
const Interface* InterfaceFactory::create()
///////////////////////////////////////////////////////////////////////////////
{
  // Get user's choice of interface
  Configuration& config = Configuration::instance();
  std::string interface_config = config.get_interface_config();
  if (interface_config == Configuration::UNSET) {
    interface_config = DEFAULT_INTERFACE;
  }

  // Create and return the desired interface
  if (interface_config == TEXT_INTERFACE) {
    return new InterfaceText;
  }
  else if (interface_config == GRAPHICAL_INTERFACE) {
    return new InterfaceGraphical;
  }
  else {
    RequireUser(false, "Invalid choice of interface: " << interface_config);
    return NULL;
  }
}
