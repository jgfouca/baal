#! /usr/bin/env python

"""
The file contains classes having to do with weather and climate (and classes
closely coupled to those concepts)
"""

from __future__ import print_function
import unittest, random

from baal_common import prequire, SmartEnum, ProgramError, Location, \
    cprint, BLUE, GREEN, RED, YELLOW, \
    grant_access, check_access, check_callers, \
    set_prequire_handler, raising_prequire_handler
from drawable import Drawable, DrawMode, curr_draw_mode
import baal_common
from baal_time import Season
from world_tile import WorldTile

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
    _NAMES = [ name for name in dir() if name.isupper() and name.isalpha() ]

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
    Every tile has a climate. Averate temp, average rainfall, and
    prevailing wind.

    TODO: This needs to be different from season to season.
    """

    def __init__(self, temperature, rainfall, wind):
        self.__temperature = temperature # deg farenheit
        self.__rainfall    = rainfall    # inches water precip / year
        self.__wind        = wind

    def temperature(self, season): return self.__temperature

    def rainfall(self, season): return self.__rainfall

    def wind(self, season): return self.__wind

    def to_xml(self):
        # TODO - Aaron
        pass

###############################################################################
class Atmosphere(Drawable):
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

    def rainfall(self): return self.__rainfall

    def pressure(self): return self.__pressure

    def wind(self): return self.__wind

    def to_xml(self): return self.__to_xml_impl()

    #
    # Drawing API
    #

    def draw_text(self): return self.__draw_text_impl()

    def draw_graphics(self): return self.__draw_graphics_impl()

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

    def _compute_color(self, draw_mode, field_value):
        return self.__compute_color_impl(draw_mode, field_value)

    def _get_field_for_draw_mode(self, draw_mode):
        return self.__get_field_for_draw_mode_impl(draw_mode)

    #
    # ==== Class constants ====
    #

    NORMAL_PRESSURE  = 1000

    ALLOW_CYCLE_TURN      = "_allow_atmos_cycle_turn"
    ALLOW_SET_TEMPERATURE = "_allow_atmos_set_temperature"
    ALLOW_SET_WIND        = "_allow_atmos_set_wind"

    # Describes how to draw the various fields. The first value in the pair
    # is the upper-bound for the corresponding color.
    _MAX = 999999
    _FIELD_COLOR_MAP = {
        DrawMode.WIND        : ((10, GREEN), (20, YELLOW), (_MAX, RED)),
        DrawMode.DEWPOINT    : ((32, RED),   (55, YELLOW), (_MAX, GREEN)),
        DrawMode.TEMPERATURE : ((32, BLUE),  (20, YELLOW), (_MAX, RED)),
        DrawMode.PRESSURE    : ((10, GREEN), (20, YELLOW), (_MAX, RED)),
        DrawMode.RAINFALL    : ((2,  RED),   (10, YELLOW), (_MAX, RED))
    }

    #
    # ==== Implementation ====
    #

    ###########################################################################
    def __init_impl(self, climate):
    ###########################################################################
        season = iter(Season).next()

        self.__temperature = climate.temperature(season)
        self.__rainfall    = climate.rainfall(season)
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
    def __compute_color_impl(self, draw_mode, field_value):
    ###########################################################################
        for upper_bound, color in self._FIELD_COLOR_MAP[draw_mode]:
            if (field_value < upper_bound):
                return color

        prequire(False,
                 "Failed to find color for ", draw_mode, ", val ", field_value)

    ###########################################################################
    def __get_field_for_draw_mode_impl(self, draw_mode):
    ###########################################################################
        if (draw_mode == DrawMode.WIND):
            return self.wind()
        elif (draw_mode == DrawMode.DEWPOINT):
            return self.dewpoint()
        elif (draw_mode == DrawMode.TEMPERATURE):
            return self.temperature()
        elif (draw_mode == DrawMode.PRESSURE):
            return self.pressure()
        elif (draw_mode == DrawMode.RAINFALL):
            return self.rainfall()
        else:
            prequire(False, "Bad draw mode: ", draw_mode)

    ###########################################################################
    def __draw_text_impl(self):
    ###########################################################################
        draw_mode = curr_draw_mode()

        field = self.wind()
        color = self._compute_color(draw_mode, field)
        str_  = str(field).center(WorldTile.TILE_TEXT_WIDTH)

        cprint(color, str_)

    ###########################################################################
    def __draw_graphics_impl(self):
    ###########################################################################
        # TODO
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
            temp_modifier     += anomaly.temp_effect(location)
            pressure_modifier += anomaly.pressure_effect(location)

        self.__temperature = self.__climate.temperature(season) + temp_modifier
        self.__pressure    = self.NORMAL_PRESSURE + pressure_modifier
        self.__rainfall    = self.__climate.rainfall(season) * precip_modifier

        self.__dewpoint = self._compute_dewpoint()

        # TODO: Need to compute wind speed changes due to pressure
        self.__wind = self.__climate.wind(season)

    ###########################################################################
    def __set_temperature_impl(self, new_temperature):
    ###########################################################################
        check_access(self.ALLOW_CYCLE_TURN)
        self.__temperature = new_temperature

    ###########################################################################
    def __set_wind_impl(self, new_wind):
    ###########################################################################
        check_access(self.ALLOW_SET_WIND)
        self.__wind = new_wind

    ###########################################################################
    def __compute_dewpoint_impl(self):
    ###########################################################################
        # TODO - Probably function of temp and rainfall
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
    RAINFALL, \
    PRESSURE = range(3)

    # Derive names from class members.
    _NAMES = [ name for name in dir() if name.isupper() and name.isalpha() ]

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

    def to_xml(self): return self.__to_xml_impl()

    #
    # Drawing API
    #

    def draw_text(self): return self.__draw_text_impl()

    def draw_graphics(self): return self.__draw_graphics_impl()

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
        return pow((cls.MAX_INTENSITY - 1) / cls.MAX_INTENSITY, -intensity)

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

        if (self.__category != AnomalyCategory.RAINFALL or
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

    ###########################################################################
    def __draw_text_impl(self):
    ###########################################################################
        print("Level:", self.__intensity, self.__category,
              "anomaly at location", self.__location,
              end='')

    ###########################################################################
    def __draw_graphics_impl(self):
    ###########################################################################
        # TODO
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
                         DrawMode.RAINFALL,
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
            anom = Anomaly.generate_anomaly(AnomalyCategory.RAINFALL, None)
        anom.precip_effect(None)

    ###########################################################################
    def test_weather(self):
    ###########################################################################
        # Change to raising handler for unit-testing
        set_prequire_handler(raising_prequire_handler)

        temp, rain, wind = (80, 10, Wind(10, Direction.NNW))
        climate = Climate(temp, rain, wind)
        atmos = Atmosphere(climate)

        first_season = iter(Season).next()
        self.assertEqual(climate.temperature(first_season), atmos.temperature())
        self.assertEqual(climate.rainfall(first_season),    atmos.rainfall())
        self.assertEqual(climate.wind(first_season),        atmos.wind())

        self.assertRaises(ProgramError, atmos.cycle_turn, None, None, None)

        grant_access(self, Atmosphere.ALLOW_CYCLE_TURN)
        atmos.cycle_turn([], Location(0,0), first_season)
        self.assertEqual(climate.temperature(first_season), atmos.temperature())
        self.assertEqual(climate.rainfall(first_season),    atmos.rainfall())
        self.assertEqual(climate.wind(first_season),        atmos.wind())

if (__name__ == "__main__"):
    unittest.main()
