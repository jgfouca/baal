#ifndef WorldTile_hpp
#define WorldTile_hpp

namespace baal {

enum TileType {MTN, PLAIN, OCEAN, UNDEFINED};

class WorldTile
{
 public:
  WorldTile() : m_type(UNDEFINED) {}

  WorldTile(TileType type) : m_type(type) {}

  TileType type() const { return m_type; }
 private:
  TileType m_type;
};

}

#endif
