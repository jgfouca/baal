#! /usr/bin/env python

"""
We put all WorldTile classes in this header to avoid
generating a ton of header files.
"""

import unittest

from baal_common import prequire, ProgramError, \
    check_access, check_callers, grant_access
from weather import Atmosphere, is_atmospheric, Climate
from geology import is_geological, Geology

###############################################################################
class Yield(object):
###############################################################################
    """
    A simple structure that specifies yields for tiles. You can think of this
    class as a named pair.
    """

    #
    # ==== Public API ====
    #

    def __init__(self, food, prod):
        prequire(food == 0 or prod == 0, "Tile cannot yield food and prod")
        prequire(food >= 0, "Cannot have negative yields")
        prequire(prod >= 0, "Cannot have negative yields")
        self.food = food
        self.prod = prod

    def __mul__(self, multiplier):
        return Yield(self.food * multiplier, self.prod * multiplier)

    def to_xml(self):
        # TODO - Aaron
        pass

# TODO - Support named properties instead of having individual getters/setters
# for each property?

###############################################################################
class WorldTile(object):
###############################################################################
    """
    WorldTile is the abstract base class of all tiles.

    Every tile has an atmosphere, climate, geology, and yield.

    This class also serves as an aggregate of all the *query*
    interfaces of the Tile subclasses. Anything you can possibly ask of
    any WorldTile must be associated with some method in the WorldTile
    class. None will be returned for non-sensical queries such as asking
    a mountain tile what its ocean-depth is.

    TODO: There is probably a better way to design this... consider not
    relying so heavily on inheritence.
    """

    #
    # ==== Public API ====
    #

    def __init__(self, yield_, climate, geology, location):
        self.__init_impl(yield_, climate, geology, location)

    #
    # Query/getter API
    #

    def location(self): return self.__location

    def yield_(self): return self.__yield_impl()

    def worked(self): return self.__worked

    def already_casted(self, spell): return spell.name() in self.__casted_spells

    def atmosphere(self): return self.__atmosphere

    def climate(self): return self.__climate

    def geology(self): return self.__geology

    def to_xml(self): return self.__to_xml_impl()

    def infra_level(self): return None

    def can_build_infra(self): return False

    def city(self): return None

    def supports_city(self): return False

    def soil_moisture(self): return None

    def depth(self): return None

    def sea_surface_temp(self): return None

    def elevation(self): return None

    def snowpack(self): return None

    #
    # Modification API
    #

    def work(self):
        """
        Mark this tile as being worked by a civilian
        """
        return self.__work_impl()

    def cycle_turn(self, anomalies, season):
        """
        Notify this tile that the turn is cycling, providing current
        anomalies and season so that we know how to adjust our atmosphere.
        """
        return self.__cycle_turn_impl(anomalies, season)

    def register_casted_spell(self, spell):
        """
        Notify this tile that a spell has been cast on it. We forbid casting
        the same spell multiple times on the same tile.
        """
        return self.__register_casted_spell_impl(spell)

    #
    # ==== Class constants ====
    #

    ALLOW_WORK           = "_world_tile_allow_work"
    ALLOW_CYCLE_TURN     = "_world_tile_allow_cycle_turn"
    ALLOW_REGISTER_SPELL = "_world_tile_allow_register_spell"

    #
    # ==== Implementation ====
    #

    ###########################################################################
    def __init_impl(self, yield_, climate, geology, location):
    ###########################################################################
        self.__base_yield    = yield_
        self.__climate       = climate
        self.__geology       = geology
        self.__atmosphere    = Atmosphere(climate)
        self.__location      = location
        self.__worked        = False
        self.__casted_spells = []

    ###########################################################################
    def __yield_impl(self):
    ###########################################################################
        from engine import engine

        # AI tech-level affects yield
        return engine().ai_player().get_adjusted_yield(self.__base_yield)

    ###########################################################################
    def __to_xml_impl(self):
    ###########################################################################
        # TODO - Aaron
        pass

    ###########################################################################
    def __work_impl(self):
    ###########################################################################
        check_access(self.ALLOW_WORK)

        prequire(not self.worked(), "Tile already being worked")
        self.__worked = True

    ###########################################################################
    def __cycle_turn_impl(self, anomalies, season):
    ###########################################################################
        check_access(self.ALLOW_CYCLE_TURN)

        self.__geology.cycle_turn()
        self.__atmosphere.cycle_turn(anomalies, self.location(), season)
        self.__worked = False

        self.__casted_spells = []

    ###########################################################################
    def __register_casted_spell_impl(self, spell):
    ###########################################################################
        check_access(self.ALLOW_REGISTER_SPELL)

        prequire(spell.name() not in self.__casted_spells)

        self.__casted_spells.append(spell.name())

