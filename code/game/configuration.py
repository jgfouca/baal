#! /usr/bin/env python

import inspect, unittest

from baal_common import prequire, ProgramError

###############################################################################
class Configuration(object):
###############################################################################
    """
    A singleton class, the global object will contain the user's configuration
    choices for this execution of the game.

    Note, no knowledge of configuration semantics should be exposed here. This
    class only knows what configuration items it has, not what they mean.
    """

    __instance = None

    #
    # Public API
    #

    ###########################################################################
    @classmethod
    def instance(cls):
    ###########################################################################
        """
        Retrieve singleton instance. Note this does *not* construct the
        singleton if it does not exist; the private creation class method
        needs to be called to create the singleton.
        """
        prequire(cls.__instance is not None, "Uninitialized global instance")
        return cls.__instance

    # Getters for config pieces

    def interface_config(self): return self.__interface_config

    def world_config(self): return self.__world_config

    def player_config(self): return self.__player_config

    #
    # Private API
    #

    ###########################################################################
    @classmethod
    def _create(cls, interface_config, world_config, player_config):
    ###########################################################################
        prequire(cls.__instance is None, "Already created")
        cls.__instance = Configuration(interface_config,
                                       world_config,
                                       player_config,
                                       inspect.currentframe())
        return cls.__instance

    ###########################################################################
    def __init__(self, interface_config, world_config, player_config, caller):
    ###########################################################################
        prequire(inspect.getframeinfo(caller).function == "_create",
                 "This is a private constructor; "
                 "use _create to create a configuration object")

        self.__interface_config = interface_config
        self.__world_config = world_config
        self.__player_config = player_config

#
# Tests
#

###############################################################################
class TestConfiguration(unittest.TestCase):
###############################################################################

    ###########################################################################
    def test_configuration(self):
    ###########################################################################
        # Check trying to grab instance before calling _create
        self.assertRaises(ProgramError, Configuration.instance)

        instance = Configuration._create("interface", "world", "player")

        # Check double-call of _create
        self.assertRaises(ProgramError, Configuration._create, "", "", "")

        # Check attempt to create Configuration directly
        self.assertRaises(ProgramError, Configuration,
                          "", "", "", inspect.currentframe())

        # Verify config values
        self.assertEqual(instance.interface_config(), "interface")
        self.assertEqual(instance.world_config(), "world")
        self.assertEqual(instance.player_config(), "player")

        # Verify global instance
        self.assertIs(instance, Configuration.instance())

if (__name__ == "__main__"):
    unittest.main()