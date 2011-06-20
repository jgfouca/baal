#! /usr/bin/env python

import unittest

from drawable import Drawable
from configuration import Configuration
from talents import Talents
from baal_common import urequire, prequire, check_access, cprint, \
    GREEN, BLUE, YELLOW, ProgramError, UserError, Location, \
    set_prequire_handler, raising_prequire_handler

###############################################################################
class Player(Drawable):
###############################################################################
    """
    Encapsulates player state.
    """

    #
    # ==== Public API ====
    #

    def __init__(self): self.__init_impl()

    #
    # Getter / Query API
    #

    def talents(self):
        """
        Iterate over known spells
        """
        return iter(self.__talents)

    def learnable(self):
        """
        Iterate over learnable spells
        """
        return self.__talents.learnable()

    def next_level_cost(self): return self.__next_level_cost

    def exp(self): return self.__exp

    def level(self): return self.__level

    def mana(self): return self.__mana

    def max_mana(self): return self.__max_mana

    def verify_cast(self, spell):
        """
        Check if this player can cast a spell. Throws a user error if the
        answer is no.
        """
        return self.__verify_cast_impl(spell)

    def to_xml(self): return self.__to_xml_impl()

    #
    # Drawing API
    #

    def draw_text(self): return self.__draw_text_impl()

    def draw_graphics(self): return self.__draw_graphics_impl()

    #
    # Modification API
    #

    def cycle_turn(self, caller):
        """
        Inform this player that the turn has cycled.
        """
        return self.__cycle_turn_impl(caller)

    def cast(self, caller, spell):
        """
        Adjust player state appropriately assuming that spell was cast. This
        method should never throw a user error. verify_cast should have been
        called prior to ensure that the casting of this spell is valid from
        this player's point-of-view.
        """
        return self.__cast_impl(caller, spell)

    def learn(self, caller, name):
        """
        Have this player learn a spell by name. If the player already
        knows the spell, the player's skill in that spell will be
        increased by one.  This method will throw a user error if the
        player cannot learn the spell.
        """
        return self.__learn_impl(caller, name)

    def gain_exp(self, caller, exp):
        """
        Inform the player that they have gained some exp.
        """
        return self.__gain_exp_impl(caller, exp)

    #
    # ==== Class constants ====
    #

    #
    # Tweakable constants
    #

    __STARTING_MANA            = 100
    __FIRST_LEVELUP_EXP_COST   = 100
    __MANA_REGEN_RATE          = 1.0 / 20
    __DEFAULT_PLAYER_NAME      = "human"

    @classmethod
    def __MANA_POOL_FUNC(cls, level):
        # 40% gain per lvl
        return cls.__STARTING_MANA * pow(1.4, level - 1)

    @classmethod
    def __EXP_LEVEL_COST_FUNC(cls, level):
        # 40% gain per lvl
        return cls.__FIRST_LEVELUP_EXP_COST * pow(1.4, level - 1)

    #
    # Modification-control keys
    #

    ALLOW_PLAYER_CYCLE_TURN    = "_allow_player_cycle_turn"
    ALLOW_PLAYER_CAST          = "_allow_player_cast"
    ALLOW_PLAYER_LEARN         = "_allow_player_learn"
    ALLOW_PLAYER_GAIN_EXP      = "_allow_player_gain_exp"

    #
    # ==== Implementation ====
    #

    ###########################################################################
    def __init_impl(self):
    ###########################################################################
        cls = self.__class__

        self.__name = Configuration.instance().player_config()
        self.__mana = cls.__STARTING_MANA
        self.__max_mana = self.__mana
        self.__mana_regen = self.__mana * cls.__MANA_REGEN_RATE
        self.__exp = 0
        self.__level = 1
        self.__next_level_cost = cls.__FIRST_LEVELUP_EXP_COST
        self.__talents = Talents(self)

    ###########################################################################
    def __verify_cast_impl(self, spell):
    ###########################################################################
        urequire(spell.cost() <= self.__mana,
                 "Spell requires ", spell.cost(),
                 " mana, player only has ", self.__mana, " mana")
        urequire(spell in self.__talents, "Player cannot cast spell ", spell)

    ###########################################################################
    def __to_xml_impl(self):
    ###########################################################################
        # TODO - Aaron
        pass

    ###########################################################################
    def __draw_text_impl(self):
    ###########################################################################
        print "PLAYER STATS:"
        print "  name:", self.__name
        print "  level:",
        cprint(GREEN, self.__level)
        print "  mana:",
        cprint(BLUE, self.__mana, "/", self.__max_mana)
        print "  exp:",
        cprint(YELLOW, self.__exp, "/", self.__next_level_cost)

    ###########################################################################
    def __draw_graphics_impl(self):
    ###########################################################################
        # TODO
        pass

    ###########################################################################
    def __cycle_turn_impl(self, caller):
    ###########################################################################
        check_access(caller, self.ALLOW_PLAYER_CYCLE_TURN)

        self.__mana += self.__mana_regen_rate
        if (self.__mana > self.__max_mana):
            self.__mana = self.__max_mana

        # Maintain mana invariant
        prequire(self.__mana <= self.__max_mana,
               "m_mana(", self.__mana, ") > m_max_mana(", self.__max_mana, ")")

    ###########################################################################
    def __cast_impl(self, caller, spell):
    ###########################################################################
        check_access(caller, self.ALLOW_PLAYER_CAST)

        self.__mana -= spell.cost()

        # Maintain mana invariant (m_mana is unsigned, so going below zero will
        # cause it to become enormous).
        prequire(self.__mana <= self.__max_mana,
                 "mana(", self.__mana, ") > max_mana(", self.__max_mana, ")")

    ###########################################################################
    def __learn_impl(self, caller, name):
    ###########################################################################
        check_access(caller, self.ALLOW_PLAYER_LEARN)

        self.__talents.add(name)

    ###########################################################################
    def __gain_exp_impl(self, caller, exp):
    ###########################################################################
        check_access(caller, self.ALLOW_PLAYER_GAIN_EXP)

        self.__exp += exp

        # Check for level-up
        while (self.__exp >= self.__next_level_cost):
            self.__level += 1

            old_max = self.__max_mana
            self.__max_mana = self.__MANA_POOL_FUNC(self.level())
            self.__mana_regen_rate = self.__max_mana * self.__MANA_REGEN_RATE
            increase = self.__max_mana - old_max
            self.__mana += increase

            self.__exp -= self.__next_level_cost
            self.__next_level_cost = self.__EXP_LEVEL_COST_FUNC(self.level())

        # Check exp invariant
        prequire(self.__exp < self.__next_level_cost,
                 "exp(", self.__exp,  ") > ",
                 "next_level_cost(", self.__next_level_cost, ")")

        # Check mana invariant
        prequire(self.__mana <= self.__max_mana,
                 "mana(", self.__mana, ") > max_mana(", self.__max_mana, ")")

