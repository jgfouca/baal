#! /usr/bin/env python

import unittest

from baal_common import prequire, urequire, UserError, grant_access
from city import City
from engine import engine
from baal_math import exp_growth, poly_growth, fibonacci_div
from world_tile import OceanTile, MountainTile, FoodTile, PlainsTile, LushTile
from weather import Atmosphere

###############################################################################
class Spell(object):
###############################################################################
    """
    Abstract base class for all spells. The base class will take
    care of everything except how the spell affects the world.
    """

    #
    # ==== Public API ====
    #

    def __init__(self, level, location): self.__init_impl(level, location)

    #
    # Class-level property getters (default definitions provided)
    #

    @classmethod
    def name(cls):
        """
        Get the name of the spell; this is what the user will type to refer
        to this spell.
        """
        return cls._NAME

    @classmethod
    def base_cost(cls):
        """
        The base (level-1) cost of this spell.
        """
        return cls._BASE_COST

    @classmethod
    def prereqs(cls):
        """
        Returns the prerequisites for this spell.
        """
        return cls._PREREQS

    @classmethod
    def info(cls):
        """
        Returns some basic documentation about this spell
        """
        return cls.__doc__

    #
    # Instance getters/queries
    #

    def cost(self):
        """
        Return the mana cost of this spell
        """
        return self.__cost_impl()

    def level(self): return self.__level

    def location(self): return self.__location

    def __str__(self): return self.__str_impl()

    #
    # Abstract API
    #

    def verify_apply(self):
        """
        Verify that the user's attempt to cast this spell is valid. This method
        will throw a user-error if there's a problem.
        """
        prequire(False, "Called abstract version of verify_apply")

    def apply(self):
        """
        Apply the casting of this spell to the game state.
        This should NEVER throw. verify_apply should have been called prior
        to ensure the casting of this spell is valid from the spell's point
        of view.
        """
        prequire(False, "Called abstract version of apply")

    #
    # ==== Class constants ====
    #

    # These are intended to be overridden
    _NAME      = None
    _BASE_COST = None
    _PREREQS   = None


    #
    # Tweakable constants
    #

    __CITY_DESTROY_EXP_BONUS = 1000
    __CHAIN_REACTION_BONUS = 2
    __INFRA_EXP_FUNC = lambda infra_destroyed: pow(2, infra_destroyed) * 200

    @classmethod
    def _COST_FUNC(cls, level):
        "Default cost function; override to change"
        return cls.base_cost() * pow(1.3, level - 1) # 30% per lvl

    #
    # ==== Internal API ====
    #

    def _kill(self, city, pct_killed):
        """
        Kill-off a certain percent of the population of a city.
        Returns the exp gained.
        """
        return self.__kill_impl(city, pct_killed)

    def _destroy_infra(self, tile, max_destroyed):
        """
        Reduce the infra of a tile by up to max_destroyed.
        """
        return self.__destroy_infra_impl(tile, max_destroyed)

    def _damage_tile(self, tile, pct_damaged):
        """
        Damage a tile by pct_damaged.
        """
        return self.__damage_tile_impl(tile, pct_damaged)

    def _spawn(self, name, level):
        """
        Some disasters can spawn other disasters (chain reaction). This method
        encompasses the implementation of this phenominon. The amount of exp
        gained is returned.
        """
        return self.__spawn_impl(name, level)

    def _report(self, *args):
        """
        Report things that are happening because of the spell.
        """
        return self.__report_impl(*args)

    #
    # ==== Implementation ====
    #

    ###########################################################################
    def __init_impl(self, level, location):
    ###########################################################################
        self.__level    = level
        self.__location = location

    ###########################################################################
    def __cost_impl(self):
    ###########################################################################
        return self._COST_FUNC(self.level())

    ###########################################################################
    def __str_impl(self):
    ###########################################################################
        return "%s[%d]" % (self.name(), self.level())

    ###########################################################################
    def __report_impl(self, *args):
    ###########################################################################
        msg = "%s: %s" % (self.name(), "".join([str(arg) for arg in args]))
        engine().interface().spell_report(msg)

    ###########################################################################
    def __kill_impl(self, city, pct_killed):
    ###########################################################################
        prequire(pct_killed > 0.0, "Do not call this if no one killed")

        pct_killed = min(pct_killed, 100.0)
        num_killed = city.population() * (pct_killed / 100)
        city.kill(num_killed)
        self._report("killed ", num_killed)

        if (city.population() < City.MIN_CITY_SIZE):
            self._report("obliterated city '", city.name(), "'")
            engine().world().remove_city(city)
            city.kill(city.population())
            num_killed += city.population()

            # TODO: Give bigger city-kill bonus based on maximum
            # attained rank of city.
            return num_killed + self.__CITY_DESTROY_EXP_BONUS
        else:
            return num_killed

    ###########################################################################
    def __destroy_infra_impl(self, tile, max_destroyed):
    ###########################################################################
        if (max_destroyed == 0):
            return 0

        # Cannot destroy infra that's not there
        num_destroyed = min(tile.infra_level(), max_destroyed)
        tile.destroy_infra(num_destroyed)
        self._report("destroyed ", num_destroyed, " levels of infrastructure")

        # Convert to exp
        return self.__INFRA_EXP_FUNC(num_destroyed)

    ###########################################################################
    def __damage_tile_impl(self, tile, pct_damaged):
    ###########################################################################
        if (pct_damaged > 0.0):
            pct_damaged = min(pct_damaged, 100.0)

            tile.damage(pct_damaged)

            self._report("caused ", pct_damaged, "% damage to tile")

    ###########################################################################
    def __spawn_impl(self, name, level):
    ###########################################################################
        from spell_factory import SpellFactory

        spell = SpellFactory.create_spell(name, level, self.location())

        # Check if this spell can be applied here
        try:
            spell.verify_apply()

            self._report("caused a level ", level, " ", name)

            return self.__CHAIN_REACTION_BONUS * spell.apply()
        except UserError:
            pass

        return 0 # No exp if spell could not be applied

