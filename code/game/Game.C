#include "BaalCommon.hpp"
#include "BaalException.hpp"

#include <iostream>

using namespace baal;

int main(int argc, char** argv)
{
  // Sanity check build system

#ifndef NDEBUG
  const bool isopt = false;
#else
  const bool isopt = true;
#endif
  
  if (is_opt()) {
    ThrowRequire(isopt, "Expect opt, build system is broken");
    std::cout << "Running in opt mode" << std::endl;
  }
  else {
    ThrowRequire(!isopt, "Expect dbg, build system is broken");
    std::cout << "Running in dbg mode" << std::endl;
  }

  return 0;
}
