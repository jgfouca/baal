#! /usr/bin/env python

"""
The file contains classes having to do with weather and climate (and classes
closely coupled to those concepts)
"""

import unittest, random, sys

from baal_common import prequire, SmartEnum, ProgramError, Location, \
    grant_access, check_access, check_callers, \
    set_prequire_handler, raising_prequire_handler, \
    create_names_by_enum_value
from draw_mode import DrawMode
import baal_common
from baal_time import Season

class _DirectionMeta(type):
    def __iter__(mcs): return Direction._iter_hook()

    def __contains__(mcs, value): return Direction._in_hook(value)

###############################################################################
class Direction(SmartEnum):
###############################################################################
    """
    Describes directionality
    """

    __metaclass__ = _DirectionMeta

    # Enum values. Note these are *placeholders*, the free code below this
    # class will replace these values with proper Direction objects.
    N,   \
    NNE, \
    NE,  \
    ENE, \
    E,   \
    ESE, \
    SE , \
    SSE, \
    S,   \
    SSW, \
    SW,  \
    WSW, \
    W,   \
    WNW, \
    NW,  \
    NNW = range(16)

    # Derive names from class members.
    _NAMES = create_names_by_enum_value(vars())

    def __init__(self, value):
        super(self.__class__, self).__init__(value)

# Finish Direction initialization
for name in Direction._names():
    setattr(Direction, name, Direction(name))

###############################################################################
class Wind(object):
###############################################################################
    """
    Very simple class; encapsulates the data necessary to define wind.
    Immutable.
    """

    def __init__(self, speed, direction):
        self.__speed = speed # mph
        self.__direction = direction

    def speed(self):
        return self.__speed

    def direction(self):
        return self.__direction

    def __add__(self, rhs):
        if (type(rhs) == Wind):
            return self.__add_wind(rhs)
        else:
            return Wind(self.__speed + rhs, self.__direction)

    def __add_wind(self, rhs):
        # TODO: Implement vector sum
        pass

    def __str__(self):
        return "%s%s" % (str(self.direction()).ljust(3),
                         str(self.speed()).rjust(3))

###############################################################################
class Climate(object):
###############################################################################
    """
    Every tile has a climate. Averate temp, average precip, and
    prevailing wind.

    This needs to be different from season to season.
    """

    def __init__(self, temperature, precip, wind):
        num_seasons = Season.size()

        prequire(len(temperature) == num_seasons, "wrong number of temps")
        prequire(len(precip)      == num_seasons, "wrong number of precips")
        prequire(len(wind)        == num_seasons, "wrong number of winds")

        self.__temperature = temperature # deg farenheit
        self.__precip      = precip      # inches water precip / year
        self.__wind        = wind        # average wind dir and speed

    def temperature(self, season): return self.__temperature[int(season)]

    def precip(self, season): return self.__precip[int(season)]

    def wind(self, season): return self.__wind[int(season)]

    def to_xml(self):
        # TODO - Aaron
        pass

###############################################################################
class Atmosphere(object):
###############################################################################
    """
    Every tile has atmosphere above it. Atmosphere has dewpoint,
    temperature, wind vector, and pressure.
    """

    #
    # ==== Public API ====
    #

    def __init__(self, climate): self.__init_impl(climate)

    #
    # Getter / Query API
    #

    def temperature(self): return self.__temperature

    def dewpoint(self): return self.__dewpoint

    def precip(self): return self.__precip

    def pressure(self): return self.__pressure

    def wind(self): return self.__wind

    def to_xml(self): return self.__to_xml_impl()

    #
    # Modification API
    #

    def cycle_turn(self, anomalies, location, season):
        """
        Notify this atmosphere that the turn has cycled. Provide a container
        of anomalies and a location so that the impact of the anomalies on this
        location can be determined. The season is used to establish the
        base (no-anomaly) values.
        """
        return self.__cycle_turn_impl(anomalies, location, season)

    def set_temperature(self, new_temperature):
        return self.__set_temperature_impl(new_temperature)

    def set_wind(self, new_wind):
        return self.__set_wind_impl(new_wind)

    #
    # ==== Internal-only API ====
    #

    def _compute_dewpoint(self):
        return self.__compute_dewpoint_impl()

    #
    # ==== Class constants ====
    #

    NORMAL_PRESSURE  = 1000

    ALLOW_CYCLE_TURN      = "_allow_atmos_cycle_turn"
    ALLOW_SET_TEMPERATURE = "_allow_atmos_set_temperature"
    ALLOW_SET_WIND        = "_allow_atmos_set_wind"

    #
    # ==== Implementation ====
    #

    ###########################################################################
    def __init_impl(self, climate):
    ###########################################################################
        season = iter(Season).next()

        self.__temperature = climate.temperature(season)
        self.__precip    = climate.precip(season)
        self.__pressure    = self.NORMAL_PRESSURE
        self.__wind        = climate.wind(season)
        self.__climate     = climate
        self.__dewpoint    = self._compute_dewpoint()

    ###########################################################################
    def __to_xml_impl(self):
    ###########################################################################
        # TODO - Aaron
        pass

    ###########################################################################
    def __cycle_turn_impl(self, anomalies, location, season):
    ###########################################################################
        check_access(self.ALLOW_CYCLE_TURN)

        # Gather all modifiers from all anomalies
        precip_modifier = 1.0
        temp_modifier = 0
        pressure_modifier = 0
        for anomaly in anomalies:
            precip_modifier   *= anomaly.precip_effect(location)
            temp_modifier     += anomaly.temperature_effect(location)
            pressure_modifier += anomaly.pressure_effect(location)

        self.__temperature = self.__climate.temperature(season) + temp_modifier
        self.__pressure    = self.NORMAL_PRESSURE + pressure_modifier
        self.__precip    = self.__climate.precip(season) * precip_modifier

        self.__dewpoint = self._compute_dewpoint()

        # TODO: Need to compute wind speed changes due to pressure
        self.__wind = self.__climate.wind(season)

    ###########################################################################
    def __set_temperature_impl(self, new_temperature):
    ###########################################################################
        check_access(self.ALLOW_SET_TEMPERATURE)
        self.__temperature = new_temperature

    ###########################################################################
    def __set_wind_impl(self, new_wind):
    ###########################################################################
        check_access(self.ALLOW_SET_WIND)
        self.__wind = new_wind

    ###########################################################################
    def __compute_dewpoint_impl(self):
    ###########################################################################
        # TODO - Probably function of temp and precip
        return self.temperature() - 20

