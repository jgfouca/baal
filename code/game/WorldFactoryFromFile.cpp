#include "WorldFactoryFromFile.hpp"
#include "BaalExceptions.hpp"
#include "Weather.hpp"
#include "Geology.hpp"
#include "City.hpp"

#include <string.h>

namespace baal {

const std::string WorldFactoryFromFile::WORLD_FILE_EXT = ".baalmap";

///////////////////////////////////////////////////////////////////////////////
std::shared_ptr<World> WorldFactoryFromFile::create(const std::string& mapfilename,
                                                    Engine& engine)
///////////////////////////////////////////////////////////////////////////////
{
  WorldFactoryFromFile wfff(mapfilename.c_str(), engine);

  return wfff.load();
}

///////////////////////////////////////////////////////////////////////////////
std::shared_ptr<World> WorldFactoryFromFile::load()
///////////////////////////////////////////////////////////////////////////////
{
  m_mapfile = xmlParseFile(m_mapfilename);
  RequireUser(m_mapfile != nullptr,
              "Map file " << m_mapfilename << " not parsed successfully.");

  m_curr_node = xmlDocGetRootElement(m_mapfile);
  if (m_curr_node == nullptr) {
    xmlFreeDoc(m_mapfile);
    RequireUser(false, "Map file " << m_mapfilename << " is empty.");
  }
  if (xmlStrcmp(m_curr_node->name, (const xmlChar *) "baalmap")) {
    xmlFreeDoc(m_mapfile);
    RequireUser(false, "Map file " << m_mapfilename << " is not a Baal map.");
  }

  int map_width = get_data_from_parent<int>("map_width");
  int map_height = get_data_from_parent<int>("map_height");

  std::shared_ptr<World> world =
    std::shared_ptr<World>(new World(map_width, map_height, m_engine));
  m_curr_node = m_curr_node->xmlChildrenNode;
  while (m_curr_node != nullptr) {
    if (!xmlStrcmp(m_curr_node->name, (const xmlChar *)"tile")) {
      int row = get_data_from_parent<int>("row");
      int col = get_data_from_parent<int>("col");
      world->m_tiles[row][col] = &parse_Tile(row, col);
    }
    else if (!xmlStrcmp(m_curr_node->name, (const xmlChar *)"city")) {
      int row = get_data_from_parent<int>("row");
      int col = get_data_from_parent<int>("col");
      char* name = get_element("name");
      LandTile* landtile = dynamic_cast<LandTile*>(world->m_tiles[row][col]);
      RequireUser(landtile != nullptr,
                  "Tried to place city at " << row << ", " << col <<
                  "; which is not a landtile");
      world->place_city(Location(row, col), name);
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
  while (curr_node != nullptr) {
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
WorldTile& WorldFactoryFromFile::parse_Tile(int row, int col)
///////////////////////////////////////////////////////////////////////////////
{
  // m_curr_node is tile
  // Climate(int temperature, unsigned precip, Wind wind)

  Location location(row, col);
  char *type = get_element("type");
  Climate& climate = get_Climate_from_parent();
  Geology& geology = get_Geology_from_parent();
  if (!strcmp(type, "OceanTile")) {
    unsigned depth = get_data_from_parent<unsigned>("depth");
    return *new OceanTile(location, depth, climate, geology);
  }
  else if (!strcmp(type, "DesertTile")) {
    return *new DesertTile(location, climate, geology);
  }
  else if (!strcmp(type, "LushTile")) {
    return *new LushTile(location, climate, geology);
  }
  else if (!strcmp(type, "MountainTile")) {
    unsigned elevation = get_data_from_parent<unsigned>("elevation");
    return *new MountainTile(location, elevation, climate, geology);
  }
  else if (!strcmp(type, "TundraTile")) {
    return *new TundraTile(location, climate, geology);
  }
  else if (!strcmp(type, "PlainsTile")) {
    return *new PlainsTile(location, climate, geology);
  }
  else if (!strcmp(type, "HillsTile")) {
    return *new HillsTile(location, climate, geology);
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
  while (m_curr_node != nullptr) {
    if (!xmlStrcmp(m_curr_node->name, (const xmlChar *)"Climate")) {
      std::vector<int> temperature = get_data_per_season_from_parent<int>("temperature");
      std::vector<float> precip = get_data_per_season_from_parent<float>("precip");
      std::vector<Wind> wind = get_data_per_season_from_parent<Wind>("wind");
      m_curr_node = parent;
      return *new Climate(temperature, precip, wind);
    }
    m_curr_node = m_curr_node->next;
  }
  Require(false, "Could not find Climate in a tile.");
}

///////////////////////////////////////////////////////////////////////////////
Geology& WorldFactoryFromFile::get_Geology_from_parent()
///////////////////////////////////////////////////////////////////////////////
{
  // m_curr_node is tile
  xmlNodePtr parent = m_curr_node;
  m_curr_node = m_curr_node->xmlChildrenNode;

  while (m_curr_node != nullptr) {
    if (!xmlStrcmp(m_curr_node->name, (const xmlChar *)"Geology")) {
      char *geotype = get_element("type");
      Geology* rv;
      if (!strcmp(geotype, "Inactive")) {
        rv = new Inactive;
      }
      else if (!strcmp(geotype, "Subducting")) {
        float plate_movement = get_data_from_parent<float>("plate_movement");
        rv = new Subducting(plate_movement);
      }
      else if (!strcmp(geotype, "Transform")) {
        float plate_movement = get_data_from_parent<float>("plate_movement");
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
bool is_baal_map_file(const std::string& filename)
///////////////////////////////////////////////////////////////////////////////
{
  const std::string file_ext = WorldFactoryFromFile::WORLD_FILE_EXT;

  const unsigned expected_file_ext_pos = filename.length() - file_ext.length();

  return filename.rfind(file_ext) == expected_file_ext_pos;
}

}
