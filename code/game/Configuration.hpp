#ifndef Configuration_hpp
#define Configuration_hpp

namespace baal {

/**
 * A singleton class, the global object will contain the user's configuration
 * choices for this execution of the game.
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
  static Configuration& instance()
  {
    static Configuration global_config;
    return global_config;
  }

  ~Configuration() {}

  // Getters
  
  char get_interface() const { return m_interface; }

  char get_world() const { return m_world; }
  
 private:

  /**
   * Private constructor. Use public instance method to get the Configuration.
   */
  Configuration() {}

  // Disallowed methods
  Configuration(const Configuration&);
  Configuration& operator=(const Configuration&);

  // Configuration items are all instance variables
  char m_interface;
  char m_world;

  // Any modifier/setter of Configuration has to be a friend
  //friend baal::parse_args;
  friend bool parse_args(int argc, char** argv);
};

}

#endif
