#! /usr/bin/env python

import unittest
import os.path

from baal_common import prequire, urequire, Location, grant_access
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
        tiles = []

        # Row 1
        tiles.append(TundraTile(Climate(30, 10, Wind(10, Direction.WSW)),
                                Inactive))
        tiles.append(PlainsTile(Climate(40, 12, Wind(10, Direction.WSW)),
                                Inactive))
        tiles.append(HillsTile(Climate(35, 14, Wind(15, Direction.WSW)),
                               Inactive))
        tiles.append(MountainTile(5000,
                                  Climate(20, 40, Wind(25, Direction.WSW)),
                                  Subducting(2.0)))
        tiles.append(LushTile(Climate(60, 30, Wind(10, Direction.WSW)),
                              Subducting(2.0)))
        tiles.append(OceanTile(1000,
                               Climate(70, 35, Wind(10, Direction.SW)),
                               Subducting(2.0)))

        # Row 2
        tiles.append(DesertTile(Climate(50, 10, Wind(10, Direction.SW)),
                                Inactive))
        tiles.append(DesertTile(Climate(50, 10, Wind(10, Direction.SW)),
                                Inactive))
        tiles.append(MountainTile(5000,
                                  Climate(20, 40, Wind(25, Direction.SW)),
                                  Inactive))
        tiles.append(HillsTile(Climate(40, 40, Wind(15, Direction.SW)),
                               Subducting(2.0)))
        tiles.append(LushTile(Climate(62, 30, Wind(10, Direction.SW)),
                              Subducting(2.0)))
        tiles.append(OceanTile(1000,
                               Climate(72, 35, Wind(10, Direction.SSW)),
                               Subducting(2.0)))

        # Row 3
        tiles.append(DesertTile(Climate(55, 10, Wind(10, Direction.S)),
                                Inactive))
        tiles.append(MountainTile(5000,
                                  Climate(25, 45, Wind(25, Direction.SSW)),
                                  Inactive))
        tiles.append(HillsTile(Climate(45, 40, Wind(15, Direction.SSW)),
                               Subducting(3.0)))
        tiles.append(LushTile(Climate(65, 35, Wind(10, Direction.SSW)),
                              Subducting(3.0)))
        tiles.append(OceanTile(1000,
                               Climate(75, 35, Wind(10, Direction.S)),
                               Subducting(3.0)))
        tiles.append(OceanTile(1000,
                               Climate(75, 40, Wind(10, Direction.S)),
                               Inactive))

        # Row 4
        tiles.append(HillsTile(Climate(55, 15, Wind(15, Direction.S)),
                               Inactive))
        tiles.append(MountainTile(5000,
                                  Climate(30, 50, Wind(25, Direction.S)),
                                  Inactive))
        tiles.append(LushTile(Climate(70, 40, Wind(10, Direction.S)),
                              Subducting(2.0)))
        tiles.append(LushTile(Climate(70, 40, Wind(10, Direction.S)),
                              Subducting(2.0)))
        tiles.append(OceanTile(1000,
                               Climate(80, 45, Wind(10, Direction.SSE)),
                               Subducting(2.0)))
        tiles.append(OceanTile(1000,
                               Climate(80, 45, Wind(10, Direction.SSE)),
                               Inactive))

        # Row 5
        tiles.append(PlainsTile(Climate(70, 20, Wind(10, Direction.SSE)),
                                Transform(2.0)))
        tiles.append(LushTile(Climate(70, 30, Wind(10, Direction.SSE)),
                              Transform(2.0)))
        tiles.append(LushTile(Climate(70, 30, Wind(10, Direction.SSE)),
                              Transform(2.0)))
        tiles.append(OceanTile(1000,
                               Climate(80, 45, Wind(10, Direction.SE)),
                               Subducting(1.0)))
        tiles.append(OceanTile(1000,
                               Climate(80, 45, Wind(10, Direction.SE)),
                               Inactive))
        tiles.append(OceanTile(1000,
                               Climate(80, 45, Wind(10, Direction.SE)),
                               Inactive))

        # Row 6
        tiles.append(OceanTile(1000,
                               Climate(85, 45, Wind(10, Direction.ESE)),
                               Inactive))
        tiles.append(OceanTile(1000,
                               Climate(85, 45, Wind(10, Direction.ESE)),
                               Inactive))
        tiles.append(OceanTile(1000,
                               Climate(85, 45, Wind(10, Direction.ESE)),
                               Inactive))
        tiles.append(OceanTile(1000,
                               Climate(85, 45, Wind(10, Direction.ESE)),
                               Inactive))
        tiles.append(OceanTile(1000,
                               Climate(85, 45, Wind(10, Direction.ESE)),
                               Inactive))
        tiles.append(OceanTile(1000,
                               Climate(85, 45, Wind(10, Direction.ESE)),
                               Inactive))

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