# We allow Spell to be able to kill
grant_access(Spell, City.ALLOW_KILL)

###############################################################################
class _SpellPrereq(object):
###############################################################################
    """
    Encapsulates data needed to describe a spell prerequisite. This class is
    more like a named pair than a full-blown class.
    """

    def __init__(self, min_player_level, must_know_spells):
        self.__min_player_level = min_player_level
        self.__must_know_spells = tuple(must_know_spells)

    def min_player_level(self): return self.__min_player_level

    def must_know_spells(self): return self.__must_know_spells

# TODO - Do we want spells for controlling all the basic properties
# of the atmosphere? Or do we want to leave some up to pure chance (pressure)?

# Spell header components are designed to maximize tweakability from
# the header file.

#
# Spell Implementations. The rest of the system does not care about anything
# below. Only the abstract base class (Spell) should be exposed.
#

###############################################################################
class _Hot(Spell):
###############################################################################
    """
    Increases the immediate temperature of a region. High temperatures can
    kill people in cities or deplete soil moisture on farms. This spell is not
    intended to be a primary damage dealer; instead, you should be using this
    spell to enhance the more-powerful spells.

    Enhanced by high temps, high dewpoints. Decreased by AI tech level.

    This is a tier 1 spell
    """

    #
    # ==== Class Constants ====
    #

    _NAME      = "hot"
    _BASE_COST = 50
    _PREREQS   = _SpellPrereq(1, ()) # No prereqs

    #
    # Tweakable constants
    #

    @classmethod
    def _DEGREES_HEATED_ATMOS_FUNC(cls, curr_temp, dewpoint, level):
        # TODO: high dewpoint makes it harder to heat?
        # curr_temp may be used in the future to implement diminishing returns
        return 7 * level  # very simple for now, 7 deg per level

    @classmethod
    def _DEGREES_HEATED_OCEAN_FUNC(cls, curr_surf_temp, level):
        return 2 * level

    @classmethod
    def _BASE_KILL_FUNC(cls, temp, dewpoint):
        # TODO: Dewpoint should enhance killing
        # (temp-threshold)^1.5 / 8
        return poly_growth(temp - 100, 1.5, 8)

    @classmethod
    def _TECH_PENALTY_FUNC(cls, tech_level):
        return poly_growth(tech_level, 0.5) #sqrt

    #
    # Public API
    #

    ###########################################################################
    def __init__(self, level, location):
    ###########################################################################
        super(self.__class__, self).__init__(level, location)

    ###########################################################################
    def verify_apply(self):
    ###########################################################################
        # No-op. This spell can be cast anywhere, anytime.
        pass

    ###########################################################################
    def apply(self):
    ###########################################################################
        world = engine().world()
        tile  = world.tile(self.location())
        atmos = tile.atmosphere()
        tech  = engine().ai_player().tech_level()
        exp   = 0

        # Regardless of tile type, atmosphere is warmed
        orig_temp    = atmos.temperature()
        dewpoint     = atmos.dewpoint()
        new_temp     = orig_temp + \
            self._DEGREES_HEATED_ATMOS_FUNC(orig_temp, dewpoint, self.level())
        atmos.set_temperature(new_temp)
        self._report("raised temperature from ", orig_temp, " to ", new_temp)

        # If ocean tile, sea temps are warmed too
        if (isinstance(tile, OceanTile)):
            orig_ocean_temp = tile.surface_temp()
            new_ocean_temp  = orig_ocean_temp + \
                self._DEGREES_HEATED_OCEAN_FUNC(orig_ocean_temp, self.level())
            tile.set_surface_temp(new_ocean_temp)
            self._report("raised ocean temperature from ", orig_ocean_temp,
                         " to ", new_ocean_temp)

        # If mountain tile, snowpack will be melted quickly, potentially
        # causing a flood.
        elif (isinstance(tile, MountainTile)):
            # TODO
            pass

        # This spell can kill if cast on a city and temps get high enough
        city = tile.city()
        if (city is not None):
            base_kill_pct = self._BASE_KILL_FUNC(new_temp, dewpoint)
            if (base_kill_pct > 0.0):
                tech_penalty  = self._TECH_PENALTY_FUNC(tech)
                pct_killed    = base_kill_pct / tech_penalty

                self._report("base kill % is ", base_kill_pct)
                self._report("tech penalty (divisor) is ", tech_penalty)
                self._report("final kill % is ", pct_killed)

                exp += self._kill(city, pct_killed)

        return exp
# Hot can change temp
grant_access(_Hot, Atmosphere.ALLOW_SET_TEMPERATURE)

