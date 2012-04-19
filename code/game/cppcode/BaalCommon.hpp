#ifndef BaalCommon_hpp
#define BaalCommon_hpp

#include <climits>
#include <string>
#include <vector>
#include <iostream>
#include <iterator>
#include <cstdlib>

#include <boost/range.hpp>

// Put simple, generic free functions in this file

namespace baal {

class Engine;

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
bool contains(const T& t, const Container& container)
///////////////////////////////////////////////////////////////////////////////
{
  return container.find(t) != container.end();
}

///////////////////////////////////////////////////////////////////////////////
template <class T>
inline
bool contains(const T& t, const std::vector<T>& container)
///////////////////////////////////////////////////////////////////////////////
{
  return std::find(std::begin(container), std::end(container), t) !=
         container.end();
}

///////////////////////////////////////////////////////////////////////////////
std::vector<std::string> split(const std::string& str,
                               const std::string& sep);
///////////////////////////////////////////////////////////////////////////////

//
// AdjacentLocationIterator and associated free functions
//

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
