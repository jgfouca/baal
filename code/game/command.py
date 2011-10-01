#! /usr/bin/env python

import time

from spell_factory import SpellFactory
from baal_common import prequire, urequire, Location, UserError, grant_access
from engine import engine
from draw_mode import DrawMode, _set_draw_mode, explain_draw_mode
from player import Player

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
    # ==== Public API ====
    #

    #
    # Instance API
    #

    def apply(self):
        """
        Apply this command to this system.
        """
        prequire(False, "Called abstract version of apply")

    #
    # Class-method API (with default implementations)
    #

    @classmethod
    def name(cls):
        """
        Get the name of the command, this is what the user will type to issue
        the command.
        """
        return cls._NAME

    @classmethod
    def aliases(cls):
        """
        Get aliases for the command, the user can type these as a shortcut for
        the command.
        """
        return cls._ALIASES

    @classmethod
    def help(cls, extra_args=None):
        """
        Get help information for this command.
        """
        if (_is_text_interface()):
            return _create_text_help_str(cls, cls._TEXT_USAGE)
        else:
            return cls._GRAPHICAL_USAGE

    #
    # ==== Class Constants ====
    #

    # These should be overridden
    _NAME            = None
    _ALIASES         = None
    _TEXT_USAGE      = None
    _GRAPHICAL_USAGE = None

#
# Command implementations. The rest of the system does not care about anything
# below. Only the abstract base class (Command) should be exposed.
#

###############################################################################
class _HelpCommand(Command):
###############################################################################

    # Class variables
    _NAME            = "help"
    _ALIASES         = ("h",)
    _TEXT_USAGE      = \
"""%s [<command>]
  Returns info/syntax help for a command or all commands if no argument
""" % _NAME
    _GRAPHICAL_USAGE = "Right click on items to get help"

    ###########################################################################
    def __init__(self, args):
    ###########################################################################
        self.__cmd = None
        self.__extra_args = None
        if (args):
            # Verify arg is a valid command name and get handle to cmd class
            from command_factory import CommandFactory
            self.__cmd = CommandFactory.get(args[0])
            if (len(args) > 1):
                self.__extra_args = args[1:]

    ###########################################################################
    def apply(self):
    ###########################################################################
        interface = engine().interface()

        # If user provided arg, give help for that cmd, otherwise give help for
        # all commands.
        if (self.__cmd is not None):
            interface.help(self.__cmd.help(self.__extra_args))
        else:
            from command_factory import CommandFactory
            for cmd_cls in CommandFactory:
                interface.help(cmd_cls.help())

###############################################################################
class _EndTurnCommand(Command):
###############################################################################

    # Class variables
    _NAME             = "end"
    _ALIASES          = ("n",)
    _TEXT_USAGE       = \
"""%s [<num-turns>]
Ends the current turn. Optional arg to skip ahead many turns
""" % _NAME
    _GRAPHICAL_USAGE  = "Ends the current turn."
    __MAX_SKIP_TURNS   = 100

    ###########################################################################
    def __init__(self, args):
    ###########################################################################
        urequire(len(args) <= 1, self.name(), " takes at most one argument")

        self.__num_turns = 1
        if (args):
            # Parse num turns
            try:
                self.__num_turns = int(args[0])
            except ValueError:
                urequire(False, "'%s' is not a valid integer" % args[0])
            urequire(self.__num_turns > 0 and
                     self.__num_turns <= self.__MAX_SKIP_TURNS,
                     "num-turns must be between 0 and ", self.__MAX_SKIP_TURNS)

    ###########################################################################
    def apply(self):
    ###########################################################################
        engine().interface().end_turn(self.__num_turns)

###############################################################################
class _QuitCommand(Command):
###############################################################################

    # Class variables
    _NAME            = "quit"
    _ALIASES         = ("q", "exit")
    _TEXT_USAGE      = \
"""%s
  Ends the game.
""" % _NAME
    _GRAPHICAL_USAGE = "Ends the game."

    ###########################################################################
    def __init__(self, args):
    ###########################################################################
        urequire(len(args) == 0, self.name(), " takes no arguments")

    ###########################################################################
    def apply(self):
    ###########################################################################
        engine().interface().end_turn()
        engine().quit()

###############################################################################
class _SaveCommand(Command):
###############################################################################

    # Class variables
    _NAME            = "save"
    _ALIASES         = ("s",)
    _TEXT_USAGE      = \
"""%s [<filename>]
  Saves the game; if no name provided, a name based on data/time will be used.
""" % _NAME
    _GRAPHICAL_USAGE = "Save the game."

    ###########################################################################
    def __init__(self, args):
    ###########################################################################
        urequire(len(args) <= 1, self.name(), " takes at most one argument")

        if (args):
            self.__savegame = args[0]
        else:
            self.__savegame = time.strftime("baal_%Y-%m-%d__%H:%M:%S.save")

    ###########################################################################
    def apply(self):
    ###########################################################################
        # TODO: Aaron, please re-implement in python
        urequire(False, "Not implemented yet")

###############################################################################
class _CastCommand(Command):
###############################################################################

    # Class variables
    _NAME            = "cast"
    _ALIASES         = ("c", "spell")
    _TEXT_USAGE      = \
"""%s <spell-name> <level> <row,col>
  Casts spell of type <spell-name> and level <level> at location <row>,<col>
""" % _NAME
    _GRAPHICAL_USAGE = \
