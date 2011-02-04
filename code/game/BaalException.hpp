#ifndef BaalException_hpp
#define BaalException_hpp

#include <string>
#include <sstream>
#include <stdexcept>

// This file defines the exceptions and macros to be used for error
// handling. Do not use the exception classes directly, use the macros
// instead.

namespace baal {

class BaalException : public std::exception
{
 public:
  BaalException(const std::string& expr,
                const std::string& file,
                unsigned line,
                const std::string& message,
                bool attach);

  virtual ~BaalException() throw() {}

  virtual const char* what() const throw();
  
 private:
  std::string m_expr;
  std::string m_file;
  unsigned    m_line;
  std::string m_message;
};

// An internal macro used by the public macros, do not use this directly
#define ThrowGeneric(expr, message, attach)                             \
  do {                                                                  \
    if ( !(expr) ) {                                                    \
      std::ostringstream baal_internal_throw_require_oss;               \
      baal_internal_throw_require_oss << message;                       \
      throw baal::BaalException( #expr,                                 \
                                 __FILE__,                              \
                                 __LINE__,                              \
                                 baal_internal_throw_require_oss.str(), \
                                 attach );                              \
    }                                                                   \
  } while (false)

// Requires will always be checked, Asserts will only be checked in debug.
// Use the attach versions of the macros for checks in complex contexts where
// viewing the failing stackframe in GDB would be helpful.
#define ThrowRequire(expr, msg)       ThrowGeneric(expr, msg, false)
#define ThrowRequireAttach(expr, msg) ThrowGeneric(expr, msg, true)
#ifndef NDEBUG
#define ThrowAssert(expr, msg)       ThrowRequire(expr, msg)
#define ThrowAssertAttach(expr, msg) ThrowRequireAttach(expr, msg)
#else
#define ThrowAssert(expr, msg)       ((void) (0))
#define ThrowAssertAttach(expr, msg) ((void) (0))
#endif

}

#endif