grant_access(WorldTile, Atmosphere.ALLOW_CYCLE_TURN)
grant_access(WorldTile, Geology.ALLOW_CYCLE_TURN)

###############################################################################
class OceanTile(WorldTile):
###############################################################################
    """
    Represents an ocean tile. Ocean tiles have depth and surface temperature
    properties in addition to the normal set.
    """

    #
    # ==== Public API ====
    #

    def __init__(self, depth, climate, geology, location):
        self.__init_impl(depth, climate, geology, location)

    #
    # Queries/Getters
    #

    def depth(self): return self.__depth

    def sea_surface_temp(self): return self.__surface_temp

    #
    # Modification API
    #

    def cycle_turn(self, anomalies, season):
        return self.__cycle_turn_impl(anomalies, season)

    def set_surface_temp(self, new_temp):
        return self.__set_surface_temp_impl(new_temp)

    #
    # ==== Class Constants ====
    #

    ALLOW_SET_SURFACE_TEMP = "_ocean_tile_allow_set_surface_temp"
    _FOOD_YIELD = 3

    @classmethod
    def _NEW_SURFACE_TEMP_FUNC(cls, orig_surf_temp, new_air_temp):
        # Just average the two for now
        return (orig_surf_temp + new_air_temp) / 2

    #
    # ==== Implementation ====
    #

    ###########################################################################
    def __init_impl(self, depth, climate, geology, location):
    ###########################################################################
         super(OceanTile, self).__init__(Yield(self._FOOD_YIELD, 0),
                                         climate,
                                         geology,
                                         location)
         self.__depth = depth
         # TODO - Needs to be an average of all 4 seasons
         self.__surface_temp = self.atmosphere().temperature()

    ###########################################################################
    def __cycle_turn_impl(self, anomalies, season):
    ###########################################################################
        super(OceanTile, self).cycle_turn(anomalies, season)

        # Sea temperatures retain some heat, so new sea temps have to take old
        # sea temps into account. Here, we just average season temp and prior
        # sea temp together as a very simple model.
        self.__surface_temp = \
            self._NEW_SURFACE_TEMP_FUNC(self.__surface_temp,
                                        self.atmosphere().temperature())

    ###########################################################################
    def __set_surface_temp_impl(self, new_temp):
    ###########################################################################
        check_access(self.ALLOW_SET_SURFACE_TEMP)

        self.__surface_temp = new_temp

