#include "World.hpp"
#include "City.hpp"

#include <algorithm>
#include <iostream>
#include <iomanip>

namespace baal {

///////////////////////////////////////////////////////////////////////////////
World::World(unsigned width, unsigned height, Engine& engine)
///////////////////////////////////////////////////////////////////////////////
  : m_width(width),
    m_height(height),
    m_engine(engine)
{
  for (unsigned i = 0; i < height; ++i) {
    std::vector<WorldTile*> row(width, nullptr);
    m_tiles.push_back(row);
  }
}

///////////////////////////////////////////////////////////////////////////////
World::~World()
///////////////////////////////////////////////////////////////////////////////
{
  for (unsigned row = 0; row < height(); ++row) {
    for (unsigned col = 0; col < width(); ++col) {
      delete m_tiles[row][col];
    }
  }

  clear_anomalies();
}

///////////////////////////////////////////////////////////////////////////////
Location World::get_location(const WorldTile& tile) const
///////////////////////////////////////////////////////////////////////////////
{
  for (unsigned row = 0; row < height(); ++row) {
    for (unsigned col = 0; col < width(); ++col) {
      if (m_tiles[row][col] == &tile) {
        return Location(row, col);
      }
    }
  }
  Require(false, "Failed to find tile");
  return Location();
}

///////////////////////////////////////////////////////////////////////////////
void World::cycle_turn()
///////////////////////////////////////////////////////////////////////////////
{
  // Phase 1: Generate anomalies.
  // TODO: How to handle overlapping anomalies of same category?
  clear_anomalies();
  for (unsigned row = 0; row < height(); ++row) {
    for (unsigned col = 0; col < width(); ++col) {
      Location location(row, col);
      for (Anomaly::AnomalyCategory anom_itr = Anomaly::FIRST; ; ++anom_itr) {
        const Anomaly* anomaly = Anomaly::generate_anomaly(anom_itr,
                                                           location,
                                                           *this);
        if (anomaly) {
          m_recent_anomalies.push_back(anomaly);
        }

        if (anom_itr == Anomaly::LAST) {
          break;
        }
      }
    }
  }

  // Phase 2 of World turn-cycle: Simulate the inter-turn (long-term) weather
  // Every turn, the weather since the last turn will be randomly simulated.
  // There will be random abnormal areas, with the epicenter of the abnormality
  // having the most extreme deviations from the normal climate and peripheral
  // tiles having smaller deviations from normal.
  // Abnormalilty types are: drought, moist, cold, hot, high/low pressure
  // Based on our model of time, we are at the beginning of the current
  // season, so anomalies affect this season; that is why time is not
  // incremented until later.
  // Current conditions for the next turn will be derived from these anomalies.
  for (unsigned row = 0; row < height(); ++row) {
    for (unsigned col = 0; col < width(); ++col) {
      Location location(row, col);
      m_tiles[row][col]->cycle_turn(m_recent_anomalies,
                                    location,
                                    m_time.season());
    }
  }

  // Phase 3: Increment time
  ++m_time;
}

///////////////////////////////////////////////////////////////////////////////
void World::place_city(const std::string& name, const Location& location)
///////////////////////////////////////////////////////////////////////////////
{
  City* new_city = new City(name, location, m_engine);
  WorldTile& tile = get_tile(location);
  dynamic_cast<LandTile&>(tile).place_city(*new_city);
  m_cities.push_back(new_city);
}

///////////////////////////////////////////////////////////////////////////////
void World::place_city(const Location& location)
///////////////////////////////////////////////////////////////////////////////
{
  std::ostringstream out;
  out << "City " << m_cities.size() + 1;
  place_city(out.str(), location);
}

///////////////////////////////////////////////////////////////////////////////
void World::remove_city(City& city)
///////////////////////////////////////////////////////////////////////////////
{
  std::vector<City*>::iterator itr = std::find(m_cities.begin(),
                                               m_cities.end(),
                                               &city);
  Require(itr != m_cities.end(),
          "City '" << city.name() << "' not in m_cities");
  m_cities.erase(itr);

  WorldTile& tile = get_tile(city.location());
  dynamic_cast<LandTile&>(tile).remove_city();
  delete &city;
}

///////////////////////////////////////////////////////////////////////////////
xmlNodePtr World::to_xml()
///////////////////////////////////////////////////////////////////////////////
{
  xmlNodePtr World_node = xmlNewNode(nullptr, BAD_CAST "World");

  /*unsigned m_width;
  unsigned m_height;
  std::vector<std::vector<WorldTile*> > m_tiles;
  Time m_time;
  std::vector<const Anomaly*> m_recent_anomalies;
  std::vector<City*> m_cities;*/

  std::ostringstream width_oss;
  width_oss << m_width;
  xmlNewChild(World_node, nullptr, BAD_CAST "m_width", BAD_CAST width_oss.str().c_str());

  std::ostringstream height_oss;
  height_oss << m_height;
  xmlNewChild(World_node, nullptr, BAD_CAST "m_height", BAD_CAST height_oss.str().c_str());

  // I figure there's an easier Iterator here; not sure how to use it.
  for (unsigned int row = 0; row < m_height; row++) {
    for (unsigned int col = 0; col < m_width; col++) {
      xmlNodePtr Tile_node = m_tiles[row][col]->to_xml();
      std::ostringstream row_oss, col_oss;
      row_oss << row;
      col_oss << col;
      xmlNewChild(Tile_node, nullptr, BAD_CAST "row", BAD_CAST row_oss.str().c_str());
      xmlNewChild(Tile_node, nullptr, BAD_CAST "col", BAD_CAST col_oss.str().c_str());
      xmlAddChild(World_node, Tile_node);
    }
  }

  xmlAddChild(World_node, m_time.to_xml());

  for (std::vector<const Anomaly*>::iterator itr = m_recent_anomalies.begin();
       itr != m_recent_anomalies.end();
       ++itr) {
    const Anomaly* anomaly = *itr;
    xmlAddChild(World_node, anomaly->to_xml());
  }

  for (std::vector<City*>::const_iterator itr = m_cities.begin();
       itr != m_cities.end();
       ++itr) {
    City* city = *itr;
    xmlAddChild(World_node, city->to_xml());
  }

  return World_node;
}

///////////////////////////////////////////////////////////////////////////////
void World::clear_anomalies()
///////////////////////////////////////////////////////////////////////////////
{
  for (std::vector<const Anomaly*>::iterator
       itr = m_recent_anomalies.begin();
       itr != m_recent_anomalies.end();
       ++itr) {
    delete (*itr);
  }
  m_recent_anomalies.clear();
}

}
