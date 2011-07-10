#! /usr/bin/env python

import unittest

from drawable import Drawable
from baal_common import prequire, cprint, GREEN, BLUE, \
    check_access, grant_access
from engine import engine
from baal_math import poly_growth
from city import City

###############################################################################
class PlayerAI(Drawable):
###############################################################################
    """
    Manages the "global" (higher-than-city) affairs of the AI. Since most
    of the AI's "thought" goes into city management, most of the AI code is
    actually in the City class.
    """

    #
    # ==== Public API ====
    #

    def __init__(self): self.__init_impl()

    #
    # Queries / getters
    #

    def get_adjusted_yield(self, base_yield):
        """
        The AI receives yield bonuses based on their tech-level. This method
        returns the tech-adjusted yield for a base yield.
        """
        return self.__get_adjusted_yield_impl(base_yield)

    def population(self): return self.__population

    def tech_level(self): return self.__tech_level

    def to_xml(self): return self.__to_xml_impl()

    #
    # Drawing API
    #

    def draw_text(self): return self.__draw_text_impl()

    def draw_graphics(self): return self.__draw_graphics_impl()

    #
    # Modifying API
    #

    def cycle_turn(self):
        """
        Notify the AI player that the turn has cycled.
        """
        self.__cycle_turn_impl()

    #
    # ==== Class constants ====
    #

    ALLOW_CYCLE_TURN = "_player_ai_allow_cycle_turn"

    __STARTING_TECH_LEVEL = 1
    __FIRST_TECH_LEVEL_COST = 1000

    # We express arbitrary rule choices as short classmethods that are
    # treated like callable constants to facilitate tweakability

    @classmethod
    def __TECH_NEXT_LEVEL_COST_FUNC(cls, tech_level):
        # first * tech_level ^ 1.5
        return cls.__FIRST_TECH_LEVEL_COST * \
            poly_growth(tech_level - cls.__STARTING_TECH_LEVEL, 1.5, 1)

    @classmethod
    def __TECH_POINT_FUNC(cls, population):
        return population / 100 # 1 tech point per 100 people

    @classmethod
    def __ADJUSTED_YIELD_FUNC(cls, base_yield, tech_level):
        # 10% per tech level
        return base_yield * \
            (1 + ((tech_level - cls.__STARTING_TECH_LEVEL) * 0.1))

    #
    # ==== Implementaion ====
    #

    ###########################################################################
    def __init_impl(self):
    ###########################################################################
        cls = self.__class__

        self.__tech_level           = cls.__STARTING_TECH_LEVEL
        self.__tech_points          = 0
        self.__next_tech_level_cost = cls.__FIRST_TECH_LEVEL_COST
        self.__population           = 0

        grant_access(self, City.ALLOW_CYCLE_TURN)

    ###########################################################################
    def __get_adjusted_yield_impl(self, base_yield):
    ###########################################################################
        cls = self.__class__
        return cls.__ADJUSTED_YIELD_FUNC(base_yield, self.tech_level())

    ###########################################################################
    def __to_xml_impl(self):
    ###########################################################################
        # TODO Aaron
        pass

    ###########################################################################
    def __draw_text_impl(self):
    ###########################################################################
        print "AI PLAYER STATS:"
        print "  tech level:",
        cprint(GREEN, self.tech_level())
        print "\n  population:",
        cprint(BLUE, self.population())

    ###########################################################################
    def __draw_graphics_impl(self):
    ###########################################################################
        # TODO
        pass

    ###########################################################################
    def __cycle_turn_impl(self):
    ###########################################################################
        check_access(self.ALLOW_CYCLE_TURN)

        cls = self.__class__
        world = engine().world()
        cities_orig = list(world.cities())

        # Manage cities. Note that this may cause additional cities to
        # be created, which is why we needed the list copy above
        for city in cities_orig:
            city.cycle_turn()

        # Compute population
        self.__population = 0
        for city in world.cities():
            self.__population += city.population()

        # Adjust tech based on population
        tech_points = cls.__TECH_POINT_FUNC(self.population())
        self.__tech_points += tech_points

        # Check if level-up in tech
        while (self.__tech_points >= self.__next_tech_level_cost):
            self.__tech_level += 1
            self.__tech_points -= self.__next_tech_level_cost # rollover points
            self.__next_tech_level_cost = \
                cls.__TECH_NEXT_LEVEL_COST_FUNC(self.tech_level())

        # Tech invariants
        prequire(self.__tech_points < self.__next_tech_level_cost,
                 "Expect tech-points(", self.__tech_points,
                 ") < tech-cost(", self.__next_tech_level_cost, ")")

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
