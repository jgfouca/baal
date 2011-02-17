#ifndef Configuration_hpp
#define Configuration_hpp

#include <string>

namespace baal {

/**
 * A singleton class, the global object will contain the user's configuration
 * choices for this execution of the game.
 *
 * Note, no knowledge of configuration semantics should be exposed here.
 */
class Configuration
{
 public:

  /**
   * Retrieve global singleton instance.
   *
   * It's OK to return a non-const since there are no non-const methods in the
   * public interface.
   */
  static Configuration& instance();

  // Getters

  const std::string& get_interface_config() const
  { return m_interface_config; }

  const std::string& get_world_config() const
  { return m_world_config; }

  const std::string& get_player_name() const
  { return m_player_name; }

  static const std::string UNSET;

 private:

  /**
   * Private constructor. Use public instance method to get the Configuration.
   *
   * Unspecied configuration items will be left as '' so that it may be handled
   * by the knowledgable party.
   */
  Configuration();

  // Disallowed methods
  Configuration(const Configuration&);
  Configuration& operator=(const Configuration&);

  // Configuration items are all instance variables
  std::string m_interface_config;
  std::string m_world_config;
  std::string m_player_name;

  // Any modifier/setter of Configuration has to be a friend
  friend bool parse_args(int argc, char** argv);
};

}

#endif