###############################################################################
class LandTile(WorldTile):
###############################################################################
    """
    Base class for all land tiles. Adds the concepts of tile damage, cities,
    infrastructure, elevation, and snowpack.
    """

    #
    # ==== Public API ====
    #

    def __init__(self, elevation, yield_, climate, geology, location):
        self.__init_impl(elevation, yield_, climate, geology, location)

    #
    # Queries/Getter API
    #

    def yield_(self): return self.__yield_impl()

    def infra_level(self): return self.__infra_level

    def city(self): return self.__city

    def supports_city(self): return True

    def can_build_infra(self):
        return self.infra_level() < self.LAND_TILE_MAX_INFRA

    def snowpack(self): return self.__snowpack

    def elevation(self): return self.__elevation

    #
    # Modification API
    #

    def damage(self, dmg): return self.__damage_impl(dmg)

    def cycle_turn(self, anomalies, season):
        return self.__cycle_turn_impl(anomalies, season)

    def build_infra(self): return self.__build_infra_impl()

    def destroy_infra(self, num_lvls_destroyed):
        return self.__destroy_infra_impl(num_lvls_destroyed)

    def place_city(self, city): return self.__place_city_impl(city)

    def remove_city(self): return self.__remove_city_impl()

    def set_snowpack(self, new_snowpack):
        return self.__set_snowpack_impl(new_snowpack)

    #
    # ==== Class Constants ====
    #

    LAND_TILE_MAX_INFRA = 5

    ALLOW_DAMAGE        = "_land_tile_damage"
    ALLOW_BUILD_INFRA   = "_land_tile_allow_build_infra"
    ALLOW_DESTROY_INFRA = "_land_tile_allow_destroy_infra"
    ALLOW_SET_SNOWPACK  = "_land_tile_allow_set_snowpack"

    #
    # Tweakable Constants
    #

    @classmethod
    def _LAND_TILE_RECOVERY_FUNC(cls, prior):
        # Recovers 10% of full hp per turn
        new_dmg = prior + .10
        return 1.0 if (new_dmg > 1.0) else new_dmg

    @classmethod
    def _COMPUTE_YIELD_FUNC(cls, normal_yield, infra_level, tile_hp):
        return normal_yield * (1 + infra_level) * tile_hp

    @classmethod
    def _PORTION_OF_PRECIP_THAT_FALLS_AS_SNOW_FUNC(cls, temp):
        if (temp < 30):
            return 1.0
        elif (temp < 60):
            return float(60 - temp) / 30
        else:
            return 0.0

    @classmethod
    def _PORTION_OF_SNOWPACK_THAT_MELTED(cls, temp):
        if (temp < 15):
            return 0.0
        elif (temp < 75):
            return float(temp - 15) / 60
        else:
            return 1.0

    #
    # ==== Implementation ====
    #

    ###########################################################################
    def __init_impl(self, elevation, yield_, climate, geology, location):
    ###########################################################################
        super(LandTile, self).__init__(yield_, climate, geology, location)

        self.__hp          = 1.0 # 0..1
        self.__infra_level = 0
        self.__city        = None
        self.__elevation   = elevation
        self.__snowpack    = 0 # TODO - Hack, should have preexiting snowpack

    ###########################################################################
    def __yield_impl(self):
    ###########################################################################
        normal_yield = super(LandTile, self).yield_()
        return self._COMPUTE_YIELD_FUNC(normal_yield,
                                        self.infra_level(),
                                        self.__hp)

    ###########################################################################
    def __damage_impl(self, dmg):
    ###########################################################################
        check_access(self.ALLOW_DAMAGE)

        prequire(dmg >= 0.0 and dmg <= 1.0, "Invalid ", dmg)

        self.__hp *= (1.0 - dmg)

        prequire(self.__hp >= 0.0 and self.__hp <= 1.0,
                 "Invariant failed ", self.__hp)

    ###########################################################################
    def __cycle_turn_impl(self, anomalies, season):
    ###########################################################################
        super(LandTile, self).cycle_turn(anomalies, season)

        # Compute HP recovery
        self.__hp = self._LAND_TILE_RECOVERY_FUNC(self.__hp)

        # Compute change in snowpack
        precip = self.atmosphere().precip()
        temp   = self.atmosphere().temperature()

        # TODO - Take elevation into account?

        snowfall_portion = \
            self._PORTION_OF_PRECIP_THAT_FALLS_AS_SNOW_FUNC(temp)
        snowpack_melt_portion = self._PORTION_OF_SNOWPACK_THAT_MELTED(temp)

        snowfall = (precip * 12) * snowfall_portion # 12 inches snow per inch

        self.__snowpack = (snowfall + self.snowpack()) * (1 - snowpack_melt_portion)

    ###########################################################################
    def __build_infra_impl(self):
    ###########################################################################
        check_access(self.ALLOW_BUILD_INFRA)

        prequire(self.infra_level() < self.LAND_TILE_MAX_INFRA, "max infra")
        prequire(self.city() is None, "Cannot build infra on city")

        self.__infra_level += 1

    ###########################################################################
    def __destroy_infra_impl(self, num_lvls_destroyed):
    ###########################################################################
        check_access(self.ALLOW_DESTROY_INFRA)

        prequire(self.infra_level() >= num_lvls_destroyed, "too high")

        self.__infra_level -= num_lvls_destroyed

    ###########################################################################
    def __place_city_impl(self, city):
    ###########################################################################
        check_callers(["place_city", "__place_city_impl_world"])

        prequire(self.infra_level() == 0, "Tile already has infra on it")
        prequire(self.supports_city(), "Tile type does not support cities")
        prequire(self.city() is None,  "Tile already has city")

        self.__city = city

    ###########################################################################
    def __remove_city_impl(self):
    ###########################################################################
        check_callers(["remove_city", "__remove_city_impl_world"])

        prequire(self.city() is not None, "No city to remove")

        self.__city = None

    ###########################################################################
    def __set_snowpack_impl(self, new_snowpack):
    ###########################################################################
        check_access(self.ALLOW_SET_SNOWPACK)

        self.__snowpack = new_snowpack

###############################################################################
class MountainTile(LandTile):
###############################################################################
    """
    Represents mountain tiles. Mountains don't add any new concepts. Cities
    can't be built on mountains.
    """

    def __init__(self, elevation, climate, geology, location):
        super(MountainTile, self).__init__(elevation,
                                           Yield(0, self._PROD_YIELD),
                                           climate,
                                           geology,
                                           location)

    def supports_city(self): return False

    _PROD_YIELD = 2

###############################################################################
class DesertTile(LandTile):
###############################################################################
    """
    Represents desert tiles. Deserts add no concepts, so this class is simple.
    """

    def __init__(self, elevation, climate, geology, location):
        super(DesertTile, self).__init__(elevation,
                                         Yield(0, self._PROD_YIELD),
                                         climate,
                                         geology,
                                         location)

    _PROD_YIELD = 0.5

