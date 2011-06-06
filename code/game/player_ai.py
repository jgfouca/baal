#! /usr/bin/env python

import unittest

from drawable import Drawable

###############################################################################
class PlayerAI(Drawable):
###############################################################################
    """
    TODO stub
    """

    def __init__(self): pass

    def tech_yield_multiplier(self): return 0

    def cycle_turn(self): pass

    def population(self): return 0

    def tech_level(self): return 0

#
# Tests
#

###############################################################################
class TestPlayerAI(unittest.TestCase):
###############################################################################

    ###########################################################################
    def test_player_ai(self):
    ###########################################################################
        pass

if (__name__ == "__main__"):
    unittest.main()
