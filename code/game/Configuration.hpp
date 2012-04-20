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
 */
class Configuration
{
 public:

  static const std::string UNSET;

  /**
   * Unspecied configuration items will be left as '' so that it may be handled
   * by the knowledgable party.
   */
  Configuration(const std::string& interface_config = UNSET,
                const std::string& world_config     = UNSET,
                const std::string& player_name      = UNSET);

  // Getters

  const std::string& get_interface_config() const
  { return m_interface_config; }

  const std::string& get_world_config() const
  { return m_world_config; }

  const std::string& get_player_name() const
  { return m_player_name; }

 private:

  // Configuration items are all instance variables
  std::string m_interface_config;
  std::string m_world_config;
  std::string m_player_name;
};

}

#endif
