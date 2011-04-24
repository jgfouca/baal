#include "World.hpp"
#include "City.hpp"

#include <algorithm>
#include <iostream>
#include <iomanip>

using namespace baal;

///////////////////////////////////////////////////////////////////////////////
World::World(unsigned width, unsigned height)
///////////////////////////////////////////////////////////////////////////////
  : m_width(width),
    m_height(height),
    m_time()
{
  for (unsigned i = 0; i < height; ++i) {
    std::vector<WorldTile*> row(width, NULL);
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
void World::draw_text(std::ostream& out) const
///////////////////////////////////////////////////////////////////////////////
{
  DrawMode real_draw_mode = s_draw_mode;

  // Draw time
  m_time.draw_text(out);

  // Make room for row labels
  out << "  ";

  // Draw column labels. Need to take 1 char space separator into account.
  {
    unsigned ws_lead = WorldTile::TILE_TEXT_WIDTH / 2;
    unsigned col_width = WorldTile::TILE_TEXT_WIDTH - ws_lead + 1; // 1->sep
    for (unsigned col = 0; col < width(); ++col) {
      for (unsigned w = 0; w < ws_lead; ++w) {
        out << " ";
      }
      out << std::left << std::setw(col_width) << col;
    }
    out << std::right << "\n";
  }

  // Draw tiles
  for (unsigned row = 0; row < height(); ++row) {
    for (unsigned height = 0; height < WorldTile::TILE_TEXT_HEIGHT; ++height) {
      // Middle of tile displays "overlay" info, for the rest of the tile,
      // just draw the land.
      if (height == WorldTile::TILE_TEXT_HEIGHT / 2) {
        s_draw_mode = real_draw_mode;
        out << row << " ";  // row labels
      }
      else {
        s_draw_mode = LAND;
        out << "  ";
      }

      for (unsigned col = 0; col < width(); ++col) {
        m_tiles[row][col]->draw_text(out);
        out << " ";
      }
      out << "\n";
    }
    out << "\n";
  }
  s_draw_mode = real_draw_mode;

  // Draw recent anomalies
  for (std::vector<const Anomaly*>::const_iterator
       itr = m_recent_anomalies.begin();
       itr != m_recent_anomalies.end();
       ++itr) {
    (*itr)->draw_text(out);
    out << "\n";
  }
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
  City* new_city = new City(name, location);
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