class _AnomalyCategoryMeta(type):
    def __iter__(mcs): return AnomalyCategory._iter_hook()

    def __contains__(mcs, value): return AnomalyCategory._in_hook(value)

###############################################################################
class AnomalyCategory(SmartEnum):
###############################################################################
    """
    Describes the types of anomalies that can happen
    """

    __metaclass__ = _AnomalyCategoryMeta

    # Enum values. Note these are *placeholders*, the free code below this
    # class will replace these values with proper AnomalyCategory objects.
    TEMPERATURE, \
    PRECIP, \
    PRESSURE = range(3)

    # Derive names from class members.
    _NAMES = create_names_by_enum_value(vars())

    def __init__(self, value):
        super(self.__class__, self).__init__(value)

# Finish AnomalyCategory initialization
for name in AnomalyCategory._names():
    setattr(AnomalyCategory, name, AnomalyCategory(name))

###############################################################################
class Anomaly(object):
###############################################################################
    """
    An Anomaly represents a deviation from normal weather patterns
    over a certain area and of a certain intensity. Each level of
    intensity is linearly more intense than the prior level but
    exponentially less likely. The area affected by the anomaly
    will depend on the size of the map.
    """

    #
    # ==== Public API ====
    #

    @classmethod
    def generate_anomaly(cls, category, location):
        """
        Generates an anomaly. Returns None if the dice roll did not
        merit the creation of an anomaly.
        """
        return cls._generate_anomaly_impl(category, location)

    #
    # Getters / Queries
    #

    def precip_effect(self, location):
        """
        Return this anomaly's effect on a location as a % of
        normal value of precip.
        """
        return self.__precip_effect_impl(location)


    def temperature_effect(self, location):
        """
        Return this anomaly's effect on a location's temperature
        in terms of degrees of deviation from norm.
        """
        return self.__temperature_effect_impl(location)

    def pressure_effect(self, location):
        """
        Return this anomaly's effect on a location's pressure in
        terms of millibars of deviation from norm.
        """
        return self.__pressure_effect_impl(location)

    def intensity(self): return self.__intensity

    def category(self): return self.__category

    def location(self): return self.__location

    def to_xml(self): return self.__to_xml_impl()

    #
    # ==== Internal methods ====
    #

    def __init__(self, category, intensity, location):
        self.__init_impl(category, intensity, location)

    #
    # ==== Class constants ====
    #

    MAX_INTENSITY = 3 # anomalies are on a scale from +/- 1 -> MAX_INTENSITY

    @classmethod
    def _PRECIP_CHANGE_FUNC(cls, intensity):
        # Returns a multiplier on average precip
        # (max - 1 / max)^-intensity
        return pow(float(cls.MAX_INTENSITY - 1) / cls.MAX_INTENSITY,
                   -intensity)

    @classmethod
    def _TEMPERATURE_CHANGE_FUNC(cls, intensity):
        return 7 * intensity

    @classmethod
    def _PRESSURE_CHANGE_FUNC(cls, intensity):
        return 15 * intensity

    @classmethod
    def _GENERATE_ANOMALY_INTENSITY_FUNC(cls):
        roll = random.random()  # [0, 1)
        negative_anom = cls.MAX_INTENSITY / 100.0
        positive_anom = (100 - cls.MAX_INTENSITY) / 100.0
        intensity = 0

        # Normalize so that negative/positive rolls look the same, the only
        # difference is the modifier
        if (roll > positive_anom):
            roll -= positive_anom
            modifier = 1
        else:
            modifier = -1

        while (roll < negative_anom):
            intensity += modifier
            negative_anom /= 2
            if (abs(intensity) == cls.MAX_INTENSITY):
                break

        prequire(intensity >= -cls.MAX_INTENSITY, "below min: ", intensity)
        prequire(intensity <= cls.MAX_INTENSITY, "above max: ", intensity)

        return intensity

    #
    # ==== Implementation ====
    #

    ###########################################################################
    def __init_impl(self, category, intensity, location):
    ###########################################################################
        check_callers(["__init__", "_generate_anomaly_impl"])

        self.__category    = category
        self.__intensity   = intensity
        self.__location    = location

    ###########################################################################
    @classmethod
    def _generate_anomaly_impl(cls, category, location):
    ###########################################################################
        intensity = cls._GENERATE_ANOMALY_INTENSITY_FUNC()
        if (intensity != 0):
            return Anomaly(category, intensity, location)
        else:
            return None

    ###########################################################################
    def __precip_effect_impl(self, location):
    ###########################################################################
        # TODO - In all effect methods, need a more sophisticated
        # model for determining an anomaly's effect on a location. For
        # the moment, anomalies only affect the immediate location.

        if (self.__category != AnomalyCategory.PRECIP or
            self.__location != location):
            return 1.0 # No effect
        else:
            return self._PRECIP_CHANGE_FUNC(self.__intensity)

    ###########################################################################
    def __temperature_effect_impl(self, location):
    ###########################################################################
        if (self.__category != AnomalyCategory.TEMPERATURE or
            self.__location != location):
            return 0 # No effect
        else:
            return self._TEMPERATURE_CHANGE_FUNC(self.__intensity)

    ###########################################################################
    def __pressure_effect_impl(self, location):
    ###########################################################################
        if (self.__category != AnomalyCategory.PRESSURE or
            self.__location != location):
            return 0 # No effect
        else:
            return self._PRESSURE_CHANGE_FUNC(self.__intensity)

    ###########################################################################
    def __to_xml_impl(self):
    ###########################################################################
        # TODO - Aaron
        pass

