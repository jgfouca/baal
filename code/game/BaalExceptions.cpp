#include <iostream>

#ifndef WINDOWS
#include <unistd.h>
#endif

#include "BaalExceptions.hpp"

namespace baal {

///////////////////////////////////////////////////////////////////////////////
ProgramError::ProgramError(const std::string& expr,
                           const std::string& file,
                           unsigned line,
                           const std::string& message,
                           bool attach)
///////////////////////////////////////////////////////////////////////////////
  : m_expr(expr),
    m_file(file),
    m_line(line),
    m_message(message)
{
#ifndef WINDOWS
  if (attach) {
    std::cerr << what() << "\n\n"
              << "Process ID: " << getpid() << "\n"
              << "Attach via command: 'gdb <exe> <pid>'" << std::endl;
    sleep(999999);
  }
#endif
}

///////////////////////////////////////////////////////////////////////////////
const char* ProgramError::what() const throw()
///////////////////////////////////////////////////////////////////////////////
{
  static std::ostringstream out;
  out.str("");
  out.clear();

  out << "Error at " << m_file << ":" << m_line << "\n"
      << "Expression: " << m_expr << " FAILED\n"
      << "Message: " << m_message << std::endl;
  return out.str().c_str();
}

}
