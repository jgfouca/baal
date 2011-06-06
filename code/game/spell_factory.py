#! /usr/bin/env python

import unittest

###############################################################################
class SpellFactory(object):
###############################################################################
    """
    TODO stub
    """

    @classmethod
    def create_spell(cls, name, level, location):
        return None

    @classmethod
    def get(cls, name):
        return None


#
# Tests
#

###############################################################################
class TestSpellFactory(unittest.TestCase):
###############################################################################

    ###########################################################################
    def test_spell_factory(self):
    ###########################################################################
        pass

if (__name__ == "__main__"):
    unittest.main()
