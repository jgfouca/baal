#ifndef BaalCommon_hpp
#define BaalCommon_hpp

#include "BaalExceptions.hpp"

#include <climits>
#include <string>
#include <vector>
#include <iostream>
#include <iterator>
#include <cstdlib>

#include <boost/range.hpp>
#include <boost/range/counting_range.hpp>

// Put simple, generic free functions in this file

namespace baal {

typedef std::vector<std::string> vecstr_t;

//
// Stuff for smart enums
//

template <typename Enum>
Enum get_first();

template <typename Enum>
Enum get_last();

template <typename Enum>
Enum from_string(const std::string& str);

template <typename Enum>
boost::iterator_range<boost::counting_iterator<Enum> >
iterate()
{
  return boost::counting_range(get_first<Enum>(), get_last<Enum>());
}

}

#define SMART_ENUM(Name, ...)                                           \
  namespace baal {                                                      \
                                                                        \
  enum Name {                                                           \
    Name##FIRST,                                                        \
    __VA_ARGS__,                                                        \
    Name##LAST                                                          \
  };                                                                    \
                                                                        \
  inline Name& operator++(Name& val)                                    \
  {                                                                     \
    Require(val != Name##LAST, "Ran off end of enum " << #Name);        \
    int i = static_cast<int>(val);                                      \
    ++i;                                                                \
    return val = static_cast<Name>(i);                                  \
  }                                                                     \
                                                                        \
  inline Name& operator--(Name& val)                                    \
  {                                                                     \
    Require(val != Name##FIRST, "Ran off front of enum " << #Name);     \
    int i = static_cast<int>(val);                                      \
    --i;                                                                \
    return val = static_cast<Name>(i);                                  \
  }                                                                     \
                                                                        \
  template <>                                                           \
  inline                                                                \
  Name get_first<Name>()                                                \
  {                                                                     \
    Name rv = Name##FIRST;                                              \
    ++rv;                                                               \
    return rv;                                                          \
  }                                                                     \
                                                                        \
  template <>                                                           \
  inline                                                                \
  Name get_last<Name>()                                                 \
  { return Name##LAST; }                                                \
                                                                        \
  inline const std::string& to_string(Name val)                         \
  {                                                                     \
    Require(val != Name##FIRST && val != Name##LAST, "Bad value " << val); \
    static vecstr_t strs = split(#__VA_ARGS__, ", ");   \
    int idx = static_cast<int>(val) - 1;                                \
    return strs[idx];                                                   \
  }                                                                     \
                                                                        \
  inline std::ostream& operator<<(std::ostream& out, Name val)          \
  {                                                                     \
    out << to_string(val);                                              \
    return out;                                                         \
  }                                                                     \
                                                                        \
  template<>                                                            \
  inline                                                                \
  Name from_string<Name>(const std::string& str)                        \
  {                                                                     \
    static vecstr_t strs = split(#__VA_ARGS__, ", ");   \
    std::string up = str;                                               \
    std::transform(up.begin(), up.end(), up.begin(), ::toupper);        \
    auto fitr = std::find(strs.begin(), strs.end(), up);                \
    RequireUser(fitr != strs.end(),                                     \
                "String '" << str << "' not a valid" << #Name);         \
    return static_cast<Name>(fitr-strs.begin() + 1);                    \
  }                                                                     \
                                                                        \
  }                                                                     \
                                                                        \
  namespace std {                                                       \
                                                                        \
  template<>                                                            \
  struct iterator_traits<baal::Name>                                    \
  {                                                                     \
    typedef ptrdiff_t difference_type;                                  \
    typedef baal::Name value_type;                                      \
    typedef baal::Name* pointer;                                        \
    typedef baal::Name& reference;                                      \
    typedef bidirectional_iterator_tag iterator_category;               \
  };                                                                    \
                                                                        \
  }

//
// Stuff for Location
//

namespace baal {

extern const unsigned INVALID;

// Basically a named pair defining a location in a 2D space
struct Location
{
  Location() : row(INVALID), col(INVALID) {}

  Location(unsigned r, unsigned c) : row(r), col(c) {}

  Location(const std::string& str_location);

  bool operator==(const Location& location) const;

  bool operator!=(const Location& location) const;

  unsigned row;
  unsigned col;
};

// Inlined methods

///////////////////////////////////////////////////////////////////////////////
inline
bool Location::operator==(const Location& location) const
///////////////////////////////////////////////////////////////////////////////
{
  return (row == location.row && col == location.col);
}

///////////////////////////////////////////////////////////////////////////////
inline
bool Location::operator!=(const Location& location) const
///////////////////////////////////////////////////////////////////////////////
{
  return !(*this == location);
}

///////////////////////////////////////////////////////////////////////////////
inline
std::ostream& operator<<(std::ostream& out, const Location& location)
///////////////////////////////////////////////////////////////////////////////
{
  return out << location.row << ',' << location.col;
}

//
// Misc free functions
//

/**
 * Returns true if this file was compiled in optimized mode.
 *
 * Used for sanity-checking build system.
 */
///////////////////////////////////////////////////////////////////////////////
inline
bool is_opt()
///////////////////////////////////////////////////////////////////////////////
{
#ifndef NDEBUG
  return false;
#else
  return true;
#endif
}

///////////////////////////////////////////////////////////////////////////////
inline
void clear_screen()
///////////////////////////////////////////////////////////////////////////////
{
#ifndef WINDOWS
  std::system("clear");
#endif
}

///////////////////////////////////////////////////////////////////////////////
inline
bool is_valid(Location location)
///////////////////////////////////////////////////////////////////////////////
{
  return location != Location();
}

///////////////////////////////////////////////////////////////////////////////
template <class T, class Container>
inline
bool contains(const Container& container, const T& t)
///////////////////////////////////////////////////////////////////////////////
{
  return container.find(t) != container.end();
}

///////////////////////////////////////////////////////////////////////////////
template <class T>
inline
bool contains(const std::vector<T>& container, const T& t)
///////////////////////////////////////////////////////////////////////////////
{
  return std::find(std::begin(container), std::end(container), t) !=
         container.end();
}

///////////////////////////////////////////////////////////////////////////////
vecstr_t split(const std::string& str,
                               const std::string& sep);
///////////////////////////////////////////////////////////////////////////////

//
// AdjacentLocationIterator and associated free functions
//

class Engine;

class AdjacentLocationIterator :
    public std::iterator<std::forward_iterator_tag,
                         Location,
                         std::ptrdiff_t,
                         Location*,
                         Location>
{
 public:
  AdjacentLocationIterator(Location center, const Engine& engine) :
    m_center(center),
    m_current(),
    m_engine(engine)
  {
    advance();
  }

  // Construct end iterator
  AdjacentLocationIterator(const Engine& engine) :
    m_center(),
    m_current(),
    m_engine(engine)
  {}

  AdjacentLocationIterator& operator++()
  {
    advance();
    return *this;
  }

  AdjacentLocationIterator operator++(int)
  {
    AdjacentLocationIterator temp = *this;
    ++*this;;
    return temp;
  }

  bool operator==(const AdjacentLocationIterator& rhs) const
  {
    return m_center == rhs.m_center && m_current == rhs.m_current;
  }

  bool operator!=(const AdjacentLocationIterator& rhs) const
  {
    return !(*this == rhs);
  }

  Location operator*() const
  {
    return m_current;
  }

  Location* operator->()
  {
    return &this->m_current;
  }

 private:
  void advance();

  Location m_center;
  Location m_current;
  const Engine& m_engine;
};


typedef boost::iterator_range<AdjacentLocationIterator> AdjacentLocationRange;

inline
AdjacentLocationRange get_adjacent_location_range(Location center,
                                                  const Engine& engine)
{
  return boost::make_iterator_range(AdjacentLocationIterator(center, engine),
                                    AdjacentLocationIterator(engine));
}

}

#endif
