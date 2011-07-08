#! /usr/bin/env python

"""
This file contains the API and implementation of everything having to do
with interfaces in baal.
"""

import readline, sys, unittest

from configuration import Configuration
from baal_common import prequire, clear_screen, SmartEnum, UserError, cprint, \
    RED, GREEN
from command_factory import CommandFactory
from engine import engine

class _InterfacesMeta(type):
    def __iter__(mcs): return Interfaces._iter_hook()

    def __contains__(mcs, value): return Interfaces._in_hook(value)

###############################################################################
class Interfaces(SmartEnum):
###############################################################################
    """
    An enum that lists the various interface choices
    """

    __metaclass__ = _InterfacesMeta

    # Enum values. Note these are *placeholders*, the free code below this
    # class will replace these values with proper Interfaces objects.
    #
    # First listed interface will be default
    TEXT, \
    GRAPHICS = range(2)

    # Derive names from class members.
    _NAMES = [ name for name in dir() if name.isupper() and name.isalpha() ]

    def __init__(self, value):
        super(self.__class__, self).__init__(value)

# Finish Interfaces initialization
for name in Interfaces._names():
    setattr(Interfaces, name, Interfaces(name))

###############################################################################
class Interface(object):
###############################################################################
    """
    Interfaces are responsible for presenting information to the
    player and turning low-level player actions into official commands.
    Interfaces create Command objects which apply themselves to
    the game.

    This is an abstract base class.
    """

    #
    # Public API
    #

    def __init__(self):
        self.__end_turns = 0

    def draw(self):
        """
        Draw the game state.
        """
        prequire(False, "Must override")

    def interact(self):
        """
        Interact with the player until the turn is complete.
        """
        prequire(False, "Must override")

    def help(self, helpmsg):
        """
        Display a help message to the player
        """
        prequire(False, "Must override")

    def spell_report(self, report):
        """
        Report spell result to the player
        """
        prequire(False, "Must override")

    def end_turn(self, num_turns=1):
        """
        End the current turn, optionally ending multiple turns.
        """
        prequire(num_turns >= 0, "Bad end_turns: ", num_turns)
        self.__end_turns = num_turns

    def human_wins(self):
        """
        Inform the player that the human has won the game
        """
        prequire(False, "Must override")

    def ai_wins(self):
        """
        Inform the player that the AI has won the game
        """
        prequire(False, "Must override")

#
# Free Function API
#

###############################################################################
def create_interface():
###############################################################################
    interface = Interfaces(Configuration.instance().interface_config())

    # First is default
    if (interface == ""):
        interface = [str(item) for item in Interfaces][0]

    if (interface == Interfaces.TEXT):
        return TextInterface()
    elif (interface == Interfaces.GRAPHICS):
        return GraphicInterface()
    else:
        prequire(False, "Missing support for ", interface)

#
# Internal-only below
#

###############################################################################
def _readline_completer(text, state):
###############################################################################
    options = [cls.__name__ for cls in CommandFactory
               if cls.__name__.startswith(text)]
    try:
        return options[state]
    except IndexError:
        return None

#
# Interface Implementations
#

###############################################################################
class TextInterface(Interface):
###############################################################################
    """
    Text-based implementation of an interface. This should be the only entity
    in the system that has the right to print to the screen other than the
    draw_text methods.
    """

    ###########################################################################
    def __init__(self):
    ###########################################################################
        super(self.__class__, self).__init__()

        # Set up readline
        readline.set_completer(_readline_completer)
        readline.parse_and_bind("tab: complete")

    ###########################################################################
    def draw(self):
    ###########################################################################
        clear_screen()

        # Draw world
        engine().world().draw_text()
        print

        # Draw Player
        engine().player().draw_text()
        print

        # Draw AI Player
        engine().ai_player().draw_text()

        sys.stdout.flush()

    ###########################################################################
    def interact(self):
    ###########################################################################
        # Enter loop for this turn
        while (self.__end_turns == 0):
            # Grab a line of text
            try:
                line = raw_input("% ")
            except KeyboardInterrupt:
                # User ctrl-d
                engine().quit()
                break

            # Add to history and process if not empty string
            if (line):
                try:
                    command = CommandFactory.parse_command(line)
                    command.apply()
                except UserError, error:
                    print "ERROR:", error
                    print "\nType: 'help [command]' for assistence"
                    sys.stdout.flush()

        self.__end_turns -= 1

    ###########################################################################
    def help(self, helpmsg):
    ###########################################################################
        print helpmsg
        sys.stdout.flush()

    ###########################################################################
    def spell_report(self, report):
    ###########################################################################
        cprint(RED, report)
        sys.stdout.flush()

    ###########################################################################
    def human_wins(self):
    ###########################################################################
        cprint(GREEN, "GRATZ, UR WINNAR!")
        sys.stdout.flush()

    ###########################################################################
    def ai_wins(self):
    ###########################################################################
        cprint(RED, "UR LUZER. LOL, GET PWNED")
        sys.stdout.flush()

###############################################################################
class GraphicInterface(Interface):
###############################################################################
    # TODO
    pass

#
# Tests
#

###############################################################################
class TestInterface(unittest.TestCase):
###############################################################################

    ###########################################################################
    def test_interface(self):
    ###########################################################################
        # Test create_interface
        Configuration._create(interface_config=str(Interfaces.TEXT))
        interface = create_interface()
        self.assertEqual(type(interface), TextInterface)

if (__name__ == "__main__"):
    unittest.main()