"""
Click to cast a specific spell. Once you've clicked this button, your next
click should be on the square of the world where you want the spell to be
casted.
"""

    ###########################################################################
    def __init__(self, args):
    ###########################################################################
        urequire(len(args) == 3, self.name(), " takes three arguments")

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

        grant_access(self, Player.ALLOW_CAST)
        grant_access(self, Player.ALLOW_GAIN_EXP)

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

    ###########################################################################
    @classmethod
    def help(cls, extra_args=None):
    ###########################################################################
        if (_is_text_interface()):
            full_usage = "%s  Castable spells:\n" % cls._TEXT_USAGE

            # Add info on castable spells to usage string
            for spell_name, spell_level in engine().player().talents():
                full_usage += "    %s : %d\n" % (spell_name, spell_level)

            return _create_text_help_str(cls, full_usage)
        else:
            prequire(extra_args is not None and len(extra_args) == 1,
                     "Expect spell name in extra_args")

            spell_name = extra_args[0]
            spell = SpellFactory.create_spell(spell_name)

            help_str = cls._GRAPHICAL_USAGE
            help_str += "\nDescription of %s spell:\n" % spell_name
            help_str += spell.info() + "\n"
            help_str += "Player has skill level %d in this spell\n" % \
                engine().player().spell_skill(spell_name)

            return help_str

###############################################################################
class _LearnCommand(Command):
###############################################################################

    # Class variables
    _NAME            = "learn"
    _ALIASES         = ("l",)
    _TEXT_USAGE      = \
"""%s <spell-name>
  Player learns spell of type <spell-name>. If spell already known, then
  skill in this spell is increased by one.
""" % _NAME
    _GRAPHICAL_USAGE = \
"""
Click to learn this skill OR increase skill in this spell.
"""

    ###########################################################################
    def __init__(self, args):
    ###########################################################################
        urequire(len(args) == 1, self.name(), " takes one argument")

        self.__spell_cls = SpellFactory.get(args[0])

        grant_access(self, Player.ALLOW_LEARN)

    ###########################################################################
    def apply(self):
    ###########################################################################
        engine().player().learn(self.__spell_cls.name())

    ###########################################################################
    @classmethod
    def help(cls, extra_args=None):
    ###########################################################################
        if (_is_text_interface()):
            full_usage = "%s  Learnable spells:\n" % cls._TEXT_USAGE

            # Add info on learnable spells to usage string
            for spell_name, spell_level in engine().player().learnable():
                full_usage += "    %s%s\n" % (spell_name,
                                          "" if spell_level == 1 else " (new)")

            return _create_text_help_str(cls, full_usage)
        else:
            prequire(extra_args is not None and len(extra_args) == 1,
                     "Expect spell name in extra_args")

            spell_name = extra_args[0]
            spell = SpellFactory.create_spell(spell_name)

            help_str = cls._GRAPHICAL_USAGE
            help_str += "\nDescription of %s spell:\n" % spell_name
            help_str += spell.info() + "\n"
            help_str += "Player has skill level %d in this spell\n" % \
                engine().player().spell_skill(spell_name)

            return help_str

###############################################################################
class _DrawCommand(Command):
###############################################################################

    # Class variables
    _NAME            = "draw"
    _ALIASES         = ("d", "show")
    _TEXT_USAGE      = \
"""%s [<draw-mode>]
  Re-draw the world; if arg is provided, draw mode is switched to that mode.
  Available draw modes:
    %s
""" % (_NAME,
       "\n    ".join([str(mode) for mode in DrawMode]))
    _GRAPHICAL_USAGE = \
"""
Re-draw the world with a diverent overlay.
"""

    ###########################################################################
    def __init__(self, args):
    ###########################################################################
        urequire(len(args) <= 1, self.name(), " takes at most one argument")

        # Parse draw mode
        self.__new_mode = None
        if (args):
            self.__new_mode = DrawMode(args[0])

    ###########################################################################
    def apply(self):
    ###########################################################################
        if (self.__new_mode is not None):
            # DrawCommand has the right to change draw mode
            _set_draw_mode(self.__new_mode)

        engine().interface().draw() # redraw

    ###########################################################################
    @classmethod
    def help(cls, extra_args=None):
    ###########################################################################
        if (_is_text_interface()):
            return _create_text_help_str(cls, cls._TEXT_USAGE)
        else:
            prequire(extra_args is not None and len(extra_args) == 1,
                     "Expect draw_mode in extra_args")
            draw_mode = extra_args[0]

            help_str =  cls._GRAPHICAL_USAGE
            help_str += "\nDescription of draw-mode %s:\n" % draw_mode
            help_str += explain_draw_mode(draw_mode)

            return help_str

###############################################################################
class _HackCommand(Command):
###############################################################################

    # Class variables
    _NAME            = "hack"
    _ALIASES         = ()
    _TEXT_USAGE      = \
"""%s [<exp>]
  Gives the player free arbitrary exp. If no arg is provided, enough exp is
  awarded to get the player to the next level. This is a cheat put in for
  testing.
""" % _NAME
    _GRAPHICAL_USAGE = \
"""
Click to give yourself a free level!
"""

    ###########################################################################
    def __init__(self, args):
    ###########################################################################
        urequire(len(args) <= 1, self.name(), " takes at most one argument")

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

#
# Internal methods
#

###############################################################################
def _create_text_help_str(cmd_cls, usage):
###############################################################################
    help_str = usage

    aliases = cmd_cls.aliases()
    if (aliases):
        help_str += "  Aliases: " + " ".join(aliases)

    return help_str

###############################################################################
def _is_text_interface():
###############################################################################
    # Importing interface within the global namespace will cause a circular
    # import, so we must wrap is_text_interface within an internal free func
    from interface import is_text_interface
    return is_text_interface()

#
# Tests
#

###############################################################################
def _test():
###############################################################################
    pass

if (__name__ == "__main__"):
    _test()
