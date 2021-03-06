#! /usr/bin/env python

import unittest

from baal_common import prequire, ProgramError, check_access, grant_access, \
    set_prequire_handler, raising_prequire_handler, check_callers
from configuration import Configuration

###############################################################################
class Engine(object):
###############################################################################
    """
    Engine will serve as a mediator between the other key classes. It also
    facilitates interaction between objects by providing a way to get handles
    to the other key objects. Finally, it manages the high-level flow of the
    game.

    Design: To avoid the tedium of passing this class around everywhere,
    we've decided to make this class a singleton. The downside to this
    choice is that it makes relationships between the classes poorly
    defined since every class has access to each other. It also makes any
    testing that might have required 2 engines impossible.
    """

    __instance = None
    __initializing = False

    #
    # ==== Public API ====
    #

    def play(self):
        """
        Starts the game's main loop. Does not return until game is over.
        """
        self.__play_impl()

    def quit(self):
        """
        Quit the game.
        """
        self.__quit = True
        self.interface().end_turn()

    #
    # Public Getters
    #

    def world(self): return self.__world

    def interface(self): return self.__interface

    def player(self): return self.__player

    def ai_player(self): return self.__ai_player

    #
    # ==== Class constants ====
    #

    __AI_WINS_AT_TECH_LEVEL = 100

    #
    # ==== Private API / Implemetation ====
    #

    ###########################################################################
    def __init__(self):
    ###########################################################################
        """
        Do not call. Use free function to get a handle to an Engine
        """
        # Dump imports here to avoid circular imports
        from interface import create_interface
        from world_factory import create_world
        from player import Player
        from player_ai import PlayerAI
        from world import World

        grant_access(self, Player.ALLOW_CYCLE_TURN)
        grant_access(self, PlayerAI.ALLOW_CYCLE_TURN)
        grant_access(self, World.ALLOW_CYCLE_TURN)

        # This should only be invoked through _instance which should only
        # be invoked through engine(). We cannot use the check_access system
        # here because that system cannot be used to grant access to a free
        # function.
        check_callers(["_instance", "engine"])

        self.__interface = create_interface()
        self.__world     = create_world()
        self.__player    = Player()
        self.__ai_player = PlayerAI(self.__world)
        self.__quit      = False

    ###########################################################################
    @classmethod
    def _instance(cls):
    ###########################################################################
        """
        Do not call. Use free function to get a handle to an Engine
        """
        if (cls.__instance is None):
            prequire(not cls.__initializing,
                     "Engine instantiation has re-entered itself")
            cls.__initializing = True
            cls.__instance = Engine()
            cls.__initializing = False

        return cls.__instance

    ###########################################################################
    def __play_impl(self):
    ###########################################################################
        # Game loop, each iteration of this loop is a full game turn
        while (not self.__quit):

            # Draw current game state
            self.__interface.draw()

            # Human player takes turn
            self.__interface.interact()
            self.__player.cycle_turn()

            # AI player takes turn
            self.__ai_player.cycle_turn()

            # Cycle world. Note this should always be the last item to cycle.
            self.__world.cycle_turn()

            # Check for game-ending state
            if (self.__ai_player.population() == 0):
                self.__interface.human_wins()
                break
            elif (self.__ai_player.tech_level() >=
                  self.__AI_WINS_AT_TECH_LEVEL):
                self.__interface.ai_wins()
                break

#
# Free function API
#

###############################################################################
def engine():
###############################################################################
    """
    Get the current engine.
    """
    # Hides the fact that Engine is a singleton in case we want to change this
    # later.
    return Engine._instance()

#
# Tests
#

###############################################################################
class TestEngine(unittest.TestCase):
###############################################################################

    ###########################################################################
    def test_engine(self):
    ###########################################################################
        # Change to raising handler for unit-testing
        set_prequire_handler(raising_prequire_handler)

        # Test that we cannot create instances of Engine
        self.assertRaises(ProgramError, Engine)

        # Create configuration
        from interface import Interfaces
        config = Configuration._create(interface_config=str(Interfaces.TEXT),
                                       world_config="1")

        # Grab global instance
        eng = engine()

        # Integration test
        self.assertEqual(eng.ai_player().population(), 1000)

if (__name__ == "__main__"):
    unittest.main()
