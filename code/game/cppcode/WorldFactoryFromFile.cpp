#include "WorldFactoryFromFile.hpp"
#include "BaalExceptions.hpp"
#include "Weather.hpp"
#include "Geology.hpp"
#include "City.hpp"

#include <string.h>

using namespace baal;

const std::string WorldFactoryFromFile::WORLD_FILE_EXT = ".baalmap";

///////////////////////////////////////////////////////////////////////////////
World& WorldFactoryFromFile::create(const std::string& mapfilename)
///////////////////////////////////////////////////////////////////////////////
{
  WorldFactoryFromFile wfff(mapfilename.c_str());

  return wfff.load();
}

///////////////////////////////////////////////////////////////////////////////
World& WorldFactoryFromFile::load()
///////////////////////////////////////////////////////////////////////////////
{
  m_mapfile = xmlParseFile(m_mapfilename);
  RequireUser(m_mapfile != NULL,
              "Map file " << m_mapfilename << " not parsed successfully.");

  m_curr_node = xmlDocGetRootElement(m_mapfile);
  if (m_curr_node == NULL) {
    xmlFreeDoc(m_mapfile);
    RequireUser(false, "Map file " << m_mapfilename << " is empty.");
  }
  if (xmlStrcmp(m_curr_node->name, (const xmlChar *) "baalmap")) {
    xmlFreeDoc(m_mapfile);
    RequireUser(false, "Map file " << m_mapfilename << " is not a Baal map.");
  }

  int map_width = get_int_from_parent("map_width");
  int map_height = get_int_from_parent("map_height");

  World& world = *(new World(map_width, map_height));
  m_curr_node = m_curr_node->xmlChildrenNode;
  while (m_curr_node != NULL) {
    if (!xmlStrcmp(m_curr_node->name, (const xmlChar *)"tile")) {
      int row = get_int_from_parent("row");
      int col = get_int_from_parent("col");
      world.m_tiles[row][col] = &parse_Tile();
    }
    else if (!xmlStrcmp(m_curr_node->name, (const xmlChar *)"city")) {
      int row = get_int_from_parent("row");
      int col = get_int_from_parent("col");
      char* name = get_element("name");
      LandTile* landtile = dynamic_cast<LandTile*>(world.m_tiles[row][col]);
      RequireUser(landtile != NULL,
                  "Tried to place city at " << row << ", " << col <<
                  "; which is not a landtile");
      world.place_city(name, Location(row, col));
    }
    m_curr_node = m_curr_node->next;
  }

  return world;
}

///////////////////////////////////////////////////////////////////////////////
char* WorldFactoryFromFile::get_element(const char *elemname)
///////////////////////////////////////////////////////////////////////////////
{
  xmlNodePtr curr_node = m_curr_node->xmlChildrenNode;
  while (curr_node != NULL) {
    if (!xmlStrcmp(curr_node->name, (const xmlChar *)elemname)) {
      return (char *)xmlNodeListGetString(m_mapfile,
                                          curr_node->xmlChildrenNode,
                                          1);
    }
    curr_node = curr_node->next;
  }
  RequireUser(false, "Could not find element named " << elemname << ".");
}

///////////////////////////////////////////////////////////////////////////////
int WorldFactoryFromFile::get_int_from_parent(const char *elemname)
///////////////////////////////////////////////////////////////////////////////
{
  char *s = get_element(elemname);
  int r;
  std::istringstream(s) >> r;
  return r;
}

///////////////////////////////////////////////////////////////////////////////
unsigned WorldFactoryFromFile::get_unsigned_from_parent(const char *elemname)
///////////////////////////////////////////////////////////////////////////////
{
  char *s = get_element(elemname);
  unsigned r;
  std::istringstream(s) >> r;
  return r;
}

///////////////////////////////////////////////////////////////////////////////
float WorldFactoryFromFile::get_float_from_parent(const char *elemname)
///////////////////////////////////////////////////////////////////////////////
{
  char *s = get_element(elemname);
  float r;
  std::istringstream(s) >> r;
  return r;
}

///////////////////////////////////////////////////////////////////////////////
WorldTile& WorldFactoryFromFile::parse_Tile()
///////////////////////////////////////////////////////////////////////////////
{
  // m_curr_node is tile
  // Climate(int temperature, unsigned rainfall, Wind wind)

  char *type = get_element("type");
  Climate& climate = get_Climate_from_parent();
  Geology& geology = get_Geology_from_parent();
  if (!strcmp(type, "OceanTile")) {
    unsigned depth = get_unsigned_from_parent("depth");
    return *new OceanTile(depth, climate, geology);
  }
  else if (!strcmp(type, "DesertTile")) {
    return *new DesertTile(climate, geology);
  }
  else if (!strcmp(type, "LushTile")) {
    return *new LushTile(climate, geology);
  }
  else if (!strcmp(type, "MountainTile")) {
    unsigned elevation = get_unsigned_from_parent("elevation");
    return *new MountainTile(elevation, climate, geology);
  }
  else if (!strcmp(type, "TundraTile")) {
    return *new TundraTile(climate, geology);
  }
  else if (!strcmp(type, "PlainsTile")) {
    return *new PlainsTile(climate, geology);
  }
  else if (!strcmp(type, "HillsTile")) {
    return *new HillsTile(climate, geology);
  }
  else {
    Require(false, "Unknown tile type \"" << type << "\".");
  }
}

