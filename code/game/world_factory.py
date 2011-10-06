#! /usr/bin/env python

import unittest
import os.path

from baal_common import prequire, urequire, Location, grant_access, \
    LocationIterator
from configuration import Configuration
from world import World
from world_tile import TundraTile, PlainsTile, HillsTile, MountainTile, \
    LushTile, DesertTile, OceanTile
from weather import Climate, Wind, Direction
from geology import Inactive, Subducting, Transform

#
# Free Function API
#

###############################################################################
def create_world():
###############################################################################
    """
    Create a World object based on the current state of the game's
    configuration.

    Encapsulates knowledge of which factories are available and what the
    config string to select them looks like.
    """
    # Get user's choice of world
    config = Configuration.instance().world_config()
    if (config == ""):
        config = _DEFAULT_WORLD_CONFIG

    if (config.isdigit()):
        # Numeric implies hardcoded world
        factory = _WorldFactoryHardcoded(config)
    elif (config == _WorldFactoryRandom.CONFIG):
        factory = _WorldFactoryRandom(config)
    elif (_WorldFactoryFromFile.is_baal_map_file(config)):
        factory = _WorldFactoryFromFile(config)
    else:
        urequire(False, "Invalid world configuration '%s'" % config)

    return factory.create()

#
# Internal-only
#

_DEFAULT_WORLD_CONFIG = "1" # Hardcoded world 1

###############################################################################
class _WorldFactoryBase(object):
###############################################################################
    """
    Only used to allow all subclasses to have create-city access
    """
    pass
grant_access(_WorldFactoryBase, World.ALLOW_PLACE_CITY)

