#! /usr/bin/env python

from baal_common import prequire, urequire, subclasses
from command import Command

class _CommandFactoryMeta(type):
    def __iter__(mcs): return CommandFactory._iter_hook()

#
# Internal methods
#

###############################################################################
def _create_cmd_map():
###############################################################################
    rv = {}
    for cmd_cls in subclasses(Command):
        _no_dup_insert(rv, cmd_cls.name(), cmd_cls)
        for alias in cmd_cls.aliases():
            _no_dup_insert(rv, alias, cmd_cls)

    return rv

###############################################################################
def _no_dup_insert(dict_, key, item):
###############################################################################
    prequire(key not in dict_, "Found duplicate key: ", key)
    dict_[key] = item

#
# Class
#

###############################################################################
class CommandFactory(object):
###############################################################################
    """
    A CommandFactory creates commands. It encapsulates the knowledge of the
    set of available commands.
    """
    __metaclass__ = _CommandFactoryMeta

    # Class variables
    __cmd_map = _create_cmd_map()

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

        # Check that command name is valid
        urequire(name in cls.__cmd_map,
               "'%s'" % name, " is not a valid command. Type 'help' for help.")

        # Instantiate and return new Command object
        return cls.__cmd_map[name](tokens[1:])

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
        return iter([cmdcls
                     for cmdname, cmdcls in sorted(cls.__cmd_map.iteritems())
                     if cmdname in real_names])

    ###########################################################################
    @classmethod
    def get(cls, name):
    ###########################################################################
        """
        Get Command class object associate with name.
        """
        urequire(name in cls.__cmd_map,
                 "'%s'" % name, " is not a valid command")
        return cls.__cmd_map[name]

#
# Tests
#

###############################################################################
def _tests():
###############################################################################
    pass

if (__name__ == "__main__"):
    _tests()