///////////////////////////////////////////////////////////////////////////////
Climate& WorldFactoryFromFile::get_Climate_from_parent()
///////////////////////////////////////////////////////////////////////////////
{
  // m_curr_node is tile
  xmlNodePtr parent = m_curr_node;
  m_curr_node = m_curr_node->xmlChildrenNode;
  while (m_curr_node != NULL) {
    if (!xmlStrcmp(m_curr_node->name, (const xmlChar *)"Climate")) {
      int temperature = get_int_from_parent("temperature");
      unsigned rainfall = get_unsigned_from_parent("rainfall");
      Wind wind = get_Wind_from_parent();
      m_curr_node = parent;
      return *new Climate(temperature, rainfall, wind);
    }
    m_curr_node = m_curr_node->next;
  }
  Require(false, "Could not find Climate in a tile.");
}

///////////////////////////////////////////////////////////////////////////////
Wind WorldFactoryFromFile::get_Wind_from_parent()
///////////////////////////////////////////////////////////////////////////////
{
  // m_curr_node is Climate
  // Wind(unsigned speed, Direction direction)
  xmlNodePtr parent = m_curr_node;
  m_curr_node = m_curr_node->xmlChildrenNode;

  while (m_curr_node != NULL) {
    if (!xmlStrcmp(m_curr_node->name, (const xmlChar *)"Wind")) {
      unsigned speed = get_unsigned_from_parent("speed");
      Direction direction = get_Direction_from_parent();
      Wind wind(speed, direction);
      m_curr_node = parent;
      return wind;
    }
    m_curr_node = m_curr_node->next;
  }
  RequireUser(false, "Could not find Wind.");
}

///////////////////////////////////////////////////////////////////////////////
Direction WorldFactoryFromFile::get_Direction_from_parent()
///////////////////////////////////////////////////////////////////////////////
{
  char *s = get_element("direction");
  if (!strcmp(s,"N"))        { return N; }
  else if (!strcmp(s,"NNE")) { return NNE; }
  else if (!strcmp(s,"NE"))  { return NE; }
  else if (!strcmp(s,"ENE")) { return ENE; }
  else if (!strcmp(s,"E"))   { return E; }
  else if (!strcmp(s,"ESE")) { return ESE; }
  else if (!strcmp(s,"SE"))  { return SE; }
  else if (!strcmp(s,"SSE")) { return SSE; }
  else if (!strcmp(s,"S"))   { return S; }
  else if (!strcmp(s,"SSW")) { return SSW; }
  else if (!strcmp(s,"SW"))  { return SW; }
  else if (!strcmp(s,"WSW")) { return WSW; }
  else if (!strcmp(s,"W"))   { return W; }
  else if (!strcmp(s,"WNW")) { return WNW; }
  else if (!strcmp(s,"NW"))  { return NW; }
  else if (!strcmp(s,"NNW")) { return NNW; }
  else { Require(false, "Invalid direction specified, " << s); }
}

///////////////////////////////////////////////////////////////////////////////
Geology& WorldFactoryFromFile::get_Geology_from_parent()
///////////////////////////////////////////////////////////////////////////////
{
  // m_curr_node is tile
  xmlNodePtr parent = m_curr_node;
  m_curr_node = m_curr_node->xmlChildrenNode;

  while (m_curr_node != NULL) {
    if (!xmlStrcmp(m_curr_node->name, (const xmlChar *)"Geology")) {
      char *geotype = get_element("type");
      Geology* rv;
      if (!strcmp(geotype, "Inactive")) {
        rv = new Inactive;
      }
      else if (!strcmp(geotype, "Subducting")) {
        float plate_movement = get_float_from_parent("plate_movement");
        rv = new Subducting(plate_movement);
      }
      else if (!strcmp(geotype, "Transform")) {
        float plate_movement = get_float_from_parent("plate_movement");
        rv = new Transform(plate_movement);
      }
      else {
        Require(false, "Unknown geology type \"" << geotype << "\".");
      }

      m_curr_node = parent;
      return *rv;
    }
    m_curr_node = m_curr_node->next;
  }
  RequireUser(false, "Could not find Geology.");
}

///////////////////////////////////////////////////////////////////////////////
bool baal::is_baal_map_file(const std::string& filename)
///////////////////////////////////////////////////////////////////////////////
{
  const std::string file_ext = WorldFactoryFromFile::WORLD_FILE_EXT;

  const unsigned expected_file_ext_pos = filename.length() - file_ext.length();

  return filename.rfind(file_ext) == expected_file_ext_pos;
}
