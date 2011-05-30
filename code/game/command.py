#! /usr/bin/env python

import time

from spell_factory import SpellFactory
from baal_common import prequire, urequire, Location, UserError
from command_factory import CommandFactory
from engine import engine
from drawable import DrawMode
import drawable

###############################################################################
class Command(object):
###############################################################################
    """
    A Command is the entity by which a player affects the game state.
    This class is an abstract base class intended to impose an interface on all
    Commands. Subclasses of Command are automatically added to the
    CommandFactory.
    """

    def __init__(self): prequire(False, "Do not instantiate Command")

    #
    # Instance API
    #

    def apply(self):
        """
        Apply this command to this system.
        """
        prequire(False, "Called abstract version of apply")

    #
    # Class-method API
    #

    @classmethod
    def name(cls):
        """
        Get the name of the command, this is what the user will type to issue
        the command.
        """
        prequire(False, "Called abstract version of name")

    @classmethod
    def aliases(cls):
        """
        Get aliases for the command, the user can type these as a shortcut for
        the command.
        """
        prequire(False, "Called abstract version of aliases")

    @classmethod
    def help(cls):
        """
        Get help information for this command.
        """
        prequire(False, "Called abstract version of help")

###############################################################################
class HelpCommand(Command):
###############################################################################

    # Class variables
    __NAME    = "help"
    __ALIASES = ("h",)
    __USAGE   = \
"""%s [<command>]
  Returns info/syntax help for a command or all commands if no argument
""" % __NAME

    ###########################################################################
    def __init__(self, args):
    ###########################################################################
        urequire(len(args) <= 1,
                 HelpCommand.name(), " takes at most one argument")

        self.__cmd = None
        if (args):
            # Verify arg is a valid command name and get handle to cmd class
            self.__cmd = CommandFactory.get(args[0])

    ###########################################################################
    def apply(self):
    ###########################################################################
        interface = engine().interface()

        # If user provided arg, give help for that cmd, otherwise give help for
        # all commands.
        if (self.__cmd is not None):
            interface.help(self.__cmd.help())
        else:
            for cmd_cls in CommandFactory.iter_commands():
                interface.help(cmd_cls.help())

    @classmethod
    def name(cls): return cls.__NAME

    @classmethod
    def aliases(cls): return cls.__ALIASES

    @classmethod
    def help(cls): return _create_help_str(cls, cls.__USAGE)

###############################################################################
class EndTurnCommand(Command):
###############################################################################

    # Class variables
    __NAME    = "end"
    __ALIASES = ("n",)
    __USAGE   = \
"""%s [<num-turns>]
  Ends the current turn. Optional arg to skip ahead many turns
""" % __NAME
    __MAX_SKIP_TURNS = 100

    ###########################################################################
    def __init__(self, args):
    ###########################################################################
        cls = self.__class__
        urequire(len(args) <= 1, cls.name(), " takes at most one argument")

        self.__num_turns = 1
        if (args):
            # Parse num turns
            try:
                self.__num_turns = int(args[0])
            except ValueError:
                urequire(False, "'%s' is not a valid integer" % args[0])
            urequire(self.__num_turns > 0 and
                     self.__num_turns <= cls.__MAX_SKIP_TURNS,
                     "num-turns must be between 0 and ", cls.__MAX_SKIP_TURNS)

    ###########################################################################
    def apply(self):
    ###########################################################################
        engine().interface().end_turn(self.__num_turns)

    @classmethod
    def name(cls): return cls.__NAME

    @classmethod
    def aliases(cls): return cls.__ALIASES

    @classmethod
    def help(cls): return _create_help_str(cls, cls.__USAGE)

###############################################################################
class QuitCommand(Command):
###############################################################################

    # Class variables
    __NAME    = "quit"
    __ALIASES = ("q", "exit")
    __USAGE   = \
