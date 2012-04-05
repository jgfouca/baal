#include "Configuration.hpp"

namespace baal {

const std::string Configuration::UNSET = "";

///////////////////////////////////////////////////////////////////////////////
Configuration::Configuration(const std::string& interface_config,
                             const std::string& world_config,
                             const std::string& player_name)
///////////////////////////////////////////////////////////////////////////////
  : m_interface_config(interface_config),
    m_world_config(world_config),
    m_player_name(player_name)
{}

}
