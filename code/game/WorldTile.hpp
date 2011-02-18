#ifndef WorldTile_hpp
#define WorldTile_hpp

#include <sge.h>

namespace baal {

enum TileType {MTN, PLAIN, OCEAN, UNDEFINED};

class WorldTile
{
 public:
  WorldTile() : m_type(UNDEFINED), m_hp(1.0) {}

  WorldTile(TileType type) : m_type(type)
  {
  }

  TileType type() const { return m_type; }

  void setSprite(SGESPRITE *sprite) { m_sprite = sprite; }

  SGESPRITE * sprite() const { return m_sprite; }

  void damage(float dmg) { m_hp *= dmg; }
 private:
  TileType m_type;
  float    m_hp;
  SGESPRITE* m_sprite;
};

}

#endif