###############################################################################
class _Cold(Spell):
###############################################################################
    """
    Decreases the immediate temperature of a region. Cold temperatures can
    kill people in cities or kill crops. This spell is not
    intended to be a primary damage dealer; instead, you should be using this
    spell to enhance the more-powerful spells.

    Enhanced by low temps, low dewpoints, high winds. Decreased by AI tech
    level.

    This is a tier 1 spell
    """

    #
    # ==== Class Variables ====
    #

    _NAME      = "cold"
    _BASE_COST = 50
    _PREREQS   = _SpellPrereq(1, ()) # No prereqs

    #
    # Tweakable constants
    #

    @classmethod
    def _DEGREES_COOLED_ATMOS_FUNC(cls, curr_temp, dewpoint, level):
        # TODO: high dewpoint makes it harder to cool
        # curr_temp may be used in the future to implement diminishing returns
        return -7 * level  # very simple for now, 7 deg per level

    @classmethod
    def _DEGREES_COOLED_OCEAN_FUNC(cls, curr_surf_temp, level):
        return -2 * level

    @classmethod
    def _BASE_KILL_FUNC(cls, temp):
        # TODO: Low dewpoint, wind should enhance killing
        # (threshold - temp)^1.5 / 8
        return poly_growth(0 - temp, 1.5, 8)

    @classmethod
    def _WIND_BONUS_FUNC(cls, wind):
        # 1.02^speed, diminishing returns at 40, no threshold
        return exp_growth(1.02, wind.speed(), diminishing_returns=40)

    @classmethod
    def _FAMINE_BONUS_FUNC(cls, city):
        # Double-damage if city in famine
        return 2.0 if city.famine() else 1.0

    @classmethod
    def _TECH_PENALTY_FUNC(cls, tech_level):
        return tech_level # linear decrease

    #
    # Public API
    #

    ###########################################################################
    def __init__(self, level, location):
    ###########################################################################
        super(self.__class__, self).__init__(level, location)

    ###########################################################################
    def verify_apply(self):
    ###########################################################################
        # No-op. This spell can be cast anywhere, anytime.
        pass

    ###########################################################################
    def apply(self):
    ###########################################################################
        world = engine().world()
        tile  = world.tile(self.location())
        atmos = tile.atmosphere()
        tech  = engine().ai_player().tech_level()
        exp   = 0

        # Regardless of tile type, atmosphere is cooled
        orig_temp      = atmos.temperature()
        dewpoint       = atmos.dewpoint()
        wind           = atmos.wind()
        new_temp       = orig_temp - \
            self._DEGREES_COOLED_ATMOS_FUNC(orig_temp, dewpoint, self.level())
        atmos.set_temperature(new_temp)
        self._report("lowered temperature from ", orig_temp, " to ", new_temp)

        # If ocean tile, sea temps are warmed too
        if (isinstance(tile, OceanTile)):
            orig_ocean_temp = tile.surface_temp()
            new_ocean_temp  = orig_ocean_temp - \
                self._DEGREES_COOLED_OCEAN_FUNC(orig_ocean_temp, self.level())
            tile.set_surface_temp(new_ocean_temp)
            self._report("lowered ocean temperature from ", orig_ocean_temp,
                         " to ", new_ocean_temp)

        # This spell can kill if cast on a city and temps get high enough
        city = tile.city()
        if (city is not None):
            base_kill_pct = self._BASE_KILL_FUNC(new_temp)
            if (base_kill_pct > 0.0):
                wind_bonus    = self._WIND_BONUS_FUNC(wind)
                famine_bonus  = self._FAMINE_BONUS_FUNC(city)
                tech_penalty  = self._TECH_PENALTY_FUNC(tech)

                pct_killed    = \
                    (base_kill_pct * wind_bonus * famine_bonus) / tech_penalty

                self._report("base kill % is ", base_kill_pct)
                self._report("wind bonus (multiplier) is ", wind_bonus)
                self._report("famine bonus (multiplier) is ", famine_bonus)
                self._report("tech penalty (divisor) is ", tech_penalty)
                self._report("final kill % is ", pct_killed)

                exp += self._kill(city, pct_killed)

        return exp
# Cold can change temp
grant_access(_Cold, Atmosphere.ALLOW_SET_TEMPERATURE)

###############################################################################
class _Infect(Spell):
###############################################################################
    """
    A weak direct damage spell against cities, this spell causes
    an infection to spread within a city. This spell should be very
    useful for getting players to the higher level spells.

    Enhanced by extreme temperatures, recent famine, and size of city.
    Decreased by AI tech level.

    This is a tier 1 spell
    """

    #
    # ==== Class Variables ====
    #

    _NAME      = "infect"
    _BASE_COST = 50
    _PREREQS   = _SpellPrereq(1, ()) # No prereqs

    #
    # Tweakable constants
    #

    @classmethod
    def _BASE_KILL_FUNC(cls, level):
        # level^1.3
        return poly_growth(level, 1.3)

    @classmethod
    def _CITY_SIZE_BONUS_FUNC(cls, city_size):
        # 1.05^city_size, no theshold or diminishing returns
        return exp_growth(1.05, city_size)

    @classmethod
    def _FAMINE_BONUS_FUNC(cls, city):
        # Double-damage if city in famine
        return 2.0 if city.famine() else 1.0

    @classmethod
    def _EXTREME_TEMP_BONUS_FUNC(cls, temp):
        if (temp > 90): # hot
            degrees_extreme = temp - 90
        elif (temp < 30):
            degrees_extreme = 30 - temp
        else:
            degrees_extreme = 0
        # 1.03^deg_extreme
        return exp_growth(1.03, degrees_extreme, diminishing_returns=20)

    @classmethod
    def _TECH_PENALTY_FUNC(cls, tech_level):
        return tech_level # linear decrease

    #
    # Public API
    #

    ###########################################################################
    def __init__(self, level, location):
    ###########################################################################
        super(self.__class__, self).__init__(level, location)

    ###########################################################################
    def verify_apply(self):
    ###########################################################################
        # This spell can only be cast on cities
        tile = engine().world().tile(self.location())
        urequire(tile.city() is not None,
                 "Must cast ", self.name(), " on a city.")

    ###########################################################################
    def apply(self):
    ###########################################################################
        world = engine().world()
        tile  = world.tile(self.location())
        atmos = tile.atmosphere()
        tech  = engine().ai_player().tech_level()
        exp   = 0

        city = tile.city()
        prequire(city is not None, "Verification did not catch None city")

        # Calculate num killed
        base_kill_pct      = self._BASE_KILL_FUNC(self.level())
        city_size_bonus    = self._CITY_SIZE_BONUS_FUNC(city.rank())
        extreme_temp_bonus = self._EXTREME_TEMP_BONUS_FUNC(atmos.temperature())
        famine_bonus       = self._FAMINE_BONUS_FUNC(city)
        tech_penalty       = self._TECH_PENALTY_FUNC(tech)

        pct_killed = ((base_kill_pct *
                       city_size_bonus *
                       extreme_temp_bonus *
                       famine_bonus) /
                      tech_penalty)

        self._report("base kill % is ", base_kill_pct)
        self._report("city-size bonus (multiplier) is ", city_size_bonus)
        self._report("extreme-temp bonus (multiplier) is ", extreme_temp_bonus)
        self._report("famine bonus (multiplier) is ", famine_bonus)
        self._report("tech penalty (divisor) is ", tech_penalty)
        self._report("final kill % is ", pct_killed)

        exp += self._kill(city, pct_killed)

        return exp

