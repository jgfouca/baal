#! /usr/bin/env python

import unittest

from baal_time import Time
from world_tile import WorldTile
from weather import AnomalyCategory, Anomaly
from baal_common import prequire, check_access, grant_access
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

    def __init__(self, width, height, tiles):
        self.__init_impl(width, height, tiles)

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

    def time(self): return self.__time

    def iter_rows(self): return iter(self.__rows)

    def iter_anomalies(self): return iter(self.__recent_anomalies)

    def to_xml(self): return self.__to_xml_impl()

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
        return self.__place_city_impl_world(location, name)

    def remove_city(self, city):
        """
        Remove a city from the world.
        """
        return self.__remove_city_impl_world(city)

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
    def __init_impl(self, width, height, tiles):
    ###########################################################################
        self.__width  = width
        self.__height = height
        self.__time   = Time()

        self.__rows = []
        total = 0
        for idx, tile in enumerate(tiles):
            if (idx % width == 0):
                self.__rows.append([])
            self.__rows[-1].append(tile)
            total += 1

        prequire(total == width * height, "Wrong number of tiles")

        self.__recent_anomalies = []
        self.__cities           = []

    ###########################################################################
    def __in_bounds_impl(self, location):
    ###########################################################################
        return location.row >= 0 and location.row < self.height() and \
               location.col >= 0 and location.col < self.width()

    ###########################################################################
    def __tile_impl(self, location):
    ###########################################################################
        prequire(self.in_bounds(location), location, " out of bounds")
        return self.__rows[location.row][location.col]

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
    def __cycle_turn_impl(self):
    ###########################################################################
        check_access(self.ALLOW_CYCLE_TURN)

        # Phase 1: Increment time
        self.__time.next()

        # Phase 2: Generate anomalies.
        # TODO: How to handle overlapping anomalies of same category?
        self.__recent_anomalies = []
        for row in self.__rows:
            for tile in row:
                for anomaly_category in AnomalyCategory:
                    anomaly = Anomaly.generate_anomaly(anomaly_category,
                                                       tile.location())
                    if (anomaly is not None):
                        self.__recent_anomalies.append(anomaly)

        # Phase 3 of World turn-cycle: Simulate the inter-turn
        # (long-term) weather Every turn, the weather since the last
        # turn will be randomly simulated. There will be random
        # abnormal areas, with the epicenter of the abnormality having
        # the most extreme deviations from the normal climate and
        # peripheral tiles having smaller deviations from normal.
        # Abnormalilty types are: drought, moist, cold, hot, high/low
        # pressure. Anomalies affect this season.
        for row in self.__rows:
            for tile in row:
                tile.cycle_turn(self.__recent_anomalies,
                                self.__time.season())

    ###########################################################################
    def __place_city_impl_world(self, location, name):
    ###########################################################################
        check_access(self.ALLOW_PLACE_CITY)

        if (name is None):
            name = "City %d" % len(self.__cities)

        city = City(name, location)

        self.__cities.append(city)

        tile = self.tile(location)
        tile.place_city(city)

    ###########################################################################
    def __remove_city_impl_world(self, city):
    ###########################################################################
        check_access(self.ALLOW_REMOVE_CITY)

        self.__cities.remove(city)

        self.tile(city.location()).remove_city()

grant_access(World, WorldTile.ALLOW_CYCLE_TURN)

#
# Tests
#

###############################################################################
class TestWorld(unittest.TestCase):
###############################################################################

    ###########################################################################
    def test_world(self):
    ###########################################################################
        from configuration import Configuration
        from world_factory import create_world

        Configuration._create()

        world = create_world()

        grant_access(self, World.ALLOW_CYCLE_TURN)

        # Test cycle turn
        time1_val = int(world.time().season())
        world.cycle_turn()
        time2 = world.time()
        self.assertEqual(time1_val + 1, int(time2.season()))

if (__name__ == "__main__"):
    unittest.main()
