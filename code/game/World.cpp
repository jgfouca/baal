#include "World.hpp"
#include "City.hpp"

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
}

///////////////////////////////////////////////////////////////////////////////
void World::cycle_turn()
///////////////////////////////////////////////////////////////////////////////
{
  // Phase 1 of World turn-cycle: Cities are cycled. We want to do this first
  // so that cities feel the "pain" of baal's attacks; otherwise, the tiles
  // would have a a chance to heal before they were harvested, lessening the
  // effect of Baal's attacks.
  for (unsigned row = 0; row < height(); ++row) {
    for (unsigned col = 0; col < width(); ++col) {
      City* city = m_tiles[row][col]->city();
      if (city) {
        city->cycle_turn();
      }
    }
  }

  // Phase 2: Generate anomalies.
  // TODO: How to handle overlapping anomalies of same category?
  std::vector<const Anomaly*> anomalies;
  for (unsigned row = 0; row < height(); ++row) {
    for (unsigned col = 0; col < width(); ++col) {
      Location location(row, col);
      for (Anomaly::AnomalyCategory anom_itr = Anomaly::FIRST; ; ++anom_itr) {
        const Anomaly* anomaly = Anomaly::generate_anomaly(anom_itr,
                                                           location,
                                                           *this);
        if (anomaly) {
          anomalies.push_back(anomaly);
        }

        if (anom_itr == Anomaly::LAST) {
          break;
        }
      }
    }
  }

  // Phase 3 of World turn-cycle: Simulate the inter-turn (long-term) weather
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
      m_tiles[row][col]->cycle_turn(anomalies);
    }
  }

  // Phase 3: Increment time
  ++m_time;
}
