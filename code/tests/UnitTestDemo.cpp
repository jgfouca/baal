#include <gtest/gtest.h>

#include <stdexcept>

void throwing_func()
{
  throw std::runtime_error("dummy");
}

TEST(DemoTest, Demonstration)
{
  // Demonstrate some of the macroes that gtest provides us.

  // ASSERT failures will end the test immediately, EXPECT failures will be
  // logged but the test will continue.
  
  ASSERT_TRUE(true);
  EXPECT_TRUE(true);
  
  ASSERT_EQ(1, 1);
  EXPECT_EQ(1, 1);
  
  ASSERT_THROW(throwing_func(), std::runtime_error);
  EXPECT_THROW(throwing_func(), std::runtime_error);
}