###############################################################################
class _Wind(Spell):
###############################################################################
    """
    Increases the immediate wind speed of a region. High wind speeds can
    kill, but this spell is generally more useful in combinations rather
    than a direct damage spell. High wind speeds can damage
    infrastructure.

    Killing capability enhanced by low temps. Decreased by AI tech level.
    Decreased by city defense.

    This is a tier 1 spell
    """

    #
    # ==== Class Constants ====
    #

    _NAME      = "wind"
    _BASE_COST = 50
    _PREREQS   = _SpellPrereq(1, ()) # No prereqs

    #
    # Tweakable constants
    #

    @classmethod
    def _WIND_INCREASE_FUNC(cls, curr_wind, level):
        # curr_wind may be used in the future to implement diminishing returns
        return 20 * level  # very simple for now, 20 mph per level

    @classmethod
    def _BASE_KILL_FUNC(cls, wind):
        # 1.03^(wind_speed-80)
        return exp_growth(1.03, wind.speed(), threshold=80)

    @classmethod
    def _BASE_INFRA_DESTROY_FUNC(cls, wind):
        # 1.03^(wind_speed-60)
        return exp_growth(1.03, wind.speed(), threshold=60)

    @classmethod
    def _COLD_BONUS_FUNC(cls, temp, orig_wind, new_wind):
        # 1.02^(new_wind) - 1.02^(orig_wind)
        if (temp < 0):
            return (
                exp_growth(1.02, new_wind.speed(),  diminishing_returns=40) -
                exp_growth(1.02, orig_wind.speed(), diminishing_returns=40)
                )
        else:
            return 1 # No effect

    @classmethod
    def _TECH_PENALTY_FUNC(cls, tech_level):
        return poly_growth(tech_level, 0.5) # sqrt

    @classmethod
    def _DEFENSE_PENALTY_FUNC(cls, defense_level):
        return poly_growth(defense_level, 0.5) # sqrt

    #
    # Public API
    #

    ###########################################################################
    def __init__(self, level, location):
    ###########################################################################
        super(self.__class__, self).__init__(level, location)

    ###########################################################################
    def verify_apply(self):
    ###########################################################################
        # No-op. This spell can be cast anywhere, anytime.
        pass

    ###########################################################################
    def apply(self):
    ###########################################################################
        world = engine().world()
        tile  = world.tile(self.location())
        atmos = tile.atmosphere()
        tech  = engine().ai_player().tech_level()
        exp   = 0

        # Regardless of tile type, atmosphere is warmed
        temp         = atmos.temperature()
        orig_wind    = atmos.wind()
        new_wind     = orig_wind + \
            self._WIND_INCREASE_FUNC(orig_wind, self.level())
        atmos.set_wind(new_wind)
        self._report("increased wind from ", orig_wind, " to ", new_wind)

        # Wind can destroy infrastructure
        if (tile.infra_level() is not None and tile.infra_level() > 0):
            base_infra_destroy = self._BASE_INFRA_DESTROY_FUNC(new_wind)
            if (base_infra_destroy > 0.0):
                tech_penalty = self._TECH_PENALTY_FUNC(tech)

                max_infra_destroyed = round(base_infra_destroy / tech_penalty)

                self._report("base infra damage % is ", base_infra_destroy)
                self._report("tech penalty (divisor) is ", tech_penalty)

                exp += self._destroy_infra(tile, max_infra_destroyed);

        # This spell can kill if cast on a city and winds get high enough
        city = tile.city()
        if (city is not None):
            base_kill_pct = self._BASE_KILL_FUNC(new_wind)
            if (base_kill_pct > 0.0):
                cold_bonus      = self._COLD_BONUS_FUNC(temp,
                                                        orig_wind,
                                                        new_wind)
                tech_penalty    = self._TECH_PENALTY_FUNC(tech)
                defense_penalty = self._DEFENSE_PENALTY_FUNC(city.defense())

                pct_killed = (((base_kill_pct * cold_bonus)
                               / tech_penalty) /
                              defense_penalty)

                self._report("base kill % is ", base_kill_pct)
                self._report("cold bonus (multiplier) is ", cold_bonus)
                self._report("tech penalty (divisor) is ", tech_penalty)
                self._report("defense penalty (divisor) is ", defense_penalty)
                self._report("final kill % is ", pct_killed)

                exp += self._kill(city, pct_killed)

        return exp
# Wind can change wind
grant_access(_Wind, Atmosphere.ALLOW_SET_WIND)

