#! /usr/bin/env python

import unittest

from baal_common import prequire, urequire, UserError, ProgramError, \
    SmartEnum, set_prequire_handler, raising_prequire_handler, \
    create_names_by_enum_value

# TODO: is it possible to make SmartEnums even slicker? It would be nice if
# we could auto-generate the meta class instead of having to have a custom
# meta class for each smart enum.

class _DrawModeMeta(type):
    def __iter__(mcs): return DrawMode._iter_hook()

    def __contains__(mcs, value): return DrawMode._in_hook(value)

###############################################################################
class DrawMode(SmartEnum):
###############################################################################
    """
    Describes which draw mode we're in.
    """

    # Allows us to change how the class works. In this case, I want to be able
    # to iterate over the class with the semantics of iterating over all the
    # possible enum values. I also want to be able to use the "in" operator
    # to check if something is a valid enum value.
    __metaclass__ = _DrawModeMeta

    # Enum values. Note these are *placeholders*, the free code below this
    # class will replace these values with proper DrawMode objects.
    CIV, \
    LAND, \
    YIELD, \
    MOISTURE, \
    GEOLOGY, \
    MAGMA, \
    TENSION, \
    WIND, \
    TEMPERATURE, \
    PRESSURE, \
    RAINFALL, \
    DEWPOINT = range(12)

    # Derive names from class members.
    _NAMES = create_names_by_enum_value(vars())

    def __init__(self, value):
        super(self.__class__, self).__init__(value)

# Finish DrawMode initialization
for name in DrawMode._names():
    setattr(DrawMode, name, DrawMode(name))

#
# Free-function API for accessing/changing the current global draw mode
#

_draw_mode = DrawMode.CIV

###############################################################################
def curr_draw_mode():
###############################################################################
    return _draw_mode

###############################################################################
def _set_draw_mode(draw_mode):
###############################################################################
    """
    Not public. Do not call this unless you know what you are doing.
    """
    global _draw_mode
    _draw_mode = draw_mode

###############################################################################
def explain_draw_mode(draw_mode):
###############################################################################
    if (draw_mode == DrawMode.CIV):
        return \
"""
Draws things the civ has built, like cities and infrastructure.
""".strip()
    elif (draw_mode == DrawMode.LAND):
        return \
"""
Draws the basic lay of the land.
""".strip()
    elif (draw_mode == DrawMode.YIELD):
        return \
"""
Draws tile yields (food and production).
""".strip()
    elif (draw_mode == DrawMode.MOISTURE):
        return \
"""
Draws soil moisture as a % of normal soil moisture.
""".strip()
    elif (draw_mode == DrawMode.GEOLOGY):
        return \
"""
Draws the plate tectonics underneath the land.
""".strip()
    elif (draw_mode == DrawMode.MAGMA):
        return \
"""
Draws the amount of magma buildup on a scale from 0 to 1
""".strip()
    elif (draw_mode == DrawMode.TENSION):
        return \
"""
Draws the amount of plate tension buildup on a scale from 0 to 1
""".strip()
    elif (draw_mode == DrawMode.WIND):
        return \
"""
Draws the speed and direction of the wind in mph.
""".strip()
    elif (draw_mode == DrawMode.TEMPERATURE):
        return \
"""
Draws the current average (not high or low) temperature in degrees farenheit.
""".strip()
    elif (draw_mode == DrawMode.PRESSURE):
        return \
"""
Draws the current pressure in millibars (1000 is average).
""".strip()
    elif (draw_mode == DrawMode.RAINFALL):
        return \
"""
Draws the amount of rainfall, in inches, that has fallen this season.
""".strip()
    elif (draw_mode == DrawMode.DEWPOINT):
        return \
"""
Draws the current dewpoint in farenheit.
""".strip()
    else:
        prequire(False, "Unhandled draw mode:", draw_mode)

#
# Tests
#

###############################################################################
class TestDrawMode(unittest.TestCase):
###############################################################################

    ###########################################################################
    def test_draw_mode(self):
    ###########################################################################
        # Change to raising handler for unit-testing
        set_prequire_handler(raising_prequire_handler)

        # Check bad object creation
        self.assertRaises(UserError, DrawMode, "asdasd")
        self.assertRaises(UserError, DrawMode, 98712983)
        self.assertRaises(ProgramError, DrawMode, [])

        # Create some objs
        civ_dm_1 = DrawMode("CIV")
        civ_dm_2 = DrawMode(int(civ_dm_1))
        civ_dm_3 = DrawMode.CIV
        self.assertEqual(type(civ_dm_3), DrawMode)
        land_dm_1 = DrawMode("land")

        # Check obj-obj equality
        self.assertEqual(civ_dm_1, civ_dm_2)
        self.assertFalse(civ_dm_1 != civ_dm_2)
        self.assertNotEqual(civ_dm_1, land_dm_1)

        # Check obj-int equality
        self.assertEqual(civ_dm_1, int(DrawMode.CIV))
        self.assertFalse(civ_dm_1 != int(DrawMode.CIV))
        self.assertNotEqual(civ_dm_1, int(DrawMode.LAND))

        # Check obj-str equality
        self.assertEqual(civ_dm_1, "civ")
        self.assertEqual(civ_dm_1, "CiV")
        self.assertEqual(civ_dm_1, "CIV")
        self.assertFalse(civ_dm_1 != "civ")
        self.assertNotEqual(civ_dm_1, "land")

        # Check invalid equality check
        self.assertRaises(ProgramError, lambda: civ_dm_1 == [])

        # Check contains
        self.assertIn("civ", DrawMode)
        self.assertIn(int(DrawMode.CIV), DrawMode)
        self.assertNotIn("asdasda", DrawMode)
        self.assertNotIn(9812312, DrawMode)

        # Check various casts
        self.assertEqual(str(civ_dm_1), "CIV")
        self.assertEqual(int(civ_dm_1), civ_dm_1._value())

        # Check invalid contains
        self.assertRaises(ProgramError, lambda: [] in DrawMode)

        # Check iteration
        num_iterations = 0
        for draw_mode in DrawMode:
            draw_mode_obj = DrawMode(str(draw_mode))
            self.assertEqual(draw_mode_obj, draw_mode)
            num_iterations += 1

        # Check iteration of instance
        ditr = iter(DrawMode)
        first = ditr.next()
        second = ditr.next()
        self.assertEqual(first, civ_dm_1)
        civ_dm_1.next()
        self.assertEqual(second, civ_dm_1)

        self.assertEqual(num_iterations, len(DrawMode._names()))

    ###########################################################################
    def test_global_draw_mode(self):
    ###########################################################################
        new_draw_mode = DrawMode.LAND
        _set_draw_mode(new_draw_mode)
        self.assertEqual(curr_draw_mode(), new_draw_mode)

if (__name__ == "__main__"):
    unittest.main()