#
# Tests
#

###############################################################################
class TestPlayer(unittest.TestCase):
###############################################################################

    ###########################################################################
    def test_player(self):
    ###########################################################################
        # Change to raising handler for unit-testing
        set_prequire_handler(raising_prequire_handler)

        Configuration._create("", "", "")
        player = Player()
        spell = "hot"

        exp_needed = player.next_level_cost() - player.exp()

        self.assertRaises(ProgramError, player.gain_exp, self, exp_needed)

        setattr(self, Player.ALLOW_PLAYER_GAIN_EXP, True)
        player.gain_exp(self, exp_needed)
        self.assertEqual(player.level(), 2)

        from spell_factory import SpellFactory

        hot_spell_1 = SpellFactory.create_spell(spell, 1)

        # TODO - Uncomment once Talents are implemented
        #self.assertRaises(UserError, player.verify_cast, hot_spell_1)

        self.assertRaises(ProgramError, player.learn, self, spell)
        setattr(self, Player.ALLOW_PLAYER_LEARN, True)
        player.learn(self, spell)

        player.verify_cast(hot_spell_1)

        self.assertRaises(ProgramError, player.cast, self, hot_spell_1)
        setattr(self, Player.ALLOW_PLAYER_CAST, True)
        player.cast(self, hot_spell_1)

        self.assertEqual(player.mana(), player.max_mana() - hot_spell_1.cost())

if (__name__ == "__main__"):
    unittest.main()