###############################################################################
class _Fire(Spell):
###############################################################################
    """
    Starts a fire at a location. Fires will kill people in cities and
    destroy infrastructure.
    TODO: has a chance to spread?

    Enhanced by high wind, low dewpoint, high temperature, and low soil
    moisture. Reduced by city defense and tech level.

    This is a tier 2 spell
    """

    #
    # ==== Class Variables ====
    #

    _NAME      = "fire"
    _BASE_COST = 100
    _PREREQS   = _SpellPrereq(5, ((_Hot.name(), 1),) ) # Requires hot

    #
    # Tweakable Constants
    #

    @classmethod
    def _BASE_DESTRUCTIVENESS_FUNC(cls, level):
        # level^1.3
        return poly_growth(level, 1.3)

    @classmethod
    def _WIND_EFFECT_FUNC(cls, wind):
        # 1.05^(speed - tipping_pt(20)), diminishes @ 30 beyond thresh
        return exp_growth(1.05, wind.speed(), threshold=20,
                          diminishing_returns=30)

    @classmethod
    def _TEMP_EFFECT_FUNC(cls, temp):
        # 1.03^(temp - tipping_pt(75))
        return exp_growth(1.03, temp, threshold=75)

    @classmethod
    def _MOISTURE_EFFECT_FUNC(cls, raw_moisture):
        # 1.05^(tipping_pt(75) - raw_moisture*100), diminishes at 30% below dry
        return exp_growth(1.05, 75 - raw_moisture*100, diminishing_returns=30)

    @classmethod
    def _BASE_INFRA_DESTROY_FUNC(cls, destructiveness):
        # 1.05^destructiveness
        return exp_growth(1.05, destructiveness)

    @classmethod
    def _BASE_KILL_FUNC(cls, destructiveness):
        return destructiveness # Linear

    @classmethod
    def _DEFENSE_PENALTY_FUNC(cls, defense):
        # sqrt(defense)
        return poly_growth(defense, 0.5)

    @classmethod
    def _TECH_PENALTY_FUNC(cls, tech):
        # sqrt(tech)
        return poly_growth(tech, 0.5)

    #
    # Public API
    #

    ###########################################################################
    def __init__(self, level, location):
    ###########################################################################
        super(self.__class__, self).__init__(level, location)

    ###########################################################################
    def verify_apply(self):
    ###########################################################################
        # This spell can only be cast on tiles with plant growth
        tile = engine().world().tile(self.location())
        urequire(isinstance(tile, FoodTile),
                 "Fire can only be cast on tiles with plant growth")

    ###########################################################################
    def apply(self):
    ###########################################################################
        world = engine().world()
        tile  = world.tile(self.location())
        atmos = tile.atmosphere()
        tech  = engine().ai_player().tech_level()
        exp   = 0

        # Query properties relevant to fire destructiveness
        wind          = atmos.wind()
        temp          = atmos.temperature()
        dewpoint      = atmos.dewpoint()
        soil_moisture = tile.soil_moisture()

        # Compute destructiveness of fire
        base_destructiveness = self._BASE_DESTRUCTIVENESS_FUNC(self.level())
        temp_multiplier      = self._TEMP_EFFECT_FUNC(temp)
        wind_multiplier      = self._WIND_EFFECT_FUNC(wind)
        moisture_multiplier  = self._MOISTURE_EFFECT_FUNC(soil_moisture)
        destructiveness      = (base_destructiveness *
                                temp_multiplier *
                                wind_multiplier *
                                moisture_multiplier)

        self._report("base destructiveness ",   base_destructiveness)
        self._report("temperature multiplier ", temp_multiplier)
        self._report("wind multiplier ",        wind_multiplier)
        self._report("moisture multiplier ",    moisture_multiplier)
        self._report("total destructiveness ",  destructiveness)

        # Fire can destroy infrastructure
        if (tile.infra_level() is not None and tile.infra_level() > 0):
            base_infra_destroy = self._BASE_INFRA_DESTROY_FUNC(destructiveness)
            tech_penalty       = self._TECH_PENALTY_FUNC(tech)

            max_infra_destroyed = round(base_infra_destroy / tech_penalty)

            self._report("base infra damage % is ", base_infra_destroy)
            self._report("tech penalty (divisor) is ", tech_penalty)

            exp += self._destroy_infra(tile, max_infra_destroyed)

            self._damage_tile(tile, destructiveness)

        # This spell will kill if cast on a city
        city = tile.city()
        if (city is not None):
            base_kill_pct   = self._BASE_KILL_FUNC(destructiveness)
            tech_penalty    = self._TECH_PENALTY_FUNC(tech)
            defense_penalty = self._DEFENSE_PENALTY_FUNC(city.defense())

            pct_killed = (base_kill_pct / tech_penalty) / defense_penalty

            self._report("base kill % is ", base_kill_pct)
            self._report("tech penalty (divisor) is ", tech_penalty)
            self._report("defense penalty (divisor) is ", defense_penalty)
            self._report("final kill % is ", pct_killed)

            exp += self._kill(city, pct_killed)

        return exp

