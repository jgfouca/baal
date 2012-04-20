#ifndef InterfaceText_hpp
#define InterfaceText_hpp

#include "Interface.hpp"

#include <iosfwd>
#include <sstream>

namespace baal {

class Engine;
class Geology;
class Player;
class PlayerAI;
class Time;
class Atmosphere;
class Anomaly;
class World;
class WorldTile;

/**
 * Text-based implementation of an interface
 */
class InterfaceText : public Interface
{
 public:
  InterfaceText(std::ostream& out, std::istream& in, Engine& engine);

  ~InterfaceText();

  virtual void draw();

  // Draw individual items
  virtual void draw(const Geology&);
  virtual void draw(const Player&);
  virtual void draw(const PlayerAI&);
  virtual void draw(const Time&);
  virtual void draw(const Atmosphere&);
  virtual void draw(const Anomaly&);
  virtual void draw(const World&);
  virtual void draw(const WorldTile&);

  virtual void interact();

  virtual void help(const std::string& helpmsg);

  virtual void spell_report(const std::string& report);

  virtual void human_wins();

  virtual void ai_wins();

  // Common ascii colors
  static constexpr const char* BLACK   = "30m";
  static constexpr const char* RED     = "31m";
  static constexpr const char* GREEN   = "32m";
  static constexpr const char* YELLOW  = "33m";
  static constexpr const char* BLUE    = "34m";
  static constexpr const char* MAGENTA = "35m";
  static constexpr const char* CYAN    = "36m";
  static constexpr const char* WHITE   = "37m";

 private:
  struct stream_wrap
  {
    stream_wrap() : m_stream() {}

    template <typename T>
    explicit
    stream_wrap(const T& t) : m_stream()
    { m_stream << t; }

    template <typename T>
    stream_wrap&
    operator<<(const T& t)
    {
      m_stream << t;
      return *this;
    }

    std::string str() const
    { return m_stream.str(); }

    std::ostringstream m_stream;
  };

  typedef stream_wrap stream;

  void print(const std::string& string, const char* color = nullptr);
  void print(const stream& out,         const char* color = nullptr)
  { print(out.str(), color); }

  void draw_land(const WorldTile& tile);

  std::ostream& m_ostream;
  std::istream& m_istream;
  const bool    m_is_interactive;
  Engine&       m_engine;

  // Ascii bold prefix/postfix
  static constexpr const char* BOLD_COLOR = "\033[1;";
  static constexpr const char* CLEAR_ALL  = "\033[0m";

  static const unsigned TILE_TEXT_HEIGHT = 5;
  static const unsigned TILE_TEXT_WIDTH = 5;
};

}

#endif
