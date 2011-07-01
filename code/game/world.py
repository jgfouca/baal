#! /usr/bin/env python

from __future__ import print_function
import unittest

from baal_time import Time
from drawable import curr_draw_mode, DrawMode
from world_tile import WorldTile
from weather import AnomalyCategory, Anomaly
from baal_common import prequire, check_access
from city import City

###############################################################################
class World(object):
###############################################################################
    """
    Represents the world.
    """

    #
    # ==== Public API ====
    #

    def __init__(self, width, height): self.__init_impl(width, height)

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
    ALLOW_CYCLE_TURN = "_world_allow_cycle_turn"
    ALLOW_PLACE_CITY = "_world_allow_place_city"
    ALLOW_REMOVE_CITY = "_world_allow_remove_city"

    #
    # ==== Implementation ====
    #

    ###########################################################################
    def __init_impl(self, width, height):
    ###########################################################################
        self.__width  = width
        self.__height = height
        self.__time   = Time()

        self.__tiles = []
        for i in xrange(height):
            self.__tiles.append([None] * width)

        self.__recent_anomalies = []
        self.__cities           = []

    ###########################################################################
    def __in_bounds_impl(self, location):
    ###########################################################################
        return location.row < self.height() and location.col < self.width()

    ###########################################################################
    def __tile_impl(self, location):
    ###########################################################################
        prequire(self.in_bounds(location), location, " out of bounds")
        return self.__tiles[location.row][location.col]

    ###########################################################################
    def __cities_impl(self):
    ###########################################################################
        for city in self.__cities:
            yield city

    ###########################################################################
    def __to_xml_impl(self):
    ###########################################################################
        # TODO - Aaron
        pass

    ###########################################################################
    def __draw_text_impl(self):
    ###########################################################################
        real_draw_mode = curr_draw_mode()

        self.__time.draw_text()

        # Make room for row labels
        print("  ", end="")

        # Draw column labels. Need to take 1 char space separator into account.
        for col_id in xrange(self.width()):
            print(("%d" % col_id).center(WorldTile.TILE_TEXT_WIDTH), end=" ")
        print()

        # Draw tiles
        for row_id, row in enumerate(self.__tiles):
            for height in xrange(WorldTile.TILE_TEXT_HEIGHT):
                # Middle of tile displays "overlay" info, for the rest
                # of the tile, just draw the land.
                if (height == WorldTile.TILE_TEXT_HEIGHT / 2):
                    draw_mode = real_draw_mode
                    print(row_id, end=" ") # row label
                else:
                    draw_mode = DrawMode.LAND
                    print("  ", end="") # no label

                # Draw tiles
                for tile in row:
                    tile.draw_text(draw_mode)
                    print(" ", end="") # col separator

                print()

            print()

        # Draw recent anomalies
        for anomaly in self.__recent_anomalies:
            anomaly.draw_text()
        print()

    ###########################################################################
    def __draw_graphics_impl(self):
    ###########################################################################
        # TODO
        pass

    ###########################################################################
    def __cycle_turn_impl(self):
    ###########################################################################
        check_access(self.ALLOW_CYCLE_TURN)

        # Phase 1: Generate anomalies.
        # TODO: How to handle overlapping anomalies of same category?
        self.__recent_anomalies = []
        for row in self.__tiles:
            for tile in row:
                for anomaly_category in AnomalyCategory:
                    anomaly = Anomaly.generate_anomaly(anomaly_category,
                                                       tile.location())
                    if (anomaly is not None):
                        self.__recent_anomalies.append(anomaly)

        # Phase 2 of World turn-cycle: Simulate the inter-turn
        # (long-term) weather Every turn, the weather since the last
        # turn will be randomly simulated. There will be random
        # abnormal areas, with the epicenter of the abnormality having
        # the most extreme deviations from the normal climate and
        # peripheral tiles having smaller deviations from normal.
        # Abnormalilty types are: drought, moist, cold, hot, high/low
        # pressure Based on our model of time, we are at the beginning
        # of the current season, so anomalies affect this season; that
        # is why time is not incremented until later. Current
        # conditions for the next turn will be derived from these
        # anomalies.
        for row in self.__tiles:
            for tile in row:
                tile.cycle_turn(self.__recent_anomalies,
                                tile.location(),
                                self.__time.season())

        # Phase 3: Increment time
        self.__time.next()

    ###########################################################################
    def __place_city_impl(self, location, name):
    ###########################################################################
        check_access(self.ALLOW_PLACE_CITY)

        if (name is None):
            name = "City %d" % len(self.__cities)

        city = City(name, location)

        self.__cities.append(city)

        tile = self.tile(location)
        tile.place_city(city)

    ###########################################################################
    def __remove_city_impl(self, city):
    ###########################################################################
        check_access(self.ALLOW_REMOVE_CITY)

        self.__cities.remove(city)

        self.tile(city.location()).remove_city()

#
# Tests
#

###############################################################################
class TestWorld(unittest.TestCase):
###############################################################################

    ###########################################################################
    def test_world(self):
    ###########################################################################
        # TODO
        pass

if (__name__ == "__main__"):
    unittest.main()
