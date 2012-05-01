#include "InterfaceText.hpp"
#include "Engine.hpp"
#include "World.hpp"
#include "BaalExceptions.hpp"
#include "BaalCommon.hpp"
#include "CommandFactory.hpp"
#include "Command.hpp"
#include "Player.hpp"
#include "PlayerAI.hpp"
#include "Util.hpp"
#include "Weather.hpp"
#include "Geology.hpp"
#include "WorldTile.hpp"

#include <iostream>
#include <string>
#include <cstring>
#include <fstream>
#include <limits>
#include <iomanip>
#include <readline/readline.h>
#include <readline/history.h>
#include <boost/algorithm/string.hpp>

namespace baal {

///////////////////////////////////////////////////////////////////////////////
InterfaceText::InterfaceText(std::ostream& out,
                             std::istream& in,
                             Engine& engine)
///////////////////////////////////////////////////////////////////////////////
 : Interface(),
   m_ostream(out),
   m_istream(in),
   m_is_interactive(&m_ostream == &std::cout && &m_istream == &std::cin),
   m_engine(engine)
{
  Require( !(!m_is_interactive &&
             (&m_ostream == &std::cout || &m_istream == &std::cin)),
           "Cannot be partially interactive");

  if (m_is_interactive) {
    initialize_readline();
  }
}

///////////////////////////////////////////////////////////////////////////////
InterfaceText::~InterfaceText()
///////////////////////////////////////////////////////////////////////////////
{
  std::ofstream*      outfile = dynamic_cast<std::ofstream*>(&m_ostream);
  std::ifstream*      infile  = dynamic_cast<std::ifstream*>(&m_istream);
#ifndef NDEBUG
  std::ostringstream* outss = dynamic_cast<std::ostringstream*>(&m_ostream);
  std::istringstream* inss  = dynamic_cast<std::istringstream*>(&m_istream);
#endif

  // Check assumptions: that both streams are one of:
  //   file stream, string stream, or cin/cout
  Require(outfile != nullptr || outss != nullptr || m_ostream == &std::cout,
          "m_ostream is not file-stream, string-stream, or cout");
  Require(infile != nullptr || inss != nullptr || m_istream == &std::cin,
          "m_istream is not file-stream, string-stream, or cin");

  // File streams need to be closed

  if (outfile != nullptr) {
    outfile->close();
  }

  if (infile != nullptr) {
    infile->close();
  }

  // Streams other than cout/cin need to be deleted

  if (&m_ostream != &std::cout) {
    delete &m_ostream;
  }

  if (&m_istream != &std::cin) {
    delete &m_istream;
  }
}

///////////////////////////////////////////////////////////////////////////////
void InterfaceText::draw()
///////////////////////////////////////////////////////////////////////////////
{
  // DESIGN: Should "drawable" items know how to draw themselves? That might
  // reduce coupling between those classes and the interface classes.

  if (m_is_interactive) {
    clear_screen();
  }

  // Draw world
  draw(m_engine.world());

  print("\n");

  // Draw Player
  draw(m_engine.player());

  print("\n");

  // Draw AI Player
  draw(m_engine.ai_player());
}

///////////////////////////////////////////////////////////////////////////////
void InterfaceText::help(const std::string& helpmsg)
///////////////////////////////////////////////////////////////////////////////
{
  print(helpmsg + "\n");
}

///////////////////////////////////////////////////////////////////////////////
void InterfaceText::spell_report(const std::string& report)
///////////////////////////////////////////////////////////////////////////////
{
  print(stream("!! ") << report << "\n", RED);
}

///////////////////////////////////////////////////////////////////////////////
void InterfaceText::interact()
///////////////////////////////////////////////////////////////////////////////
{
  // Get handle to command factory
  const CommandFactory& cmd_factory = CommandFactory::instance();

  // Enter loop for this turn
  if (m_is_interactive) {
    // readline library is in C - here comes the diarrhea
    char *line = nullptr;

    while (m_end_turns == 0) {
      m_ostream.flush();
      // Grab a line of text
      line = readline("% ");
      if (line == nullptr){
        // User ctrl-d
        m_engine.quit();
        break;
      }

      // Add to history and process if not empty string
      if (std::strlen(line) > 0) {
        add_history(line);
        std::string command_str(line);
        boost::trim(command_str);
        try {
          auto command =
            cmd_factory.parse_command(command_str, m_engine);
          command->apply();
        }
        catch (UserError& error) {
          print(std::string("ERROR: ") + error.what() + "\n");
          print("\nType: 'help [command]' for assistence\n");
        }
      }
      free(line);
    }
  }
  else {
    // non-interactive, skip all the readline crap
    while (m_end_turns == 0) {
      std::string command_str;
      if (!std::getline(m_istream, command_str)) {
        // User ctrl-d
        m_engine.quit();
        break;
      }
      else if (command_str.empty()) {
        continue;
      }

      try {
        auto command =
          cmd_factory.parse_command(command_str, m_engine);
        command->apply();
      }
      catch (UserError& error) {
        print(std::string("ERROR: ") + error.what() + "\n");
        print("\nType: 'help [command]' for assistence\n");
      }
    }
    m_ostream.flush();
  }

  --m_end_turns;
}

///////////////////////////////////////////////////////////////////////////////
void InterfaceText::human_wins()
///////////////////////////////////////////////////////////////////////////////
{
  print("YOU'RE WINNAR!!\n", GREEN);
}

///////////////////////////////////////////////////////////////////////////////
void InterfaceText::ai_wins()
///////////////////////////////////////////////////////////////////////////////
{
  print("YOU'RE LOZER!!\n", RED);
}

//
// How to draw various items...
//

namespace {

static const char* RED    = InterfaceText::RED;
static const char* GREEN  = InterfaceText::GREEN;
static const char* YELLOW = InterfaceText::YELLOW;
static const char* BLUE   = InterfaceText::BLUE;
static const char* WHITE  = InterfaceText::WHITE;

static const int MAX_INT = std::numeric_limits<int>::max();
static std::map<DrawMode, std::vector<std::pair<int, const char*> > >
ATMOS_FIELD_COLOR_MAP =
{
  {WIND,        { {10 , GREEN}, {20  , YELLOW}, {MAX_INT, RED}   } },
  {DEWPOINT,    { {32 , RED},   {55  , YELLOW}, {MAX_INT, GREEN} } },
  {TEMPERATURE, { {32 , BLUE},  {80  , YELLOW}, {MAX_INT, RED}   } },
  {PRESSURE,    { {975, GREEN}, {1025, YELLOW}, {MAX_INT, RED}   } },
  {PRECIP,      { {2  , RED},   {10  , YELLOW}, {MAX_INT, GREEN} } }
};

///////////////////////////////////////////////////////////////////////////////
double get_field_for_draw_mode(const Atmosphere& atmos, DrawMode mode)
///////////////////////////////////////////////////////////////////////////////
{
  switch (mode) {
  case WIND:
    return atmos.wind().m_speed;
  case DEWPOINT:
    return atmos.dewpoint();
  case TEMPERATURE:
    return atmos.temperature();
  case PRESSURE:
    return atmos.pressure();
  case PRECIP:
    return atmos.precip();
  default:
    Require(false, "Bad draw-mode: " << mode);
    return 0;
  }
}

typedef std::pair<char, const char*> tile_draw_spec;
typedef std::pair<std::string, const char*> geology_draw_spec;

///////////////////////////////////////////////////////////////////////////////
tile_draw_spec get_draw_spec(const WorldTile& tile)
///////////////////////////////////////////////////////////////////////////////
{
  if (dynamic_cast<const OceanTile*>(&tile) != nullptr) {
    return std::make_pair('~', BLUE);
  }
  else if (dynamic_cast<const MountainTile*>(&tile) != nullptr) {
    return std::make_pair('^', WHITE);
  }
  else if (dynamic_cast<const DesertTile*>(&tile) != nullptr) {
    return std::make_pair('-', YELLOW);
  }
  else if (dynamic_cast<const TundraTile*>(&tile) != nullptr) {
    return std::make_pair('-', WHITE);
  }
  else if (dynamic_cast<const HillsTile*>(&tile) != nullptr) {
    return std::make_pair('^', GREEN);
  }
  else if (dynamic_cast<const PlainsTile*>(&tile) != nullptr) {
    return std::make_pair('_', GREEN);
  }
  else if (dynamic_cast<const LushTile*>(&tile) != nullptr) {
    return std::make_pair('=', GREEN);
  }
  else {
    Require(false, "No spec for this type of world tile");
  }
}

///////////////////////////////////////////////////////////////////////////////
geology_draw_spec get_draw_spec(const Geology& geology)
///////////////////////////////////////////////////////////////////////////////
{
  if (dynamic_cast<const Divergent*>(&geology) != nullptr) {
    return std::make_pair(" <-->", BLUE);
  }
  else if (dynamic_cast<const Subducting*>(&geology) != nullptr) {
    return std::make_pair(" -v<-", RED);
  }
  else if (dynamic_cast<const Orogenic*>(&geology) != nullptr) {
    return std::make_pair(" -><-", GREEN);
  }
  else if (dynamic_cast<const Transform*>(&geology) != nullptr) {
    return std::make_pair(" vv^^", YELLOW);
  }
  else if (dynamic_cast<const Inactive*>(&geology) != nullptr) {
    return std::make_pair("     ", WHITE);
  }
  else {
    Require(false, "No spec for this type of geology");
  }
}

}

///////////////////////////////////////////////////////////////////////////////
void InterfaceText::draw(const Geology& geology)
///////////////////////////////////////////////////////////////////////////////
{
  const char* invalid_color = "";
  const char* color = invalid_color;
  std::string invalid_symbol = "????";
  std::string symbol = invalid_symbol;
  float property;
  std::ostringstream oss;
  auto draw_spec = get_draw_spec(geology);

  switch (m_draw_mode) {
  case GEOLOGY:
    symbol = draw_spec.first;
    color  = draw_spec.second;
    break;
  case TENSION:
  case MAGMA:
    property = m_draw_mode == TENSION ? geology.tension() : geology.magma();
    if (property < .333) {
      color = GREEN;
    }
    else if (property < .666) {
      color = YELLOW;
    }
    else {
      color = RED;
    }
    oss << std::setw(TILE_TEXT_WIDTH)
        << std::setprecision(3)
        << std::fixed
        << property;
    symbol = oss.str();
    break;
  default:
    Require(false, "Should not draw geology in mode: " << m_draw_mode);
  }

  Require(color  != invalid_color , "Color was not set");
  Require(symbol != invalid_symbol, "Symbol was not set");
  Require(symbol.size() <= TILE_TEXT_WIDTH,
          "Symbol '" << symbol << "' is too long");

  print(symbol, color);
}

///////////////////////////////////////////////////////////////////////////////
void InterfaceText::draw(const Player& player)
///////////////////////////////////////////////////////////////////////////////
{
  print("PLAYER STATS:\n");
  print(stream("  name: ")  << player.name() << "\n");
  print(stream("  level: ") << player.level() << "\n", GREEN);
  print(stream("  mana: ")  << player.mana() << "/" << player.max_mana() << "\n", BLUE);
  print(stream("  exp: ")   << player.exp() << "/" << player.next_level_cost() << "\n", YELLOW);
}

///////////////////////////////////////////////////////////////////////////////
void InterfaceText::draw(const PlayerAI& ai)
///////////////////////////////////////////////////////////////////////////////
{
  print("AI PLAYER STATS:\n");
  print(stream("  tech level: ") << ai.tech_level() << "\n", GREEN);
  print(stream("  population: ") << ai.population() << "\n", BLUE);
}

///////////////////////////////////////////////////////////////////////////////
void InterfaceText::draw(const Time& time)
///////////////////////////////////////////////////////////////////////////////
{
  Season season = time.season();
  std::string name;
  const char* color = nullptr;

  switch(season) {
  case WINTER:
    name = "Winter";
    color = BLUE;
    break;
  case SPRING:
    name = "Spring";
    color = GREEN;
    break;
  case SUMMER:
    name = "Summer";
    color = RED;
    break;
  case FALL:
    name = "Fall";
    color = YELLOW;
    break;
  default:
    Require(false, "Unknown season: " << season);
  }

  print(stream(name) << ", Year " << time.year() << "\n", color);
}

///////////////////////////////////////////////////////////////////////////////
void InterfaceText::draw(const Atmosphere& atmos)
///////////////////////////////////////////////////////////////////////////////
{
  const double field_value = get_field_for_draw_mode(atmos, m_draw_mode);
  for(auto spec : ATMOS_FIELD_COLOR_MAP[m_draw_mode]) {
    const int upper_bound = spec.first;
    const char* color     = spec.second;
    if (field_value < upper_bound) {
      print(stream()
            << std::setw(TILE_TEXT_WIDTH)
            << std::setprecision(3)
            << std::fixed
            << field_value,
            color);
      return;
    }
  }
  Require(false,
        "Failed to find color for " << m_draw_mode << ", val " << field_value);
}

///////////////////////////////////////////////////////////////////////////////
void InterfaceText::draw(const Anomaly& anomaly)
///////////////////////////////////////////////////////////////////////////////
{
  print(stream("Level: ") << Anomaly::type_to_str(anomaly.type())
        << anomaly.intensity() << " "
        << Anomaly::category_to_str(anomaly.category())
        << " anomaly at location: " << anomaly.location());
}

///////////////////////////////////////////////////////////////////////////////
void InterfaceText::draw(const World& world)
///////////////////////////////////////////////////////////////////////////////
{
  DrawMode real_draw_mode = m_draw_mode;

  // Draw time
  draw(world.time());

  // Make room for row labels
  print("  ");

  // Draw column labels. Need to take 1 char space separator into account.
  {
    unsigned ws_lead = TILE_TEXT_WIDTH / 2;
    unsigned col_width = TILE_TEXT_WIDTH - ws_lead + 1; // 1->sep
    for (unsigned col = 0; col < world.width(); ++col) {
      for (unsigned w = 0; w < ws_lead; ++w) {
        print(" ");
      }
      print(stream() << std::left << std::setw(col_width) << col);
    }
    print("\n");
  }

  // Draw tiles
  for (unsigned row = 0; row < world.height(); ++row) {
    for (unsigned height = 0; height < TILE_TEXT_HEIGHT; ++height) {
      // Middle of tile displays "overlay" info, for the rest of the tile,
      // just draw the land.
      if (height == TILE_TEXT_HEIGHT / 2) {
        m_draw_mode = real_draw_mode;
        print(stream() << row << " ");  // row labels
      }
      else {
        m_draw_mode = LAND; // temporary override of draw mode
        print("  ");
      }

      for (unsigned col = 0; col < world.width(); ++col) {
        draw(world.get_tile(Location(row, col)));
        print(" ");
      }
      print("\n");
    }
    print("\n");
  }
  m_draw_mode = real_draw_mode;

  // Draw recent anomalies
  for (const Anomaly* anomaly : world.anomalies()) {
    draw(*anomaly);
    print("\n");
  }
}

///////////////////////////////////////////////////////////////////////////////
void InterfaceText::draw_land(const WorldTile& tile)
///////////////////////////////////////////////////////////////////////////////
{
  auto draw_spec = get_draw_spec(tile);
  for (unsigned w = 0; w < TILE_TEXT_WIDTH; ++w) {
    print(stream() << draw_spec.first, draw_spec.second);
  }
}

///////////////////////////////////////////////////////////////////////////////
void InterfaceText::draw(const WorldTile& tile)
///////////////////////////////////////////////////////////////////////////////
{
  if (m_draw_mode == LAND) {
    draw_land(tile);
  }
  else if (m_draw_mode == CIV) {
    if (tile.city() != nullptr) {
      print(stream(" C:") << std::setw(TILE_TEXT_WIDTH - 3)
            << tile.city()->rank(),
            RED);
    }
    else if (tile.infra_level() > 0) {
      print(stream(" I:") << std::setw(TILE_TEXT_WIDTH- 3) <<
            tile.infra_level(),
            YELLOW);
    }
    else {
      draw_land(tile);
    }
  }
  else if (m_draw_mode == MOISTURE) {
    const FoodTile* tile_ptr =
      dynamic_cast<const FoodTile*>(this);
    if (tile_ptr != nullptr) {
      float moisture = tile_ptr->soil_moisture();
      const char* color = nullptr;
      if (moisture < 1.0) {
        color = YELLOW;
      }
      else if (moisture < FoodTile::FLOODING_THRESHOLD) {
        color = GREEN;
      }
      else if (moisture < FoodTile::TOTALLY_FLOODED) {
        color = BLUE;
      }
      else {
        color = RED;
      }
      print(stream() << std::setprecision(3) << std::setw(TILE_TEXT_WIDTH) << moisture,
            color);
    }
    else {
      draw_land(tile);
    }
  }
  else if (m_draw_mode == YIELD) {
    Yield y = tile.yield();
    if (y.m_food > 0) {
      print(stream() << std::setprecision(3) << std::setw(TILE_TEXT_WIDTH) << y.m_food,
            GREEN);
    }
    else {
      print(stream() << std::setprecision(3) << std::setw(TILE_TEXT_WIDTH) << y.m_prod,
            RED);
    }
  }
  else if (Geology::is_geological(m_draw_mode)) {
    draw(tile.geology());
  }
  else if (Atmosphere::is_atmospheric(m_draw_mode)) {
    draw(tile.atmosphere());
  }
  else {
    Require(false, "Unhandled mode: " << m_draw_mode);
  }
}

///////////////////////////////////////////////////////////////////////////////
void InterfaceText::print(const std::string& string, const char* color)
///////////////////////////////////////////////////////////////////////////////
{
  if (m_is_interactive && color != nullptr) {
    m_ostream << BOLD_COLOR << color << string << CLEAR_ALL;
  }
  else {
    m_ostream << string;
  }
}

}
