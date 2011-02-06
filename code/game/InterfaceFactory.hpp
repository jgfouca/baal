#ifndef InterfaceFactory_hpp
#define InterfaceFactory_hpp

#include <string>

namespace baal {

class Interface;

class InterfaceFactory
{
 public:
  // Note, client is responsible for deletion
  static const Interface* create();

  static const std::string TEXT_INTERFACE;
  static const std::string GRAPHICAL_INTERFACE;
  static const std::string DEFAULT_INTERFACE;
};

}

#endif
