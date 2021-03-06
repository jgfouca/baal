#ifndef InterfaceFactory_hpp
#define InterfaceFactory_hpp

#include <string>
#include <memory>

namespace baal {

class Interface;
class Engine;

/**
 * Creates Interface objects. This class encapsulates the knowledge
 * that there are many types of interfaces; no other entity should
 * have to know about that.
 */
class InterfaceFactory
{
 public:
  // Note, client is responsible for deletion
  static std::shared_ptr<Interface> create(Engine& engine);

  static const std::string TEXT_INTERFACE;
  static const std::string GRAPHICAL_INTERFACE;
  static const std::string DEFAULT_INTERFACE;
  static const std::string SEPARATOR;
  static const std::string TEXT_WITH_COUT;
  static const std::string TEXT_WITH_CIN;
  static const std::string TEXT_WITH_OSTRINGSTREAM;
  static const std::string TEXT_WITH_ISTRINGSTREAM;
};

}

#endif
