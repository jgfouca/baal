#include "WorldFactoryFromFile.hpp"
#include "World.hpp"
#include "BaalExceptions.hpp"
#include "Weather.hpp"
#include "Geology.hpp"
#include "City.hpp"

#include <libxml/parser.h>
#include <string.h>

using namespace baal;

///////////////////////////////////////////////////////////////////////////////
World& WorldFactoryFromFile::create(char * mapfilename)
///////////////////////////////////////////////////////////////////////////////
{
  return load_world_from_file(mapfilename);
}

///////////////////////////////////////////////////////////////////////////////
World& WorldFactoryFromFile::load_world_from_file(char * mapfilename)
///////////////////////////////////////////////////////////////////////////////
{
  xmlDocPtr mapfile;
  xmlNodePtr cur;

  mapfile = xmlParseFile(mapfilename);
  if (mapfile == NULL ) {
    RequireUser(false, "Map file " << mapfilename << " not parsed successfully.");
  }

  cur = xmlDocGetRootElement(mapfile);
  if (mapfile == NULL) {
    xmlFreeDoc(mapfile);
    RequireUser(false, "Map file " << mapfilename << " is empty.");
  }
  if (xmlStrcmp(cur->name, (const xmlChar *) "baalmap")) {
    xmlFreeDoc(mapfile);
    RequireUser(false, "Map file " << mapfilename << " is not a Baal map.");
  }

  int map_width = get_int_from_parent(mapfile, cur, "map_width");
  int map_height = get_int_from_parent(mapfile, cur, "map_height");

  World& world = *(new World(map_width, map_height));
  cur = cur->xmlChildrenNode;
  while (cur != NULL) {
    if (!xmlStrcmp(cur->name, (const xmlChar *)"tile")) {
      int row = get_int_from_parent(mapfile, cur, "row");
      int col = get_int_from_parent(mapfile, cur, "col");
      world.m_tiles[row][col] = parse_Tile(mapfile, cur);
    }
    else if (!xmlStrcmp(cur->name, (const xmlChar *)"city")) {
      int row = get_int_from_parent(mapfile, cur, "row");
      int col = get_int_from_parent(mapfile, cur, "col");
      char* name = get_element(mapfile, cur, "name");
      dynamic_cast<LandTile*>(world.m_tiles[row][col])->place_city(*new City(name));
    }
    cur = cur->next;
  }

  return world;
}

///////////////////////////////////////////////////////////////////////////////
char* WorldFactoryFromFile::get_element(xmlDocPtr mapfile, xmlNodePtr cur, const char *elemname)
///////////////////////////////////////////////////////////////////////////////
{
  cur = cur->xmlChildrenNode;
  while (cur != NULL) {
    if (!xmlStrcmp(cur->name, (const xmlChar *)elemname)) {
      return (char *)xmlNodeListGetString(mapfile, cur->xmlChildrenNode, 1);
    }
    cur = cur->next;
  }
  RequireUser(false, "Could not find element named " << elemname << ".");
}

///////////////////////////////////////////////////////////////////////////////
int WorldFactoryFromFile::get_int_from_parent(xmlDocPtr mapfile, xmlNodePtr cur, const char *elemname)
///////////////////////////////////////////////////////////////////////////////
{
  char *s = get_element(mapfile, cur, elemname);
  int r;
  std::istringstream(s) >> r;
  return r;
}

///////////////////////////////////////////////////////////////////////////////
unsigned WorldFactoryFromFile::get_unsigned_from_parent(xmlDocPtr mapfile, xmlNodePtr cur, const char *elemname)
///////////////////////////////////////////////////////////////////////////////
{
  char *s = get_element(mapfile, cur, elemname);
  unsigned r;
  std::istringstream(s) >> r;
  return r;
}

///////////////////////////////////////////////////////////////////////////////
float WorldFactoryFromFile::get_float_from_parent(xmlDocPtr mapfile, xmlNodePtr cur, const char *elemname)
///////////////////////////////////////////////////////////////////////////////
{
  char *s = get_element(mapfile, cur, elemname);
  float r;
  std::istringstream(s) >> r;
  return r;
}