###############################################################################
class _Tstorm(Spell):
###############################################################################
    """
    Spawn severe thunderstorms. These storms have a chance to cause
    weak floods, tornadoes, and high winds, making this a good spell for
    causing chain-reactions. Lightning can kill city dwellers and is
    the only way for a tstorm to directly get kills, but it's not a very
    effective killer. Most of the damage will be done by chain-reaction
    events.

    Enhanced by high wind, high dewpoint, high temperature, low pressure, and
    high temperature differentials.

    This is a tier 2 spell
    """

    #
    # ==== Class Variables ====
    #

    _NAME      = "tstorm"
    _BASE_COST = 100
    _PREREQS   = _SpellPrereq(5, ((_Wind.name(), 1),) ) # Requires wind

    #
    # Tweakable Constants
    #

    @classmethod
    def _BASE_DESTRUCTIVENESS_FUNC(cls, level):
        # level^1.3
        return poly_growth(level, 1.3)

    @classmethod
    def _WIND_EFFECT_FUNC(cls, wind):
        # 1.03^(speed - tipping_pt(15)), diminishes @ 15 beyond thresh
        return exp_growth(1.03, wind.speed(), threshold=15,
                          diminishing_returns=15)

    @classmethod
    def _TEMP_EFFECT_FUNC(cls, temp):
        # 1.03^(temp - tipping_pt(85))
        return exp_growth(1.03, temp, threshold=85, diminishing_returns=15)

    @classmethod
    def _PRESSURE_EFFECT_FUNC(cls, pressure):
        # 1.05^(pressure - tipping_pt(990))
        return exp_growth(1.05, pressure, threshold=990)

    @classmethod
    def _WIND_SPAWN_LEVEL_FUNC(cls, destructiveness):
        return fibonacci_div(destructiveness, 10)

    @classmethod
    def _FLOOD_SPAWN_LEVEL_FUNC(cls, destructiveness):
        return fibonacci_div(destructiveness, 15)

    @classmethod
    def _TORNADO_SPAWN_LEVEL_FUNC(cls, destructiveness):
        return fibonacci_div(destructiveness, 20)

    @classmethod
    def _BASE_KILL_FUNC(cls, destructiveness):
        # 2% of destructiveness
        return 0.02 * destructiveness # Linear

    @classmethod
    def _DEFENSE_PENALTY_FUNC(cls, defense):
        # sqrt(defense)
        return poly_growth(defense, 0.5)

    @classmethod
    def _TECH_PENALTY_FUNC(cls, tech):
        # sqrt(tech)
        return poly_growth(tech, 0.5)

    #
    # Public API
    #

    ###########################################################################
    def __init__(self, level, location):
    ###########################################################################
        super(self.__class__, self).__init__(level, location)

    ###########################################################################
    def verify_apply(self):
    ###########################################################################
        # This spell can only be cast on plains or lush tiles
        tile = engine().world().tile(self.location())
        urequire(isinstance(tile, PlainsTile) or isinstance(tile, LushTile),
                 "Tstorms can only be cast on plains or lush tiles")

    ###########################################################################
    def apply(self):
    ###########################################################################
        world = engine().world()
        tile  = world.tile(self.location())
        atmos = tile.atmosphere()
        tech  = engine().ai_player().tech_level()
        exp   = 0

        # Query properties relevant to fire destructiveness
        wind          = atmos.wind().speed()
        temp          = atmos.temperature()
        dewpoint      = atmos.dewpoint()
        pressure      = atmos.pressure()

        # Compute destructiveness of fire
        # TODO - Include instability/temperature differential/dewpoint
        base_destructiveness = self._BASE_DESTRUCTIVENESS_FUNC(self.level())
        temp_multiplier      = self._TEMP_EFFECT_FUNC(temp)
        wind_multiplier      = self._WIND_EFFECT_FUNC(wind)
        pressure_multiplier  = self._PRESSURE_EFFECT_FUNC(pressure)
        destructiveness      = (base_destructiveness *
                                temp_multiplier *
                                wind_multiplier *
                                pressure_multiplier)

        self._report("base destructiveness ",   base_destructiveness)
        self._report("temperature multiplier ", temp_multiplier)
        self._report("wind multiplier ",        wind_multiplier)
        self._report("presure multiplier ",     pressure_multiplier)
        self._report("total destructiveness ",  destructiveness)

        # Spawn chain-reaction events

        wind_spawn_lvl    = self._WIND_SPAWN_LEVEL_FUNC(destructiveness)
        flood_spawn_lvl   = self._FLOOD_SPAWN_LEVEL_FUNC(destructiveness)
        tornado_spawn_lvl = self._TORNADO_SPAWN_LEVEL_FUNC(destructiveness)

        if (wind_spawn_lvl > 0):
            exp += self._spawn(_Wind.name(), wind_spawn_lvl)

        if (flood_spawn_lvl > 0):
            exp += self._spawn(_Flood.name(), flood_spawn_lvl)

        if (tornado_spawn_lvl > 0):
            exp += self._spawn(_Tornado.name(), tornado_spawn_lvl)

        # This spell will kill if cast on a city
        city = tile.city()
        if (city is not None):
            base_kill_pct   = self._BASE_KILL_FUNC(destructiveness)
            tech_penalty    = self._TECH_PENALTY_FUNC(tech)
            defense_penalty = self._DEFENSE_PENALTY_FUNC(city.defense())

            pct_killed = (base_kill_pct / tech_penalty) / defense_penalty

            self._report("base kill % is ", base_kill_pct)
            self._report("tech penalty (divisor) is ", tech_penalty)
            self._report("defense penalty (divisor) is ", defense_penalty)
            self._report("final kill % is ", pct_killed)

            exp += self._kill(city, pct_killed)

        return exp

###############################################################################
class _Snow(Spell):
###############################################################################
    """
    Spawn a large snow storm. Temporarily drastically reduces yields on tiles.

    Enhanced by high dewpoint, low temperature, low pressure.

    This is a tier 2 spell
    """

    #
    # Class Variables
    #

    _NAME      = "snow"
    _BASE_COST = 100
    _PREREQS   = _SpellPrereq(5, ((_Cold.name(), 1),) ) # Requires cold

    #
    # Public API
    #

    ###########################################################################
    def __init__(self, level, location):
    ###########################################################################
        super(self.__class__, self).__init__(level, location)

    ###########################################################################
    def verify_apply(self):
    ###########################################################################
        # TODO
        pass

    ###########################################################################
    def apply(self):
    ###########################################################################
        # TODO
        # Increase precip
        # Increase snowpack
        return 0

###############################################################################
class _Avalanche(Spell):
###############################################################################
    """
    Cause an avalanche. This can devastate mountain infrasture and mountain
    cities.

    Enhanced by high snowpack, ongoing snowstorm/blizzard.

    This is a tier 3 spell
    """

    #
    # Class Variables
    #

    _NAME      = "avalanche"
    _BASE_COST = 200
    _PREREQS   = _SpellPrereq(10, ((_Snow.name(), 1),) ) # Requires snow

    #
    # Public API
    #

    ###########################################################################
    def __init__(self, level, location):
    ###########################################################################
        super(self.__class__, self).__init__(level, location)

    ###########################################################################
    def verify_apply(self):
    ###########################################################################
        # TODO
        pass

    ###########################################################################
    def apply(self):
    ###########################################################################
        # TODO
        return 0