###############################################################################
class _WorldFactoryHardcoded(_WorldFactoryBase):
###############################################################################

    ###########################################################################
    def __init__(self, config):
    ###########################################################################
        self.__worldnum = int(config)

    ###########################################################################
    def create(self):
    ###########################################################################
        if (self.__worldnum == 1):
            return self._generate_world_1()
        else:
            prequire(False, "Cannot generate world: ", self.__worldnum)

    ###########################################################################
    def _generate_world_1(self):
    ###########################################################################
        """
        This is the world that will be generated:

        T P H M L O
        D D M H L O
        D M H L O O
        H M L L O O
        P L L O O O
        O O O O O O
        """
        loc_iter = LocationIterator(6, 6)
        tiles = []

        # Row 1
        tiles.append(TundraTile(Climate([10, 30, 50, 30],
                                        [4, 2, .5, 2],
                                        [Wind(10, Direction.WSW)]*4),
                                Inactive(),
                                loc_iter.next()))
        tiles.append(PlainsTile(Climate([20, 40, 60, 40],
                                        [5, 2.5, 1, 2.5],
                                        [Wind(10, Direction.WSW)]*4),
                                Inactive(),
                                loc_iter.next()))
        tiles.append(HillsTile(Climate([15, 35, 50, 35],
                                       [6, 3.5, 2, 3.5],
                                       [Wind(15, Direction.WSW)]*4),
                               Inactive(),
                               loc_iter.next()))
        tiles.append(MountainTile(5000,
                                  Climate([10, 25, 40, 25],
                                          [12, 7, 8, 7],
                                          [Wind(25, Direction.WSW)]*4),
                                  Subducting(2.0),
                                  loc_iter.next()))
        tiles.append(LushTile(Climate([50, 60, 70, 60],
                                      [8, 8, 8, 8],
                                      [Wind(10, Direction.WSW)]*4),
                              Subducting(2.0),
                              loc_iter.next()))
        tiles.append(OceanTile(1000,
                               Climate([65, 70, 75, 65],
                                       [9, 9, 9, 9],
                                       [Wind(10, Direction.SW)]*4),
                               Subducting(2.0),
                               loc_iter.next()))

        # Row 2
        tiles.append(DesertTile(Climate([25, 50, 75, 50],
                                        [4, 1.5, 1, 1.5],
                                        [Wind(10, Direction.SW)]*4),
                                Inactive(),
                                loc_iter.next()))
        tiles.append(DesertTile(Climate([30, 55, 80, 55],
                                        [4, 1.5, 1, 1.5],
                                        [Wind(10, Direction.SW)]*4),
                                Inactive(),
                                loc_iter.next()))
        tiles.append(MountainTile(5000,
                                  Climate([12, 27, 42, 27],
                                          [12, 7, 8, 7],
                                          [Wind(25, Direction.SW)]*4),
                                  Inactive(),
                                  loc_iter.next()))
        tiles.append(HillsTile(Climate([40, 55, 70, 55],
                                       [10, 10, 10, 10],
                                       [Wind(15, Direction.SW)]*4),
                               Subducting(2.0),
                               loc_iter.next()))
        tiles.append(LushTile(Climate([52, 62, 72, 62],
                                      [8, 8, 8, 8],
                                      [Wind(10, Direction.SW)]*4),
                              Subducting(2.0),
                              loc_iter.next()))
        tiles.append(OceanTile(1000,
                               Climate([67, 72, 77, 67],
                                       [9, 9, 9, 9],
                                       [Wind(10, Direction.SSW)]*4),
                               Subducting(2.0),
                               loc_iter.next()))

        # Row 3
        tiles.append(DesertTile(Climate([30, 55, 80, 55],
                                        [4, 1.5, 1, 1.5],
                                        [Wind(10, Direction.S)]*4),
                                Inactive(),
                                loc_iter.next()))
        tiles.append(MountainTile(5000,
                                  Climate([14, 29, 44, 29],
                                          [13, 8, 10, 8],
                                          [Wind(25, Direction.SSW)]*4),
                                  Inactive(),
                                  loc_iter.next()))
        tiles.append(HillsTile(Climate([42, 57, 72, 57],
                                       [11, 11, 11, 11],
                                       [Wind(15, Direction.SSW)]*4),
                               Subducting(3.0),
                               loc_iter.next()))
        tiles.append(LushTile(Climate([55, 65, 75, 65],
                                      [9, 9, 9, 9],
                                      [Wind(10, Direction.SSW)]*4),
                              Subducting(3.0),
                              loc_iter.next()))
        tiles.append(OceanTile(1000,
                               Climate([70, 75, 80, 75],
                                       [10, 10, 10, 10],
                                       [Wind(10, Direction.S)]*4),
                               Subducting(3.0),
                               loc_iter.next()))
        tiles.append(OceanTile(1000,
                               Climate([70, 75, 80, 75],
                                       [10, 10, 10, 10],
                                       [Wind(10, Direction.S)]*4),
                               Inactive(),
                               loc_iter.next()))

        # Row 4
        tiles.append(HillsTile(Climate([30, 50, 65, 50],
                                       [4, 4, 4, 4],
                                       [Wind(15, Direction.S)]*4),
                               Inactive(),
                               loc_iter.next()))
        tiles.append(MountainTile(5000,
                                  Climate([18, 33, 48, 33],
                                          [10, 9, 13, 9],
                                          [Wind(25, Direction.S)]*4),
                                  Inactive(),
                                  loc_iter.next()))
        tiles.append(LushTile(Climate([60, 70, 80, 70],
                                      [8, 10, 12, 10],
                                      [Wind(10, Direction.S)]*4),
                              Subducting(2.0),
                              loc_iter.next()))
        tiles.append(LushTile(Climate([60, 70, 80, 70],
                                      [8, 10, 12, 8],
                                      [Wind(10, Direction.S)]*4),
                              Subducting(2.0),
                              loc_iter.next()))
        tiles.append(OceanTile(1000,
                               Climate([75, 80, 85, 80],
                                       [11, 11, 11, 11],
                                       [Wind(10, Direction.SSE)]*4),
                               Inactive(),
                               loc_iter.next()))
        tiles.append(OceanTile(1000,
                               Climate([75, 80, 85, 80],
                                       [11, 11, 11, 11],
                                       [Wind(10, Direction.SSE)]*4),
                               Inactive(),
                               loc_iter.next()))

        # Row 5
        tiles.append(PlainsTile(Climate([40, 70, 90, 70],
                                        [3, 4, 8, 4],
                                        [Wind(10, Direction.SSE)]*4),
                                Transform(2.0),
                                loc_iter.next()))
        tiles.append(LushTile(Climate([57, 67, 77, 67],
                                      [6, 8, 16, 8],
                                      [Wind(10, Direction.SSE)]*4),
                              Transform(2.0),
                              loc_iter.next()))
        tiles.append(LushTile(Climate([59, 69, 79, 69],
                                      [8, 10, 16, 10],
                                      [Wind(10, Direction.SSE)]*4),
                              Transform(2.0),
                              loc_iter.next()))
        tiles.append(OceanTile(1000,
                               Climate([75, 80, 85, 80],
                                       [12, 12, 12, 12],
                                       [Wind(10, Direction.SE)]*4),
                               Subducting(1.0),
                               loc_iter.next()))
        tiles.append(OceanTile(1000,
                               Climate([75, 80, 85, 80],
                                       [12, 12, 12, 12],
                                       [Wind(10, Direction.SE)]*4),
                               Inactive(),
                               loc_iter.next()))
        tiles.append(OceanTile(1000,
                               Climate([75, 80, 85, 80],
                                       [12, 12, 12, 12],
                                       [Wind(10, Direction.SE)]*4),
                               Inactive(),
                               loc_iter.next()))

        # Row 6
        tiles.append(OceanTile(1000,
                               Climate([80, 85, 90, 85],
                                       [12, 12, 12, 12],
                                       [Wind(10, Direction.ESE)]*4),
                               Inactive(),
                               loc_iter.next()))
        tiles.append(OceanTile(1000,
                               Climate([80, 85, 90, 85],
                                       [12, 12, 12, 12],
                                       [Wind(10, Direction.ESE)]*4),

                               Inactive(),
                               loc_iter.next()))
        tiles.append(OceanTile(1000,
                               Climate([80, 85, 90, 85],
                                       [12, 12, 12, 12],
                                       [Wind(10, Direction.ESE)]*4),

                               Inactive(),
                               loc_iter.next()))
        tiles.append(OceanTile(1000,
                               Climate([80, 85, 90, 85],
                                       [12, 12, 12, 12],
                                       [Wind(10, Direction.ESE)]*4),

                               Inactive(),
                               loc_iter.next()))
        tiles.append(OceanTile(1000,
                               Climate([80, 85, 90, 85],
                                       [12, 12, 12, 12],
                                       [Wind(10, Direction.ESE)]*4),

                               Inactive(),
                               loc_iter.next()))
        tiles.append(OceanTile(1000,
                               Climate([80, 85, 90, 85],
                                       [12, 12, 12, 12],
                                       [Wind(10, Direction.ESE)]*4),

                               Inactive(),
                               loc_iter.next()))

        world = World(6, 6, tiles)
        world.place_city(Location(4, 2), "Capital")

        return world

###############################################################################
class _WorldFactoryRandom(_WorldFactoryBase):
###############################################################################
    # TODO

    CONFIG = "r"

    def __init__(self, config): pass

    def create(self): return None

###############################################################################
class _WorldFactoryFromFile(_WorldFactoryBase):
###############################################################################
    # TODO - Aaron

    _WORLD_FILE_EXT = "baalmap"

    def __init__(self, config): pass

    def create(self): return None

    @classmethod
    def is_baal_map_file(cls, filename):
        ext = os.path.splitext(filename)[1]
        return ext == cls._WORLD_FILE_EXT

#
# Tests
#

###############################################################################
class TestWorldFactory(unittest.TestCase):
###############################################################################

    ###########################################################################
    def test_world_factory(self):
    ###########################################################################
        Configuration._create()

        create_world()

if (__name__ == "__main__"):
    unittest.main()
