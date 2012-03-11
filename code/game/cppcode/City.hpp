#ifndef City_hpp
#define City_hpp

#include "BaalCommon.hpp"
#include "CityImpl.hpp"

#include <string>
#include <list>
#include <libxml/parser.h>

namespace baal {

/**
 * Represents human-built cities.
 * TODO: describe conceptually how cities work.
 */
class City
{
 public:

  //
  // ==== Public API ====
  //

  City(const std::string& name, Location location) :
    m_impl(name, location)
  {}

  City & operator=(const City&) = delete;
  City(const City&)             = delete;
  City()                        = delete;

  //
  // Query / Getter API
  //

  const std::string& name() const { return m_impl.name(); }

  unsigned population() const { return m_impl.population(); }

  unsigned rank() const { return m_impl.rank(); }

  Location location() const { return m_impl.location(); }

  bool famine() const { return m_impl.famine(); }

  unsigned defense() const { return m_impl.defense(); }

  xmlNodePtr to_xml() const { return m_impl.to_xml(); }

  //
  // Modification API
  //

  /**
   * Tell this city that the turn is cycling
   */
  void cycle_turn() { m_impl.cycle_turn(); }

  /**
   * Kill off some of this city's citizens
   */
  void kill(unsigned killed) { m_impl.kill(killed); }

  /**
   * Destroy city defenses, lowering them by an amount equal to the
   * 'levels' parameter.
   */
  void destroy_defense(unsigned levels) { m_impl.destroy_defense(levels); }

  //
  // ==== Class constants ====
  //

  static constexpr unsigned MIN_CITY_SIZE =
    details::CityImpl::CITY_STARTING_POP / 5;

  //
  // ==== Private members ====
  //
 private:
  details::CityImpl m_impl;
};

}

#endif
