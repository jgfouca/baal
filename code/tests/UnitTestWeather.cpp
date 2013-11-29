#include "Weather.hpp"
#include "Engine.hpp"
#include "World.hpp"

#include <gtest/gtest.h>
#include <sstream>

namespace {

TEST(Weather, Wind)
{
  using namespace baal;

  Wind w(10, N);
  w += 10;
  EXPECT_EQ(w.m_speed, 20);

  std::ostringstream oss;
  oss << w;
  EXPECT_EQ(oss.str(), std::string("N   20"));

  std::istringstream iss(oss.str());
  Wind wind_input;
  iss >> wind_input;

  EXPECT_EQ(w, wind_input);
}

TEST(Weather, Anomaly)
{
  using namespace baal;

  auto engine = baal::create_engine();
  World& world = engine->world();

  Location loc(0,0);
  std::shared_ptr<const Anomaly> anom;
  while (anom == nullptr) {
    anom = Anomaly::generate_anomaly(PRECIP_ANOMALY,
                                     loc,
                                     world);
  }

  float precip_effect = anom->precip_effect(loc);
  EXPECT_LE(std::abs(anom->intensity()), Anomaly::MAX_INTENSITY);
  EXPECT_EQ(PRECIP_ANOMALY, anom->category());
  EXPECT_LT(precip_effect, 1.0);
  EXPECT_EQ(0, anom->temp_effect(loc));
  EXPECT_EQ(0, anom->pressure_effect(loc));
  EXPECT_EQ(1.0, anom->precip_effect(Location(1,0)));
}

TEST(Weather, weather)
{
  using namespace baal;

  auto engine = baal::create_engine();
  World& world = engine->world();

  std::vector<int> temps {60, 70, 80, 70};
  std::vector<float> precips {1, 2, 3, 4};
  std::vector<Wind> winds(4, Wind(10, NNW));

  Climate climate(temps, precips, winds);
  Atmosphere atmosphere(climate);

  std::vector<std::shared_ptr<const Anomaly> > anomalies;
  Location loc(0, 0);

  for (int i = 0; i < 2; ++i) {
    for (Season s : iterate<Season>()) {
      atmosphere.cycle_turn(anomalies, loc, s);

      EXPECT_EQ(climate.temperature(s), temps[s]);
      EXPECT_EQ(climate.temperature(s), atmosphere.temperature());
      EXPECT_EQ(climate.precip(s), precips[s]);
      EXPECT_EQ(climate.precip(s), atmosphere.precip());
      EXPECT_EQ(climate.wind(s), winds[s]);
      EXPECT_EQ(climate.wind(s), atmosphere.wind());
    }
  }

  std::shared_ptr<const Anomaly> anom;
  while (anom == nullptr || anom->intensity() < 0) {
    anom = Anomaly::generate_anomaly(TEMPERATURE_ANOMALY,
                                     loc,
                                     world);
  }
  anomalies.push_back(anom);

  atmosphere.cycle_turn(anomalies, loc, WINTER);

  EXPECT_GT(atmosphere.temperature(), temps[WINTER]);
  EXPECT_EQ(atmosphere.precip(), precips[WINTER]);
  EXPECT_EQ(atmosphere.wind(), winds[WINTER]);
}

}
