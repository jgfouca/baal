#! /usr/bin/env python

import unittest

from baal_common import create_subclass_map, urequire, Location
from spell import Spell

###############################################################################
class SpellFactory(object):
###############################################################################
    """
    Factory class used to create spells. This class encapsulates
    the knowledge of the set of available spells.
    """

    # Class variables
    __spell_map = create_subclass_map(Spell)

    #
    # Public API
    #

    ###########################################################################
    @classmethod
    def create_spell(cls, name, level, location):
    ###########################################################################
        return cls.get(name)(level, location)

    ###########################################################################
    @classmethod
    def get(cls, name):
    ###########################################################################
        """
        Get Spell class object associated with name
        """
        urequire(name in cls.__spell_map,
                 "'%s'" % name, " is not a valid spell.")
        return cls.__spell_map[name]

#
# Tests
#

###############################################################################
class TestSpellFactory(unittest.TestCase):
###############################################################################

    ###########################################################################
    def test_spell_factory(self):
    ###########################################################################
        spell_name = "hot"
        spell_level = 1
        spell_loc = Location(0,0)
        spell = SpellFactory.create_spell(spell_name, spell_level, spell_loc)

        self.assertEqual(spell.name(), spell_name)
        self.assertEqual(spell.level(), spell_level)
        self.assertEqual(spell.location(), spell_loc)

if (__name__ == "__main__"):
    unittest.main()
