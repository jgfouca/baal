#define private public

#include "Configuration.hpp"
#include "Engine.hpp"
#include "InterfaceText.hpp"
#include "InterfaceFactory.hpp"
#include "World.hpp"
#include "Geology.hpp"
#include "Player.hpp"
#include "PlayerAI.hpp"
#include "World.hpp"

#include <gtest/gtest.h>
#include <string>
#include <memory>

namespace {

struct checker
{
  baal::InterfaceText& m_interface;
  std::ostringstream&  m_stream;

  checker(baal::InterfaceText& interface,
          std::ostringstream& stream) :
    m_interface(interface),
    m_stream(stream)
  {}

  template <typename Drawable>
  void check_output(const Drawable& item, const std::string& expected)
  {
    m_interface.draw(item);
    EXPECT_EQ(expected, m_stream.str());
    m_stream.clear();
    m_stream.str("");
  }
};

TEST(InterfaceText, basic)
{
  using namespace baal;


  // Set up a game engine with default settings except
  // send all text to an ostringstream
  Configuration config(InterfaceFactory::TEXT_INTERFACE +
                       InterfaceFactory::SEPARATOR +
                       InterfaceFactory::TEXT_WITH_OSTRINGSTREAM +
                       InterfaceFactory::SEPARATOR +
                       "/dev/null");
  auto engine = create_engine(config);

  InterfaceText& interface = dynamic_cast<InterfaceText&>(engine->interface());
  std::ostringstream& stream = dynamic_cast<std::ostringstream&>(interface.m_ostream);
  checker check(interface, stream);

  Subducting geology(1.0);
  interface.set_draw_mode(GEOLOGY);
  check.check_output(geology, " -v<-");
  interface.set_draw_mode(CIV);

  auto& player = engine->player();
  check.check_output(player, "PLAYER STATS:\n  name: human\n  level: 1\n  mana: 100/100\n  exp: 0/100\n");

  auto& ai_player = engine->ai_player();
  check.check_output(ai_player, "AI PLAYER STATS:\n  tech level: 1\n  population: 0\n");

  auto& tile = engine->world().get_tile(Location(0,0));
  check.check_output(tile, "-----");

  auto& time = engine->world().time();
  check.check_output(time, "Winter, Year 0\n");

  auto& atmos = tile.atmosphere();
  interface.set_draw_mode(TEMPERATURE);
  check.check_output(atmos, "30.000");
  interface.set_draw_mode(CIV);

  {
    const std::string expected = "help message\n";
    interface.help("help message");
    EXPECT_EQ(expected, stream.str());

    stream.clear();
    stream.str("");
  }

  {
    const std::string expected = "!! many slain\n";
    interface.spell_report("many slain");
    EXPECT_EQ(expected, stream.str());

    stream.clear();
    stream.str("");
  }
}

}
