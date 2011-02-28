#include "BaalCommon.hpp"
#include "BaalExceptions.hpp"
#include "Engine.hpp"
#include "Configuration.hpp"
#include "InterfaceFactory.hpp"
#include "WorldFactory.hpp"
#include "WorldFactoryHardcoded.hpp"

#include <iostream>
#include <string>
#include <sstream>

namespace baal {

///////////////////////////////////////////////////////////////////////////////
std::string get_help()
///////////////////////////////////////////////////////////////////////////////
{
  // Grab config info from the various factories

  const std::string text_interface    = InterfaceFactory::TEXT_INTERFACE;
  const std::string gfx_interface     = InterfaceFactory::GRAPHICAL_INTERFACE;
  const std::string default_interface = InterfaceFactory::DEFAULT_INTERFACE;
  
  const std::string generated_world   = WorldFactory::GENERATED_WORLD;
  const unsigned num_hardcoded_worlds = WorldFactoryHardcoded::NUM_HARDCODED_WORLDS;
  const std::string default_world     = WorldFactory::DEFAULT_WORLD;

  std::ostringstream out;
  out << "<baal-exe> [-i (t|g)] [-w (r|1|2|...)] [-p <name>]\n"
      << "\n"
      << "  Use the -i option to choose interface\n"
      << "    " << text_interface << " -> text" <<
         (text_interface == default_interface ? "(default)" : "") << "\n"
      << "    " << gfx_interface << " -> graphical" <<
         (gfx_interface == default_interface ? "(default)" : "") << "\n"
      << "\n"
      << "  Use the -w option to chose world\n";
  for (unsigned i = 1; i <= num_hardcoded_worlds; ++i) {
    std::ostringstream temp;
    temp << i;
    std::string i_str = temp.str();
    out << "    " << i << " -> Hardcoded world " << i <<
      (i_str == default_interface ? "(default)" : "") << "\n";
  }
  out << "    " << generated_world << " -> randomly generated world" <<
    (generated_world == default_interface ? "(default)" : "") << "\n"
      << "\n"
      << "  Use the -p option to chose player name\n";
  return out.str();
}

/**
 * Parse args and configure Configuration singleton
 *
 * Returns true if program should continue
 */
///////////////////////////////////////////////////////////////////////////////
bool parse_args(int argc, char** argv)
///////////////////////////////////////////////////////////////////////////////
{
  // Get handle to config
  Configuration& config = Configuration::instance();

  // Parse args
  for (int i = 1; i < argc; ++i) {
    std::string arg(argv[i]);

    if (arg == "-h" || arg == "-help" || arg == "help" || arg == "--help") {
      std::cout << get_help() << std::endl;
      return false;
    }
    else if (arg == "-i" || arg == "-w" || arg == "-p") {
      // These options take an argument, try to get it
      RequireUser(i+1 < argc, "Option " << arg << " requires argument");
      std::string opt_arg = argv[++i]; // note inc of i

      if (arg == "-i") {
        config.m_interface_config = opt_arg;
      }
      else if (arg == "-w") {
        config.m_world_config     = opt_arg;
      }
      else if (arg == "-p") {
        config.m_player_name      = opt_arg;
      }
      else {
        Require(false, "Should never make it here");
      }
    }
    else {
      RequireUser(false, "Unrecognized argument: " << arg);
    }
  }

  return true;
}

} // namespace baal

///////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv)
///////////////////////////////////////////////////////////////////////////////
{
  // Sanity check build system

#ifndef NDEBUG
  const bool isopt = false;
#else
  const bool isopt = true;
#endif
  
  if (baal::is_opt()) {
    Require(isopt, "Expect opt, build system is broken");
    std::cout << "Running in opt mode" << std::endl;
  }
  else {
    Require(!isopt, "Expect dbg, build system is broken");
    std::cout << "Running in dbg mode" << std::endl;
  }

  // Parse args
  if (baal::parse_args(argc, argv)) {
    // Begin game, errors during construction are probably user-errors
    baal::Engine engine;
    engine.play();
  }

  return 0;
}
