#include "World.hpp"
#include "City.hpp"

#include <iostream>

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
  m_time.draw_text(out);
  for (unsigned row = 0; row < height(); ++row) {
    for (unsigned col = 0; col < width(); ++col) {
      m_tiles[row][col]->draw_text(out);
    }
    out << "\n";
  }
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

  // Phase 2 of World turn-cycle: Simulate the inter-turn (long-term) weather
  // Every turn, the weather since the last turn will be randomly simulated.
  // There will be random abnormal areas, with the epicenter of the abnormality
  // having the most extreme deviations from the normal climate and peripheral
  // tiles having smaller deviations from normal.
  // Long-term abnormalilty types are: drought, moist, cold, hot.
  // Based on our model of time, we are at the beginning of the current
  // season, so anomalies affect this season; that is why time is not
  // incremented until later.
  for (unsigned row = 0; row < height(); ++row) {
    for (unsigned col = 0; col < width(); ++col) {
      m_tiles[row][col]->cycle_turn( /*TODO: Pass anomaly vector*/);
    }
  }

  // Phase 3: Increment time
  ++m_time;

  // Phase 4: Simulate short-term weather conditions for next turn
  // We need to initialize weather conditions for the next turn. Again we
  // will use the notion of abnormalities.
  // Short-term abnormality types are: high/low pressure, hot/cold; wind is
  // derived from pressure abnormalities.
  for (unsigned row = 0; row < height(); ++row) {
    for (unsigned col = 0; col < width(); ++col) {
      // TODO: Manipulate tiles' atmosphere objects
    }
  }
}
