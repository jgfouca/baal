#! /usr/bin/env python

"""
This file contains the API and implementation of everything having to do
with interfaces in baal.
"""

import readline, sys, unittest, pygame

from configuration import Configuration
from baal_common import prequire, clear_screen, SmartEnum, UserError, cprint, \
    RED, GREEN, create_names_by_enum_value
from command_factory import CommandFactory
from engine import engine
from draw_text import DrawText
from draw_pygame import DrawPygame

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
    PYGAME, \
    PANDA = range(3)

    # Derive names from class members.
    _NAMES = create_names_by_enum_value(vars())

    def __init__(self, value):
        super(Interfaces, self).__init__(value)

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
        self._end_turns = 0

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
        self._end_turns = num_turns

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
    interface = _get_inteface_config_str()

    if (Interfaces.TEXT == interface):
        return TextInterface()
    elif (Interfaces.PYGAME == interface):
        return PygameInterface()
    else:
        prequire(False, "Missing support for ", interface)

###############################################################################
def is_text_interface():
###############################################################################
    interface = _get_inteface_config_str()
    return Interfaces.TEXT == interface

#
# Internal-only below
#

###############################################################################
def _get_inteface_config_str():
###############################################################################
    interface = Configuration.instance().interface_config()

    # First is default
    if (interface == ""):
        interface = [str(item) for item in Interfaces][0]

    return interface

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
        super(TextInterface, self).__init__()

        # Set up readline
        readline.set_completer(_readline_completer)
        readline.parse_and_bind("tab: complete")

        self.__drawer = DrawText()

    ###########################################################################
    def draw(self):
    ###########################################################################
        clear_screen()

        # Draw world
        self.__drawer.draw(engine().world())

        # Draw Player
        self.__drawer.draw(engine().player())
        print

        # Draw AI Player
        self.__drawer.draw(engine().ai_player())
        print

        sys.stdout.flush()

    ###########################################################################
    def interact(self):
    ###########################################################################
        # Enter loop for this turn
        while (self._end_turns == 0):
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

        self._end_turns -= 1

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
        cprint(GREEN, "GRATZ, UR WINNAR!\n")
        sys.stdout.flush()

    ###########################################################################
    def ai_wins(self):
    ###########################################################################
        cprint(RED, "UR LUZER. LOL, GET PWNED\n")
        sys.stdout.flush()

###############################################################################
class PygameInterface(Interface):
###############################################################################
    """
    Pygame-based implementation of an interface.
    """

    ###########################################################################
    def __init__(self):
    ###########################################################################
        super(PygameInterface, self).__init__()

        self.__drawer = DrawPygame()

        self.__spell_report_buffer = []

        # Very basic opening menu
        self.__drawer.popup("Menu",
"""
Welcome to Ba'al!

Explain game at a high level here.

Right click on buttons you don't understand.

Enjoy!
"""
)

    ###########################################################################
    def draw(self):
    ###########################################################################
        # Begin draw cycle
        self.__drawer.begin_draw()

        # Draw world
        self.__drawer.draw(engine().world())

        # Draw Player
        self.__drawer.draw(engine().player())

        # Draw AI Player
        self.__drawer.draw(engine().ai_player())

        # End draw cycle
        self.__drawer.end_draw()

    ###########################################################################
    def interact(self):
    ###########################################################################
        # Enter loop for this turn
        while (self._end_turns == 0):

            # Watch for pygame events.
            for event in pygame.event.get():
                if (event.type == pygame.QUIT):
                    engine().quit()
                    break
                elif (event.type == pygame.MOUSEBUTTONDOWN):
                    try:
                        button1, button2, button3 = pygame.mouse.get_pressed()
                        mouse_pos = pygame.mouse.get_pos()
                        command_str = self.__drawer.clicked(mouse_pos,
                                                            button1,
                                                            button2,
                                                            button3)
                        if (command_str is not None):
                            print command_str
                            command = CommandFactory.parse_command(command_str)
                            command.apply()
                            self.__flush_spell_report_buffer()
                    except UserError, error:
                        self.__drawer.popup("Error", error)

                    # Successful commands should redraw
                    if (command_str is not None):
                        self.draw()

        self._end_turns -= 1

    ###########################################################################
    def help(self, helpmsg):
    ###########################################################################
        self.__drawer.popup("help", helpmsg)

    ###########################################################################
    def spell_report(self, report):
    ###########################################################################
        self.__spell_report_buffer.append(report)

    ###########################################################################
    def human_wins(self):
    ###########################################################################
        self.__drawer.popup("Game Over", "GRATZ, UR WINNAR!")

    ###########################################################################
    def ai_wins(self):
    ###########################################################################
        self.__drawer.popup("Game Over", "UR LUZER. LOL, GET PWNED")

    ###########################################################################
    def __flush_spell_report_buffer(self):
    ###########################################################################
        if (self.__spell_report_buffer):
            self.__drawer.popup("Spell Report",
                                "\n".join(self.__spell_report_buffer))
            self.__spell_report_buffer = []

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
