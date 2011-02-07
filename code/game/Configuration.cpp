#include "Configuration.hpp"

using namespace baal;

const std::string Configuration::UNSET = "";

///////////////////////////////////////////////////////////////////////////////
Configuration& Configuration::instance()
///////////////////////////////////////////////////////////////////////////////
{
  static Configuration global_config;
  return global_config;
}

///////////////////////////////////////////////////////////////////////////////
Configuration::Configuration()
///////////////////////////////////////////////////////////////////////////////
  : m_interface_config(UNSET),
    m_world_config(UNSET)
{
}