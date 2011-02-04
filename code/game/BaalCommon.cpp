#include "BaalCommon.hpp"

#include <iostream>

bool baal::is_opt()
{
#ifndef NDEBUG
  return false;
#else
  return true;
#endif
}