#
# Free-function API
#

###############################################################################
def is_atmospheric(draw_mode):
###############################################################################
    return draw_mode in [DrawMode.WIND,
                         DrawMode.DEWPOINT,
                         DrawMode.TEMPERATURE,
                         DrawMode.PRECIP,
                         DrawMode.PRESSURE]

#
# Tests
#

###############################################################################
class TestWeather(unittest.TestCase):
###############################################################################

    ###########################################################################
    def test_anomaly(self):
    ###########################################################################
        # Change to raising handler for unit-testing
        set_prequire_handler(raising_prequire_handler)

        # Should not be able to directly create an Anomaly
        self.assertRaises(ProgramError, Anomaly, None, 0, None)

        anom = None
        while (anom is None):
            anom = Anomaly.generate_anomaly(AnomalyCategory.PRECIP, None)
        anom.precip_effect(None)

    ###########################################################################
    def test_weather(self):
    ###########################################################################
        # Change to raising handler for unit-testing
        set_prequire_handler(raising_prequire_handler)

        temps, precips, winds = ([60, 70, 80, 70],
                                 [1, 2, 3, 4],
                                 [Wind(10, Direction.NNW)]*4)
        climate = Climate(temps, precips, winds)
        atmos = Atmosphere(climate)

        # First season should be winter
        first_season = iter(Season).next()
        self.assertEqual(climate.temperature(first_season), atmos.temperature())
        self.assertEqual(climate.precip(first_season),      atmos.precip())
        self.assertEqual(climate.wind(first_season),        atmos.wind())

        # Check access controls
        self.assertRaises(ProgramError, atmos.cycle_turn, None, None, None)
        grant_access(self, Atmosphere.ALLOW_CYCLE_TURN)

        # Brute-force check of all seasons
        for idx, season in enumerate(Season):
            atmos.cycle_turn([], Location(0,0), season)
            self.assertEqual(climate.temperature(season), atmos.temperature())
            self.assertEqual(climate.precip(season),      atmos.precip())
            self.assertEqual(climate.wind(season),        atmos.wind())
            self.assertEqual(climate.temperature(season), temps[idx])
            self.assertEqual(climate.precip(season),      precips[idx])
            self.assertEqual(climate.wind(season),        winds[idx])

if (__name__ == "__main__"):
    unittest.main()