"""%s
  Ends the game.
""" % __NAME

    ###########################################################################
    def __init__(self, args):
    ###########################################################################
        cls = self.__class__
        urequire(len(args) == 0, cls.name(), " takes no arguments")

    ###########################################################################
    def apply(self):
    ###########################################################################
        engine().interface().end_turn()
        engine().quit()

    @classmethod
    def name(cls): return cls.__NAME

    @classmethod
    def aliases(cls): return cls.__ALIASES

    @classmethod
    def help(cls): return _create_help_str(cls, cls.__USAGE)

###############################################################################
class SaveCommand(Command):
###############################################################################

    # Class variables
    __NAME    = "save"
    __ALIASES = ("s",)
    __USAGE   = \
"""%s [<filename>]
  Saves the game; if no name provided, a name based on data/time will be used.
""" % __NAME

    ###########################################################################
    def __init__(self, args):
    ###########################################################################
        cls = self.__class__
        urequire(len(args) <= 1, cls.name(), " takes at most one argument")

        if (args):
            self.__savegame = args[0]
        else:
            self.__savegame = time.strftime("baal_%Y-%m-%d__%H:%M:%S.save")

    ###########################################################################
    def apply(self):
    ###########################################################################
        # TODO: Aaron, please re-implement in python
        urequire(False, "Not implemented yet")

    @classmethod
    def name(cls): return cls.__NAME

    @classmethod
    def aliases(cls): return cls.__ALIASES

    @classmethod
    def help(cls): return _create_help_str(cls, cls.__USAGE)

###############################################################################
class CastCommand(Command):
###############################################################################

    # Class variables
    __NAME    = "cast"
    __ALIASES = ("c", "spell")
    __USAGE   = \
"""%s <spell-name> <level> <row,col>
  Casts spell of type <spell-name> and level <level> at location <row>,<col>
""" % __NAME

    ###########################################################################
    def __init__(self, args):
    ###########################################################################
        cls = self.__class__
        urequire(len(args) == 3, cls.name(), " takes three arguments")

        # Get spell name
        self.__spell_name = args[0]

        # Parse spell level
        try:
            self.__spell_level = int(args[1])
        except ValueError:
            urequire(False, "arg ", "'%s' is not a valid integer" % args[1])

        # Parse location
        try:
            self.__spell_location = Location.parse(args[2])
        except UserError:
            urequire(False, "arg ", "'%s' not a valid location" % args[2])

    ###########################################################################
    def apply(self):
    ###########################################################################
        world  = engine().world()
        player = engine().player()

        # Ensure location is in-bounds
        urequire(world.in_bounds(self.__spell_location),
                 "Location ", self.__spell_location, " out of bounds. ",
                 "Max row is: ", world.height() - 1,
                 ", max col is: ", world.width() - 1)

        # Create the spell.
        spell = SpellFactory.create_spell(self.__spell_name,
                                          self.__spell_level,
                                          self.__spell_location)

        # Verify that player can cast this spell (can throw)
        player.verify_cast(spell)

        # Verify that it makes sense to cast this exact spell (can throw)
        spell.verify_apply()

        # These last two operations need to be atomic, neither should
        # ever throw a user error.
        try:
            # Let the player object know that the spell has been cast
            # and to adjust it's state accordingly.
            player.cast(spell)

            # Apply the spell to the world
            exp = spell.apply()

            # Give player experience
            player.gain_exp(exp)
        except UserError, e:
            prequire(False, "User error interrupted atomic operations: ", e)

    @classmethod
    def name(cls): return cls.__NAME

    @classmethod
    def aliases(cls): return cls.__ALIASES

    ###########################################################################
    @classmethod
    def help(cls):
    ###########################################################################
        full_usage = "%s  Castable spells:\n" % cls.__USAGE

        # Add info on castable spells to usage string
        for spell_name, spell_level in engine().player().talents():
            full_usage += "    %s : %d\n" % (spell_name, spell_level)

        return _create_help_str(cls, full_usage)

