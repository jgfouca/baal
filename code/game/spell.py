#! /usr/bin/env python

import unittest

from baal_common import prequire, urequire, UserError
from city import City
from engine import engine

###############################################################################
class Spell(object):
###############################################################################
    """
    Abstract base class for all spells. The base class will take
    care of everything except how the spell affects the world.
    """

    #
    # ==== Public API ====
    #

    def __init__(self, level, location): self.__init_impl(level, location)

    #
    # Class-level property getters (all abstract, subclasses must define)
    #

    @classmethod
    def name(cls):
        """
        Get the name of the spell; this is what the user will type to refer
        to this spell.
        """
        prequire(False, "Called abstract version of name")

    @classmethod
    def base_cost(cls):
        """
        The base (level-1) cost of this spell.
        """
        prequire(False, "Called abstract version of base_cost")

    @classmethod
    def cost_func(cls):
        """
        Describes how the cost of this spell increases as it's level increases
        """
        prequire(False, "Called abstract version of cost_func")

    @classmethod
    def prereqs(cls):
        """
        Returns the prerequisites for this spell.
        """
        prequire(False, "Called abstract version of prereqs")

    #
    # Instance getters/queries
    #

    def cost(self):
        """
        Return the mana cost of this spell
        """
        return self.__cost_impl()

    def level(self): return self.__level

    def location(self): return self.__location

    def __str__(self): return self.__str_impl()

    #
    # Abstract API
    #

    def verify_apply(self):
        """
        Verify that the user's attempt to cast this spell is valid. This method
        will throw a user-error if there's a problem.
        """
        prequire(False, "Called abstract version of verify_apply")

    def apply(self):
        """
        Apply the casting of this spell to the game state.
        This should NEVER throw.
        """
        prequire(False, "Called abstract version of apply")

    #
    # ==== Class constants ====
    #

    __CITY_DESTROY_EXP_BONUS = 1000
    __CHAIN_REACTION_BONUS = 2
    __INFRA_EXP_FUNC = lambda infra_destroyed: pow(2, infra_destroyed) * 200

    #
    # ==== Internal API ====
    #

    def _kill(self, city, pct_killed):
        """
        Kill-off a certain percent of the population of a city.
        Returns the exp gained.
        """
        return self.__kill_impl(city, pct_killed)

    def _destroy_infra(self, tile, max_destroyed):
        """
        Reduce the infra of a tile by up to max_destroyed.
        """
        return self.__destroy_infra_impl(tile, max_destroyed)

    def _damage_tile(self, tile, pct_damaged):
        """
        Damage a tile by pct_damaged.
        """
        return self.__damage_tile_impl(tile, pct_damaged)

    def _spawn(self, name, level):
        """
        Some disasters can spawn other disasters (chain reaction). This method
        encompasses the implementation of this phenominon. The amount of exp
        gained is returned.
        """
        return self.__spawn_impl(name, level)

    def _report(self, *args):
        """
        Report things that are happening because of the spell.
        """
        return self.__report_impl(*args)

    #
    # ==== Implementation ====
    #

    ###########################################################################
    def __init_impl(self, level, location):
    ###########################################################################
        self.__level    = level
        self.__location = location

    ###########################################################################
    def __cost_impl(self):
    ###########################################################################
        return self.cost_func()(self.base_cost(), self.level())

    ###########################################################################
    def __str_impl(self):
    ###########################################################################
        return "%s[%d]" % (self.name(), self.level())

    ###########################################################################
    def __report_impl(self, *args):
    ###########################################################################
        msg = "%s: %s" % (self.name(), "".join([str(arg) for arg in args]))
        engine().interface().spell_report(msg)

    ###########################################################################
    def __kill_impl(self, city, pct_killed):
    ###########################################################################
        prequire(pct_killed > 0.0, "Do not call this if no one killed")

        pct_killed = min(pct_killed, 100.0)
        num_killed = city.population() * (pct_killed / 100)
        city.kill(self, num_killed)
        self._report("killed ", num_killed)

        if (city.population() < City.MIN_CITY_SIZE):
            self._report("obliterated city '", city.name(), "'")
            engine().world().remove_city(city)
            city.kill(city.population())
            num_killed += city.population()

            # TODO: Give bigger city-kill bonus based on maximum
            # attained rank of city.
            return num_killed + self.__CITY_DESTROY_EXP_BONUS
        else:
            return num_killed

    ###########################################################################
    def __destroy_infra_impl(self, tile, max_destroyed):
    ###########################################################################
        if (max_destroyed == 0):
            return 0

        # Cannot destroy infra that's not there
        num_destroyed = min(tile.infra_level(), max_destroyed)
        tile.destroy_infra(num_destroyed)
        self._report("destroyed ", num_destroyed, " levels of infrastructure")

        # Convert to exp
        return self.__INFRA_EXP_FUNC(num_destroyed)

    ###########################################################################
    def __damage_tile_impl(self, tile, pct_damaged):
    ###########################################################################
        if (pct_damaged > 0.0):
            pct_damaged = min(pct_damaged, 100.0)

            tile.damage(pct_damaged)

            self._report("caused ", pct_damaged, "% damage to tile")

    ###########################################################################
    def __spawn_impl(self, name, level):
    ###########################################################################
        from spell_factory import SpellFactory

        spell = SpellFactory.create_spell(name, level, self.location())

        # Check if this spell can be applied here
        try:
            spell.verify_apply()

            self._report("caused a level ", level, " ", name)

            return self.__CHAIN_REACTION_BONUS * spell.apply()
        except UserError:
            pass

        return 0 # No exp if spell could not be applied

