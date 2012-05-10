#include "InterfaceFactory.hpp"
#include "InterfaceText.hpp"
#include "InterfaceGraphical.hpp"
#include "Engine.hpp"
#include "Configuration.hpp"

#include <gtest/gtest.h>

namespace {

TEST(InterfaceFactory, basic)
{
  using namespace baal;

  {
    auto engine = create_engine();

    Interface* interface = &engine->interface();
    InterfaceText* expected_interface = dynamic_cast<InterfaceText*>(interface);
    EXPECT_NE(nullptr, expected_interface);
  }

  {
    Configuration config(InterfaceFactory::TEXT_INTERFACE);
    auto engine = create_engine(config);

    Interface* interface = &engine->interface();
    InterfaceText* expected_interface = dynamic_cast<InterfaceText*>(interface);
    EXPECT_NE(nullptr, expected_interface);
  }

  {
    baal::Configuration config("asdasd");
    EXPECT_THROW(create_engine(config), baal::UserError);
  }

  {
    Configuration config(InterfaceFactory::GRAPHICAL_INTERFACE);
    EXPECT_THROW(create_engine(config), UserError);

    // Interface* interface = &engine->interface();
    // InterfaceGraphical* expected_interface = dynamic_cast<InterfaceGraphical*>(interface);
    // EXPECT_NE(nullptr, expected_interface);

  }

  {
    Configuration config(InterfaceFactory::TEXT_INTERFACE +
                         InterfaceFactory::SEPARATOR +
                         InterfaceFactory::TEXT_WITH_OSTRINGSTREAM +
                         InterfaceFactory::SEPARATOR +
                         InterfaceFactory::TEXT_WITH_ISTRINGSTREAM);
    auto engine = create_engine(config);

    Interface* interface = &engine->interface();
    InterfaceText* expected_interface = dynamic_cast<InterfaceText*>(interface);
    EXPECT_NE(nullptr, expected_interface);
  }
}

}
