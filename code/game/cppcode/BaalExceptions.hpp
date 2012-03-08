#ifndef BaalExceptions_hpp
#define BaalExceptions_hpp

#include <string>
#include <sstream>
#include <stdexcept>

// This file defines the exceptions and macros to be used for error
// handling. Do not use the exception classes directly, use the macros
// instead.

namespace baal {

// Note: using boost exceptions here is an option, but they don't seem to
// add a lot of value.

//
// ProgramErrors indicate programming errors. In general, these should be
// allowed to escape so that the program can crash. With certain macros,
// you can set things up to attach a debugger when these are thrown.
//
class ProgramError : public std::exception
{
 public:
  ProgramError(const std::string& expr,
               const std::string& file,
               unsigned line,
               const std::string& message,
               bool attach);

  virtual ~ProgramError() throw() {}

  virtual const char* what() const throw();

  const std::string& message() const { return m_message; }

 private:
  std::string m_expr;
  std::string m_file;
  unsigned    m_line;
  std::string m_message;
};

//
// UserErrors indicate user errors. In general, these should always be caught
// and handled. Please do not raise these directly, use the macros instead.
//
class UserError : public std::exception
{
 public:
  UserError(const std::string& expr,
            const std::string& file,
            unsigned line,
            const std::string& message,
            bool attach) : m_message(message) {}

  virtual ~UserError() throw() {}

  virtual const char* what() const throw()
  {
    return m_message.c_str();
  }

 private:
  std::string m_message;
};

// An internal macro used by the public macros, do not use this directly
#define ThrowGeneric(expr, message, EXCEPTION, attach)                  \
  do {                                                                  \
    if ( !(expr) ) {                                                    \
      std::ostringstream baal_internal_throw_require_oss;               \
      baal_internal_throw_require_oss << message;                       \
      throw baal::EXCEPTION( #expr,                                     \
                             __FILE__,                                  \
                             __LINE__,                                  \
                             baal_internal_throw_require_oss.str(),     \
                             attach );                                  \
    }                                                                   \
  } while (false)

// Requires will always be checked, Asserts will only be checked in debug.
// Use the attach versions of the macros for checks in complex contexts where
// viewing the failing stackframe in GDB would be helpful.
#define Require(expr, msg)       ThrowGeneric(expr, msg, ProgramError, false)
#define RequireAttach(expr, msg) ThrowGeneric(expr, msg, ProgramError, true)
#define RequireUser(expr, msg)   ThrowGeneric(expr, msg, UserError, false)
#ifndef NDEBUG
#define Assert(expr, msg)       Require(expr, msg)
#define AssertAttach(expr, msg) RequireAttach(expr, msg)
#else
#define Assert(expr, msg)       ((void) (0))
#define AssertAttach(expr, msg) ((void) (0))
#endif

}

#endif
