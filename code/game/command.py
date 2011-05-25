#! /usr/bin/env python

import time

from baal_common import prequire,urequire

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
"""%s [command]
  Returns info/syntax help for a command or all commands if no argument
""" % __NAME

    ###########################################################################
    def __init__(self, args):
    ###########################################################################
        urequire(len(args) <= 1,
                 HelpCommand.__NAME, " takes at most one argument")

        self.__cmd = None
        if (args):
            # Verify arg is a valid command name and get handle to cmd class
            self.__cmd = CommandFactory.get(args[0])

    ###########################################################################
    def apply(self):
    ###########################################################################
        interface = Engine.instance().interface()

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
"""%s [num-turns]
  Ends the current turn. Optional arg to skip ahead many turns
""" % __NAME
    __MAX_SKIP_TURNS = 100;

    ###########################################################################
    def __init__(self, args):
    ###########################################################################
        cls = self.__class__
        urequire(len(args) <= 1, cls.__NAME, " takes at most one argument")

        self.__num_turns = 1
        if (args):
            # Parse num turns
            try:
                self.__num_turns = int(args[0])
            except ValueError:
                urequire(False, "'%s'" % args[0], " is not a valid integer")
            urequire(self.__num_turns > 0 and
                     self.__num_turns <= cls.__MAX_SKIP_TURNS,
                     "num-turns must be between 0 and ", cls.__MAX_SKIP_TURNS)

    ###########################################################################
    def apply(self):
    ###########################################################################
        Engine.instance().interface().end_turn(self.__num_turns)

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
        urequire(len(args) == 0, cls.__NAME, " takes no arguments")

    ###########################################################################
    def apply(self):
    ###########################################################################
        engine = Engine.instance()
        engine.interface().end_turn()
        engine.quit()

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
"""%s [filename]
  Saves the game; if no name provided, a name based on data/time will be used.
""" % __NAME

    ###########################################################################
    def __init__(self, args):
    ###########################################################################
        cls = self.__class__
        urequire(len(args) <= 1, cls.__NAME, " takes at most one argument")

        if (args):
            m_savegame = args[0]
        else:
            m_savegame = time.strftime("baal_%Y-%m-%d__%H:%M:%S.save")

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
        urequire(len(args) == 3, cls.__NAME, " takes three arguments")

        # Get spell name
        m_spell_name = args[0]

        # Parse spell level
        try:
            m_spell_level = int(args[1])
        except ValueError:
            urequire(False, "arg ", "'%s'" % args[1], " not a valid integer")

        # Parse location
        try:
            m_spell_location = Location.parse(args[2])
        except UserError:
            urequire(False, "arg ", "'%s'" % args[2]. " not a valid location")

    ###########################################################################
    def apply(self):
    ###########################################################################
        engine = Engine.instance()
        world  = engine.world()
        player = engine.player()

        # Ensure location is in-bounds
        urequire(world.in_bounds(m_spell_location),
                 "Location ", m_spell_location, " out of bounds. ",
                 "Max row is: ", world.height() - 1,
                 ", max col is: ", world.width() - 1)

        # Create the spell.
        spell = SpellFactory.create_spell(m_spell_name,
                                          m_spell_level,
                                          m_spell_location)

        # Verify that player can cast this spell (can throw)
        player.verify_cast(spell);

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
        castable_spells = \
            Engine.instance().player().talents().query_all_castable_spells()
        for spell_name, spell_level in sorted(castable_spells):
            full_usage += "    %s : %d" % (spell_name, spell_level)
            
        return _create_help_str(cls, full_usage)

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
