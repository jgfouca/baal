#! /usr/bin/env python

import unittest

from baal_common import prequire, urequire, UserError, grant_access
from spell_factory import SpellFactory
from spell import Spell

###############################################################################
class Talents(object):
###############################################################################
    """
    Keeps track of a Player's talent tree and enforces spell prereqs. No one
    should interact directly with objects of this class except for Player.
    """

    #
    # ==== Public API ====
    #

    def __init__(self, player): self.__init_impl(player)

    #
    # Getter / Query API
    #

    def learnable(self):
        """
        Iterate over all learnable spells as (name, level) pairs
        """
        return self.__learnable_impl()

    def __contains__(self, spell_spec):
        """
        Check if the talent tree has a spell. The arg can either be a spell
        or a (name, level) pair
        """
        return self.__contains_impl(spell_spec)

    def __iter__(self):
        """
        Iterate over all spells this player knows as (name, level) pairs
        """
        return self.__iter_impl()

    def to_xml(self): return self.__to_xml_impl()

    #
    # Modification API
    #

    def add(self, spell_name):
        """
        Learn or add skill in spell of name for player
        """
        return self.__add_impl(spell_name)

    #
    # ==== Class constants ====
    #

    _MAX_SPELL_LEVEL = 5

    #
    # ==== Internal methods ====
    #

    def _check_prereqs(self, spell_name, spell_level):
        """
        Check if player is able to learn spell of name/level
        """
        return self.__check_prereqs_impl(spell_name, spell_level)

    def _validate_invariants(self):
        """
        Check class invariants
        """
        return self.__validate_invariants_impl()

    #
    # ==== Implementation ====
    #

    ###########################################################################
    def __init_impl(self, player):
    ###########################################################################
        self.__num_learned = 0
        self.__learned_spells = {}
        self.__player = player # The player whos talents we're managing

    ###########################################################################
    def __learnable_impl(self):
    ###########################################################################
        for spell_name in SpellFactory:
            for spell_level in xrange(1, self._MAX_SPELL_LEVEL + 1):
                # Check if we already know this spell
                if ( (spell_name, spell_level) in self ):
                    continue
                try:
                    self._check_prereqs(spell_name, spell_level)
                    yield spell_name, spell_level
                    break
                except UserError:
                    break

    ###########################################################################
    def __contains_impl(self, spell_spec):
    ###########################################################################
        if (isinstance(spell_spec, Spell)):
            spell_tuple = (spell_spec.name(), spell_spec.level())
        else:
            prequire(type(spell_spec) == tuple)
            spell_tuple = spell_spec

        spell_name, spell_level = spell_tuple
        return spell_name in self.__learned_spells and \
            spell_level <= self.__learned_spells[spell_name]

    ###########################################################################
    def __iter_impl(self):
    ###########################################################################
        for spell_name, max_level in sorted(self.__learned_spells.iteritems()):
            for level in range(1, max_level+1):
                yield spell_name, level

    ###########################################################################
    def __to_xml_impl(self):
    ###########################################################################
        # TODO - Aaron
        pass

    ###########################################################################
    def __add_impl(self, spell_name):
    ###########################################################################
        # Compute implied spell-level
        if (spell_name in self.__learned_spells):
            spell_level = self.__learned_spells[spell_name] + 1
        else:
            spell_level = 1

        # Check if it is OK for them to learn this spell
        self.__check_prereqs_impl(spell_name, spell_level)

        # Add spell
        if (spell_name in self.__learned_spells):
            self.__learned_spells[spell_name] += 1
        else:
            self.__learned_spells[spell_name] = 1

        # Increment num learned
        self.__num_learned += 1

        # Check invariants
        self.__validate_invariants_impl()

    ###########################################################################
    def __check_prereqs_impl(self, spell_name, spell_level):
    ###########################################################################
        urequire(self.__player.level() > self.__num_learned,
                 "You cannot learn any more spells until you level-up")
        urequire(spell_level <= self._MAX_SPELL_LEVEL,
                 "You've already hit the maximum level for that spell")

        # Compute prereqs
        spell = SpellFactory.create_spell(spell_name, spell_level)
        prereqs = spell.prereqs()
        min_level = prereqs.min_player_level()
        must_know_spells = prereqs.must_know_spells()

        # Check prereqs
        urequire(self.__player.level() >= min_level,
                 "You must be level ", min_level, " to learn this spell")
        for prereq_spell_spec in must_know_spells:
            urequire(prereq_spell_spec in self,
                     "Missing required prereq: ", "%s[%d]" % prereq_spell_spec)

    ###########################################################################
    def __validate_invariants_impl(self):
    ###########################################################################
        level_sum = 0
        for spell_name, spell_level in self.__learned_spells.iteritems():
            prequire(spell_level >= 1 and spell_level <= self._MAX_SPELL_LEVEL,
                     "Bad spell level: ", spell_level)
            level_sum += spell_level

        prequire(self.__num_learned == level_sum,
                 "Num-learned invariant failed")

#
# Tests
#

###############################################################################
class TestTalents(unittest.TestCase):
###############################################################################

    ###########################################################################
    def test_talents(self):
    ###########################################################################
        from player import Player
        from configuration import Configuration

        Configuration._create()
        player = Player()
        talents = Talents(player)
        tier1_spell_name = "hot"
        another_tier1_spell_name = "cold"
        tier2_spell_name = "fire"
        high_tier_spell_name = "quake"
        not_a_spell = "lol"

        # Give ourselves ability to add exp to player
        grant_access(self, Player.ALLOW_GAIN_EXP)

        # Get player up to level 10
        for i in xrange(9):
            exp_needed = player.next_level_cost() - player.exp()
            player.gain_exp(exp_needed)

        self.assertEqual(player.level(), 10)

        # Should not be able to learn fire 1 (don't know hot 1)
        self.assertRaises(UserError, talents.add, tier2_spell_name)

        # Should not be able to learn quake 1 (not high enough level)
        self.assertRaises(UserError, talents.add, high_tier_spell_name)

        # Should not be able to learn an invalid spell
        self.assertRaises(UserError, talents.add, not_a_spell)

        # Learn hot up to max
        for i in xrange(talents._MAX_SPELL_LEVEL):
            talents.add(tier1_spell_name)

        # Should not be able to learn tier1 spell again, maxed out
        self.assertRaises(UserError, talents.add, tier1_spell_name)

        # Should be able to learn fire now
        for i in xrange(talents._MAX_SPELL_LEVEL):
            talents.add(tier2_spell_name)

        # Should not be able to learn a different tier1 spell, out of points
        self.assertRaises(UserError, talents.add, another_tier1_spell_name)

        # Check iteration
        known_spells = []
        for spell_spec in talents:
            known_spells.append(spell_spec)

        # Check that we know the spells we've learned
        for spell_name in [tier1_spell_name, tier2_spell_name]:
            for spell_level in xrange(1, talents._MAX_SPELL_LEVEL):
                self.assertIn( (spell_name, spell_level), talents )
                self.assertIn( (spell_name, spell_level), known_spells )

        self.assertEqual(len(known_spells), 10)

if (__name__ == "__main__"):
    unittest.main()
