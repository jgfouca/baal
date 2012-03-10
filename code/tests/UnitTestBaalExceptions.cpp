#include "BaalExceptions.hpp"

#include <gtest/gtest.h>

#include <iostream>
#include <string>

namespace {

void require_func(bool do_throw)
{
  Require(!do_throw, "msg " << 1 << " test");
}

void assert_func(bool do_throw)
{
  Assert(!do_throw, "msg " << 2 << " test");
}

void user_require_func(bool do_throw)
{
  RequireUser(!do_throw, "msg " << 3 << " test");
}

TEST(BaalExceptions, BaalExceptionsBasic)
{
  // Test exception macros

  // Test that throws occur when they should
  EXPECT_THROW(require_func(true), baal::ProgramError);
  EXPECT_THROW(user_require_func(true), baal::UserError);
#ifndef NDEBUG
  EXPECT_THROW(assert_func(true), baal::ProgramError);
#else
  EXPECT_NO_THROW(assert_func(true));
#endif

  // Test that throws do not occur when they shouldn't
  EXPECT_NO_THROW(require_func(false));
  EXPECT_NO_THROW(user_require_func(false));
  EXPECT_NO_THROW(assert_func(false));

  // Test message
  try {
    user_require_func(true);
  }
  catch (baal::UserError& e)
  {
    std::string msg = e.what();
    EXPECT_EQ(std::string("msg 3 test"), msg);
  }
}

}
