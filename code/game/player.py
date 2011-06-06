#! /usr/bin/env python

import unittest

from drawable import Drawable

###############################################################################
class Player(Drawable):
###############################################################################
    """
    TODO stub
    """

    def __init__(self): pass

    def cycle_turn(self): pass

    def verify_cast(self, spell): return True

    def cast(self, spell): pass

    def learn(self, name): pass

    def gain_exp(self, exp): pass

    def talents(self): return None

    def exp(self): return 0

    def next_level_cost(self): return 0


#
# Tests
#

###############################################################################
class TestPlayer(unittest.TestCase):
###############################################################################

    ###########################################################################
    def test_player(self):
    ###########################################################################
        pass

if (__name__ == "__main__"):
    unittest.main()
