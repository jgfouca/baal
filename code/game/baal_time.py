#! /usr/bin/env python

import unittest

from baal_common import prequire, SmartEnum, create_names_by_enum_value

class _SeasonMeta(type):
    def __iter__(mcs): return Season._iter_hook()

    def __contains__(mcs, value): return Season._in_hook(value)

###############################################################################
class Season(SmartEnum):
###############################################################################
    """
    Describes which season we're in.
    """

    __metaclass__ = _SeasonMeta

    # Enum values. Note these are *placeholders*, the free code below this
    # class will replace these values with proper Season objects.
    WINTER, \
    SPRING, \
    SUMMER, \
    FALL = range(4)

    # Derive names from class members.
    _NAMES = create_names_by_enum_value(vars())

    def __init__(self, value):
        super(self.__class__, self).__init__(value)

# Finish Season initialization
for name in Season._names():
    setattr(Season, name, Season(name))

###############################################################################
class Time(object):
###############################################################################
    """
    Encapsulates how time elapses in the system.
    """

    #
    # ==== Public API ====
    #

    def __init__(self): self.__init_impl()

    #
    # Getter / Query API
    #

    def season(self): return self.__season

    def year(self): return self.__year

    def to_xml(self): return self.__to_xml_impl()

    #
    # Modifying API
    #

    def next(self):
        """
        Time in incremented, season-by-season, with this method
        """
        return self.__next_impl()

    #
    # ==== Class constants ====
    #

    _STARTING_YEAR = 0

    #
    # ==== Implementation ====
    #

    ###########################################################################
    def __init_impl(self):
    ###########################################################################
        self.__year   = self._STARTING_YEAR
        self.__season = iter(Season).next()

    ###########################################################################
    def __to_xml_impl(self):
    ###########################################################################
        # TODO - Aaron
        pass

    ###########################################################################
    def __next_impl(self):
    ###########################################################################
        try:
            self.__season.next()
        except StopIteration:
            self.__season = iter(Season).next()
            self.__year += 1

        return self

#
# Tests
#

###############################################################################
class TestTime(unittest.TestCase):
###############################################################################

    ###########################################################################
    def test_time(self):
    ###########################################################################
        time = Time()

        self.assertEqual(time.season(), iter(Season).next())
        self.assertEqual(time.year(), Time._STARTING_YEAR)

        for season in Season:
            time.next()

        self.assertEqual(time.season(), iter(Season).next())
        self.assertEqual(time.year(), Time._STARTING_YEAR + 1)

        time.next()
        self.assertEqual(time.season(), iter(Season).next().next())

if (__name__ == "__main__"):
    unittest.main()
