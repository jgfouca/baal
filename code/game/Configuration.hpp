#ifndef Configuration_hpp
#define Configuration_hpp

#include <string>

namespace baal {

/**
 * A class that aggregates all configuration information.
 *
 * Note, no knowledge of configuration semantics should be exposed here.
 * All we're doing here is storing strings; interpretation of the strings
 * is left to the users of the configuration.
 *
 * The empty string will always correlate with the default configuration.
 */
class Configuration
{
 public:

  /**
   * Unspecied configuration items will be left as '' so that it may be handled
   * by the knowledgable party.
   */
  Configuration(const std::string& interface_config = "",
                const std::string& world_config     = "",
                const std::string& player_config    = "")
    : m_interface_config(interface_config),
      m_world_config    (world_config),
      m_player_config   (player_config)
  {}

  Configuration(const Configuration& rhs)
    : m_interface_config(rhs.m_interface_config),
      m_world_config    (rhs.m_world_config),
      m_player_config   (rhs.m_player_config)
  {}

  Configuration(Configuration&& rhs)
    : m_interface_config(std::move(rhs.m_interface_config)),
      m_world_config    (std::move(rhs.m_world_config)),
      m_player_config   (std::move(rhs.m_player_config))
  {}

  Configuration& operator=(Configuration&& rhs)
  {
    m_interface_config = std::move(rhs.m_interface_config);
    m_world_config     = std::move(rhs.m_world_config);
    m_player_config    = std::move(rhs.m_player_config);

    return *this;
  }

  // Getters

  const std::string& get_interface_config() const
  { return m_interface_config; }

  const std::string& get_world_config() const
  { return m_world_config; }

  const std::string& get_player_config() const
  { return m_player_config; }

 private:

  // Configuration items are all instance variables
  std::string m_interface_config;
  std::string m_world_config;
  std::string m_player_config;
};

}

#endif
