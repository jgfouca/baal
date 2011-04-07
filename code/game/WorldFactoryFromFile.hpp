#ifndef WorldFactoryFromFile_hpp
#define WorldFactoryFromFile_hpp

#include "World.hpp"

#include <string>
#include <libxml/parser.h>

namespace baal {

class WorldFactoryFromFile
{
 public:
  static World& create(const std::string& mapfilename);

  static const std::string WORLD_FILE_EXT;

 private:
  WorldFactoryFromFile(const char* mapfilename)
    : m_mapfilename(mapfilename)
  {}

  // Internal methods

  World& load();

  char* get_element(const char *elemname);

  int get_int_from_parent(const char *elemname);

  unsigned get_unsigned_from_parent(const char *elemname);

  float get_float_from_parent(const char *elemname);

  WorldTile& parse_Tile();

  Climate& get_Climate_from_parent();

  Wind get_Wind_from_parent();

  Direction get_Direction_from_parent();

  Geology& get_Geology_from_parent();

  // Members

  const char* m_mapfilename;
  xmlDocPtr m_mapfile;
  xmlNodePtr m_curr_node;
};

bool is_baal_map_file(const std::string& filename);

}

#endif