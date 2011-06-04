#! /usr/bin/env python

"""
This file contains the classes having to do with Geology. The
classes here are just data-holders for the most part. As usual, we define
multiple classes here to avoid having a large number of header files for
very small classes.
"""

import unittest

from drawable import Drawable, DrawMode, curr_draw_mode, _set_draw_mode
from baal_common import prequire, ProgramError, cprint, \
                        GREEN, YELLOW, BLUE, RED, WHITE
from world_tile import WorldTile

#
# Free-function API
#

###############################################################################
def is_geological(draw_mode):
###############################################################################
    return draw_mode in [DrawMode.GEOLOGY, DrawMode.TENSION, DrawMode.MAGMA]

###############################################################################
class Geology(Drawable):
###############################################################################
    """
    Contains geology (plate-tectonic) data. This is an abstract base class for
    all Geology classes.

    Plate boundaries always fall between tiles. Boundaries can be:
    Divergent
    Convergent(ocean/land)
    Convergent(ocean/ocean)
    Convergent(land/land)
    Transform

    Every tile builds up plate tension and magma based on geology.
    """

    #
    # Public API
    #

    ###########################################################################
    def __init__(self, plate_movement):
    ###########################################################################
        self.__tension = 0.0
        self.__magma   = 0.0
        self.__plate_movement = plate_movement
        self.__tension_buildup = self._base_tension_buildup() * plate_movement
        self.__magma_buildup = self._base_magma_buildup() * plate_movement

        prequire(plate_movement >= 0.0, "Broken precondition")
        prequire(self._base_tension_buildup() >= 0.0, "Broken precondition")
        prequire(self._base_magma_buildup() >= 0.0, "Broken precondition")

    #
    # Getters / Queries
    #

    def tension(self): return self.__tension

    def magma(self): return self.__magma

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
        Notify this object that the turn has cycled
        """
        return self.__cycle_turn_impl()

    #
    # Protected Abstract API
    #

    @classmethod
    def _color(cls): prequire(False, "Must override")

    @classmethod
    def _symbol(cls): prequire(False, "Must override")

    @classmethod
    def _base_magma_buildup(cls): prequire(False, "Must override")

    @classmethod
    def _base_tension_buildup(cls): prequire(False, "Must override")

    #
    # Implementation
    #

    ###########################################################################
    def __to_xml_impl(self):
    ###########################################################################
        pass # Aaron TODO

    ###########################################################################
    def __draw_text_impl(self):
    ###########################################################################
        draw_mode = curr_draw_mode()
        expected_length = WorldTile.TILE_TEXT_WIDTH

        # Figure out what string to print and what color it should be
        if (draw_mode == DrawMode.GEOLOGY):
            color = self._color()
            to_draw = self._symbol().center(expected_length)
        elif (draw_mode in [DrawMode.TENSION, DrawMode.MAGMA]):
            value = self.__tension if draw_mode == DrawMode.TENSION \
                    else self.__magma
            to_draw = ("%.3f" % value).center(expected_length)

            if (value < .333):
                color = GREEN
            elif (value < .666):
                color = YELLOW
            else:
                color = RED
        else:
            prequire(False, "Should not draw geology in mode: ", draw_mode)

        prequire(len(to_draw) == WorldTile.TILE_TEXT_WIDTH,
                 "Symbol '", to_draw, "' is wrong length")

        cprint(color, to_draw)

    ###########################################################################
    def __draw_graphics_impl(self):
    ###########################################################################
        # TODO
        pass

    ###########################################################################
    def __cycle_turn_impl(self):
    ###########################################################################
        # Tension/magma build up more slowly as they reach 100%
        self.__tension += (1 - self.__tension) * self.__tension_buildup
        self.__magma   += (1 - self.__magma)   * self.__magma_buildup

        prequire(self.__tension < 1.0, "Invariant violated: ", self.__tension)
        prequire(self.__magma   < 1.0, "Invariant violated: ", self.__magma)

#
# Geology classes
#

###############################################################################
class Divergent(Geology):
###############################################################################
    """
    Diverging plate boundary.
    """

    _MAGMA_BUILDUP   = 0.001
    _TENSION_BUILDUP = 0.000
    _COLOR           = BLUE
    _SYMBOL          = "<-->"

    def __init__(self, plate_movement):
        super(self.__class__, self).__init__(plate_movement)

    @classmethod
    def _color(cls): return cls._COLOR

    @classmethod
    def _symbol(cls): return cls._SYMBOL

    @classmethod
    def _base_magma_buildup(cls): return cls._MAGMA_BUILDUP

    @classmethod
    def _base_tension_buildup(cls): return cls._TENSION_BUILDUP

###############################################################################
class Subducting(Geology):
###############################################################################
    """
    There is no difference between convergent land-ocean and convergent
    ocean-ocean. Both are subducting with one plate going under the other.
    """

    _MAGMA_BUILDUP   = 0.002
    _TENSION_BUILDUP = 0.002
    _COLOR           = RED
    _SYMBOL          = "-v<-"

    def __init__(self, plate_movement):
        super(self.__class__, self).__init__(plate_movement)

    @classmethod
    def _color(cls): return cls._COLOR

    @classmethod
    def _symbol(cls): return cls._SYMBOL

    @classmethod
    def _base_magma_buildup(cls): return cls._MAGMA_BUILDUP

    @classmethod
    def _base_tension_buildup(cls): return cls._TENSION_BUILDUP

###############################################################################
class Orogenic(Geology):
###############################################################################
    """
    Coverging land-land => Orogenic boundary. Neither plate goes under.
    """

    _MAGMA_BUILDUP   = 0.000
    _TENSION_BUILDUP = 0.002
    _COLOR           = GREEN
    _SYMBOL          = "-><-"

    def __init__(self, plate_movement):
        super(self.__class__, self).__init__(plate_movement)

    @classmethod
    def _color(cls): return cls._COLOR

    @classmethod
    def _symbol(cls): return cls._SYMBOL

    @classmethod
    def _base_magma_buildup(cls): return cls._MAGMA_BUILDUP

    @classmethod
    def _base_tension_buildup(cls): return cls._TENSION_BUILDUP

###############################################################################
class Transform(Geology):
###############################################################################
    """
    Transform - Two plates sliding past each other.
    """

    _MAGMA_BUILDUP   = 0.000
    _TENSION_BUILDUP = 0.003
    _COLOR           = YELLOW
    _SYMBOL          = "vv^^"

    def __init__(self, plate_movement):
        super(self.__class__, self).__init__(plate_movement)

    @classmethod
    def _color(cls): return cls._COLOR

    @classmethod
    def _symbol(cls): return cls._SYMBOL

    @classmethod
    def _base_magma_buildup(cls): return cls._MAGMA_BUILDUP

    @classmethod
    def _base_tension_buildup(cls): return cls._TENSION_BUILDUP

###############################################################################
class Inactive(Geology):
###############################################################################
    """
    Inactive - No nearby plate boundary
    """

    _MAGMA_BUILDUP   = 0.000
    _TENSION_BUILDUP = 0.000
    _COLOR           = WHITE
    _SYMBOL          = ""

    def __init__(self):
        super(self.__class__, self).__init__(0.0)

    @classmethod
    def _color(cls): return cls._COLOR

    @classmethod
    def _symbol(cls): return cls._SYMBOL

    @classmethod
    def _base_magma_buildup(cls): return cls._MAGMA_BUILDUP

    @classmethod
    def _base_tension_buildup(cls): return cls._TENSION_BUILDUP

#
# Tests
#

###############################################################################
class TestGeology(unittest.TestCase):
###############################################################################

    ###########################################################################
    def test_geology(self):
    ###########################################################################
        plate_movement = 3

        # Test that we cannot create instances of Geology
        self.assertRaises(ProgramError, Geology, plate_movement)

        # Check that classmethod structure works
        subd = Subducting(plate_movement)
        self.assertEqual(subd._color(), Subducting._COLOR)

        # Check that drawing works
        _set_draw_mode(DrawMode.CIV)
        self.assertRaises(ProgramError, subd.draw_text)
        self.assertTrue(is_geological(DrawMode.MAGMA))
        _set_draw_mode(DrawMode.MAGMA)
        subd.draw_text()
        _set_draw_mode(DrawMode.TENSION)
        subd.draw_text()

        # Check turn cycling
        subd.cycle_turn()
        subd.cycle_turn()
        tension1 = subd.tension()
        magma1 = subd.magma()
        subd.cycle_turn()
        subd.cycle_turn()
        self.assertLess(tension1, subd.tension())
        self.assertLess(magma1, subd.magma())

        for i in xrange(10000):
            subd.cycle_turn()
        self.assertLess(subd.tension(), 1.0)
        self.assertLess(subd.magma(), 1.0)

        # Check transform
        trans = Transform(plate_movement)
        magma_orig = trans.magma()
        trans.cycle_turn()
        trans.cycle_turn()
        tension1 = trans.tension()
        self.assertEqual(magma_orig, trans.magma())
        trans.cycle_turn()
        trans.cycle_turn()
        self.assertLess(tension1, trans.tension())
        self.assertEqual(magma_orig, trans.magma())

        # Check inactive
        inactive = Inactive()
        magma_orig = inactive.magma()
        tension_orig = inactive.tension()
        for i in xrange(1000):
            inactive.cycle_turn()
        self.assertEqual(magma_orig, inactive.magma())
        self.assertEqual(tension_orig, inactive.tension())

if (__name__ == "__main__"):
    unittest.main()
