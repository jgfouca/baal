#! /usr/bin/env python

import unittest

from baal_common import prequire, urequire, UserError, ProgramError

###############################################################################
class _DrawModeMeta(type):
###############################################################################
    def __iter__(mcs):
        return DrawMode._iter_hook()

    def __contains__(mcs, value):
        return DrawMode._in_hook(value)

###############################################################################
class DrawMode(object):
###############################################################################
    """
    Describes which draw mode we're in.
    TODO: It is even worth support the int API for this class?
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
    __NAMES = [ name for name in dir() if name.isupper() and name.isalpha() ]

    #
    # Public API
    #

    ###########################################################################
    def __init__(self, value):
    ###########################################################################
        cls = DrawMode
        if (type(value) == str):
            value = value.upper()
            urequire(value in cls, "Invalid draw-mode string: ", value)
            self.__value = cls._names().index(value)
        elif (type(value) == int):
            urequire(value in cls, "Invalid draw-mode int: ", value)
            self.__value = value
        else:
            prequire(False, "Must initialize with string or int, ",
                     "received: ", type(value))

    ###########################################################################
    def __eq__(self, rhs):
    ###########################################################################
        """
        Flexible equality. Can compare with string, int, or another DrawMode.
        """
        if (type(rhs) == str):
            rhs = rhs.upper()
            if (rhs in DrawMode._names()):
                return self.__value == DrawMode._names().index(rhs)
            else:
                return False
        elif (type(rhs) == int):
            return self.__value == rhs
        elif (type(rhs) == DrawMode):
            return self.__value == rhs.__value
        else:
            prequire(False, "Can only compare with str, int, or DrawMode, ",
                     "received: ", type(rhs))

    ###########################################################################
    def __ne__(self, rhs):
    ###########################################################################
        """
        Flexible inequality. See __eq__
        """
        return not self == rhs

    ###########################################################################
    def __str__(self):
    ###########################################################################
        """
        Return the name of the enum value
        """
        return DrawMode._names()[self.__value]

    ###########################################################################
    def __int__(self):
    ###########################################################################
        """
        Return the raw enum value
        """
        return self.__value

    #
    # Private API
    #

    ###########################################################################
    @classmethod
    def _iter_hook(cls):
    ###########################################################################
        return iter([DrawMode(name) for name in cls._names()])

    ###########################################################################
    @classmethod
    def _in_hook(cls, value):
    ###########################################################################
        cls = DrawMode
        if (type(value) == str):
            return value.upper() in cls._names()
        elif (type(value) == int):
            return value >= 0 and value < len(cls._names())
        else:
            prequire(False, "Only makes sense to check membership with string "
                     "or int, received: ", type(value))

    # Private getters to make pylint happy

    @classmethod
    def _names(cls): return cls.__NAMES

    def _value(self): return self.__value

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
class Drawable(object):
###############################################################################
    """
    All classes that are drawable should inherit from this class. This class
    does not have any member data and should not be instatiated. It is only
    intended to impose an interface on its subclasses.
    TODO: Drawing design needs to be reviewed
    """

    def __init__(self): prequire(False, "Do not instatiate Drawable")

    #
    # Public API
    #

    def draw_text(self): prequire(False, "Called abstract draw_test")

    def draw_graphics(self): prequire(False, "Called abstract draw_graphics")

#
# Tests
#

###############################################################################
class TestDrawMode(unittest.TestCase):
###############################################################################

    ###########################################################################
    def test_draw_mode(self):
    ###########################################################################
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

        self.assertEqual(num_iterations, len(DrawMode._names()))

    ###########################################################################
    def test_global_draw_mode(self):
    ###########################################################################
        new_draw_mode = DrawMode.LAND
        _set_draw_mode(new_draw_mode)
        self.assertEqual(curr_draw_mode(), new_draw_mode)

if (__name__ == "__main__"):
    unittest.main()