///////////////////////////////////////////////////////////////////////////////
WorldTile* WorldFactoryFromFile::parse_Tile(xmlDocPtr mapfile, xmlNodePtr cur)
///////////////////////////////////////////////////////////////////////////////
{
  //cur is tile
  //Climate(int temperature, unsigned rainfall, Wind wind)

  char *type = get_element(mapfile, cur, "type");
  Climate& climate = get_Climate_from_parent(mapfile, cur);
  Geology& geology = get_Geology_from_parent(mapfile, cur);
  if (not strcmp(type, "OceanTile")) {
    unsigned depth = get_unsigned_from_parent(mapfile, cur, "depth");
    return new OceanTile(depth, climate, geology);
  }
  else if (not strcmp(type, "DesertTile")) {
    return new DesertTile(climate, geology);
  }
  else if (not strcmp(type, "LushTile")) {
    return new LushTile(climate, geology);
  }
  else if (not strcmp(type, "MountainTile")) {
    unsigned elevation = get_unsigned_from_parent(mapfile, cur, "elevation");
    return new MountainTile(elevation, climate, geology);
  }
  else if (not strcmp(type, "TundraTile")) {
    return new TundraTile(climate, geology);
  }
  else if (not strcmp(type, "PlainsTile")) {
    return new PlainsTile(climate, geology);
  }
  else if (not strcmp(type, "HillsTile")) {
    return new HillsTile(climate, geology);
  }
  else {
    Require(false, "Unknown tile type \"" << type << "\".");
  }
}

///////////////////////////////////////////////////////////////////////////////
Climate& WorldFactoryFromFile::get_Climate_from_parent(xmlDocPtr mapfile, xmlNodePtr cur)
///////////////////////////////////////////////////////////////////////////////
{
  //cur is tile
  cur = cur->xmlChildrenNode;
  while (cur != NULL) {
    if (!xmlStrcmp(cur->name, (const xmlChar *)"Climate")) {
      int temperature = get_int_from_parent(mapfile, cur, "temperature");
      unsigned rainfall = get_unsigned_from_parent(mapfile, cur, "rainfall");
      Wind wind = get_Wind_from_parent(mapfile, cur);
      return *new Climate(temperature, rainfall, wind);
    }
    cur = cur->next;
  }
  Require(false, "Could not find Climate in a tile.");
}

///////////////////////////////////////////////////////////////////////////////
Wind WorldFactoryFromFile::get_Wind_from_parent(xmlDocPtr mapfile, xmlNodePtr cur)
///////////////////////////////////////////////////////////////////////////////
{
  //cur is Climate
  //Wind(unsigned speed, Direction direction)
  cur = cur->xmlChildrenNode;

  while (cur != NULL) {
    if (!xmlStrcmp(cur->name, (const xmlChar *)"Wind")) {
      unsigned speed = get_unsigned_from_parent(mapfile, cur, "speed");
      Direction direction = get_Direction_from_parent(mapfile, cur);
      Wind wind(speed, direction);
      return wind;
    }
    cur = cur->next;
  }
  RequireUser(false, "Could not find Wind.");
}

///////////////////////////////////////////////////////////////////////////////
Direction WorldFactoryFromFile::get_Direction_from_parent(xmlDocPtr mapfile, xmlNodePtr cur)
///////////////////////////////////////////////////////////////////////////////
{
  char *s = get_element(mapfile, cur, "direction");
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
Geology& WorldFactoryFromFile::get_Geology_from_parent(xmlDocPtr mapfile, xmlNodePtr cur)
///////////////////////////////////////////////////////////////////////////////
{
  //cur is tile
  cur = cur->xmlChildrenNode;

  while (cur != NULL) {
    if (!xmlStrcmp(cur->name, (const xmlChar *)"Geology")) {
      char *geotype = get_element(mapfile, cur, "type");
      if (!strcmp(geotype, "Inactive")) {
        return *new Inactive;
      }
      else if (!strcmp(geotype, "Subducting")) {
        float plate_movement = get_float_from_parent(mapfile, cur, "plate_movement");
        return *new Subducting(plate_movement);
      }
      else if (!strcmp(geotype, "Transform")) {
        float plate_movement = get_float_from_parent(mapfile, cur, "plate_movement");
        return *new Transform(plate_movement);
      }
      else {
        Require(false, "Unknown geology type \"" << geotype << "\".");
      }
    }
    cur = cur->next;
  }
  RequireUser(false, "Could not find Wind.");
}