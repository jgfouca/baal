#include "InterfaceFactory.hpp"
#include "InterfaceText.hpp"
#include "InterfaceGraphical.hpp"
#include "Configuration.hpp"
#include "BaalExceptions.hpp"
#include "Engine.hpp"

#include <iostream>

using namespace baal;

const std::string InterfaceFactory::TEXT_INTERFACE      = "t";
const std::string InterfaceFactory::GRAPHICAL_INTERFACE = "g";
const std::string InterfaceFactory::DEFAULT_INTERFACE   = TEXT_INTERFACE;

///////////////////////////////////////////////////////////////////////////////
Interface& InterfaceFactory::create(Engine& engine)
///////////////////////////////////////////////////////////////////////////////
{
  // Get user's choice of interface
  const Configuration& config = engine.config();
  std::string interface_config = config.get_interface_config();
  if (interface_config == Configuration::UNSET) {
    interface_config = DEFAULT_INTERFACE;
  }

  // Create and return the desired interface
  if (interface_config == TEXT_INTERFACE) {
    return *(new InterfaceText(std::cout, std::cin, engine));
  }
  //else if (interface_config == GRAPHICAL_INTERFACE) {
  //  return *(InterfaceGraphical::create());
  //}
  else {
    RequireUser(false, "Invalid choice of interface: " << interface_config);
  }
}