###############################################################################
class TundraTile(LandTile):
###############################################################################
    """
    Represents tundra tiles. Tundra add no concepts, so this class is simple.
    """

    def __init__(self, elevation, climate, geology, location):
        super(TundraTile, self).__init__(elevation,
                                         Yield(0, self._PROD_YIELD),
                                         climate,
                                         geology,
                                         location)

    _PROD_YIELD = 0.5

###############################################################################
class HillsTile(LandTile):
###############################################################################
    """
    Represents hill tiles. Hills add no concepts, so this class is simple.

    TODO: Hill-tile lack of moisture make it awkward to handle
    for many disasters. Resolve this issue.
    """

    def __init__(self, elevation, climate, geology, location):
        super(HillsTile, self).__init__(elevation,
                                        Yield(0, self._PROD_YIELD),
                                        climate,
                                        geology,
                                        location)

    _PROD_YIELD = 1

###############################################################################
class FoodTile(LandTile):
###############################################################################
    """
    Represents land tiles that yield food. Introduces the concept of soil
    moisture.
    """

    def __init__(self, elevation, yield_, climate, geology, location):
        super(FoodTile, self).__init__(elevation, yield_, climate, geology, location)
        self.__soil_moisture = 1.0 # % of normal

    def soil_moisture(self): return self.__soil_moisture

    def yield_(self):
        normal_yield = super(FoodTile, self).yield_()
        return normal_yield * \
            self._MOISTURE_YIELD_EFFECT_FUNC(self.soil_moisture())

    def cycle_turn(self, anomalies, season):
        super(FoodTile, self).cycle_turn(anomalies, season)

        # Get the parameters we need to make the calculation
        precip         = self.atmosphere().precip()
        temp           = self.atmosphere().temperature()
        av_precip      = self.climate().precip(season)
        av_temp        = self.climate().temperature(season)
        prior_moisture = self.soil_moisture()

        # Precip's effect on moisture
        precip_effect = self._PRECIP_EFFECT_ON_MOISTURE_FUNC(av_precip, precip)

        # Temp's effect on moisture
        temp_effect = self._TEMP_EFFECT_ON_MOISTURE_FUNC(av_temp, temp)

        # Compute total effect
        total_recent_effect = precip_effect * temp_effect

        # Take past moisture into account but weight current moisture more heavily
        self.__soil_moisture = self._COMPUTE_MOISTURE(self.soil_moisture(),
                                                      total_recent_effect)

        prequire(self.__soil_moisture >= 0.0 and self.__soil_moisture < 100,
                "Moisture ", self.__soil_moisture, " not valid")

    FLOODING_THRESHOLD = 1.5
    TOTALLY_FLOODED    = 2.75

    @classmethod
    def _MOISTURE_YIELD_EFFECT_FUNC(cls, moisture):
        if (moisture < cls.FLOODING_THRESHOLD):
            # Up to the flooding threshold, yields improve as moisture
            # increases
            return moisture
        elif (moisture < cls.TOTALLY_FLOODED):
            # Yields drop quickly as soil becomes over-saturated
            return cls.FLOODING_THRESHOLD - (moisture - cls.FLOODING_THRESHOLD)
        else:
            # Things are flooded and can't get any worse. Farmers are able to
            # salvage some fixed portion of their crops.
            return 0.25

    @classmethod
    def _PRECIP_EFFECT_ON_MOISTURE_FUNC(cls, average_precip, precip):
        return float(precip) / average_precip

    @classmethod
    def _TEMP_EFFECT_ON_MOISTURE_FUNC(cls, average_temp, temp):
        return 1.0 + ( 0.01 * (average_temp - temp) )

    @classmethod
    def _COMPUTE_MOISTURE(cls, prior, current_forcing):
        return ((current_forcing * 2) + prior) / 3

###############################################################################
class PlainsTile(FoodTile):
###############################################################################
    """
    Represents plains tiles. Plains add no concepts, so this class is simple.
    """

    def __init__(self, elevation, climate, geology, location):
        super(PlainsTile, self).__init__(elevation,
                                         Yield(self._FOOD_YIELD, 0),
                                         climate,
                                         geology,
                                         location)

    _FOOD_YIELD = 1

###############################################################################
class LushTile(FoodTile):
###############################################################################
    """
    Represents lush tiles. Lush add no concepts, so this class is simple. Lush
    tiles are like plains tiles except they have higher food yields.
    """

    def __init__(self, elevation, climate, geology, location):
        super(LushTile, self).__init__(elevation,
                                       Yield(self._FOOD_YIELD, 0),
                                       climate,
                                       geology,
                                       location)

    _FOOD_YIELD = 2

#
# Tests
#

###############################################################################
class TestWorldTile(unittest.TestCase):
###############################################################################

    ###########################################################################
    def test_world_tile(self):
    ###########################################################################
        # TODO
        pass

if (__name__ == "__main__"):
    unittest.main()
