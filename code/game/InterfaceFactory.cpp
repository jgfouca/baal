#include "InterfaceFactory.hpp"
#include "InterfaceText.hpp"
#include "InterfaceGraphical.hpp"
#include "Configuration.hpp"
#include "BaalExceptions.hpp"
#include "BaalCommon.hpp"
#include "Engine.hpp"

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>

namespace baal {

const std::string InterfaceFactory::TEXT_INTERFACE          = "t";
const std::string InterfaceFactory::GRAPHICAL_INTERFACE     = "g";
const std::string InterfaceFactory::DEFAULT_INTERFACE       = TEXT_INTERFACE;
const std::string InterfaceFactory::SEPARATOR               = ":";
const std::string InterfaceFactory::TEXT_WITH_COUT          = "cout";
const std::string InterfaceFactory::TEXT_WITH_CIN           = "cin";
const std::string InterfaceFactory::TEXT_WITH_OSTRINGSTREAM = "oss";
const std::string InterfaceFactory::TEXT_WITH_ISTRINGSTREAM = "iss";

///////////////////////////////////////////////////////////////////////////////
Interface& InterfaceFactory::create(Engine& engine)
///////////////////////////////////////////////////////////////////////////////
{
  // Get user's choice of interface
  const Configuration& config = engine.config();
  std::string interface_config = config.get_interface_config();
  if (interface_config == "") {
    interface_config = DEFAULT_INTERFACE;
  }

  // Create and return the desired interface
  auto tokens = split(interface_config, SEPARATOR);
  if (tokens[0] == TEXT_INTERFACE) {
    std::ostream* out = nullptr;
    std::istream* in  = nullptr;

    if (tokens.size() < 2 || tokens[1] == TEXT_WITH_COUT) {
      out = &std::cout;
    }
    else if (tokens[1] == TEXT_WITH_OSTRINGSTREAM) {
      out = new std::ostringstream();
    }
    else {
      out = new std::ofstream(tokens[1]);
      RequireUser(!out->fail(), "Could not open " << tokens[1]);
    }

    if (tokens.size() < 3 || tokens[2] == TEXT_WITH_CIN) {
      in = &std::cin;
    }
    else if (tokens[1] == TEXT_WITH_ISTRINGSTREAM) {
      in = new std::istringstream();
    }
    else {
      in = new std::ifstream(tokens[2]);
      RequireUser(!in->fail(), "Could not open " << tokens[2]);
    }

    return *(new InterfaceText(*out, *in, engine));
  }
  //else if (tokens[0] == GRAPHICAL_INTERFACE) {
  //  return *(InterfaceGraphical::create());
  //}
  else {
    RequireUser(false, "Invalid choice of interface: " << interface_config);
  }
}

}