# We allow Spell to be able to kill
setattr(Spell, City.ALLOW_CITY_KILL, True)

###############################################################################
class SpellPrereq(object):
###############################################################################
    """
    Encapsulates data needed to describe a spell prerequisite. This class is
    more like a named pair than a full-blown class.
    """

    def __init__(self, min_player_level, must_know_spells):
        self.min_player_level = min_player_level
        self.must_know_spells = tuple(must_know_spells)

# TODO - Do we want spells for controlling all the basic properties
# of the atmosphere? Or do we want to leave some up to pure chance (pressure)?

# Spell header components are designed to maximize tweakability from
# the header file. TODO: should things like degrees-heated for Hot also be
# represented by a function? Yes!

#
# Spell Implementations. The rest of the system does not care about anything
# below. Only the abstract base class (Spell) should be exposed.
#

###############################################################################
class Hot(Spell):
###############################################################################
    """
    Increases the immediate temperature of a region. High temperatures can
    kill people in cities or deplete soil moisture on farms. This spell is not
    intended to be a primary damage dealer; instead, you should be using this
    spell to enhance the more-powerful spells.

    Enhanced by high temps, high dewpoints. Decreased by AI tech level.

    This is a tier 1 spell
    """

    #
    # Class Variables
    #

    __NAME = "hot"
    __BASE_COST = 50
    __COST_FUNC = lambda base, level: base * pow(1.3, level - 1) # 30% per lvl
    __PREREQS = SpellPrereq(1, ()) # No prereqs

    #
    # Public API
    #

    @classmethod
    def name(cls): return cls.__NAME

    @classmethod
    def base_cost(cls): return cls.__BASE_COST

    @classmethod
    def cost_func(cls): return cls.__COST_FUNC

    @classmethod
    def prereqs(cls): return cls.__PREREQS

    ###########################################################################
    def __init__(self, level, location):
    ###########################################################################
        super(self.__class__, self).__init__(level, location)

    ###########################################################################
    def verify_apply(self):
    ###########################################################################
        # No-op. This spell can be cast anywhere, anytime.
        pass

    ###########################################################################
    def apply(self):
    ###########################################################################
        # TODO
        return 0

#
# Tests
#

###############################################################################
class TestSpell(unittest.TestCase):
###############################################################################

    ###########################################################################
    def test_spell(self):
    ###########################################################################
        pass

if (__name__ == "__main__"):
    unittest.main()