###############################################################################
class _Flood(Spell):
###############################################################################
    """
    Cause a flooding rainstorm. Can kill people in cities and destroy
    infrastructure.

    Enhanced by high soil moisture, high dewpoints, low pressure.

    This is a tier 3 spell
    """

    #
    # Class Variables
    #

    _NAME      = "flood"
    _BASE_COST = 200
    _PREREQS   = _SpellPrereq(10, ((_Tstorm.name(), 1),) ) # Requires tstorm

    #
    # Public API
    #

    ###########################################################################
    def __init__(self, level, location):
    ###########################################################################
        super(self.__class__, self).__init__(level, location)

    ###########################################################################
    def verify_apply(self):
    ###########################################################################
        # TODO
        pass

    ###########################################################################
    def apply(self):
    ###########################################################################
        # TODO
        # Change atmosphere to increase precip
        return 0

###############################################################################
class _Dry(Spell):
###############################################################################
    """
    Causes abnormally dry weather. Hurts food production by reducing soil
    moisture.

    Enhanced by high temperatures, low dewpoints, high pressure.

    This is a tier 3 spell
    """

    #
    # Class Variables
    #

    _NAME      = "dry"
    _BASE_COST = 200
    _PREREQS   = _SpellPrereq(10, ((_Fire.name(), 1),) ) # Requires fire

    #
    # Public API
    #

    ###########################################################################
    def __init__(self, level, location):
    ###########################################################################
        super(self.__class__, self).__init__(level, location)

    ###########################################################################
    def verify_apply(self):
    ###########################################################################
        # TODO
        pass

    ###########################################################################
    def apply(self):
    ###########################################################################
        # TODO
        return 0

###############################################################################
class _Blizzard(Spell):
###############################################################################
    """
    Causes a massive snow/wind storm. Drastically reduces temperatures. Will
    kill people in cities and drastically reduce tile yields temporarily.

    Enhanced by low temperatures, high dewpoints, low pressure, high winds.

    This is a tier 3 spell
    """

    #
    # Class Variables
    #

    _NAME      = "blizzard"
    _BASE_COST = 200
    _PREREQS   = _SpellPrereq(10, ((_Snow.name(), 1),) ) # Requires snow

    #
    # Public API
    #

    ###########################################################################
    def __init__(self, level, location):
    ###########################################################################
        super(self.__class__, self).__init__(level, location)

    ###########################################################################
    def verify_apply(self):
    ###########################################################################
        # TODO
        pass

    ###########################################################################
    def apply(self):
    ###########################################################################
        # TODO
        return 0

###############################################################################
class _Tornado(Spell):
###############################################################################
    """
    Causes a large severe thunderstorm outbreak with tornadoes. Severe storms
    impact a large area. Each tornado has a chance of scoring a "direct hit".
    Infrastructure/cities that get a direct hit will take serious damage.

    Enhanced by same things that enhance thunderstorms.

    This is a tier 3 spell
    """

    #
    # Class Variables
    #

    _NAME      = "tornado"
    _BASE_COST = 200
    _PREREQS   = _SpellPrereq(10, ((_Tstorm.name(), 1),) ) # Requires tstorm

    #
    # Public API
    #

    ###########################################################################
    def __init__(self, level, location):
    ###########################################################################
        super(self.__class__, self).__init__(level, location)

    ###########################################################################
    def verify_apply(self):
    ###########################################################################
        # TODO
        pass

    ###########################################################################
    def apply(self):
    ###########################################################################
        # TODO
        return 0

###############################################################################
class _Heatwave(Spell):
###############################################################################
    """
    Will cause the next season to be abnormally warm for the surrounding region.
    Note that this spell affects long-term weather.

    Enhanced by low soil moisture.

    This is a tier 4 spell
    """

    #
    # Class Variables
    #

    _NAME      = "heatwave"
    _BASE_COST = 400
    _PREREQS   = _SpellPrereq(15, ((_Dry.name(), 1),) ) # Requires dry

    #
    # Public API
    #

    ###########################################################################
    def __init__(self, level, location):
    ###########################################################################
        super(self.__class__, self).__init__(level, location)

    ###########################################################################
    def verify_apply(self):
    ###########################################################################
        # TODO
        pass

    ###########################################################################
    def apply(self):
    ###########################################################################
        # TODO
        return 0

###############################################################################
class _Coldwave(Spell):
###############################################################################
    """
    Will cause the next season to be abnormally cold for the surrounding region.
    Note that this spell affects long-term weather.

    Enhanced by high snowpack?

    This is a tier 4 spell
    """

    #
    # Class Variables
    #

    _NAME      = "coldwave"
    _BASE_COST = 400
    _PREREQS   = _SpellPrereq(15, ((_Blizzard.name(), 1),) ) # Requires blizzard

    #
    # Public API
    #

    ###########################################################################
    def __init__(self, level, location):
    ###########################################################################
        super(self.__class__, self).__init__(level, location)

    ###########################################################################
    def verify_apply(self):
    ###########################################################################
        # TODO
        pass

    ###########################################################################
    def apply(self):
    ###########################################################################
        # TODO
        return 0

###############################################################################
class _Drought(Spell):
###############################################################################
    """
    Will cause the next season to be abnormally dry for the surrounding region.
    Note that this spell affects long-term weather.

    Enhanced by low soil moisture.

    This is a tier 4 spell
    """

    #
    # Class Variables
    #

    _NAME      = "drought"
    _BASE_COST = 400
    _PREREQS   = _SpellPrereq(15, ((_Dry.name(), 1),) ) # Requires dry

    #
    # Public API
    #

    ###########################################################################
    def __init__(self, level, location):
    ###########################################################################
        super(self.__class__, self).__init__(level, location)

    ###########################################################################
    def verify_apply(self):
    ###########################################################################
        # TODO
        pass

    ###########################################################################
    def apply(self):
    ###########################################################################
        # TODO
        return 0

