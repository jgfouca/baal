#! /usr/bin/env python

import unittest

from drawable import Drawable
from baal_common import prequire, cprint, GREEN, BLUE
from engine import engine
from baal_math import poly_growth

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

    def get_adjusted_yield(self, tile_yield):
        """
        The AI receives yield bonuses based on their tech-level. This method
        returns the multiplier that should be applied to the base yield for
        this AI.
        """
        return self.__get_adjusted_yield_impl(tile_yield)

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

    __STARTING_TECH_LEVEL = 1
    __FIRST_TECH_LEVEL_COST = 1000

    # We express arbitrary rule choices as lambdas to facilitate tweakability

    __TECH_NEXT_LEVEL_COST_FUNC = \
        lambda cls, tech_level: cls.__FIRST_TECH_LEVEL_COST * poly_growth(tech_level - cls.__STARTING_TECH_LEVEL, 1.5, 1) \
        # tech_level ^ 1.5

    __TECH_POINT_FUNC = \
        lambda cls, population: population / 100 # 1 tech point per 100 people

    __ADJUSTED_YIELD_FUNC = \
        lambda cls, tile_yield, tech_level: tile_yield * (1 + ((tech_level - cls.__STARTING_TECH_LEVEL) * 0.1)) \
        # 10% per tech level

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

    ###########################################################################
    def __get_adjusted_yield_impl(self, tile_yield):
    ###########################################################################
        cls = self.__class__
        return cls.__ADJUSTED_YIELD_FUNC(cls, tile_yield, self.tech_level())

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
        print "  population:",
        cprint(BLUE, self.population())

    ###########################################################################
    def __draw_graphics_impl(self):
    ###########################################################################
        # TODO
        pass

    ###########################################################################
    def __cycle_turn_impl(self):
    ###########################################################################
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
        tech_points = cls.__TECH_POINT_FUNC(cls, self.population())
        self.__tech_points += tech_points

        # Check if level-up in tech
        while (self.__tech_points >= self.__next_tech_level_cost):
            self.__tech_level += 1
            self.__tech_points -= self.__next_tech_level_cost # rollover points
            self.__next_tech_level_cost = \
                cls.__TECH_NEXT_LEVEL_COST_FUNC(cls, self.tech_level())

        # Tech invariants
        prequire(self.__tech_points < self.__next_tech_level_cost,
                 "Expect tech-points(" << self.__tech_points <<
                 ") < tech-cost(" << self.__next_tech_level_cost << ")")

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
