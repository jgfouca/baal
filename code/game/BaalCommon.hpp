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
  Enum last = get_last<Enum>();
  ++last;
  return boost::counting_range(get_first<Enum>(), last);
}

template <typename Enum>
int size()
{
  return boost::size(iterate<Enum>());
}

}

#define SMART_ENUM(Name, ...)                                           \
  namespace baal {                                                      \
                                                                        \
  enum Name {                                                           \
    Name##FIRST = -1,                                                   \
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
  {                                                                     \
    Name rv = Name##LAST;                                               \
    --rv;                                                               \
    return rv;                                                          \
  }                                                                     \
                                                                        \
  inline const std::string& to_string(Name val)                         \
  {                                                                     \
    Require(val != Name##FIRST && val != Name##LAST, "Bad value " << val); \
    static vecstr_t strs = split(#__VA_ARGS__, ", ");                   \
    return strs[val];                                                   \
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
    return static_cast<Name>(fitr-strs.begin());                        \
  }                                                                     \
                                                                        \
  inline std::istream& operator>>(std::istream& in, Name& val)          \
  {                                                                     \
    std::string name;                                                   \
    in >> name;                                                         \
    val = from_string<Name>(name);                                      \
    return in;                                                          \
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

  unsigned distance(const Location& location) const;

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

///////////////////////////////////////////////////////////////////////////////
struct LocationIterator :
    public std::iterator<std::forward_iterator_tag,
                         Location,
                         std::ptrdiff_t,
                         Location*,
                         Location>
///////////////////////////////////////////////////////////////////////////////
{
  LocationIterator(unsigned start_row, unsigned start_col, unsigned max_row, unsigned max_col) :
    m_start_col(start_col),
    m_curr_row(start_row),
    m_curr_col(start_col),
    m_max_row(max_row),
    m_max_col(max_col)
  {
    Require(start_row <= max_row, start_row << ", " << max_row);
    Require(start_col < max_col, start_col << ", " << max_col);
  }

  LocationIterator(Location center, unsigned radius) :
    LocationIterator( center.row < radius ? 0 : center.row - radius,
                      center.col < radius ? 0 : center.col - radius,
                      center.row + radius + 1,
                      center.col + radius + 1)
  {}

  LocationIterator(unsigned max_row, unsigned max_col) :
    LocationIterator(0, 0, max_row, max_col)
  {}

  Location operator*() const
  {
    return Location(m_curr_row, m_curr_col);
  }

  // Location* operator->() const
  // {
  // }

  LocationIterator& operator++()
  {
    advance();
    return *this;
  }

  LocationIterator operator++(int)
  {
    LocationIterator rv = *this;
    advance();
    return rv;
  }

  // Given the current configuration, return the ending iterator
  LocationIterator end() const
  {
    return LocationIterator(m_max_row, m_start_col, m_max_row, m_max_col);
  }

  bool operator==(LocationIterator const& rhs) const
  {
    return m_start_col == rhs.m_start_col &&
           m_curr_row  == rhs.m_curr_row &&
           m_curr_col  == rhs.m_curr_col &&
           m_max_row   == rhs.m_max_row &&
           m_max_col   == rhs.m_max_col;
  }

  bool operator!=(LocationIterator const& rhs) const
  {
    return !(*this == rhs);
  }

  unsigned m_start_col;
  unsigned m_curr_row;
  unsigned m_curr_col;
  unsigned m_max_row;
  unsigned m_max_col;

 private:
  void advance()
  {
    Require(m_curr_row < m_max_row, "Iterating past end");

    if (m_curr_col + 1 == m_max_col) {
      m_curr_col = m_start_col;
      ++m_curr_row;
    }
    else {
      ++m_curr_col;
    }

    Require(m_curr_col < m_max_col, m_curr_col);
    Require(m_curr_row <= m_max_row, m_curr_row);
  }
};

}

#endif
