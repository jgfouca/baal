#include "Engine.hpp"

#include <gtest/gtest.h>

namespace {

TEST(Engine, Basic)
{
  using namespace baal;

  // Test that quit works.
  Engine engine;
  engine.quit();
  engine.play();
}

}