###############################################################################
class _Monsoon(Spell):
###############################################################################
    """
    Will cause the next season to be abnormally moist for the surrounding region.
    Note that this spell affects long-term weather.

    Enhanced by high soil moisture.

    This is a tier 4 spell
    """

    #
    # Class Variables
    #

    _NAME      = "monsoon"
    _BASE_COST = 400
    _PREREQS   = _SpellPrereq(15, ((_Flood.name(), 1),) ) # Requires flood

    #
    # Public API
    #

    ###########################################################################
    def __init__(self, level, location):
    ###########################################################################
        super(self.__class__, self).__init__(level, location)

    ###########################################################################
    def verify_apply(self):
    ###########################################################################
        # TODO
        pass

    ###########################################################################
    def apply(self):
    ###########################################################################
        # TODO
        return 0

###############################################################################
class _Disease(Spell):
###############################################################################
    """
    Will cause a disease to breakout in the targetted city.

    Enhanced by extreme temperatures, famine, and large cities.

    This is a tier 5 spell
    """

    #
    # Class Variables
    #

    _NAME      = "disease"
    _BASE_COST = 800
    _PREREQS   = _SpellPrereq(20, ((_Infect.name(), 1),) ) # Requires infect

    #
    # Public API
    #

    ###########################################################################
    def __init__(self, level, location):
    ###########################################################################
        super(self.__class__, self).__init__(level, location)

    ###########################################################################
    def verify_apply(self):
    ###########################################################################
        # TODO
        pass

    ###########################################################################
    def apply(self):
    ###########################################################################
        # TODO
        return 0

###############################################################################
class _Earthquake(Spell):
###############################################################################
    """
    Will cause an earthquake to occur. Devastates nearby cities and
    infrastructure.

    Enhanced by plate tension.

    This is a tier 5 spell
    """

    #
    # Class Variables
    #

    _NAME      = "quake"
    _BASE_COST = 800
    _PREREQS   = _SpellPrereq(20, ()) # Requires player lvl 20, no spell prereqs

    #
    # Public API
    #

    ###########################################################################
    def __init__(self, level, location):
    ###########################################################################
        super(self.__class__, self).__init__(level, location)

    ###########################################################################
    def verify_apply(self):
    ###########################################################################
        # No-op. This spell can be cast anywhere, anytime.
        pass

    ###########################################################################
    def apply(self):
    ###########################################################################
        # TODO
        return 0

###############################################################################
class _Hurricane(Spell):
###############################################################################
    """
    Will spawn a hurricane. Causes floods, high winds, tornadoes, and tstorms
    over a large region.

    Enhanced by warm sea temperatures, high dewpoints, high pressure and the
    farther away from land they start, the larger they get, but the higher
    chance they have to miss land.

    This is a tier 5 spell
    """

    #
    # Class Variables
    #

    _NAME      = "hurricane"
    _BASE_COST = 800
    _PREREQS   = _SpellPrereq(20, ((_Monsoon.name(), 1),) ) # Requires monsoon

    #
    # Public API
    #

    ###########################################################################
    def __init__(self, level, location):
    ###########################################################################
        super(self.__class__, self).__init__(level, location)

    ###########################################################################
    def verify_apply(self):
    ###########################################################################
        # No-op. This spell can be cast anywhere, anytime.
        pass

    ###########################################################################
    def apply(self):
    ###########################################################################
        # TODO
        return 0

###############################################################################
class _Plague(Spell):
###############################################################################
    """
    Will cause a large outbreak affecting all nearby cities.

    Same enhancements as disease.

    This is a tier 6 spell
    """

    #
    # Class Variables
    #

    _NAME      = "plague"
    _BASE_COST = 1600
    _PREREQS   = _SpellPrereq(25, ((_Disease.name(), 1),) ) # Requires disease

    #
    # Public API
    #

    ###########################################################################
    def __init__(self, level, location):
    ###########################################################################
        super(self.__class__, self).__init__(level, location)

    ###########################################################################
    def verify_apply(self):
    ###########################################################################
        # No-op. This spell can be cast anywhere, anytime.
        pass

    ###########################################################################
    def apply(self):
    ###########################################################################
        # TODO
        return 0

###############################################################################
class _Volcano(Spell):
###############################################################################
    """
    Will cause a large volcanic eruption; everything nearby will be
    eradicated.

    Enhanced by magma build-up.

    This is a tier 6 spell
    """

    #
    # Class Variables
    #

    _NAME      = "volcano"
    _BASE_COST = 1600
    _PREREQS   = _SpellPrereq(25, ((_Earthquake.name(), 1),) ) # Requires quake

    #
    # Public API
    #

    ###########################################################################
    def __init__(self, level, location):
    ###########################################################################
        super(self.__class__, self).__init__(level, location)

    ###########################################################################
    def verify_apply(self):
    ###########################################################################
        # No-op. This spell can be cast anywhere, anytime.
        pass

    ###########################################################################
    def apply(self):
    ###########################################################################
        # TODO
        return 0

###############################################################################
class _Asteroid(Spell):
###############################################################################
    """
    Hurls an asteriod at the planet. There is a chance the asteroid
    will miss or hit somewhere other than the targetted area. Will
    eradicate everything over a large area.

    This is a tier 7 spell
    """

    #
    # Class Variables
    #

    _NAME      = "asteroid"
    _BASE_COST = 3200
    _PREREQS   = _SpellPrereq(30, ((_Volcano.name(), 1),) ) # Requires volcano

    #
    # Public API
    #

    ###########################################################################
    def __init__(self, level, location):
    ###########################################################################
        super(self.__class__, self).__init__(level, location)

    ###########################################################################
    def verify_apply(self):
    ###########################################################################
        # No-op. This spell can be cast anywhere, anytime.
        pass

    ###########################################################################
    def apply(self):
    ###########################################################################
        # TODO
        return 0

#
# Tests
#

###############################################################################
class TestSpell(unittest.TestCase):
###############################################################################

    ###########################################################################
    def test_spell(self):
    ###########################################################################
        pass

if (__name__ == "__main__"):
    unittest.main()
