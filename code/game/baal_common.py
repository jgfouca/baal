#! /usr/bin/env python

"""
This file contains various commonly used free functions and data structures
"""

import curses, subprocess, pdb

#
# API for error handling
#

###############################################################################
class UserError(Exception):
###############################################################################
    """
    Doesn't do anything different from any other exception type, just used to
    indentify user error. In general, these should always be caught and
    handled. Please do not raise these directly, use the
    """
    pass

###############################################################################
class ProgramError(Exception):
###############################################################################
    """
    Doesn't do anything different from any other exception, just used to
    identify program errors. In general, these should not be caught so that
    the program can crash and produce a stacktrace.
    """
    pass

# Change this to False for production release
_DEBUG = True

###############################################################################
def debugger_prequire_handler(str_):
###############################################################################
    print str_
    pdb.set_trace()

###############################################################################
def raising_prequire_handler(str_):
###############################################################################
    raise ProgramError(str_)

_PREQUIRE_HANDLER = debugger_prequire_handler

###############################################################################
def set_prequire_handler(new_handler):
###############################################################################
    global _PREQUIRE_HANDLER
    _PREQUIRE_HANDLER = new_handler

###############################################################################
def prequire(expr, *msg_args):
###############################################################################
    """
    Use this to express a required program-error check. In general, this will
    launch the python debugger, but that makes prequire not usable within
    unit-tests, so we need a way to change behavior.
    """
    if (not expr):
        _PREQUIRE_HANDLER("".join([str(arg) for arg in msg_args]))

###############################################################################
def passert(expr, *msg_args):
###############################################################################
    """
    Use this to express a debug-only program-error check
    """
    if (_DEBUG):
        prequire(expr, *msg_args)

###############################################################################
def urequire(expr, *msg_args):
###############################################################################
    """
    Use this to check for user error
    """
    if (not expr):
        raise UserError("".join([str(arg) for arg in msg_args]))

#
# API for color printing
#

# Available colors
BLACK, RED, GREEN, YELLOW, BLUE, MAGENTA, CYAN, WHITE = range(8)

# Private
_COLOR_MAP = ["30m", "31m", "32m", "33m", "34m", "35m", "36m", "37m"]
_BOLD_COLOR_PREFIX = "\033[1;"
_CLEAR = "\033[0m"

###############################################################################
def cprint(color, *args):
###############################################################################
    """
    USAGE: cprint(RED, "one is: ", 1)
    """
    #if (curses.has_colors()):
    print _BOLD_COLOR_PREFIX + _COLOR_MAP[color] + \
        "".join([str(arg) for arg in args]) + \
        _CLEAR
    #else:
    #    print "".join([str(arg) for arg in args])

#
# Misc API
#

###############################################################################
def check_access(caller, attr_key):
###############################################################################
    prequire(getattr(caller, attr_key, False), "Illegal access")

###############################################################################
def subclasses(cls):
###############################################################################
    """
    Get the subclasses of a class. Pylint flags this if you try to call it on
    a class directly, so we use this free function instead.
    """
    return cls.__subclasses__()

###############################################################################
def create_subclass_map(cls, include_aliases=False):
###############################################################################
    """
    Given a class, create a map that maps the names of all subclasses of cls
    to the class-object of the subclass. Useful for factory classes. Expects
    that cls.name() is callable for all subclasses. If include_aliases is
    True, the subclasses must also support the aliases() classmethod.
    """
    rv = {}
    for subcls in subclasses(cls):
        _no_dup_insert(rv, subcls.name(), subcls)
        if (include_aliases):
            for alias in subcls.aliases():
                _no_dup_insert(rv, alias, subcls)

    return rv

###############################################################################
def _no_dup_insert(dict_, key, item):
###############################################################################
    prequire(key not in dict_, "Found duplicate key: ", key)
    dict_[key] = item

