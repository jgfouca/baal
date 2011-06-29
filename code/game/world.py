#! /usr/bin/env python

import unittest

###############################################################################
class WorldTile(object):
###############################################################################
    """
    Represents the world.
    """

    #
    # ==== Public API ====
    #

    def __init__(self): self.__init_impl()

    #
    # Getters / Queries
    #

    def in_bounds(self, location):
        """
        Return true if the given location is in bounds
        """
        return self.__in_bounds_impl(location)

    def tile(self, location):
        """
        Given a location, return the WorldTile at that location
        """
        return self.__tile_impl(location)

    def location(self, tile):
        """
        Given a WorldTile, return it's location (expensive)
        """
        return self.__location_impl(tile)

    def width(self): return self.__width

    def height(self): return self.__height

    def cities(self):
        """
        Iterate over all cities currently in world.
        """
        return self.__cities_impl()

    def to_xml(self): return self.__to_xml_impl()

    #
    # Drawing API
    #

    def draw_text(self): return self.__draw_text_impl()

    def draw_graphics(self): return self.__draw_graphics_impl()

    #
    # Modification API
    #

    def cycle_turn(self):
        """
        Notify the world that the turn has cycled
        """
        return self.__cycle_turn_impl()

    def place_city(self, location, name=None):
        """
        Place a new city in the world.
        """
        return self.__place_city_impl(location, name)

    def remove_city(self, city):
        """
        Remove a city from the world.
        """
        return self.__remove_city_impl(city)

    #
    # ==== Class Constants ====
    #

    TILE_TEXT_WIDTH = 6

    #
    # ==== Implementation ====
    #

    ###########################################################################
    def __init_impl(self):
    ###########################################################################
        pass

    ###########################################################################
    def __in_bounds_impl(self, location):
    ###########################################################################
        pass

    ###########################################################################
    def __tile_impl(self, location):
    ###########################################################################
        pass

    ###########################################################################
    def __location_impl(self, tile):
    ###########################################################################
        pass

    ###########################################################################
    def __cities_impl(self):
    ###########################################################################
        pass

    ###########################################################################
    def __to_xml_impl(self):
    ###########################################################################
        pass

    ###########################################################################
    def __draw_text_impl(self):
    ###########################################################################
        pass

    ###########################################################################
    def __draw_graphics_impl(self):
    ###########################################################################
        pass

    ###########################################################################
    def __cycle_turn_impl(self):
    ###########################################################################
        pass

    ###########################################################################
    def __place_city_impl(self, location, name):
    ###########################################################################
        pass

    ###########################################################################
    def __remove_city_impl(self, city):
    ###########################################################################
        pass

#
# Tests
#

###############################################################################
class TestWorld(unittest.TestCase):
###############################################################################

    ###########################################################################
    def test_world(self):
    ###########################################################################
        pass

if (__name__ == "__main__"):
    unittest.main()
