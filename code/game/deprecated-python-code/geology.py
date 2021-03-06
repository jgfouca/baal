#! /usr/bin/env python

"""
This file contains the classes having to do with Geology. The
classes here are just data-holders for the most part. As usual, we define
multiple classes here to avoid having a large number of header files for
very small classes.
"""

import unittest

from draw_mode import DrawMode
from baal_common import prequire, ProgramError, check_access, grant_access, \
                        set_prequire_handler, raising_prequire_handler

#
# Free-function API
#

###############################################################################
def is_geological(draw_mode):
###############################################################################
    return draw_mode in [DrawMode.GEOLOGY, DrawMode.TENSION, DrawMode.MAGMA]

###############################################################################
class Geology(object):
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
    # ==== Public API ====
    #

    def __init__(self, plate_movement): self.__init_impl(plate_movement)

    #
    # Getters / Queries
    #

    def tension(self): return self.__tension

    def magma(self): return self.__magma

    def to_xml(self): return self.__to_xml_impl()

    #
    # Modification API
    #

    def cycle_turn(self):
        """
        Notify this object that the turn has cycled
        """
        return self.__cycle_turn_impl()

    #
    # ==== Class Constants ====
    #

    ALLOW_CYCLE_TURN = "_geology_allow_cycle_turn"

    #
    # ==== Protected Abstract API ====
    #

    @classmethod
    def _base_magma_buildup(cls): prequire(False, "Must override")

    @classmethod
    def _base_tension_buildup(cls): prequire(False, "Must override")

    #
    # ==== Implementation ====
    #

    ###########################################################################
    def __init_impl(self, plate_movement):
    ###########################################################################
        self.__tension = 0.0
        self.__magma   = 0.0
        self.__plate_movement = plate_movement
        self.__tension_buildup = self._base_tension_buildup() * plate_movement
        self.__magma_buildup = self._base_magma_buildup() * plate_movement

        prequire(plate_movement >= 0.0, "Broken precondition")
        prequire(self._base_tension_buildup() >= 0.0, "Broken precondition")
        prequire(self._base_magma_buildup() >= 0.0, "Broken precondition")

    ###########################################################################
    def __to_xml_impl(self):
    ###########################################################################
        pass # Aaron TODO

    ###########################################################################
    def __cycle_turn_impl(self):
    ###########################################################################
        check_access(self.ALLOW_CYCLE_TURN)

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

    def __init__(self, plate_movement):
        super(Divergent, self).__init__(plate_movement)

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

    def __init__(self, plate_movement):
        super(Subducting, self).__init__(plate_movement)

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

    def __init__(self, plate_movement):
        super(Orogenic, self).__init__(plate_movement)

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

    def __init__(self, plate_movement):
        super(Transform, self).__init__(plate_movement)

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

    def __init__(self):
        super(Inactive, self).__init__(0.0)

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
        # Change to raising handler for unit-testing
        set_prequire_handler(raising_prequire_handler)

        grant_access(self, Geology.ALLOW_CYCLE_TURN)

        plate_movement = 3

        # Test that we cannot create instances of Geology
        self.assertRaises(ProgramError, Geology, plate_movement)

        # Check that is_geological works
        self.assertTrue(is_geological(DrawMode.MAGMA))

        subd = Subducting(plate_movement)

        # Check turn cycling
        subd.cycle_turn()
        subd.cycle_turn()
        tension1 = subd.tension()
        magma1 = subd.magma()
        subd.cycle_turn()
        subd.cycle_turn()
        self.assertLess(tension1, subd.tension())
        self.assertLess(magma1, subd.magma())

        for i in xrange(500):
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
        for i in xrange(100):
            inactive.cycle_turn()
        self.assertEqual(magma_orig, inactive.magma())
        self.assertEqual(tension_orig, inactive.tension())

if (__name__ == "__main__"):
    unittest.main()
