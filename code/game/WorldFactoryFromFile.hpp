#ifndef WorldFactoryFromFile_hpp
#define WorldFactoryFromFile_hpp

#include "World.hpp"
#include <libxml/parser.h>

namespace baal {

class World;

class WorldFactoryFromFile
{
 public:
  static World& create(char * mapfilename);

 private:
  static World& load_world_from_file(char * mapfilename);
  static char* get_element(xmlDocPtr mapfile, xmlNodePtr cur, const char *elemname);
  static int get_int_from_parent(xmlDocPtr mapfile, xmlNodePtr cur, const char *elemname);
  static unsigned get_unsigned_from_parent(xmlDocPtr mapfile, xmlNodePtr cur, const char *elemname);
  static float get_float_from_parent(xmlDocPtr mapfile, xmlNodePtr cur, const char *elemname);
  static WorldTile* parse_Tile(xmlDocPtr mapfile, xmlNodePtr cur);
  static Climate& get_Climate_from_parent(xmlDocPtr mapfile, xmlNodePtr cur);
  static Wind get_Wind_from_parent(xmlDocPtr mapfile, xmlNodePtr cur);
  static Direction get_Direction_from_parent(xmlDocPtr mapfile, xmlNodePtr cur);
  static Geology& get_Geology_from_parent(xmlDocPtr mapfile, xmlNodePtr cur);
};

}

#endif
