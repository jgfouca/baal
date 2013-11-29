#ifndef WorldFactoryFromFile_hpp
#define WorldFactoryFromFile_hpp

#include "World.hpp"

#include <string>
#include <memory>

#include <libxml/parser.h>

namespace baal {

class Engine;

class WorldFactoryFromFile
{
 public:
  static std::shared_ptr<World> create(const std::string& mapfilename, Engine& engine);

  static const std::string WORLD_FILE_EXT;

 private:
  WorldFactoryFromFile(const char* mapfilename, Engine& engine)
    : m_mapfilename(mapfilename),
      m_engine(engine)
  {}

  // Internal methods

  std::shared_ptr<World> load();

  char* get_element(const char *elemname);

  template <typename T>
  T get_data_from_parent(const char *elemname)
  {
    char *s = get_element(elemname);
    T data;
    std::istringstream(s) >> data;
    return data;
  }

  template <typename T>
  std::vector<T> get_data_per_season_from_parent(const char* elemname)
  {
    char *s = get_element(elemname);
    std::vector<T> data(4);
    Require(size<Season>() == 4, "Invalid assumption in this code");
    std::istringstream(s) >> data[0];// >> " " >> data[1] >> " " >> data[2] >> " " >> data[3];
    return data;
  }

  WorldTile& parse_Tile(int row, int col);

  Climate& get_Climate_from_parent();

  Geology& get_Geology_from_parent();

  // Members

  const char* m_mapfilename;
  xmlDocPtr   m_mapfile;
  xmlNodePtr  m_curr_node;
  Engine&     m_engine;
};

bool is_baal_map_file(const std::string& filename);

}

#endif
