#ifndef InterfaceFactory_hpp
#define InterfaceFactory_hpp

#include <string>

namespace baal {

class Interface;

/**
 * Creates Interface objects. This class encapsulates the knowledge
 * that there are many types of interfaces; no other entity should
 * have to know about that.
 */
class InterfaceFactory
{
 public:
  // Note, client is responsible for deletion
  static Interface& create();

  static const std::string TEXT_INTERFACE;
  static const std::string GRAPHICAL_INTERFACE;
  static const std::string DEFAULT_INTERFACE;
};

}

#endif
