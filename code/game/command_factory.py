#! /usr/bin/env python

from baal_common import prequire, urequire, create_subclass_map, subclasses
from command import Command

class _CommandFactoryMeta(type):
    def __iter__(mcs): return CommandFactory._iter_hook()

###############################################################################
class CommandFactory(object):
###############################################################################
    """
    A CommandFactory creates commands. It encapsulates the knowledge of the
    set of available commands.
    """
    __metaclass__ = _CommandFactoryMeta

    # Class variables
    __cmd_map = create_subclass_map(Command, include_aliases=True)

    #
    # Public API
    #

    ###########################################################################
    @classmethod
    def parse_command(cls, text):
    ###########################################################################
        """
        Given some command text, create a Command object
        """
        # Get command name
        tokens = text.split()
        prequire(tokens, "Empty string made it into CommandFactory")
        name = tokens[0]

        # Instantiate and return new Command object
        return cls.get(name)(tokens[1:])

    ###########################################################################
    @classmethod
    def get(cls, name):
    ###########################################################################
        """
        Get Command class object associate with name.
        """
        urequire(name in cls.__cmd_map,
               "'%s'" % name, " is not a valid command. Type 'help' for help.")
        return cls.__cmd_map[name]

    #
    # Private API
    #

    ###########################################################################
    @classmethod
    def _iter_hook(cls):
    ###########################################################################
        """
        Iterate over the command classes. Do not call directly... invoke by
        'for cls in CommandFactory'.
        """
        real_names = [cmd.name() for cmd in subclasses(Command)]
        # Filter out alias entries
        return sorted([cmdcls
                       for cmdname, cmdcls in sorted(cls.__cmd_map.iteritems())
                       if cmdname in real_names])

#
# Tests
#

###############################################################################
def _tests():
###############################################################################
    pass

if (__name__ == "__main__"):
    _tests()