###############################################################################
def clear_screen():
###############################################################################
    """
    Clears the screen; useful for text-based graphical interfaces to keep
    things in a constant screen-position.
    """
    # TODO: Re-implement with curses for better portability
    subprocess.call("clear")

#
# Location API
#

###############################################################################
class Location(object):
###############################################################################
    """
    A named pair defining a location in a 2D space
    """

    ###########################################################################
    def __init__(self, row, col):
    ###########################################################################
        self.row = row
        self.col = col

    ###########################################################################
    @classmethod
    def parse(cls, str_):
    ###########################################################################
        """
        Init from string. String is expected to be of form: 'row,col'.
        This should raise a *user* exception if there's a problem with the
        string format.
        """
        try:
            row_s, col_s = str_.split(",")
            row = int(row_s)
            col = int(col_s)
        except Exception:
            urequire(False,
                     "String ", str_, " was not in expected form 'row,col'")
        return Location(row, col)

    ###########################################################################
    def __eq__(self, loc):
    ###########################################################################
        return self.row == loc.row and self.col == loc.col

    ###########################################################################
    def __ne__(self, loc):
    ###########################################################################
        return not self == loc

    ###########################################################################
    def __str__(self):
    ###########################################################################
        return "%d,%d" % (self.row, self.col)

    ###########################################################################
    def unpack(self):
    ###########################################################################
        return self.row, self.col

#
# Smart Enum Class API
#

###############################################################################
class SmartEnum(object):
###############################################################################
    """
    Inheriting from this class will get you most of what you need to create a
    smart enum class. You'll still need to set up the metaclass and the enum
    values.

    TODO: It is even worth support the int API for this class?
    """

    _NAMES = None # Intended to be overidden

    #
    # Public API
    #

    ###########################################################################
    def __init__(self, value):
    ###########################################################################
        cls = self.__class__
        if (type(value) == str):
            value = value.upper()
            urequire(value in cls, "Invalid draw-mode string: ", value)
            self.__value = cls._names().index(value)
        elif (type(value) == int):
            urequire(value in cls, "Invalid draw-mode int: ", value)
            self.__value = value
        else:
            prequire(False, "Must initialize with string or int, ",
                     "received: ", type(value))

    ###########################################################################
    def __eq__(self, rhs):
    ###########################################################################
        """
        Flexible equality. Can compare with string, int, or another enum.
        """
        if (type(rhs) == str):
            rhs = rhs.upper()
            if (rhs in self._names()):
                return self.__value == self._names().index(rhs)
            else:
                return False
        elif (type(rhs) == int):
            return self.__value == rhs
        elif (type(rhs) == self.__class__):
            return self.__value == rhs.__value
        else:
            prequire(False, "Can only compare with str, int, or DrawMode, ",
                     "received: ", type(rhs))

    ###########################################################################
    def __ne__(self, rhs):
    ###########################################################################
        """
        Flexible inequality. See __eq__
        """
        return not self == rhs

    ###########################################################################
    def __str__(self):
    ###########################################################################
        """
        Return the name of the enum value
        """
        return self._names()[self.__value]

    ###########################################################################
    def __int__(self):
    ###########################################################################
        """
        Return the raw enum value
        """
        return self.__value

    #
    # Private API
    #

    ###########################################################################
    @classmethod
    def _iter_hook(cls):
    ###########################################################################
        return iter([cls(name) for name in cls._names()])

    ###########################################################################
    @classmethod
    def _in_hook(cls, value):
    ###########################################################################
        if (type(value) == str):
            return value.upper() in cls._names()
        elif (type(value) == int):
            return value >= 0 and value < len(cls._names())
        else:
            prequire(False, "Only makes sense to check membership with string "
                     "or int, received: ", type(value))

    # Private getters to make pylint happy

    @classmethod
    def _names(cls): return tuple(cls._NAMES)

    def _value(self): return self.__value

#
# Tests
#

###############################################################################
def _test():
###############################################################################
    pass

if (__name__ == "__main__"):
    _test()
