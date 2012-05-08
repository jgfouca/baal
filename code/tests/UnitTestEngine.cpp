#include "Engine.hpp"

#include <gtest/gtest.h>

namespace {

TEST(Engine, Basic)
{
  using namespace baal;

  // Test that quit works.
  auto engine = create_engine();
  engine->quit();
  engine->play();
}

}
