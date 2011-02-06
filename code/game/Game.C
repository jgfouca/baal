#include "BaalCommon.hpp"
#include "BaalExceptions.hpp"
#include "Engine.hpp"
#include "Configuration.hpp"

#include <iostream>
#include <string>

namespace baal {

static const std::string HELP =
  "<baal-exe> [-i (t|g)] [-w (r|1|2|...)]\n"
  "\n"
  "  Use the -i option to choose interface\n"
  "    t -> text (default)\n"
  "    g -> graphical\n"
  "\n"
  "  Use the -w option to chose world\n"
  "    1 -> hardcoded world 1 (default)\n"
  "    2 -> hardcoded world 2\n"
  "    ...\n"
  "    r -> randomly generated world\n"
  "";

/**
 * Parse args and configure Configuration singleton
 *
 * Returns true if program should continue
 */
bool parse_args(int argc, char** argv)
{
  // Default config
  char interface = 't';
  char world     = '1';

  // Parse args
  for (int i = 1; i < argc; ++i) {
    std::string arg(argv[i]);

    if (arg == "-h" || arg == "-help" || arg == "help" || arg == "--help") {
      std::cout << HELP << std::endl;
      return false;
    }
    else if (arg == "-i" || arg == "-w") {
      RequireUser(i+1 < argc, "Option " << arg << " requires argument");
      std::string opt_arg = argv[++i]; // note inc of i
      RequireUser(opt_arg.size() == 1,
                  "Arg '" << opt_arg << "' to option " << arg <<
                  " is invalid, expect single character");
      if (arg == "-i") {
        interface = opt_arg[0];
      }
      else if (arg == "-w") {
        world = opt_arg[0];
      }
      else {
        Require(false, "Should never make it here");
      }
    }
    else {
      RequireUser(false, "Unrecognized argument: " << arg);
    }
  }

  // Set configuration
  Configuration& config = Configuration::instance();
  config.m_interface = interface;
  config.m_world     = world;

  return true;
}

} // namespace baal

int main(int argc, char** argv)
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
    // Begin game
    baal::Engine engine;
    //engine.play();
  }

  return 0;
}