###############################################################################
class LearnCommand(Command):
###############################################################################

    # Class variables
    __NAME    = "learn"
    __ALIASES = ("l",)
    __USAGE   = \
"""%s <spell-name>
  Player learns spell of type <spell-name>. If spell already known, then
  skill in this spell is increased by one.
""" % __NAME

    ###########################################################################
    def __init__(self, args):
    ###########################################################################
        cls = self.__class__
        urequire(len(args) == 1, cls.name(), " takes one argument")

        self.__spell_cls = SpellFactory.get(args[0])

    ###########################################################################
    def apply(self):
    ###########################################################################
        engine().player().learn(self.__spell_cls.name())

    @classmethod
    def name(cls): return cls.__NAME

    @classmethod
    def aliases(cls): return cls.__ALIASES

    ###########################################################################
    @classmethod
    def help(cls):
    ###########################################################################
        full_usage = "%s  Learnable spells:\n" % cls.__USAGE

        # Add info on learnable spells to usage string
        for spell_name, spell_level in engine().player().talents().learnable():
            full_usage += "    %s%s" % (spell_name,
                                        "" if spell_level == 1 else " (new)")

        return _create_help_str(cls, full_usage)

###############################################################################
class DrawCommand(Command):
###############################################################################

    # Class variables
    __NAME    = "draw"
    __ALIASES = ("d", "show")
    __USAGE   = \
"""%s [<draw-mode>]
  Re-draw the world; if arg is provided, draw mode is switched to that mode.
  Available draw modes:
    %s
""" % (__NAME,
       "\n    ".join([str(mode) for mode in DrawMode]))

    ###########################################################################
    def __init__(self, args):
    ###########################################################################
        cls = self.__class__
        urequire(len(args) <= 1, cls.name(), " takes at most one argument")

        # Parse draw mode
        self.__new_mode = None
        if (args):
            self.__new_mode = DrawMode(args[0])

    ###########################################################################
    def apply(self):
    ###########################################################################
        if (self.__new_mode is not None):
            # DrawCommand has the right to change draw mode
            drawable._set_draw_mode(self.__new_mode)

        engine().interface().draw() # redraw

    @classmethod
    def name(cls): return cls.__NAME

    @classmethod
    def aliases(cls): return cls.__ALIASES

    @classmethod
    def help(cls): return _create_help_str(cls, cls.__USAGE)

###############################################################################
class HackCommand(Command):
###############################################################################

    # Class variables
    __NAME    = "hack"
    __ALIASES = ("h",)
    __USAGE   = \
"""%s [<exp>]
  Gives the player free arbitrary exp. If no arg is provided, enough exp is
  awarded to get the player to the next level. This is a cheat put in for
  testing.
""" % __NAME

    ###########################################################################
    def __init__(self, args):
    ###########################################################################
        cls = self.__class__
        urequire(len(args) <= 1, cls.name(), " takes at most one argument")

        # Parse exp
        self.__exp = None
        if (args):
            try:
                self.__exp = int(args[0])
            except ValueError:
                urequire(False, "'%s' is not a valid integer" % args[0])

    ###########################################################################
    def apply(self):
    ###########################################################################
        player = engine().player()

        if (self.__exp is not None):
            exp_gained = self.__exp
        else:
            exp_gained = player.next_level_cost() - player.exp()

        engine().player().gain_exp(exp_gained)

    @classmethod
    def name(cls): return cls.__NAME

    @classmethod
    def aliases(cls): return cls.__ALIASES

    @classmethod
    def help(cls): return _create_help_str(cls, cls.__USAGE)

#
# Internal methods
#

###############################################################################
def _create_help_str(cmd_cls, usage):
###############################################################################
    help_str = usage

    aliases = cmd_cls.aliases()
    if (aliases):
        help_str += "  Aliases: " + " ".join(aliases)

    return help_str

#
# Tests
#

###############################################################################
def _test():
###############################################################################
    pass

if (__name__ == "__main__"):
    _test()
