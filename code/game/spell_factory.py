#! /usr/bin/env python

import unittest

from baal_common import create_subclass_map, urequire, Location
from spell import Spell

class _SpellFactoryMeta(type):
    def __iter__(mcs): return SpellFactory._iter_hook()

###############################################################################
class SpellFactory(object):
###############################################################################
    """
    Factory class used to create spells. This class encapsulates
    the knowledge of the set of available spells.
    """

    __metaclass__ = _SpellFactoryMeta

    # Class variables
    __spell_map = create_subclass_map(Spell)

    #
    # Public API
    #

    ###########################################################################
    @classmethod
    def create_spell(cls, name, level, location=None):
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

    ###########################################################################
    @classmethod
    def _iter_hook(cls):
    ###########################################################################
        """
        Iterate over valid spell names.
        """
        spell_names = [name for name in sorted(cls.__spell_map)]
        return iter(spell_names)

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
